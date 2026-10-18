"use strict";

const { createGeminiDriver } = require("./gemini_driver.js");
const { createDbpiaDriver } = require("./dbpia_driver.js");

/**
 * Creates a deterministic fake Gemini driver and fake DBpia/Craft drivers
 * for offline unit tests and CI runs (spec 4.8).
 */
function createFakeDrivers(options = {}) {
  const geminiQueue = Array.isArray(options.geminiReplies)
    ? [...options.geminiReplies]
    : [];
  const dbpiaQueue = Array.isArray(options.dbpiaReplies)
    ? [...options.dbpiaReplies]
    : [];
  const calls = {
    gemini: [],
    resetChatCount: 0,
    dbpia: [],
    craft: [],
  };

  let modeVerified = options.initialModeVerified !== false;

  const geminiDriver = createGeminiDriver({
    skipRateLimit: true,
    config: options.config,
    verifyMode: async () => modeVerified,
    applyMode: async () => {
      if (options.failModeForever) {
        modeVerified = false;
      } else {
        modeVerified = true;
      }
    },
    resetGeminiChat: async () => {
      calls.resetChatCount += 1;
    },
    askGeminiWeb: async (prompt) => {
      calls.gemini.push({ prompt, resetChatCount: calls.resetChatCount });
      if (geminiQueue.length === 0) {
        if (typeof options.defaultGeminiHandler === "function") {
          return await options.defaultGeminiHandler(prompt, calls.gemini.length);
        }
        return "";
      }
      const next = geminiQueue.shift();
      if (typeof next === "function") {
        return await next(prompt, calls.gemini.length);
      }
      if (next instanceof Error) {
        throw next;
      }
      return String(next ?? "");
    },
  });

  const dbpiaDriver = createDbpiaDriver({
    getDbpiaLoginStatus: async () => ({
      loggedIn: options.dbpiaLoggedIn !== false,
    }),
    askDbpiaAi: async (query) => {
      calls.dbpia.push(query);
      if (dbpiaQueue.length > 0) {
        const item = dbpiaQueue.shift();
        if (typeof item === "function") return await item(query);
        if (item instanceof Error) throw item;
        return item;
      }
      return {
        query,
        answerText:
          options.defaultDbpiaAnswer ||
          "중부지역 원삼국시대 凸자형 주거지는 부뚜막과 구들 시설을 갖춘 취락 유구로 편년된다.",
        refs: options.defaultDbpiaRefs || [
          {
            nodeId: "NODE00000001",
            author: "홍길동",
            year: "2015",
            title: "중부지역 원삼국시대 주거지 편년 연구",
            journal: "가상고고학보",
            pages: "1~30",
            abstract: "3호 주거지와 같은 凸자형 주거지는 원삼국시대 중기 단계로 편년된다.",
          },
          {
            nodeId: "NODE00000002",
            author: "이유물",
            year: "2018",
            title: "원삼국시대 부뚜막 및 배연시설 구조 고찰",
            journal: "가상유적연구",
            pages: "45~72",
            abstract: "북벽 부뚜막과 동벽 배연시설은 중부지역 원삼국시대 취락의 전형적인 난방·취사 양식이다.",
          },
          {
            nodeId: "NODE00000003",
            author: "박층위",
            year: "2021",
            title: "중부권 원삼국시대 경질무문토기 공반 관계 분석",
            journal: "한국가상고고학",
            pages: "101~134",
            abstract: "경질무문토기 심발형 토기와 타날문 단경호는 원삼국시대 중기 주거지에서 공반된다.",
          },
        ],
      };
    },
  });

  const craftDriver = {
    checkExecutables: () => ({ pdfExists: true, wordExists: true }),
    pdfInfo: (filePath) => {
      calls.craft.push({ op: "info", filePath });
      return {
        raw: "Pages: 6",
        pageCount: options.pageCount || 6,
      };
    },
    pdfPageText: (filePath, pageNumber) => {
      calls.craft.push({ op: "text", filePath, pageNumber });
      if (Array.isArray(options.pagesText) && pageNumber >= 1) {
        return options.pagesText[pageNumber - 1] || "";
      }
      return options.defaultPageText || "";
    },
    exportDocx: async (opts) => {
      calls.craft.push({ op: "exportDocx", opts });
      return {
        ok: true,
        docxPath: opts.docxPath || "virtual_output.docx",
        pdfPath: opts.pdfPath || "virtual_output.pdf",
      };
    },
  };

  return {
    geminiDriver,
    dbpiaDriver,
    craftDriver,
    calls,
  };
}

module.exports = {
  createFakeDrivers,
};
