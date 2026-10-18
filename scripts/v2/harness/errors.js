"use strict";

const ERROR_CATALOG = {
  E_ENV_NODE: {
    cause: "Node.js 버전이 18 미만입니다.",
    action: "Node.js 18 이상(권장 20~22)을 설치한 뒤 다시 실행하십시오.",
    retryable: false,
    recovery: "none",
  },
  E_CDP_PORT: {
    cause: "Antigravity 내장 브라우저 CDP 포트를 찾지 못했습니다.",
    action: "Antigravity를 실행한 뒤 다시 시도하십시오.",
    retryable: false,
    recovery: "none",
  },
  E_GEMINI_WEBVIEW: {
    cause: "Antigravity 내장 Gemini 웹뷰 대상을 찾지 못했습니다.",
    action: "스튜디오의 [로그인 창 열기] 또는 Gemini 패널 열기를 실행하십시오.",
    retryable: false,
    recovery: "none",
  },
  E_CDP_DISCONNECT: {
    cause: "CDP 웹소켓 연결이 끊겼거나 명령 시간이 초과되었습니다.",
    action: "웹뷰 대상을 다시 탐색하고 새 CDP 세션으로 재연결합니다.",
    retryable: true,
    recovery: "reconnect-cdp",
  },
  E_GEMINI_TIMEOUT: {
    cause: "Gemini 응답 완료 대기 시간이 초과되었습니다.",
    action: "진행 중인 Gemini 생성을 정지하고 새 대화에서 다시 시도합니다.",
    retryable: true,
    recovery: "stop-and-new-chat",
  },
  E_EMPTY_REPLY: {
    cause: "Gemini 응답 텍스트가 비어 있습니다.",
    action: "새 대화를 열고 다시 요청합니다.",
    retryable: true,
    recovery: "new-chat",
  },
  E_TRUNCATED: {
    cause: "Gemini 응답 끝에 완료 표식(<<END:...>>)이 없어 잘림이 감지되었습니다.",
    action: "새 대화를 열고 입력 카드 범위를 줄여 다시 요청합니다.",
    retryable: true,
    recovery: "new-chat-shrink",
  },
  E_JSON_PARSE: {
    cause: "Gemini 응답에서 유효한 JSON 블록을 읽지 못했습니다.",
    action: "1회는 같은 대화에서 JSON만 다시 요청하고, 이후에는 새 대화에서 재요청합니다.",
    retryable: true,
    recovery: "json-reask-then-new-chat",
  },
  E_SCHEMA: {
    cause: "JSON 형식은 읽혔으나 필수 스키마 규칙을 위반했습니다.",
    action: "스키마 위반 항목을 명시하여 재요청합니다.",
    retryable: true,
    recovery: "schema-feedback",
  },
  E_GEMINI_MODE: {
    cause: "Gemini 4 Argon High(최고가) 및 '작업' 모드 활성화 확인에 실패했습니다.",
    action: "모드를 재설정한 뒤 다시 확인하거나 수동으로 4 Argon · 작업 모드를 선택하십시오.",
    retryable: true,
    recovery: "reapply-mode",
  },
  E_GEMINI_QUOTA: {
    cause: "Gemini 웹 세션에서 사용 한도 도달 문구가 감지되었습니다.",
    action: "체크포인트를 저장하고 일시정지합니다. 한도 해제 후 [이어서 실행]을 누르십시오.",
    retryable: false,
    recovery: "pause-checkpoint",
  },
  E_GEMINI_LOGIN: {
    cause: "Gemini 웹 세션 로그인이 필요합니다.",
    action: "[로그인 창 열기]에서 계정에 로그인한 뒤 [이어서 실행]을 누르십시오.",
    retryable: false,
    recovery: "pause-login",
  },
  E_DBPIA_LOGIN: {
    cause: "DBpia AI 로그인이 풀려 있거나 기관 인증이 필요합니다.",
    action: "[로그인 창 열기]에서 DBpia 로그인을 완료한 뒤 [이어서 실행]을 누르십시오.",
    retryable: false,
    recovery: "pause-login",
  },
  E_DOM_DRIFT: {
    cause: "Gemini 또는 DBpia 화면 구조(DOM 셀렉터)가 변경되었습니다.",
    action: "저장된 raw/dom_*.html 스냅샷을 확인하고 selectors.json 셀렉터를 수리하십시오.",
    retryable: false,
    recovery: "save-dom-snapshot",
  },
  E_DBPIA_EMPTY: {
    cause: "DBpia AI 응답 또는 인용 참고문헌이 비어 있습니다.",
    action: "질의를 핵심 키워드 중심으로 단순화하여 1회 재시도합니다.",
    retryable: true,
    recovery: "simplify-query",
  },
  E_CRAFT_EXE: {
    cause: "PdfCraft 또는 WordCraft CLI 실행 파일을 찾지 못했습니다.",
    action: "저장소 bin/ 폴더 내 pdfcraft-cli.exe 및 wordcraft-cli.exe 존재 여부를 확인하십시오.",
    retryable: false,
    recovery: "none",
  },
  E_CRAFT_FAIL: {
    cause: "PdfCraft 또는 WordCraft CLI 실행이 비정상 종료되었습니다.",
    action: "입력 파일 경로 및 문서 잠금 여부를 확인하고 1회 재시도합니다.",
    retryable: true,
    recovery: "retry-once",
  },
  E_DISK: {
    cause: "작업 폴더에 파일을 쓸 수 없거나 디스크 여유 공간이 부족합니다.",
    action: "쓰기 권한이 있는 다른 저장 경로를 지정하십시오.",
    retryable: false,
    recovery: "none",
  },
  E_LOCKED: {
    cause: "다른 v2 파이프라인 작업이 현재 실행 중입니다.",
    action: "실행 중인 작업이 끝날 때까지 기다리거나 중단 후 다시 실행하십시오.",
    retryable: false,
    recovery: "none",
  },
  E_GATE: {
    cause: "단계 품질 게이트(관문) 기준을 충족하지 못했습니다.",
    action: "화면의 게이트 미달 사유를 확인하고 [경고 무시하고 계속] 또는 [중단]을 선택하십시오.",
    retryable: false,
    recovery: "gate-prompt",
  },
  E_ABORTED: {
    cause: "사용자가 작업을 중단했습니다.",
    action: "마지막 완료 체크포인트부터 [이어서 실행]할 수 있습니다.",
    retryable: false,
    recovery: "save-checkpoint",
  },
};

class HarnessError extends Error {
  constructor(code, detail = "", extra = {}) {
    const entry = ERROR_CATALOG[code] || {
      cause: `알 수 없는 하네스 오류(${code})가 발생했습니다.`,
      action: "실행 로그(events.jsonl)를 확인하십시오.",
      retryable: false,
      recovery: "none",
    };
    const detailSuffix = detail ? ` (${detail})` : "";
    super(`[${code}] ${entry.cause}${detailSuffix} 조치: ${entry.action}`);
    this.name = "HarnessError";
    this.code = code;
    this.detail = detail;
    this.causeText = entry.cause + detailSuffix;
    this.actionText = entry.action;
    this.retryable = Boolean(entry.retryable);
    this.recovery = entry.recovery;
    Object.assign(this, extra);
  }

  toUserMessage() {
    return `${this.causeText}\n조치: ${this.actionText}`;
  }
}

function getErrorSpec(code) {
  return ERROR_CATALOG[code] || null;
}

function isRetryableError(err, allowedCodes = null) {
  const code = err && err.code ? err.code : null;
  if (!code) return false;
  if (Array.isArray(allowedCodes) && allowedCodes.length > 0) {
    return allowedCodes.includes(code);
  }
  const spec = ERROR_CATALOG[code];
  return Boolean(spec && spec.retryable);
}

module.exports = {
  ERROR_CATALOG,
  HarnessError,
  getErrorSpec,
  isRetryableError,
};
