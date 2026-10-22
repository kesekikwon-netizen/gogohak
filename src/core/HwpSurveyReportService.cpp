#include "HwpSurveyReportService.h"
#include "LayerFeatures.h"
#include "LayerOps.h"
#include <QFileInfo>
#include <QLocale>
#include <QRegularExpression>
#include <cmath>
#include <qgsfeature.h>
#include <qgsfeatureiterator.h>
#include <qgsgeometry.h>
#include <qgsproject.h>
#include <qgsvectorlayer.h>
#include "core/document_engine.hpp"
#include "core/utf_conv.hpp"
#include "hwpsdk_include.h"

namespace HwpSurveyReportService {
namespace {
std::u16string toU16(const QString& s) { return openhwp::core::Utf8ToUtf16(s.toStdString()); }

void bindField(HWPSDK::Document* doc, const char16_t* label, const char16_t* name, const QString& val) {
  if (!doc) return;
  doc->CreateField(label, label, name);
  doc->PutFieldText(name, toU16(val).c_str());
}

void runAction(HWPSDK::Document* doc, const char16_t* id, const std::function<void(HWPSDK::ParameterSet*)>& fill) {
  if (!doc) return;
  if (HWPSDK::Action* act = doc->CreateAction(id)) {
    if (HWPSDK::ParameterSet* set = act->CreateParameterSet()) {
      act->GetDefault(set);
      fill(set);
      act->Execute(set);
      set->Release();
    }
    act->Release();
  }
}

void insertStyledParagraph(HWPSDK::Document* doc, const QString& text,
                           unsigned int heightHwpUnit, bool bold, unsigned int alignType) {
  runAction(doc, u"CharShape", [&](HWPSDK::ParameterSet* s) {
    s->SetUInt(u"Height", heightHwpUnit);
    s->SetUInt(u"Bold", bold ? 1U : 0U);
  });
  runAction(doc, u"ParagraphShape", [&](HWPSDK::ParameterSet* s) { s->SetUInt(u"AlignType", alignType); });
  const std::u16string u16 = toU16(text + QStringLiteral("\n"));
  runAction(doc, u"InsertText", [&](HWPSDK::ParameterSet* s) { s->SetString(u"Text", u16.c_str()); });
}

void appendTableToModel(HWPSDK::Document* doc, const QVector<QStringList>& rows, uint32_t headerBgColor = 0xE8EEF5) {
  if (!doc || rows.isEmpty()) return;
  const int rCount = rows.size(), cCount = rows.first().size();
  runAction(doc, u"TableCreate", [&](HWPSDK::ParameterSet* s) {
    s->SetInt(u"Rows", rCount);
    s->SetInt(u"Cols", cCount);
    s->SetInt(u"TreatAsChar", 1);
  });
  auto* impl = dynamic_cast<openhwp::core::DocumentImpl*>(doc->GetImpl());
  if (!impl || impl->Model().tables.empty()) return;
  auto& tbl = impl->Model().tables.back();
  for (int r = 0; r < rCount && r < tbl.rows; ++r) {
    for (int c = 0; c < cCount && c < tbl.cols; ++c) {
      auto& cell = tbl.cells[static_cast<size_t>(r)][static_cast<size_t>(c)];
      cell.text_utf8 = rows[r][c].toStdString();
      if (r == 0) {
        cell.bg_color = headerBgColor;
        cell.char_shape.bold = 1;
        cell.para_shape.align_type = 3;
      }
    }
  }
}

QString buildDefaultDiscussion(const SurveySnapshot& snap) {
  const QLocale loc(QLocale::Korean);
  QStringList lines;
  lines << QStringLiteral("본 조사는 %1 일원(면적 %2㎡, 좌표계 %3)을 대상으로 수행되었다.")
               .arg(snap.surveyTitle, loc.toString(std::round(snap.totalAreaSqm)), snap.surveyCrs);
  if (snap.nearbySites.isEmpty()) {
    lines << QStringLiteral("조사구역 반경 1,000m 이내에서 확인된 기보고 유적 레이어가 없거나 아직 로드되지 않았다.");
  } else {
    int in500 = 0;
    for (const auto& it : snap.nearbySites)
      if (it.distanceMeters <= 500) ++in500;
    const auto& n = snap.nearbySites.first();
    lines << QStringLiteral("주변 유적 공간분석 결과, 반경 1,000m 이내에서 총 %1개소(500m 이내 %2개소)의 유적이 확인되며, 최단거리 유적은 %3(%4m, %5쪽)이다.")
                 .arg(snap.nearbySites.size()).arg(in500).arg(n.siteName).arg(n.distanceMeters).arg(n.direction);
  }
  return lines.join(QStringLiteral(" "));
}
}  // namespace

struct ReportDocumentSession::Impl {
  HWPSDK::Application app;
  HWPSDK::HwpObject* hwpObj = nullptr;
  HWPSDK::Document* doc = nullptr;
  Impl() {
    app.SetLicensePath(nullptr);
    app.InitInstance();
    if ((hwpObj = app.GetHwpObject()) && (doc = hwpObj->CreateDocument())) doc->SetEditMode(1);
  }
  ~Impl() {
    if (hwpObj && doc) hwpObj->ReleaseDocument(doc);
    app.ExitInstance();
  }
};

ReportDocumentSession::ReportDocumentSession() : m_impl(new Impl()) {}
ReportDocumentSession::~ReportDocumentSession() { delete m_impl; }

SurveySnapshot ReportDocumentSession::collectFromProject(QgsProject* project, bool enableRagSlot,
                                                         const RagSlotCallback& ragSlot) {
  SurveySnapshot snap;
  snap.surveyTitle = QStringLiteral("신규 고고학 지표·시굴조사");
  snap.surveyCrs = QStringLiteral("EPSG:5186");
  if (project) {
    if (!project->title().trimmed().isEmpty()) snap.surveyTitle = project->title().trimmed();
    else if (!project->fileName().isEmpty()) snap.surveyTitle = QFileInfo(project->fileName()).completeBaseName();
    if (project->crs().isValid()) snap.surveyCrs = project->crs().authid();
    for (QgsVectorLayer* areaLayer : LayerOps::findAllByLayerKey(project, QStringLiteral("survey_area"))) {
      if (!areaLayer || !areaLayer->isValid()) continue;
      QgsFeatureIterator it = areaLayer->getFeatures();
      QgsFeature f;
      while (it.nextFeature(f))
        if (f.hasGeometry()) { snap.totalAreaSqm += std::abs(f.geometry().area()); ++snap.surveyPolygonCount; }
    }
    const QList<QPair<QString, QString>> defs = {
        {QStringLiteral("survey_area"), QStringLiteral("조사구역")}, {QStringLiteral("trial_trench"), QStringLiteral("트렌치")},
        {QStringLiteral("feature_poly"), QStringLiteral("유구(면)")}, {QStringLiteral("feature_line"), QStringLiteral("유구(선)")},
        {QStringLiteral("artifact_point"), QStringLiteral("유물(점)")}, {QStringLiteral("section_line"), QStringLiteral("단면선")},
        {QStringLiteral("control_points"), QStringLiteral("기준점")}};
    for (const auto& def : defs) {
      int cnt = 0;
      for (QgsVectorLayer* vl : LayerOps::findAllByLayerKey(project, def.first))
        if (vl && vl->isValid()) cnt += static_cast<int>(LayerFeatures::count(vl));
      snap.domainCounts.push_back({def.first, def.second, cnt});
    }
    const NearbyHeritageExport::Result nearby = NearbyHeritageExport::collect(project);
    if (nearby.ok()) snap.nearbySites = nearby.items;
  }
  snap.discussionDraft = buildDefaultDiscussion(snap);
  snap.ragCitationSlotText = (enableRagSlot && ragSlot)
      ? ragSlot(snap)
      : QStringLiteral("[RAG 연결 슬롯 대기 중 — 로컬 발굴조사보고서 인덱싱 완료 후 자동 인용 연결]");
  return snap;
}

bool ReportDocumentSession::buildStandardReport(const SurveySnapshot& snap, QString* errorOut) {
  if (!m_impl || !m_impl->doc) {
    if (errorOut) *errorOut = QStringLiteral("OpenHwpSDK 문서 엔진을 초기화하지 못했습니다.");
    return false;
  }
  HWPSDK::Document* doc = m_impl->doc;
  doc->Clear();
  doc->SetEditMode(1);
  const QString areaStr = QLocale(QLocale::Korean).toString(std::round(snap.totalAreaSqm)) + QStringLiteral(" ㎡");
  insertStyledParagraph(doc, QStringLiteral("%1 발굴(시굴)조사 보고서").arg(snap.surveyTitle), 1800, true, 3);
  insertStyledParagraph(doc, QStringLiteral("Ⅰ. 조사 개요"), 1300, true, 1);
  insertStyledParagraph(doc, QStringLiteral("1. 조사명: %1\n2. 조사면적: %2 (%3개 구역)\n3. 작업좌표계: %4").arg(snap.surveyTitle, areaStr).arg(snap.surveyPolygonCount).arg(snap.surveyCrs), 1000, false, 0);
  bindField(doc, u"조사명", u"SURVEY_NAME", snap.surveyTitle);
  bindField(doc, u"조사면적", u"SURVEY_AREA_M2", areaStr);
  bindField(doc, u"좌표계", u"SURVEY_CRS", snap.surveyCrs);
  insertStyledParagraph(doc, QStringLiteral("Ⅱ. 주변 유적 현황 (<표 1>)"), 1300, true, 1);
  QVector<QStringList> siteRows = {{QStringLiteral("연번"), QStringLiteral("구분"), QStringLiteral("유적명"), QStringLiteral("거리(m)"), QStringLiteral("방위"), QStringLiteral("반경구분")}};
  int seq = 1;
  for (const auto& item : snap.nearbySites) {
    siteRows.push_back({QString::number(seq++), item.datasetName, item.siteName, QString::number(item.distanceMeters), item.direction, item.radiusLabel});
    if (seq > 40) break;
  }
  if (siteRows.size() == 1)
    siteRows.push_back({QStringLiteral("-"), QStringLiteral("-"), QStringLiteral("반경 내 로드된 주변유적 없음"), QStringLiteral("-"), QStringLiteral("-"), QStringLiteral("-")});
  appendTableToModel(doc, siteRows);

  insertStyledParagraph(doc, QStringLiteral("Ⅲ. 조사구역 내 유구·유물 집계 (<표 2>)"), 1300, true, 1);
  QVector<QStringList> domRows = {{QStringLiteral("레이어 구분"), QStringLiteral("식별 키"), QStringLiteral("도형 수(건)")}};
  for (const auto& d : snap.domainCounts)
    domRows.push_back({d.label, d.layerKey, QString::number(d.featureCount)});
  appendTableToModel(doc, domRows);

  insertStyledParagraph(doc, QStringLiteral("Ⅳ. 고찰 및 발굴보고서 RAG 슬롯"), 1300, true, 1);
  insertStyledParagraph(doc, snap.discussionDraft, 1000, false, 0);
  insertStyledParagraph(doc, snap.ragCitationSlotText, 1000, false, 0);
  bindField(doc, u"고찰본문", u"DISCUSSION_BODY", snap.discussionDraft);
  bindField(doc, u"RAG인용슬롯", u"RAG_CITATION_SLOT", snap.ragCitationSlotText);
  return true;
}

bool ReportDocumentSession::openTemplateAndFill(const std::filesystem::path& templatePath,
                                                const SurveySnapshot& snap, QString* errorOut) {
  if (!m_impl || !m_impl->doc) return false;
  if (!m_impl->doc->Open(openhwp::core::PathToUtf16(templatePath).c_str())) {
    if (errorOut) *errorOut = QStringLiteral("한글 양식 파일을 열지 못했습니다.");
    return false;
  }
  const QString areaStr = QLocale(QLocale::Korean).toString(std::round(snap.totalAreaSqm)) + QStringLiteral(" ㎡");
  for (const auto& kv : QList<QPair<QString, QString>>{
           {QStringLiteral("SURVEY_NAME"), snap.surveyTitle}, {QStringLiteral("사업명"), snap.surveyTitle},
           {QStringLiteral("SURVEY_AREA_M2"), areaStr}, {QStringLiteral("조사면적"), areaStr},
           {QStringLiteral("SURVEY_CRS"), snap.surveyCrs}, {QStringLiteral("DISCUSSION_BODY"), snap.discussionDraft},
           {QStringLiteral("고찰본문"), snap.discussionDraft}, {QStringLiteral("RAG_CITATION_SLOT"), snap.ragCitationSlotText}})
    setFieldValue(kv.first, kv.second);
  return true;
}

bool ReportDocumentSession::saveAs(const std::filesystem::path& outputPath, const QString& format, QString* errorOut) {
  if (!m_impl || !m_impl->doc) return false;
  const std::u16string fmtU16 = toU16(format.trimmed().isEmpty() ? QStringLiteral("HWPX") : format.toUpper());
  if (!m_impl->doc->SaveAs(openhwp::core::PathToUtf16(outputPath).c_str(), fmtU16.c_str())) {
    if (errorOut) *errorOut = QStringLiteral("한글 문서(%1) 저장에 실패했습니다.").arg(format);
    return false;
  }
  return true;
}

QVector<FieldEntry> ReportDocumentSession::fields() const {
  QVector<FieldEntry> out;
  if (!m_impl || !m_impl->doc) return out;
  auto* impl = dynamic_cast<openhwp::core::DocumentImpl*>(m_impl->doc->GetImpl());
  if (!impl) return out;
  for (const auto& f : impl->Model().fields)
    out.push_back({QString::fromStdString(f.name_utf8), QString::fromStdString(f.text_utf8)});
  return out;
}

bool ReportDocumentSession::setFieldValue(const QString& fieldName, const QString& value) {
  if (!m_impl || !m_impl->doc) return false;
  const std::u16string nameU16 = toU16(fieldName);
  if (!m_impl->doc->FieldExist(nameU16.c_str())) return false;
  m_impl->doc->PutFieldText(nameU16.c_str(), toU16(value).c_str());
  return true;
}

bool ReportDocumentSession::applyUserRequest(const QString& userPrompt, const SurveySnapshot& snap, QString* replyOut) {
  const QString req = userPrompt.trimmed();
  if (req.isEmpty() || !m_impl || !m_impl->doc) return false;
  HWPSDK::Document* doc = m_impl->doc;
  const QRegularExpression rx(QStringLiteral("['\"]([^'\"]+)['\"]\\s*(?:을|를)?\\s*['\"]([^'\"]+)['\"]\\s*(?:으로|로)?\\s*(?:바꿔|변경|치환)"));
  const QRegularExpressionMatch m = rx.match(req);
  if (m.hasMatch()) {
    const std::u16string fromU16 = toU16(m.captured(1)), toStrU16 = toU16(m.captured(2));
    runAction(doc, u"AllReplace", [&](HWPSDK::ParameterSet* s) {
      s->SetString(u"FindString", fromU16.c_str());
      s->SetString(u"ReplaceString", toStrU16.c_str());
    });
    if (replyOut) *replyOut = QStringLiteral("문서 내 '%1' 문구를 '%2'(으)로 일괄 치환했습니다.").arg(m.captured(1), m.captured(2));
    return true;
  }
  const QString areaStr = QLocale(QLocale::Korean).toString(std::round(snap.totalAreaSqm)) + QStringLiteral(" ㎡");
  const QString generated = req.contains(QStringLiteral("입지"))
      ? QStringLiteral("[입지 및 고고학적 배경] %1 조사지역(%2, %3)은 반경 1km 내 유적(%4개소)과 지형적 맥락을 공유하며, %5")
            .arg(snap.surveyTitle, areaStr, snap.surveyCrs).arg(snap.nearbySites.size()).arg(req)
      : QStringLiteral("[작성·보완 단락] %1 (%2 · %3 기준)").arg(req, snap.surveyTitle, areaStr);
  insertStyledParagraph(doc, generated, 1000, false, 0);
  setFieldValue(QStringLiteral("DISCUSSION_BODY"), snap.discussionDraft + QStringLiteral("\n") + generated);
  if (replyOut) *replyOut = QStringLiteral("요청 내용을 한글 본문 및 누름틀에 반영했습니다:\n\"%1\"").arg(generated);
  return true;
}

QString ReportDocumentSession::renderPreviewHtml() const {
  if (!m_impl || !m_impl->doc) return {};
  auto* impl = dynamic_cast<openhwp::core::DocumentImpl*>(m_impl->doc->GetImpl());
  if (!impl) return {};
  const auto& model = impl->Model();
  QString html = QStringLiteral("<html><body style='font-family:\"Noto Serif CJK KR\",\"NanumMyeongjo\",serif;background:#fff;color:#18181b;padding:24px;line-height:1.65;'>");
  for (const auto& p : model.paragraphs) {
    const QString txt = QString::fromStdString(p.text_utf8).trimmed();
    if (!txt.isEmpty())
      html += QStringLiteral("<p style='font-size:%1pt;font-weight:%2;text-align:%3;margin:8px 0;'>%4</p>")
                  .arg(qMax(10, static_cast<int>(p.char_shape.height / 100)))
                  .arg(p.char_shape.bold ? QStringLiteral("bold") : QStringLiteral("normal"),
                       p.para_shape.align_type == 3 ? QStringLiteral("center") : QStringLiteral("left"),
                       txt.toHtmlEscaped().replace(QLatin1Char('\n'), QStringLiteral("<br/>")));
  }
  for (const auto& tbl : model.tables) {
    html += QStringLiteral("<table border='1' cellspacing='0' cellpadding='6' style='border-collapse:collapse;width:100%;margin:14px 0;font-size:9.5pt;border-color:#94a3b8;'>");
    for (int r = 0; r < tbl.rows; ++r) {
      html += QStringLiteral("<tr>");
      for (int c = 0; c < tbl.cols; ++c)
        html += QStringLiteral("<td style='background:%1;font-weight:%2;'>%3</td>")
                    .arg(r == 0 ? QStringLiteral("#f1f5f9") : QStringLiteral("#ffffff"),
                         r == 0 ? QStringLiteral("bold") : QStringLiteral("normal"),
                         QString::fromStdString(tbl.cells[static_cast<size_t>(r)][static_cast<size_t>(c)].text_utf8).toHtmlEscaped());
      html += QStringLiteral("</tr>");
    }
    html += QStringLiteral("</table>");
  }
  if (!model.fields.empty()) {
    html += QStringLiteral("<hr style='margin:16px 0;border:0;border-top:1px solid #cbd5e1;'/><div style='font-size:9pt;color:#475569;'><b>[한글 누름틀(Field) 바인딩 현황]</b><ul>");
    for (const auto& f : model.fields)
      html += QStringLiteral("<li><code>{{%1}}</code> = %2</li>")
                  .arg(QString::fromStdString(f.name_utf8).toHtmlEscaped(), QString::fromStdString(f.text_utf8).toHtmlEscaped());
    html += QStringLiteral("</ul></div>");
  }
  return html + QStringLiteral("</body></html>");
}
}  // namespace HwpSurveyReportService
