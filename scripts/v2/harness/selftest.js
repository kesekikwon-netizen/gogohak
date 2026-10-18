"use strict";

const fs = require("node:fs");
const path = require("node:path");
const os = require("node:os");
const { execFileSync } = require("node:child_process");
const { probe } = require("./dom_probe.js");
const checkpoint = require("./checkpoint.js");
const { DEFAULT_PDFCRAFT_CLI, DEFAULT_WORDCRAFT_CLI } = require("./craft_driver.js");

async function runSelftest(deps = {}) {
  const items = [];
  const outputDir =
    deps.outputDir || path.join(os.homedir(), "archaeology_reports");
  const literatureMode = deps.literatureMode || "dbpia";

  function pushItem(id, title, ok, code, detail, action, warningOnly = false) {
    items.push({
      id,
      title,
      ok: Boolean(ok),
      warningOnly: Boolean(warningOnly),
      code: ok ? null : code,
      detail: detail || "",
      action: ok ? "" : action,
    });
  }

  // T1: Node 버전 >= 18
  const nodeMajor = Number(String(process.versions.node || "0").split(".")[0]);
  pushItem(
    "T1",
    "Node.js 버전 확인 (>= 18)",
    nodeMajor >= 18,
    "E_ENV_NODE",
    `Node v${process.versions.node}`,
    "Node 18 이상 설치"
  );

  // T2: CDP 포트
  let cdpPort = null;
  try {
    cdpPort =
      typeof deps.getCdpPort === "function" ? await deps.getCdpPort() : null;
    pushItem(
      "T2",
      "Antigravity CDP 포트 연결",
      Boolean(cdpPort),
      "E_CDP_PORT",
      cdpPort ? `CDP 포트 ${cdpPort}` : "CDP 포트 미감지",
      "Antigravity 실행"
    );
  } catch (err) {
    pushItem(
      "T2",
      "Antigravity CDP 포트 연결",
      false,
      "E_CDP_PORT",
      err.message || "CDP 포트 탐색 실패",
      "Antigravity 실행"
    );
  }

  // T3 ~ T8: 웹뷰·로그인·모드·셀렉터 점검
  let wvTarget = null;
  let dbpiaTarget = null;
  if (cdpPort && typeof deps.ensureEmbeddedGeminiWebview === "function") {
    try {
      const ensured = await deps.ensureEmbeddedGeminiWebview();
      wvTarget = ensured?.wvTarget || null;
      dbpiaTarget = ensured?.dbpiaTarget || null;
    } catch {
      // handled below
    }
  }

  pushItem(
    "T3",
    "Gemini 내장 웹뷰 대상 확인",
    Boolean(wvTarget && wvTarget.webSocketDebuggerUrl),
    "E_GEMINI_WEBVIEW",
    wvTarget ? wvTarget.url : "Gemini 웹뷰 없음",
    "Gemini 패널 열기"
  );

  if (wvTarget && typeof deps.withCdpSession === "function") {
    try {
      await deps.withCdpSession(wvTarget.webSocketDebuggerUrl, async (send) => {
        for (let i = 0; i < 20; i++) {
          const ready = await send("Runtime.evaluate", {
            expression: `location.href.includes('gemini.google.com') && Boolean(document.querySelector('.ql-editor, [contenteditable="true"]'))`,
            returnByValue: true,
          });
          if (ready.result?.value) break;
          await new Promise((r) => setTimeout(r, 400));
        }
        // T5: 4 Argon High + 작업 모드 설정 후 다시 읽기
        let modeRes = { locked: false, workModeActive: false };
        if (typeof deps.ensureArgonHighAndWorkMode === "function") {
          modeRes = await deps.ensureArgonHighAndWorkMode(send);
        }
        const verifyRead = await send("Runtime.evaluate", {
          expression: `(() => {
            const signInBtn = Array.from(document.querySelectorAll('button, a')).find(
              el => (el.textContent || '').trim() === '로그인' || (el.textContent || '').trim() === 'Sign in'
            );
            const editor = document.querySelector('.ql-editor, [contenteditable="true"]');
            const modeBtn = document.querySelector('button[aria-label*="모드 선택"], button[aria-label*="mode"], bard-mode-switcher button');
            const modeText = modeBtn ? (modeBtn.textContent || '').replace(/\\s+/g, ' ').trim() : '';
            const isWork = Boolean(
              document.querySelector('gem-segmented-button[data-value="40"].gem-segmented-button-selected, gem-segmented-button[data-value="40"][aria-checked="true"], .is-remy-mode, .remy-plan-pill')
            );
            return {
              loggedIn: Boolean(editor) && !Boolean(signInBtn),
              modeText,
              isWork
            };
          })()`,
          returnByValue: true,
        });
        const vr = verifyRead?.result?.value || {};
        pushItem(
          "T4",
          "Gemini 웹 세션 로그인 상태",
          Boolean(vr.loggedIn),
          "E_GEMINI_LOGIN",
          vr.loggedIn ? "로그인 및 입력창 확인됨" : "로그인 버튼 감지 또는 입력창 없음",
          "Gemini 로그인"
        );

        const modeOk = Boolean(vr.isWork || modeRes?.workModeActive);
        pushItem(
          "T5",
          "Gemini 모델(4 Argon High) · 작업 모드 고정 확인",
          modeOk,
          "E_GEMINI_MODE",
          `${vr.modeText || "Argon"} / 작업모드=${modeOk}`,
          "수동으로 4 Argon·작업 선택"
        );

        // T6: Gemini 셀렉터 탐침
        const probeGem = await probe(
          send,
          ["gemini.editor", "gemini.workModeButton"],
          { jobDir: outputDir }
        );
        pushItem(
          "T6",
          "Gemini 화면 셀렉터 구조 점검",
          probeGem.ok,
          "E_DOM_DRIFT",
          probeGem.ok
            ? "필수 셀렉터 정상"
            : `누락: ${probeGem.missing.join(", ")}`,
          "개발 수리 필요"
        );
      });
    } catch (err) {
      pushItem("T4", "Gemini 웹 세션 로그인 상태", false, "E_GEMINI_LOGIN", err.message, "Gemini 로그인");
      pushItem("T5", "Gemini 모델 · 작업 모드 고정 확인", false, "E_GEMINI_MODE", err.message, "수동으로 4 Argon·작업 선택");
      pushItem("T6", "Gemini 화면 셀렉터 구조 점검", false, "E_DOM_DRIFT", err.message, "개발 수리 필요");
    }
  } else {
    pushItem("T4", "Gemini 웹 세션 로그인 상태", false, "E_GEMINI_LOGIN", "웹뷰 미연결", "Gemini 로그인");
    pushItem("T5", "Gemini 모델 · 작업 모드 고정 확인", false, "E_GEMINI_MODE", "웹뷰 미연결", "수동으로 4 Argon·작업 선택");
    pushItem("T6", "Gemini 화면 셀렉터 구조 점검", false, "E_DOM_DRIFT", "웹뷰 미연결", "개발 수리 필요");
  }

  // T7 & T8: DBpia 로그인 및 셀렉터 점검 (manual 모드에서는 경고로만 처리)
  const dbpiaWarnOnly = literatureMode === "manual";
  if (cdpPort && typeof deps.getDbpiaLoginStatus === "function") {
    try {
      const dbStatus = await deps.getDbpiaLoginStatus();
      pushItem(
        "T7",
        "DBpia AI 로그인 상태",
        Boolean(dbStatus && dbStatus.loggedIn),
        "E_DBPIA_LOGIN",
        dbStatus?.loggedIn
          ? `로그인 확인 (${dbStatus.aiCountText || "정상"})`
          : `미로그인 (${dbStatus?.tooltipText || dbStatus?.reason || "무료 검색 상태"})`,
        "DBpia 로그인",
        dbpiaWarnOnly
      );
    } catch (err) {
      pushItem("T7", "DBpia AI 로그인 상태", false, "E_DBPIA_LOGIN", err.message, "DBpia 로그인", dbpiaWarnOnly);
    }
  } else {
    pushItem("T7", "DBpia AI 로그인 상태", false, "E_DBPIA_LOGIN", "CDP 미연결", "DBpia 로그인", dbpiaWarnOnly);
  }

  if (dbpiaTarget && typeof deps.withCdpSession === "function") {
    try {
      await deps.withCdpSession(dbpiaTarget.webSocketDebuggerUrl, async (send) => {
        const probeDb = await probe(send, ["dbpia.searchInput"], {
          jobDir: outputDir,
        });
        pushItem(
          "T8",
          "DBpia 화면 셀렉터 구조 점검",
          probeDb.ok,
          "E_DOM_DRIFT",
          probeDb.ok ? "DBpia 검색창 셀렉터 정상" : `누락: ${probeDb.missing.join(", ")}`,
          "개발 수리 필요",
          dbpiaWarnOnly
        );
      });
    } catch (err) {
      pushItem("T8", "DBpia 화면 셀렉터 구조 점검", false, "E_DOM_DRIFT", err.message, "개발 수리 필요", dbpiaWarnOnly);
    }
  } else {
    pushItem("T8", "DBpia 화면 셀렉터 구조 점검", false, "E_DOM_DRIFT", "DBpia 웹뷰 미연결", "개발 수리 필요", dbpiaWarnOnly);
  }

  // T9: PdfCraft CLI
  const pdfCli = deps.pdfCliPath || DEFAULT_PDFCRAFT_CLI;
  let pdfOk = false;
  let pdfDetail = "";
  if (fs.existsSync(pdfCli)) {
    try {
      const out = execFileSync(pdfCli, ["--help"], {
        encoding: "utf8",
        timeout: 10000,
        windowsHide: true,
      });
      pdfOk = true;
      pdfDetail = String(out || "").split(/\r?\n/)[0] || "pdfcraft-cli 실행 확인";
    } catch (err) {
      pdfDetail = err.message;
    }
  } else {
    pdfDetail = `파일 없음: ${pdfCli}`;
  }
  pushItem("T9", "PdfCraft CLI 실행 파일 확인", pdfOk, "E_CRAFT_EXE", pdfDetail, "저장소 bin/ 확인");

  // T10: WordCraft CLI
  const wordCli = deps.wordCliPath || DEFAULT_WORDCRAFT_CLI;
  let wordOk = false;
  let wordDetail = "";
  if (fs.existsSync(wordCli)) {
    try {
      const out = execFileSync(wordCli, ["--help"], {
        encoding: "utf8",
        timeout: 10000,
        windowsHide: true,
      });
      wordOk = true;
      wordDetail = String(out || "").split(/\r?\n/)[0] || "wordcraft-cli 실행 확인";
    } catch (err) {
      wordDetail = err.message;
    }
  } else {
    wordDetail = `파일 없음: ${wordCli}`;
  }
  pushItem("T10", "WordCraft CLI 실행 파일 확인", wordOk, "E_CRAFT_EXE", wordDetail, "저장소 bin/ 확인");

  // T11: 쓰기 권한·여유 공간
  let diskOk = false;
  let diskDetail = "";
  try {
    fs.mkdirSync(outputDir, { recursive: true });
    const probeFile = path.join(outputDir, `.selftest_${process.pid}_${Date.now()}.tmp`);
    fs.writeFileSync(probeFile, "ok", "utf8");
    fs.unlinkSync(probeFile);
    diskOk = true;
    diskDetail = `쓰기 확인 완료 (${outputDir})`;
  } catch (err) {
    diskDetail = err.message;
  }
  pushItem("T11", "작업 폴더 쓰기 권한 확인", diskOk, "E_DISK", diskDetail, "다른 저장 위치 지정");

  // T12: 동시 실행 잠금 확인
  const lockPath = deps.lockPath || checkpoint.defaultLockPath(outputDir);
  let lockOk = true;
  let lockDetail = "활성 잠금 없음";
  if (fs.existsSync(lockPath)) {
    try {
      const lockObj = JSON.parse(fs.readFileSync(lockPath, "utf8"));
      if (lockObj && lockObj.pid && checkpoint.isPidAlive(lockObj.pid)) {
        lockOk = false;
        lockDetail = `실행 중인 작업ID=${lockObj.jobId}, PID=${lockObj.pid}`;
      }
    } catch {
      // stale or malformed lock is overwritable
    }
  }
  pushItem("T12", "동시 실행 잠금 확인", lockOk, "E_LOCKED", lockDetail, "실행 중인 작업 종료 대기");

  const blockingFailures = items.filter((it) => !it.ok && !it.warningOnly);
  const result = {
    ok: blockingFailures.length === 0,
    checkedAt: new Date().toISOString(),
    items,
    failedCodes: blockingFailures.map((it) => it.code),
  };

  if (deps.saveDir) {
    try {
      fs.mkdirSync(deps.saveDir, { recursive: true });
      fs.writeFileSync(
        path.join(deps.saveDir, "selftest.json"),
        JSON.stringify(result, null, 2),
        "utf8"
      );
    } catch {
      // ignore write failure when T11 failed
    }
  }

  return result;
}

module.exports = {
  runSelftest,
};
