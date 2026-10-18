"use strict";

const test = require("node:test");
const assert = require("node:assert/strict");
const fs = require("node:fs");
const path = require("node:path");
const os = require("node:os");

const { HarnessError } = require("../scripts/v2/harness/errors.js");
const checkpoint = require("../scripts/v2/harness/checkpoint.js");
const { runSteps } = require("../scripts/v2/harness/supervisor.js");
const { probe, selectors } = require("../scripts/v2/harness/dom_probe.js");
const { createFakeDrivers } = require("../scripts/v2/harness/fake_drivers.js");
const { computeMetrics } = require("../scripts/v2/harness/events.js");

function makeTempJobDir(prefix = "v6-s2-test-") {
  return fs.mkdtempSync(path.join(os.tmpdir(), prefix));
}

test("1. 감독기(supervisor): 시간 초과 → 재시도 → 성공", async () => {
  const jobDir = makeTempJobDir();
  let calls = 0;

  const steps = [
    {
      id: "P1.extract",
      timeoutMs: 40,
      retry: {
        max: 2,
        backoffMs: [5, 10],
        on: ["E_GEMINI_TIMEOUT", "E_TRUNCATED"],
      },
      onExhausted: "needs-human",
      saveAs: "facts.json",
      validate: (val) => ({ ok: Array.isArray(val) && val.length === 1 }),
      run: async () => {
        calls += 1;
        if (calls === 1) {
          await new Promise((r) => setTimeout(r, 120));
        }
        return [{ id: "F1", subject: "1호 주거지" }];
      },
    },
  ];

  const out = await runSteps({ jobId: "job-timeout-retry", jobDir }, steps);
  assert.equal(out.status, "done");
  assert.equal(calls, 2, "1회 타임아웃 후 2회차에 성공해야 함");
  const metrics = computeMetrics(jobDir);
  assert.equal(metrics.totalRetries, 1);
  assert.equal(metrics.errorCounts.E_GEMINI_TIMEOUT, 1);
  fs.rmSync(jobDir, { recursive: true, force: true });
});

test("2. 감독기(supervisor): 재시도 소진 → needs-human 및 비재시도 오류 즉시 중단", async () => {
  const jobDir1 = makeTempJobDir();
  let retryCalls = 0;

  const resExhausted = await runSteps(
    { jobId: "job-exhausted", jobDir: jobDir1 },
    [
      {
        id: "P4.S1.r0",
        retry: { max: 2, backoffMs: [5, 5], on: ["E_TRUNCATED"] },
        onExhausted: "needs-human",
        run: async () => {
          retryCalls += 1;
          throw new HarnessError("E_TRUNCATED", "끝 표식 누락");
        },
      },
    ]
  );

  assert.equal(resExhausted.status, "needs-human");
  assert.equal(retryCalls, 3, "초기 1회 + 재시도 2회 = 총 3회 실행되어야 함");

  const jobDir2 = makeTempJobDir();
  let nonRetryCalls = 0;
  const resImmediate = await runSteps(
    { jobId: "job-nonretry", jobDir: jobDir2 },
    [
      {
        id: "P1.drift",
        retry: { max: 2, backoffMs: [5, 5], on: ["E_TRUNCATED"] },
        onExhausted: "needs-human",
        run: async () => {
          nonRetryCalls += 1;
          throw new HarnessError("E_DOM_DRIFT", "셀렉터 없음");
        },
      },
    ]
  );

  assert.equal(resImmediate.status, "needs-human");
  assert.equal(nonRetryCalls, 1, "재시도 대상이 아닌 오류는 재시도 없이 즉시 멈춰야 함");

  fs.rmSync(jobDir1, { recursive: true, force: true });
  fs.rmSync(jobDir2, { recursive: true, force: true });
});

test("3. 감독기(supervisor): 중단 후 재개 시 done 단위 건너뛰기 & 깨진 저장 파일은 재실행", async () => {
  const jobDir = makeTempJobDir();
  let step1Runs = 0;
  let step2Runs = 0;

  const makeSteps = (shouldFailStep2) => [
    {
      id: "P1.facts",
      saveAs: "facts.json",
      validate: (arr) => ({ ok: Array.isArray(arr) && arr.length > 0 }),
      run: async () => {
        step1Runs += 1;
        return [{ id: "F1" }];
      },
    },
    {
      id: "P3.outline",
      saveAs: "outline.json",
      validate: (obj) => ({ ok: Boolean(obj && obj.title) }),
      run: async () => {
        step2Runs += 1;
        if (shouldFailStep2) {
          throw new HarnessError("E_GEMINI_QUOTA", "쿼터 초과");
        }
        return { title: "Ⅳ. 고찰" };
      },
    },
  ];

  // 1차 실행: P1 완료, P3에서 멈춤
  const run1 = await runSteps({ jobId: "job-resume", jobDir }, makeSteps(true));
  assert.equal(run1.status, "needs-human");
  assert.equal(step1Runs, 1);
  assert.equal(step2Runs, 1);

  // 2차 재개: P1은 건너뛰고 P3만 실행
  const run2 = await runSteps({ jobId: "job-resume", jobDir }, makeSteps(false));
  assert.equal(run2.status, "done");
  assert.equal(step1Runs, 1, "이미 done이고 파일이 정상인 P1은 다시 실행하지 않아야 함");
  assert.equal(step2Runs, 2);

  // 3차: P1의 저장 파일(facts.json)이 깨졌을 때는 done이어도 다시 실행되어야 함
  fs.writeFileSync(path.join(jobDir, "facts.json"), "{broken-json", "utf8");
  const run3 = await runSteps({ jobId: "job-resume", jobDir }, makeSteps(false));
  assert.equal(run3.status, "done");
  assert.equal(step1Runs, 2, "저장 파일이 손상되었으면 done이어도 다시 실행해야 함");

  fs.rmSync(jobDir, { recursive: true, force: true });
});

test("4. Gemini 드라이버: 끝 표식 누락(E_TRUNCATED), 빈 응답(E_EMPTY_REPLY), 깨진 JSON 같은 대화 1회 재요청", async () => {
  const jobDir = makeTempJobDir();

  // 4-1. 끝 표식 없는 응답 -> E_TRUNCATED
  const fakeTrunc = createFakeDrivers({
    geminiReplies: ["본문만 있고 끝 표식이 없는 응답입니다."],
  });
  await assert.rejects(
    () =>
      fakeTrunc.geminiDriver.ask({
        jobDir,
        jobStep: "P4.S1.r0",
        prompt: "절 집필 요청",
      }),
    (err) => err.code === "E_TRUNCATED"
  );

  // 4-2. 빈 응답 -> E_EMPTY_REPLY
  const fakeEmpty = createFakeDrivers({
    geminiReplies: ["   "],
  });
  await assert.rejects(
    () =>
      fakeEmpty.geminiDriver.ask({
        jobDir,
        jobStep: "P4.S1.r0",
        prompt: "절 집필 요청",
      }),
    (err) => err.code === "E_EMPTY_REPLY"
  );

  // 4-3. 깨진 JSON -> 같은 대화(resetChatCount 증가 없음)에서 1회 JSON 재요청 후 성공
  const fakeJsonReask = createFakeDrivers({
    geminiReplies: [
      "이 응답은 JSON이 깨져 있습니다 ```json {broken ```\n<<END:P1.ch3>>",
      '```json\n{"cards":[{"id":"F1"}]}\n```\n<<END:P1.ch3>>',
    ],
  });
  const jsonRes = await fakeJsonReask.geminiDriver.ask({
    jobDir,
    jobStep: "P1.ch3",
    prompt: "JSON 추출",
    expectJson: true,
    newChat: true,
  });
  assert.deepEqual(jsonRes.json, { cards: [{ id: "F1" }] });
  assert.equal(fakeJsonReask.calls.gemini.length, 2, "같은 대화 재요청 포함 2회 호출");
  assert.equal(
    fakeJsonReask.calls.resetChatCount,
    1,
    "첫 호출 시 1회 새 대화 후 JSON 재요청은 같은 대화에서 수행되어야 함"
  );

  fs.rmSync(jobDir, { recursive: true, force: true });
});

test("5. 잠금 파일(checkpoint.acquireLock): 동시 실행 거부 및 죽은 PID 잠금 해제", () => {
  const jobDir = makeTempJobDir();
  const lockPath = path.join(jobDir, ".lock");

  // 살아 있는 다른 PID(예: 99999, isPidAlive=true 모의)가 잠금을 잡고 있으면 E_LOCKED
  fs.writeFileSync(
    lockPath,
    JSON.stringify({ jobId: "running-job", pid: 99999, startedAt: new Date().toISOString() }),
    "utf8"
  );

  assert.throws(
    () =>
      checkpoint.acquireLock(lockPath, "new-job", {
        pid: 11111,
        isPidAlive: (pid) => pid === 99999,
      }),
    (err) => err.code === "E_LOCKED"
  );

  // 죽은 PID(isPidAlive=false)의 잠금은 해제하고 새 잠금을 획득해야 함
  const acquired = checkpoint.acquireLock(lockPath, "new-job", {
    pid: 11111,
    isPidAlive: () => false,
  });
  assert.equal(acquired.jobId, "new-job");
  assert.equal(acquired.pid, 11111);

  checkpoint.releaseLock(lockPath, "new-job");
  assert.equal(fs.existsSync(lockPath), false);
  fs.rmSync(jobDir, { recursive: true, force: true });
});

test("6. DOM 탐침(dom_probe.js) 및 selectors.json 매핑 검증", async () => {
  const jobDir = makeTempJobDir();
  assert.ok(selectors._sourceMap["gemini.editor"], "gemini.editor 출처 줄번호가 있어야 함");
  assert.ok(selectors._sourceMap["dbpia.searchInput"], "dbpia.searchInput 출처 줄번호가 있어야 함");

  const mockEval = async () => ({
    ok: false,
    missing: ["gemini.workModeButton"],
    details: {
      "gemini.editor": { exists: true, visible: true, count: 1 },
      "gemini.workModeButton": { exists: false, visible: false, count: 0 },
    },
    domSnippet: "<html><body><div>Changed DOM Structure</div></body></html>",
  });
  mockEval.isMockEvaluator = true;

  await assert.rejects(
    () =>
      probe(mockEval, ["gemini.editor", "gemini.workModeButton"], {
        jobDir,
        throwOnMissing: true,
      }),
    (err) => {
      assert.equal(err.code, "E_DOM_DRIFT");
      assert.ok(err.snapshotPath && fs.existsSync(err.snapshotPath), "DOM 스냅샷 파일이 저장되어야 함");
      return true;
    }
  );

  fs.rmSync(jobDir, { recursive: true, force: true });
});
