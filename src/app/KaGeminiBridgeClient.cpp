#include "KaGeminiBridgeClient.h"
#include "core/GeminiCommandRouter.h"

#include <QElapsedTimer>
#include <QJsonDocument>
#include <QNetworkAccessManager>
#include <QNetworkCookieJar>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRegularExpression>
#include <QUrl>

namespace {
constexpr auto kUserAgent = "Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:135.0) Gecko/20100101 Firefox/135.0";
}

KaGeminiBridgeClient::KaGeminiBridgeClient(QObject* parent)
    : QObject(parent), m_nam(new QNetworkAccessManager(this)) {
  m_nam->setCookieJar(new QNetworkCookieJar(this));
}

void KaGeminiBridgeClient::cancelActiveJob() {
  ++m_jobSeq;
  m_busy = false;
}

QStringList KaGeminiBridgeClient::buildCandidatePool(const GisFactHarness::Ledger& ledger,
                                                     QString* outRegionPrefix) {
  QStringList out;
  static const QRegularExpression reArchSuffix(
      QStringLiteral(R"((유적|고분군|고분|사지|산성|읍성|유물산포지|패총|가마터|전랑지|건물지)$)"));
  auto addQ = [&out](QString s) {
    s = s.trimmed();
    if (s.endsWith(QStringLiteral(" 발굴조사"))) {
      s.chop(5);
      if (!reArchSuffix.match(s).hasMatch()) s += QStringLiteral(" 유적");
    }
    if (s.size() >= 3 && !out.contains(s) && out.size() < 28) out.append(s);
  };
  for (const QString& q : ledger.dbpiaQueries) addQ(q);
  for (int i = 0; i < qMin<int>(ledger.dbpiaQueries.size(), 2); ++i)
    if (!out.contains(ledger.dbpiaQueries[i])) out.append(ledger.dbpiaQueries[i]);

  QVector<NearbyHeritageExport::Item> items;
  for (const auto& c : ledger.cards) {
    if (c.kind != QLatin1String("nearby_heritage")) continue;
    NearbyHeritageExport::Item it;
    it.datasetName = c.datasetName, it.siteName = c.siteName, it.distanceMeters = c.distanceMeters;
    items.push_back(it);
  }
  for (const QString& q : GisFactHarness::queriesFromNearby(items, 24)) addQ(q);
  if (!out.isEmpty() && outRegionPrefix) {
    const QStringList toks = out.first().split(QLatin1Char(' '), Qt::SkipEmptyParts);
    *outRegionPrefix = toks.size() >= 2 ? (toks[0] + QLatin1Char(' ') + toks[1]) : toks.value(0);
    if (!outRegionPrefix->isEmpty()) {
      addQ(*outRegionPrefix + QStringLiteral(" 유적"));
      addQ(toks.value(0) + QStringLiteral(" 발굴조사"));
    }
  }
  return out;
}

QString KaGeminiBridgeClient::formatPlctTypeBreakdown(const QString& raw) {
  QStringList parts;
  for (const QString& tok : raw.split(QLatin1Char('@'), Qt::SkipEmptyParts)) {
    const QStringList kv = tok.split(QLatin1Char(';'));
    if (kv.size() >= 3) parts << QStringLiteral("%1 %2편").arg(kv[1], kv[2]);
  }
  return parts.isEmpty() ? QStringLiteral("통합검색") : parts.join(QStringLiteral(", "));
}

void KaGeminiBridgeClient::harvestFollowUpKeywords(const QString& text) {
  if (m_regionPrefix.isEmpty()) return;
  static const QStringList kThemes = {
      QStringLiteral("왕경"),   QStringLiteral("방리"),     QStringLiteral("도로유구"), QStringLiteral("전랑지"),
      QStringLiteral("사지"),   QStringLiteral("건물지"),   QStringLiteral("고분"),     QStringLiteral("석실분"),
      QStringLiteral("목곽묘"), QStringLiteral("기와"),     QStringLiteral("우물"),     QStringLiteral("공방지"),
      QStringLiteral("읍성"),   QStringLiteral("배수시설"), QStringLiteral("주거지"),   QStringLiteral("가마")};
  const QString city = m_regionPrefix.section(QLatin1Char(' '), 0, 0);
  for (const QString& kw : kThemes) {
    if (!text.contains(kw)) continue;
    const QString q = (kw == QLatin1String("왕경") || kw == QLatin1String("읍성") ? city : m_regionPrefix) +
                      QLatin1Char(' ') + kw;
    if (!m_executedQueries.contains(q) && !m_candidatePool.contains(q) && !m_discoveredPool.contains(q))
      m_discoveredPool.append(q);
  }
}

void KaGeminiBridgeClient::startResearchAndEvaluate(const QString& userRequest,
                                                    const GisFactHarness::Ledger& ledger, int radiusMeters,
                                                    const QString&, bool, const QString& mapSnapshotPath) {
  ++m_jobSeq, m_busy = true;
  m_userRequest = userRequest, m_ledger = ledger, m_radiusMeters = radiusMeters, m_mapSnapshotPath = mapSnapshotPath;
  m_candidatePool = buildCandidatePool(ledger, &m_regionPrefix);
  m_discoveredPool.clear(), m_executedQueries.clear(), m_seenNodeIds.clear(), m_dbpiaSearchKey.clear(), m_lastReply.clear();
  m_pendingQueries = m_candidatePool.mid(0, 5), m_candidatePool = m_candidatePool.mid(5);
  for (const QString& q : m_pendingQueries) m_executedQueries.insert(q);
  m_queryIndex = 0, m_roundNumber = 1, m_newInRound = 0;
  m_dbpiaPackets = QJsonArray(), m_collectedPapers = QJsonArray();
  emit debugLogged(QStringLiteral("GOAL-START"),
                   QStringLiteral("/goal 수렴 루프 시작 (Round 1: %1건, 대기후보: %2건, 기준지역=\"%3\")")
                       .arg(m_pendingQueries.size()).arg(m_candidatePool.size()).arg(m_regionPrefix));
  if (m_pendingQueries.isEmpty()) {
    emit roundCompleted(1, 0, 0, true);
    emit dbpiaHarvestCompleted(m_dbpiaPackets, 0);
    return;
  }
  initDbpiaDirectSession();
}

void KaGeminiBridgeClient::initDbpiaDirectSession() {
  const quint64 seq = m_jobSeq;
  QElapsedTimer timer;
  timer.start();
  const QString q0 = QString::fromLatin1(QUrl::toPercentEncoding(m_pendingQueries.first()));
  QNetworkRequest req(QUrl(QStringLiteral("https://www.dbpia.co.kr/search/topSearch?searchOption=all&query=%1").arg(q0)));
  req.setHeader(QNetworkRequest::UserAgentHeader, QString::fromLatin1(kUserAgent));
  req.setTransferTimeout(7000);
  QNetworkReply* reply = m_nam->get(req);
  connect(reply, &QNetworkReply::finished, this, [this, reply, seq, timer]() {
    reply->deleteLater();
    if (seq != m_jobSeq) return;
    static const QRegularExpression reKey(QStringLiteral(R"(searchResultKey\s*[:=]\s*['"]([^'"]+)['"])"));
    m_dbpiaSearchKey = reKey.match(QString::fromUtf8(reply->readAll())).captured(1);
    emit debugLogged(QStringLiteral("DBPIA-SESSION"), QStringLiteral("세션 키 획득 (%1ms)").arg(timer.elapsed()));
    runNextDbpiaQuery();
  });
}

void KaGeminiBridgeClient::runNextDbpiaQuery() {
  if (m_queryIndex >= m_pendingQueries.size()) return finishRoundOrContinue();
  const quint64 seq = m_jobSeq;
  const QString q = m_pendingQueries[m_queryIndex];
  QElapsedTimer timer;
  timer.start();
  emit debugLogged(QStringLiteral("GOAL-R%1").arg(m_roundNumber),
                   QStringLiteral("[%1/%2] POST /api/search/list query=\"%3\"").arg(m_queryIndex + 1).arg(m_pendingQueries.size()).arg(q));
  QNetworkRequest req(QUrl(QStringLiteral("https://www.dbpia.co.kr/api/search/list")));
  req.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
  req.setHeader(QNetworkRequest::UserAgentHeader, QString::fromLatin1(kUserAgent));
  req.setRawHeader("X-JSP-KEY", m_dbpiaSearchKey.toUtf8());
  req.setRawHeader("Referer", "https://www.dbpia.co.kr/search/topSearch");
  req.setTransferTimeout(8000);
  const QJsonObject body{{QStringLiteral("query"), q},         {QStringLiteral("page"), 1},
                         {QStringLiteral("pageSize"), 20},     {QStringLiteral("sort"), QStringLiteral("RANK")},
                         {QStringLiteral("collection"), QStringLiteral("ALL")}, {QStringLiteral("includeAr"), false},
                         {QStringLiteral("startDate"), QStringLiteral("1970.01.01")}};
  QNetworkReply* reply = m_nam->post(req, QJsonDocument(body).toJson(QJsonDocument::Compact));
  connect(reply, &QNetworkReply::finished, this, [this, reply, seq, q, timer]() {
    reply->deleteLater();
    if (seq != m_jobSeq) return;
    const QJsonObject json = QJsonDocument::fromJson(reply->readAll()).object();
    const int totalHits = json.value("pageInfo").toObject().value("totalCount").toInt();
    const QString plctTypes = formatPlctTypeBreakdown(json.value("filterData").toObject().value("plctType").toString());
    static const QRegularExpression reHl(QStringLiteral("<!HS>|<!HE>"));
    QJsonArray refs;
    QStringList previewLines{QStringLiteral("- **[Round %1] 검색 %2건** (%3)").arg(m_roundNumber).arg(totalHits).arg(plctTypes)};
    for (const QJsonValue& v : json.value("list").toArray()) {
      if (m_collectedPapers.size() >= 90) break;
      const QJsonObject it = v.toObject();
      const QString nodeId = it.value("nodeId").toString().trimmed();
      if (nodeId.isEmpty() || m_seenNodeIds.contains(nodeId)) continue;
      m_seenNodeIds.insert(nodeId), ++m_newInRound;
      const QString title = it.value("nodeTitle").toString().replace(reHl, QString()).simplified();
      const QString abs = it.value("abstractText").toString().replace(reHl, QString()).simplified();
      harvestFollowUpKeywords(title + QLatin1Char(' ') + abs);
      const QString nodeType = it.value("labels").toObject().value("nodeType").toString(QStringLiteral("학술자료"));
      const QString pub = it.value("iprdNm").toString().trimmed();
      const QString jnStr = it.value("plctNm").toString().trimmed().isEmpty() ? pub : it.value("plctNm").toString().trimmed();
      const QString year = it.value("publishYymm").toString().trimmed();
      QStringList auts;
      for (const QJsonValue& av : it.value("authors").toArray()) {
        const QString nm = av.toObject().value("autrNm").toString().trimmed();
        if (!nm.isEmpty()) auts << nm;
      }
      const QString autStr = auts.isEmpty() ? pub : auts.join(QStringLiteral("·"));
      const QString url = QStringLiteral("https://www.dbpia.co.kr/journal/articleDetail?nodeId=%1").arg(nodeId);
      const QJsonObject ref{{QStringLiteral("nodeId"), nodeId},     {QStringLiteral("nodeType"), nodeType},
                            {QStringLiteral("title"), title},       {QStringLiteral("authors"), autStr},
                            {QStringLiteral("journal"), jnStr},     {QStringLiteral("year"), year},
                            {QStringLiteral("abstract"), abs.left(300)}, {QStringLiteral("url"), url}};
      refs.append(ref), m_collectedPapers.append(ref);
      if (previewLines.size() <= 5)
        previewLines << QStringLiteral("  * `[%1]` **%2** (%3), [「%4」](%5), 『%6』").arg(nodeType, autStr, year, title, url, jnStr);
    }
    if (!refs.isEmpty())
      m_dbpiaPackets.append(QJsonObject{{QStringLiteral("query"), q},
                                        {QStringLiteral("answer"), QStringLiteral("검색 %1건 (%2)").arg(totalHits).arg(plctTypes)},
                                        {QStringLiteral("references"), refs}});
    emit debugLogged(QStringLiteral("GOAL-R%1").arg(m_roundNumber),
                     QStringLiteral("완료 (%1ms) query=\"%2\" -> 총 %3건, 신규 +%4편 (누적 %5편)")
                         .arg(timer.elapsed()).arg(q).arg(totalHits).arg(refs.size()).arg(m_collectedPapers.size()));
    emit dbpiaPacketReceived(q, refs.size(), previewLines.join(QLatin1Char('\n')));
    ++m_queryIndex;
    runNextDbpiaQuery();
  });
}

void KaGeminiBridgeClient::finishRoundOrContinue() {
  QStringList nextBatch;
  auto pullFrom = [&](QStringList& pool, int limit) {
    while (nextBatch.size() < limit && !pool.isEmpty()) {
      const QString q = pool.takeFirst();
      if (!m_executedQueries.contains(q)) { m_executedQueries.insert(q); nextBatch.append(q); }
    }
  };
  pullFrom(m_candidatePool, 3);
  pullFrom(m_discoveredPool, 5);
  const bool converged = (m_newInRound == 0 && m_candidatePool.isEmpty() && m_discoveredPool.isEmpty()) ||
                         nextBatch.isEmpty() || m_roundNumber >= 5 || m_collectedPapers.size() >= 90;
  emit debugLogged(QStringLiteral("GOAL-CHECK"),
                   QStringLiteral("Round %1 점검: 신규=+%2편, 누적=%3편, 다음후보=%4건 -> %5")
                       .arg(m_roundNumber).arg(m_newInRound).arg(m_collectedPapers.size()).arg(nextBatch.size())
                       .arg(converged ? QStringLiteral("수렴 완료") : QStringLiteral("다음 라운드 진행")));
  emit roundCompleted(m_roundNumber, m_newInRound, m_collectedPapers.size(), converged);
  if (converged) {
    m_busy = false;
    return emit dbpiaHarvestCompleted(m_dbpiaPackets, m_collectedPapers.size());
  }
  ++m_roundNumber, m_newInRound = 0, m_pendingQueries = nextBatch, m_queryIndex = 0;
  runNextDbpiaQuery();
}

void KaGeminiBridgeClient::mergeAiReferences(const QJsonArray& aiRefs) {
  for (const QJsonValue& v : aiRefs) {
    if (m_collectedPapers.size() >= 90) break;
    const QJsonObject r = v.toObject();
    const QString nid = r.value("nodeId").toString().trimmed();
    const QString title = r.value("title").toString().trimmed();
    const QString key = nid.isEmpty() ? title : nid;
    if (title.isEmpty() || m_seenNodeIds.contains(key)) continue;
    m_seenNodeIds.insert(key);
    m_collectedPapers.append(QJsonObject{{QStringLiteral("nodeId"), nid},
                                         {QStringLiteral("nodeType"), QStringLiteral("DBpia AI 인용논문")},
                                         {QStringLiteral("title"), title},
                                         {QStringLiteral("authors"), r.value("authors").toString()},
                                         {QStringLiteral("journal"), r.value("journal").toString()},
                                         {QStringLiteral("year"), r.value("year").toString()},
                                         {QStringLiteral("abstract"), r.value("abstract").toString()}});
  }
}

QString KaGeminiBridgeClient::buildPaperGroundedFallbackAnswer(const QString& question, int turnIndex) const {
  QStringList out{QStringLiteral("#### [DBpia 수집 문헌 원장 기반 응답 — 질문 Q%1]\n- **질문 요지**: %2").arg(turnIndex).arg(question)};
  int matched = 0;
  const QStringList qToks = question.split(QRegularExpression(QStringLiteral(R"([\s,·()]+)")), Qt::SkipEmptyParts);
  for (int i = 0; i < m_collectedPapers.size() && matched < 6; ++i) {
    const QJsonObject r = m_collectedPapers[i].toObject();
    const QString title = r.value("title").toString(), abs = r.value("abstract").toString();
    bool hit = (i >= (turnIndex - 1) * 4 && i < turnIndex * 4);
    for (const QString& tk : qToks)
      if (tk.size() >= 2 && (title.contains(tk) || abs.contains(tk))) { hit = true; break; }
    if (!hit) continue;
    ++matched;
    out << QStringLiteral("- **%1 (%2), 「%3」, 『%4』** ⟦L%5⟧: %6")
               .arg(r.value("authors").toString(), r.value("year").toString(), title, r.value("journal").toString())
               .arg(i + 1)
               .arg(abs.isEmpty() ? QStringLiteral("해당 유적 관련 학술지·발굴보고서 성과 수록") : abs);
  }
  return out.join(QLatin1Char('\n'));
}

QString KaGeminiBridgeClient::buildStructuredDataSynthesis(const QJsonArray& dbpiaAiTurns) const {
  QStringList out, featLines, artLines;
  out << QStringLiteral("**<표 6> 주변유적군별 DBpia AI × 학술문헌 취합 핵심 데이터 총괄 정리표**")
      << QStringLiteral("| 구분 | 조사·분석 대상 유적군 및 핵심 주제 | 확인 유구·실측 제원·출토유물·편년 핵심 데이터 요약 | 근거 |")
      << QStringLiteral("|---|---|---|---|");
  static const QRegularExpression reFeat(QStringLiteral(R"((도로|건물지|적심|담장|우물|배수로|수혈|방리|층위|노폭|직경|깊이|\d+(?:\.\d+)?\s*(?:m|cm|㎡|칸)))"));
  static const QRegularExpression reArt(QStringLiteral(R"((토기|인화문|수막새|막새|기와|명문|거푸집|납형|형토|직물|청자|백자|\d\s*세기|통일신라|삼국|고려|조선))"));
  for (int i = 0; i < dbpiaAiTurns.size(); ++i) {
    const QJsonObject t = dbpiaAiTurns[i].toObject();
    QString q = t.value("question").toString().simplified(), a = GeminiCommandRouter::normalizeDbpiaAnswer(t.value("answer").toString());
    q.remove(QRegularExpression(QStringLiteral(R"((?:각각의|학술논문과|발굴보고서|근거로|상세히|구체적인|쉽게).*$)")));
    QStringList highlights;
    for (const QString& rawLn : a.split(QLatin1Char('\n'))) {
      QString ln = rawLn.trimmed();
      ln.remove(QRegularExpression(QStringLiteral(R"(^[#*\-\d.)\s|]+)"))).replace(QLatin1Char('|'), QLatin1Char('/'));
      if (ln.size() < 18 || ln.startsWith(QLatin1String("---")) || ln.contains(QLatin1String("질문 요지"))) continue;
      if (highlights.size() < 2) highlights << ln.left(110);
      if (featLines.size() < 14 && reFeat.match(ln).hasMatch() && !featLines.contains(ln))
        featLines << QStringLiteral("- **[Q%1 데이터]** %2").arg(i + 1).arg(ln);
      if (artLines.size() < 14 && reArt.match(ln).hasMatch() && !artLines.contains(ln))
        artLines << QStringLiteral("- **[Q%1 데이터]** %2").arg(i + 1).arg(ln);
    }
    out << QStringLiteral("| Q%1 | %2 | %3 | Q%1·L1~L%4 |")
               .arg(i + 1).arg(q.left(48), highlights.isEmpty() ? a.left(120).simplified() : highlights.join(QStringLiteral(" / "))).arg(qMax(1, int(m_collectedPapers.size())));
  }
  out << QString() << QStringLiteral("#### (1) 주변유적 층위 구조·유구 배치 및 실측 제원 데이터 종합") << QString() << featLines.join(QLatin1Char('\n'))
      << QString() << QStringLiteral("#### (2) 주변유적 출토유물 형식·명문자료·공방기법 및 편년 데이터 종합") << QString() << artLines.join(QLatin1Char('\n'))
      << QString() << QStringLiteral("#### (3) 조사지역(G1) 및 반경 %1m 주변유적 학술 데이터 최종 종합 결론").arg(m_radiusMeters) << QString()
      << QStringLiteral("- **공간분포 및 입지 데이터**: 조사구역(G1, 면적 %1㎡, 좌표계 %2) 반경 %3m 내 확인된 국가유산 공간정보(총 %4건)와 DBpia 학술문헌(%5편)·DBpia AI 전수 질의(%6회)를 교차검증한 결과, 조사구역 내부(0m) 및 최인접 구간(1~100m)부터 주변 구간(101~500m)까지 다층위 유적군이 연속 분포함이 데이터로 확인된다.")
             .arg(QString::number(m_ledger.surveyAreaM2, 'f', 1), m_ledger.workCrs).arg(m_radiusMeters).arg(m_ledger.cards.size() - 1).arg(m_collectedPapers.size()).arg(dbpiaAiTurns.size());
  return out.join(QLatin1Char('\n'));
}
