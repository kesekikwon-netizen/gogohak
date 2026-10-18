"use strict";

const fs = require("node:fs");
const path = require("node:path");
const defaultConfig = require("../config.default.json");
const selectors = require("./selectors.json");
const { HarnessError } = require("./errors.js");

function sleep(ms) {
  return new Promise((resolve) => setTimeout(resolve, ms));
}

function makeEndMarker(stepId) {
  return `<<END:${stepId}>>`;
}

function appendEndMarkerInstruction(prompt, stepId) {
  const marker = makeEndMarker(stepId);
  const cleanPrompt = String(prompt || "").trim();
  if (cleanPrompt.includes(marker)) return cleanPrompt;
  return `${cleanPrompt}\n\n[출력 완료 표식 규칙]\n반드시 응답의 맨 마지막 줄에 다음 완료 표식을 글자 그대로 단독 줄로 기재하십시오(중간 생략·절단 금지):\n${marker}`;
}

function checkAndStripEndMarker(replyText, stepId) {
  const marker = makeEndMarker(stepId);
  const raw = String(replyText || "");
  const idx = raw.lastIndexOf(marker);
  if (idx === -1) {
    return {
      hasMarker: false,
      text: raw.trim(),
    };
  }
  const stripped = (raw.slice(0, idx) + raw.slice(idx + marker.length)).trim();
  return {
    hasMarker: true,
    text: stripped,
  };
}

function trimPromptToBudget(prompt, maxChars) {
  const limit = Number(maxChars || defaultConfig.gemini.maxPromptChars || 12000);
  const str = String(prompt || "");
  if (str.length <= limit) {
    return { prompt: str, trimmed: false, originalLength: str.length };
  }
  return {
    prompt: str.slice(0, limit),
    trimmed: true,
    originalLength: str.length,
  };
}

function saveRawReply(jobDir, stepId, attempt, text) {
  if (!jobDir) return null;
  const rawDir = path.join(jobDir, "raw");
  fs.mkdirSync(rawDir, { recursive: true });
  const safeId = String(stepId || "step").replace(/[^A-Za-z0-9._-]/g, "_");
  const rawPath = path.join(rawDir, `${safeId}.${attempt || 1}.txt`);
  fs.writeFileSync(rawPath, String(text ?? ""), "utf8");
  return rawPath;
}

function detectQuotaOrLoginFailure(replyText) {
  const text = String(replyText || "");
  const quotaPatterns = selectors.gemini.quotaTextPatterns || [];
  for (const pat of quotaPatterns) {
    if (pat && text.includes(pat)) {
      return new HarnessError("E_GEMINI_QUOTA", `감지 문구: "${pat}"`);
    }
  }
  return null;
}

function createGeminiDriver(deps = {}) {
  const cfg = deps.config || defaultConfig;
  const parseJson =
    deps.parseJsonFromGeminiReply ||
    ((rawText) => {
      if (!rawText) return null;
      const fenced = /```(?:json)?\s*([\s\S]*?)```/gi;
      let m;
      let lastCandidate = null;
      while ((m = fenced.exec(rawText)) !== null) {
        lastCandidate = m[1].trim();
      }
      const target = lastCandidate || rawText;
      const b0 = target.indexOf("{");
      const b1 = target.lastIndexOf("}");
      const a0 = target.indexOf("[");
      const a1 = target.lastIndexOf("]");
      const useArray = a0 !== -1 && (b0 === -1 || a0 < b0) && a1 > a0;
      const slice = useArray
        ? target.slice(a0, a1 + 1)
        : b0 !== -1 && b1 > b0
          ? target.slice(b0, b1 + 1)
          : "";
      if (!slice) return null;
      try {
        return JSON.parse(slice);
      } catch {
        return null;
      }
    });

  let lastCallAt = 0;

  async function verifyAndLockMode() {
    if (typeof deps.verifyMode === "function") {
      const ok = await deps.verifyMode();
      if (!ok) {
        if (typeof deps.applyMode === "function") {
          await deps.applyMode();
        }
        const okRetry = await deps.verifyMode();
        if (!okRetry) {
          throw new HarnessError("E_GEMINI_MODE", "4 Argon High + 작업 모드 재확인 실패");
        }
      }
      return true;
    }
    return true;
  }

  async function ask({
    jobDir = null,
    jobStep = "step",
    attempt = 1,
    prompt = "",
    expectJson = false,
    validateSchema = null,
    newChat = false,
    minIntervalMs = cfg.gemini.minIntervalMs,
    maxPromptChars = cfg.gemini.maxPromptChars,
  } = {}) {
    const now = Date.now();
    const waitGap = Number(minIntervalMs || 0) - (now - lastCallAt);
    if (waitGap > 0 && !deps.skipRateLimit) {
      await sleep(waitGap);
    }

    if (newChat && typeof deps.resetGeminiChat === "function") {
      await deps.resetGeminiChat();
    }

    await verifyAndLockMode();

    const budgeted = trimPromptToBudget(prompt, maxPromptChars);
    const fullPrompt = appendEndMarkerInstruction(budgeted.prompt, jobStep);

    lastCallAt = Date.now();
    const rawReply = await deps.askGeminiWeb(fullPrompt, { newChat: false });
    const replyText =
      typeof rawReply === "string" ? rawReply : String(rawReply?.text || "");

    const rawPath = saveRawReply(jobDir, jobStep, attempt, replyText);

    if (
      !replyText ||
      !replyText.trim() ||
      replyText.includes("(No response text captured within timeout)")
    ) {
      throw new HarnessError("E_EMPTY_REPLY", "응답 텍스트가 비어 있음", { rawPath });
    }

    const quotaErr = detectQuotaOrLoginFailure(replyText);
    if (quotaErr) {
      quotaErr.rawPath = rawPath;
      throw quotaErr;
    }

    const markerCheck = checkAndStripEndMarker(replyText, jobStep);
    if (!markerCheck.hasMarker) {
      throw new HarnessError(
        "E_TRUNCATED",
        `응답 말미에 ${makeEndMarker(jobStep)} 표식이 없음`,
        { rawPath, partialText: markerCheck.text }
      );
    }

    const cleanText = markerCheck.text;

    if (expectJson) {
      let parsed = parseJson(cleanText);
      if (parsed === null && typeof deps.askGeminiWeb === "function") {
        // 4.4 E_JSON_PARSE: 1회는 같은 대화에서 "JSON만 다시" 요청
        const reaskPrompt = appendEndMarkerInstruction(
          `방금 응답을 코드블록(\`\`\`json ... \`\`\`) 안의 유효한 JSON 형식으로만 다시 출력하십시오. 설명 문장은 넣지 마십시오.`,
          jobStep
        );
        const secondRaw = await deps.askGeminiWeb(reaskPrompt, { newChat: false });
        const secondText =
          typeof secondRaw === "string" ? secondRaw : String(secondRaw?.text || "");
        saveRawReply(jobDir, `${jobStep}.json_reask`, attempt, secondText);
        const secondMarker = checkAndStripEndMarker(secondText, jobStep);
        if (secondMarker.hasMarker) {
          parsed = parseJson(secondMarker.text);
        }
      }

      if (parsed === null) {
        throw new HarnessError("E_JSON_PARSE", "JSON 블록 파싱 실패", { rawPath });
      }

      if (typeof validateSchema === "function") {
        const check = validateSchema(parsed);
        if (!check || !check.ok) {
          throw new HarnessError(
            "E_SCHEMA",
            Array.isArray(check?.errors) ? check.errors.join("; ") : "스키마 검증 실패",
            { rawPath, parsed }
          );
        }
      }

      return {
        text: cleanText,
        json: parsed,
        rawPath,
        trimmedPrompt: budgeted.trimmed,
      };
    }

    return {
      text: cleanText,
      rawPath,
      trimmedPrompt: budgeted.trimmed,
    };
  }

  return {
    ask,
    verifyAndLockMode,
    makeEndMarker,
    appendEndMarkerInstruction,
    checkAndStripEndMarker,
  };
}

module.exports = {
  createGeminiDriver,
  makeEndMarker,
  appendEndMarkerInstruction,
  checkAndStripEndMarker,
  trimPromptToBudget,
  saveRawReply,
};
