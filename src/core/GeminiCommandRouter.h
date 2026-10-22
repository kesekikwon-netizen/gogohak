#pragma once
// 제미나이 GIS · DBpia 조수 자연어 명령 라우터 및 사실 기반(G 카드 + DBpia L 카드) 프롬프트 조립기.
// - 한국어 자연어 명령을 GIS 실행 Intent(버퍼·엑셀·유산·환경·도면)와 DBpia 학술 비교·평가 Intent로 분류한다.
// - GIS 실측 원장(GisFactHarness::Ledger)과 DBpia 검색 결과만 묶어 날조 차단 프롬프트를 만든다.
#include "GisFactHarness.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QSet>
#include <QString>
#include <QStringList>

namespace GeminiCommandRouter {

enum class Intent {
  ExportNearbyExcel,       // "주변유적 500m 안에 유적 엑셀로 만들어줘"
  DrawBuffer,              // "500m 버퍼 그려줘", "1000m 경계 표시"
  FetchHeritage,           // "주변 유산 받아줘"
  DownloadCadastral,       // "지적도 받아줘"
  ToggleEnvironmentLayer,  // "토양도 켜줘", "지질도", "수계도", "DEM", "고지형", "수치지형도"
  PlaceTrenchGrid,         // "시굴격자 놓아줘"
  OpenLayout,              // "도면 만들어줘"
  OpenSubmitReview,        // "검수·제출"
  ExportWordCraft,         // "고찰 워드(.docx)로 내보내줘"
  ResearchAndEvaluate      // "주변 500m 유적 데이터 조사해줘", "이 유적은 주변유적과 비교해봐", "고찰·논문 작성"
};

struct ParsedCommand {
  Intent intent = Intent::ResearchAndEvaluate;
  int radiusMeters = 500;
  QString envLayerKey;  // "soil" | "geology" | "river" | "dem" | "paleo" | "topo" | "terrain" | "contour"
  QString docType = QStringLiteral("report");  // "report" | "paper"
  bool autoExportDocx = false;
  QString rawText;
  QJsonObject harnessCommand;  // GisFactHarness::validateCommand 호환 명령
};

// 한국어 자연어 입력을 분석해 실행할 Intent와 반경(m)을 결정한다.
ParsedCommand parse(const QString& userInput, int defaultRadiusMeters = 500);

// GIS 원장(G 카드)을 패널에 즉시 표시할 마크다운/텍스트 요약으로 변환한다.
QString formatLedgerSummary(const GisFactHarness::Ledger& ledger, int radiusMeters);

// GIS 실측 원장(G 카드)의 핵심 유적과 주변유적을 바탕으로 DBpia AI 에이전트에게 연속 질문할 심층 질문 목록을 생성한다.
QStringList buildDbpiaAiQuestions(const QString& userRequest, const GisFactHarness::Ledger& ledger,
                                  int radiusMeters);

struct LoopStepDecision {
  bool sufficient = false;
  QStringList questionsToAskDbpia;
  QString finalEvaluationMarkdown;
};

// 제미나이에게 GIS 원장과 DBpia AI 누적 답변을 보여주고 다음 질문([ASK_DBPIA: ...]) 또는 최종 고찰([STATUS: SUFFICIENT])을 요청한다.
QString buildGeminiLoopPrompt(const QString& userRequest, const GisFactHarness::Ledger& ledger,
                              const QJsonArray& dbpiaAiTurns, const QJsonArray& dbpiaPackets,
                              int radiusMeters, int roundNumber,
                              const QString& mapSnapshotPath = QString());

// 제미나이 응답에서 [ASK_DBPIA: ...] 질문 목록 또는 [STATUS: SUFFICIENT] 최종 고찰 본문을 추출한다.
LoopStepDecision parseGeminiLoopReply(const QString& geminiReply, const GisFactHarness::Ledger& ledger,
                                      const QSet<QString>& alreadyAsked, int roundNumber,
                                      int radiusMeters = 500);

// GIS 실측 원장(G 카드) + DBpia 검색 결과(L 카드) + 사용자 요청을 묶어 날조 금지 프롬프트를 만든다.
QString buildEvaluationPrompt(const QString& userRequest, const GisFactHarness::Ledger& ledger,
                              const QJsonArray& dbpiaPackets, int radiusMeters,
                              const QString& mapSnapshotPath = QString());

// DBpia AI 웹 답변 텍스트의 대화체 인사말·각주 번호를 정리하고 탭(TSV) 표를 마크다운 표로 변환한다.
QString normalizeDbpiaAnswer(const QString& rawAnswer);

}  // namespace GeminiCommandRouter
