#include "core/HwpSurveyReportService.h"

#include <QFileInfo>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QtTest>

class TestHwpSurveyReport : public QObject {
  Q_OBJECT

 private slots:
  void initTestCase() { QStandardPaths::setTestModeEnabled(true); }

  void buildReportAndHtmlPreviewWithoutRagBlocking() {
    HwpSurveyReportService::SurveySnapshot snap =
        HwpSurveyReportService::ReportDocumentSession::collectFromProject(nullptr, false, {});
    snap.surveyTitle = QStringLiteral("경주 황남동 120번지 유적 시굴조사");
    snap.surveyCrs = QStringLiteral("EPSG:5187");
    snap.totalAreaSqm = 2450.5;
    snap.surveyPolygonCount = 1;
    snap.domainCounts = {
        {QStringLiteral("survey_area"), QStringLiteral("조사구역"), 1},
        {QStringLiteral("trial_trench"), QStringLiteral("트렌치"), 3},
        {QStringLiteral("feature_poly"), QStringLiteral("유구(면)"), 4},
        {QStringLiteral("artifact_point"), QStringLiteral("유물(점)"), 11},
    };

    NearbyHeritageExport::Item site;
    site.datasetName = QStringLiteral("국가유산 공간정보");
    site.siteName = QStringLiteral("경주 황남대총");
    site.distanceMeters = 185;
    site.direction = QStringLiteral("남동");
    site.radiusLabel = QStringLiteral("500m 이내");
    snap.nearbySites.push_back(site);

    HwpSurveyReportService::ReportDocumentSession session;
    QString err;
    QVERIFY2(session.buildStandardReport(snap, &err), qPrintable(err));

    const auto fields = session.fields();
    QVERIFY(fields.size() >= 5);

    const QString html = session.renderPreviewHtml();
    QVERIFY(html.contains(QStringLiteral("경주 황남동 120번지 유적 시굴조사")));
    QVERIFY(html.contains(QStringLiteral("경주 황남대총")));
    QVERIFY(html.contains(QStringLiteral("RAG 연결 슬롯 대기 중")));
  }

  void writeAndReloadHwpxAndHwp5() {
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    HwpSurveyReportService::SurveySnapshot snap =
        HwpSurveyReportService::ReportDocumentSession::collectFromProject(nullptr, false, {});
    snap.surveyTitle = QStringLiteral("부여 관북리 유적 발굴조사");
    snap.surveyCrs = QStringLiteral("EPSG:5186");
    snap.totalAreaSqm = 1280.0;
    snap.surveyPolygonCount = 1;

    HwpSurveyReportService::ReportDocumentSession session;
    QString err;
    QVERIFY2(session.buildStandardReport(snap, &err), qPrintable(err));

    const QString hwpxPath = dir.filePath(QStringLiteral("survey_report.hwpx"));
    QVERIFY2(session.saveAs(std::filesystem::path(hwpxPath.toStdString()), QStringLiteral("HWPX"), &err),
             qPrintable(err));
    QVERIFY(QFileInfo::exists(hwpxPath));
    QVERIFY(QFileInfo(hwpxPath).size() > 256);

    const QString hwpPath = dir.filePath(QStringLiteral("survey_report.hwp"));
    QVERIFY2(session.saveAs(std::filesystem::path(hwpPath.toStdString()), QStringLiteral("HWP"), &err),
             qPrintable(err));
    QVERIFY(QFileInfo::exists(hwpPath));
    QVERIFY(QFileInfo(hwpPath).size() > 512);

    // 기존 양식(.hwpx) 불러오기 + 누름틀 필드 치환 검증
    HwpSurveyReportService::SurveySnapshot updated = snap;
    updated.surveyTitle = QStringLiteral("부여 관북리 유적 정밀발굴조사 (수정본)");
    HwpSurveyReportService::ReportDocumentSession tplSession;
    QVERIFY2(tplSession.openTemplateAndFill(std::filesystem::path(hwpxPath.toStdString()), updated, &err),
             qPrintable(err));
    QVERIFY(tplSession.renderPreviewHtml().contains(QStringLiteral("부여 관북리 유적 정밀발굴조사 (수정본)")));

    // 사용자 직접 요청(프롬프트) 반영 및 일괄 치환 검증
    QString reply;
    QVERIFY(tplSession.applyUserRequest(
        QStringLiteral("고찰에 백제 사비기 왕궁지와의 공간적 위계 관계를 추가해줘"), updated, &reply));
    QVERIFY(tplSession.renderPreviewHtml().contains(QStringLiteral("백제 사비기 왕궁지")));
    QVERIFY(tplSession.applyUserRequest(
        QStringLiteral("'왕궁지'를 '핵심 유구군'으로 바꿔줘"), updated, &reply));
    QVERIFY(tplSession.renderPreviewHtml().contains(QStringLiteral("핵심 유구군")));
  }
};

QTEST_MAIN(TestHwpSurveyReport)
#include "test_hwp_survey_report.moc"
