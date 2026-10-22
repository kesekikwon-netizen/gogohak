#include "HwpAcademicVisuals.h"

#include <QJsonObject>
#include <QLocale>
#include <cmath>

namespace HwpAcademicVisuals {
namespace {

struct RingBins {
  int d0 = 0;
  int d100 = 0;
  int d300 = 0;
  int d500 = 0;
  int d1000 = 0;
  int maxCount() const { return qMax(1, qMax(qMax(d0, d100), qMax(d300, qMax(d500, d1000)))); }
};

RingBins computeRingBins(const HwpSurveyReportService::SurveySnapshot& snap) {
  RingBins b;
  for (const auto& s : snap.nearbySites) {
    if (s.distanceMeters <= 0) ++b.d0;
    else if (s.distanceMeters <= 100) ++b.d100;
    else if (s.distanceMeters <= 300) ++b.d300;
    else if (s.distanceMeters <= 500) ++b.d500;
    else ++b.d1000;
  }
  return b;
}

QString renderBarRow(const QString& label, int count, int total, int maxVal, const QString& barColor) {
  const int pctWidth = qBound(4, count * 100 / qMax(1, maxVal), 100);
  const double ratio = total > 0 ? (count * 100.0 / total) : 0.0;
  return QStringLiteral(
      "<tr><td width='140' style='font-size:9pt;font-weight:bold;color:#1e293b;padding:4px 6px;'>%1</td>"
      "<td style='padding:4px 8px;background:#f8fafc;'>"
      "<table width='%2%' cellspacing='0' cellpadding='0'><tr>"
      "<td style='background:%3;height:14px;border-radius:2px;'>&nbsp;</td></tr></table></td>"
      "<td width='95' align='right' style='font-size:9pt;font-weight:bold;color:#0f172a;padding:4px 6px;'>"
      "%4건 (%5%)</td></tr>")
      .arg(label.toHtmlEscaped())
      .arg(pctWidth)
      .arg(barColor)
      .arg(count)
      .arg(QString::number(ratio, 'f', 1));
}

}  // namespace

QString renderVisualsAndDbpiaSectionHtml(const HwpSurveyReportService::SurveySnapshot& snap,
                                         const DbpiaSummary& dbpia) {
  const RingBins bins = computeRingBins(snap);
  const int totalSites = snap.nearbySites.size();
  int cJournal = 0, cThesis = 0, cConf = 0, cReport = 0;
  for (const QJsonValue& v : dbpia.collectedPapers) {
    const QString tp = v.toObject().value(QStringLiteral("nodeType")).toString();
    if (tp.contains(QStringLiteral("저널")) || tp.contains(QStringLiteral("학술지"))) ++cJournal;
    else if (tp.contains(QStringLiteral("학위"))) ++cThesis;
    else if (tp.contains(QStringLiteral("학술대회"))) ++cConf;
    else ++cReport;
  }
  const int totalPapers = dbpia.collectedPapers.size();
  const int maxPaperBin = qMax(1, qMax(qMax(cJournal, cThesis), qMax(cConf, cReport)));

  QString html;
  // 1. 전문 모식도 1: 고고학 고찰·논문 논증 구조 모식도 (SmartArt Process Diagram)
  html += QStringLiteral(
      "<div style='margin:18px 0;padding:12px;border:2px solid #1e3a8a;background:#f8fafc;'>"
      "<div style='font-size:10pt;font-weight:bold;color:#1e3a8a;text-align:center;margin-bottom:10px;'>"
      "&lt;모식도 1&gt; 발굴조사보고서 고찰 및 학술논문 다중 근거 교차검증 모식도 (GIS × DBpia AI)</div>"
      "<table width='100%' cellspacing='4' cellpadding='8' style='border-collapse:separate;'><tr>"
      "<td width='23%' align='center' style='background:#dbeafe;border:1px solid #1d4ed8;font-size:8.5pt;'>"
      "<b>[1단계] GIS 실측 원장</b><br/>조사구역(%1㎡)<br/>주변유적 <b>%2개소</b></td>"
      "<td width='3%' align='center' style='font-size:12pt;font-weight:bold;color:#1e3a8a;'>➔</td>"
      "<td width='23%' align='center' style='background:#e0f2fe;border:1px solid #0284c7;font-size:8.5pt;'>"
      "<b>[2단계] DBpia 문헌 집성</b><br/>학술지·학위논문<br/>총 <b>%3편</b> 수렴</td>"
      "<td width='3%' align='center' style='font-size:12pt;font-weight:bold;color:#0f766e;'>➔</td>"
      "<td width='23%' align='center' style='background:#ccfbf1;border:1px solid #0f766e;font-size:8.5pt;'>"
      "<b>[3단계] DBpia AI 분석</b><br/>유구·유물·편년 질의<br/>총 <b>%4회</b> 교차검증</td>"
      "<td width='3%' align='center' style='font-size:12pt;font-weight:bold;color:#4f46e5;'>➔</td>"
      "<td width='22%' align='center' style='background:#ede9fe;border:1px solid #6d28d9;font-size:8.5pt;'>"
      "<b>[4단계] 고찰·논문 도출</b><br/>공간위계·성격 규명<br/>한글(.hwpx/.hwp) 조판</td>"
      "</tr></table></div>")
              .arg(QLocale(QLocale::Korean).toString(std::round(snap.totalAreaSqm)))
              .arg(totalSites)
              .arg(totalPapers)
              .arg(dbpia.dbpiaAiTurns.size());

  // 2. 전문 모식도 2: 조사구역 중심 8방위·거리권역 공간배치 모식도
  QMap<QString, QStringList> dirMap;
  for (const auto& s : snap.nearbySites) {
    const QString d = s.direction.isEmpty() ? QStringLiteral("인접") : s.direction;
    if (dirMap[d].size() < 2)
      dirMap[d] << QStringLiteral("%1(%2m)").arg(s.siteName.left(10)).arg(s.distanceMeters);
  }
  auto cellForDir = [&](const QString& dir) {
    const QString items = dirMap.value(dir).isEmpty() ? QStringLiteral("분포 유적 없음") : dirMap.value(dir).join(QStringLiteral("<br/>"));
    return QStringLiteral("<td width='33%' align='center' style='background:#ffffff;border:1px solid #94a3b8;padding:6px;font-size:8.5pt;'>"
                          "<b style='color:#0f766e;'>[%1쪽 권역]</b><br/>%2</td>").arg(dir, items);
  };
  html += QStringLiteral(
      "<div style='margin:16px 0;padding:12px;border:1px solid #475569;background:#f1f5f9;'>"
      "<div style='font-size:10pt;font-weight:bold;color:#0f172a;text-align:center;margin-bottom:8px;'>"
      "&lt;모식도 2&gt; 조사구역 중심 8방위 주변유적 공간배치 모식도 (반경 500m~1,000m)</div>"
      "<table width='100%' cellspacing='3' cellpadding='4'>")
      + QStringLiteral("<tr>") + cellForDir(QStringLiteral("북서")) + cellForDir(QStringLiteral("북")) + cellForDir(QStringLiteral("북동")) + QStringLiteral("</tr>")
      + QStringLiteral("<tr>") + cellForDir(QStringLiteral("서"))
      + QStringLiteral("<td width='34%' align='center' style='background:#fef3c7;border:2px solid #d97706;padding:8px;font-size:9pt;'>"
                       "<b style='color:#92400e;'>★ 기준 조사구역 (G1)</b><br/>%1<br/>(%2 ㎡ · %3)</td>")
            .arg(snap.surveyTitle.toHtmlEscaped(), QLocale(QLocale::Korean).toString(std::round(snap.totalAreaSqm)), snap.surveyCrs)
      + cellForDir(QStringLiteral("동")) + QStringLiteral("</tr>")
      + QStringLiteral("<tr>") + cellForDir(QStringLiteral("남서")) + cellForDir(QStringLiteral("남")) + cellForDir(QStringLiteral("남동")) + QStringLiteral("</tr>")
      + QStringLiteral("</table></div>");

  // 3. 전문 차트·그래프 1: 이격거리 구간별 주변유적 분포 막대 그래프
  html += QStringLiteral(
      "<div style='margin:16px 0;padding:12px;border:1px solid #cbd5e1;background:#ffffff;'>"
      "<div style='font-size:10pt;font-weight:bold;color:#0f172a;margin-bottom:8px;'>"
      "&lt;그림 1&gt; 이격거리 구간별 주변유적 분포 현황 그래프 (총 %1개소)</div>"
      "<table width='100%' cellspacing='0' cellpadding='2' style='border-top:2px solid #0f172a;border-bottom:2px solid #0f172a;'>")
              .arg(totalSites)
      + renderBarRow(QStringLiteral("조사구역 내 (0m)"), bins.d0, totalSites, bins.maxCount(), QStringLiteral("#dc2626"))
      + renderBarRow(QStringLiteral("최인접 (1~100m)"), bins.d100, totalSites, bins.maxCount(), QStringLiteral("#ea580c"))
      + renderBarRow(QStringLiteral("인접권 (101~300m)"), bins.d300, totalSites, bins.maxCount(), QStringLiteral("#0284c7"))
      + renderBarRow(QStringLiteral("주변권 (301~500m)"), bins.d500, totalSites, bins.maxCount(), QStringLiteral("#0d9488"))
      + renderBarRow(QStringLiteral("광역권 (500m 초과)"), bins.d1000, totalSites, bins.maxCount(), QStringLiteral("#64748b"))
      + QStringLiteral("</table></div>");

  // 4. 전문 차트·그래프 2: DBpia 발행유형별 학술문헌 분포 차트
  html += QStringLiteral(
      "<div style='margin:16px 0;padding:12px;border:1px solid #cbd5e1;background:#ffffff;'>"
      "<div style='font-size:10pt;font-weight:bold;color:#0f172a;margin-bottom:8px;'>"
      "&lt;그림 2&gt; DBpia AI · 학술문헌 발행유형별 수집 현황 차트 (총 %1편 · 라운드 %2)</div>"
      "<table width='100%' cellspacing='0' cellpadding='2' style='border-top:2px solid #0f172a;border-bottom:2px solid #0f172a;'>")
              .arg(totalPapers)
              .arg(dbpia.roundNumber)
      + renderBarRow(QStringLiteral("학술지 논문"), cJournal, totalPapers, maxPaperBin, QStringLiteral("#1d4ed8"))
      + renderBarRow(QStringLiteral("학위 논문(석·박사)"), cThesis, totalPapers, maxPaperBin, QStringLiteral("#7c3aed"))
      + renderBarRow(QStringLiteral("학술대회 발표논문"), cConf, totalPapers, maxPaperBin, QStringLiteral("#0f766e"))
      + renderBarRow(QStringLiteral("발굴보고서·단행본"), cReport, totalPapers, maxPaperBin, QStringLiteral("#b45309"))
      + QStringLiteral("</table></div>");

  // 5. DBpia 수집 학술논문 표 (<표 3>) 및 고찰·논문 종합 서술
  if (!dbpia.collectedPapers.isEmpty()) {
    html += QStringLiteral(
        "<div style='font-size:11pt;font-weight:bold;color:#0f172a;margin-top:16px;'>"
        "Ⅴ. DBpia 학술문헌 및 DBpia AI 교차검증 고찰 (&lt;표 3&gt;)</div>"
        "<table border='1' cellspacing='0' cellpadding='5' style='border-collapse:collapse;width:100%;margin:10px 0;font-size:8.5pt;border-top:2px solid #0f172a;border-bottom:2px solid #0f172a;border-color:#475569;'>"
        "<tr style='background:#e2e8f0;font-weight:bold;text-align:center;'>"
        "<td width='40'>번호</td><td width='70'>유형</td><td width='95'>저자(연도)</td><td>논문명 / 수록 학술지</td></tr>");
    const int limit = qMin<int>(15, dbpia.collectedPapers.size());
    for (int i = 0; i < limit; ++i) {
      const QJsonObject p = dbpia.collectedPapers[i].toObject();
      html += QStringLiteral("<tr><td align='center'>L%1</td><td align='center'>%2</td><td>%3 (%4)</td><td><b>%5</b>, 『%6』</td></tr>")
                  .arg(i + 1)
                  .arg(p.value(QStringLiteral("nodeType")).toString().toHtmlEscaped(),
                       p.value(QStringLiteral("authors")).toString().toHtmlEscaped(),
                       p.value(QStringLiteral("year")).toString().toHtmlEscaped(),
                       p.value(QStringLiteral("title")).toString().toHtmlEscaped(),
                       p.value(QStringLiteral("journal")).toString().toHtmlEscaped());
    }
    html += QStringLiteral("</table>");
  }
  if (!dbpia.synthesizedDiscussion.trimmed().isEmpty()) {
    html += QStringLiteral("<div style='margin:12px 0;padding:12px;background:#f8fafc;border-left:4px solid #1d4ed8;font-size:9.5pt;line-height:1.7;'>"
                           "<b>[DBpia AI × GIS 실측 고찰·논문 종합]</b><br/>%1</div>")
                .arg(dbpia.synthesizedDiscussion.toHtmlEscaped().replace(QLatin1Char('\n'), QStringLiteral("<br/>")));
  }
  return html;
}

void appendDbpiaAndVisualSummaryToDoc(const HwpSurveyReportService::SurveySnapshot& snap,
                                      const DbpiaSummary& dbpia,
                                      HwpSurveyReportService::ReportDocumentSession* session) {
  if (!session) return;
  const RingBins bins = computeRingBins(snap);
  const QString chartSummary = QStringLiteral(
      "\n[모식도 및 차트·그래프 분석 요약]\n"
      "• <모식도 1~2> GIS 실측 원장(%1개소) × DBpia 학술문헌(%2편) × DBpia AI 교차검증(%3회) 4단계 논증 구조 및 8방위 공간배치\n"
      "• <그림 1> 거리 구간별 주변유적 분포: 조사구역 내(0m) %4개소, 1~100m %5개소, 101~300m %6개소, 301~500m %7개소, 500m 초과 %8개소\n")
      .arg(snap.nearbySites.size()).arg(dbpia.collectedPapers.size()).arg(dbpia.dbpiaAiTurns.size())
      .arg(bins.d0).arg(bins.d100).arg(bins.d300).arg(bins.d500).arg(bins.d1000);
  session->setFieldValue(QStringLiteral("DISCUSSION_BODY"),
                         snap.discussionDraft + chartSummary + dbpia.synthesizedDiscussion);
}

}  // namespace HwpAcademicVisuals
