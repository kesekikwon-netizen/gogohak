"use strict";

const { HarnessError } = require("./errors.js");

function simplifyKoreanQuery(q) {
  const clean = String(q || "")
    .replace(/[?？!！,，.。()[\]「」『』'"]/g, " ")
    .replace(
      /\b(?:어떠한가|무엇인가|알려\s*줘|비교해\s*줘|검토|분석|연구|논문|대한|관한|중심으로|바탕으로)\b/g,
      " "
    )
    .replace(/\s+/g, " ")
    .trim();
  const tokens = clean.split(" ").filter((t) => t.length >= 2);
  return tokens.slice(0, 5).join(" ") || String(q || "").trim();
}

function createDbpiaDriver(deps = {}) {
  async function query(rawQuery, options = {}) {
    const q = String(rawQuery || "").trim();
    if (!q) {
      throw new HarnessError("E_DBPIA_EMPTY", "빈 DBpia 질의");
    }

    if (typeof deps.getDbpiaLoginStatus === "function") {
      const loginState = await deps.getDbpiaLoginStatus();
      if (loginState && loginState.loggedIn === false && !options.allowGuest) {
        throw new HarnessError(
          "E_DBPIA_LOGIN",
          loginState.reason || "DBpia 로그인 상태가 아님"
        );
      }
    }

    const targetQuery = options.simplified ? simplifyKoreanQuery(q) : q;
    const res = await deps.askDbpiaAi(targetQuery);
    const hasAnswer = Boolean(res && String(res.answerText || "").trim().length > 0);
    const hasRefs = Boolean(res && Array.isArray(res.refs) && res.refs.length > 0);

    if (!hasAnswer && !hasRefs) {
      throw new HarnessError(
        "E_DBPIA_EMPTY",
        `질의 "${targetQuery}"에 대한 DBpia 답변 및 참고문헌이 비어 있음`
      );
    }

    return {
      query: targetQuery,
      answerText: String(res.answerText || "").trim(),
      refs: Array.isArray(res.refs) ? res.refs : [],
      followUps: Array.isArray(res.followUps) ? res.followUps : [],
      formatted: res.formatted || "",
    };
  }

  return {
    query,
    simplifyKoreanQuery,
  };
}

module.exports = {
  createDbpiaDriver,
  simplifyKoreanQuery,
};
