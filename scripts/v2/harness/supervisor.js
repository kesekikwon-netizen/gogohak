"use strict";

const fs = require("node:fs");
const path = require("node:path");
const { HarnessError, isRetryableError } = require("./errors.js");
const { appendEvent } = require("./events.js");
const checkpoint = require("./checkpoint.js");

function sleep(ms) {
  return new Promise((resolve) => setTimeout(resolve, ms));
}

function writeStepArtifact(jobDir, saveAsRelPath, value) {
  const absPath = path.join(jobDir, saveAsRelPath);
  fs.mkdirSync(path.dirname(absPath), { recursive: true });
  const content =
    saveAsRelPath.endsWith(".json") || (typeof value === "object" && value !== null)
      ? JSON.stringify(value, null, 2)
      : String(value ?? "");
  const tmpPath = `${absPath}.${process.pid}.${Date.now()}.tmp`;
  fs.writeFileSync(tmpPath, content, "utf8");
  fs.renameSync(tmpPath, absPath);
  return absPath;
}

async function runWithTimeout(promiseFactory, timeoutMs, onTimeout) {
  if (!timeoutMs || timeoutMs <= 0) {
    return await promiseFactory();
  }
  let timer = null;
  const timeoutPromise = new Promise((_, reject) => {
    timer = setTimeout(async () => {
      try {
        if (typeof onTimeout === "function") {
          await onTimeout();
        }
      } catch {
        // ignore stop hook failure
      }
      reject(new HarnessError("E_GEMINI_TIMEOUT", `${timeoutMs}ms 초과`));
    }, timeoutMs);
  });

  try {
    return await Promise.race([promiseFactory(), timeoutPromise]);
  } finally {
    if (timer) clearTimeout(timer);
  }
}

async function runSteps(job, steps, deps = {}) {
  const jobDir = job.jobDir;
  const jobId = job.jobId || path.basename(jobDir);
  const addPipelineLog = deps.addPipelineLog || null;
  const waitWhilePausedOrAborted = deps.waitWhilePausedOrAborted || (async () => {});
  const results = {};

  const state = checkpoint.load(jobDir);
  state.jobId = jobId;
  state.status = "running";
  checkpoint.save(jobDir, state);

  for (const step of steps) {
    await waitWhilePausedOrAborted();

    const validateFn =
      typeof step.validate === "function"
        ? step.validate
        : (res) => ({ ok: res !== undefined && res !== null });

    if (checkpoint.isDone(jobDir, step.id, validateFn)) {
      const entry = checkpoint.load(jobDir).steps[step.id];
      if (entry && entry.file) {
        const loaded = checkpoint.readSavedArtifact(jobDir, entry.file);
        results[step.id] = loaded.value;
      } else {
        results[step.id] = true;
      }
      appendEvent(
        jobDir,
        {
          job: jobId,
          step: step.id,
          attempt: entry?.attempts || 1,
          type: "resume",
          detail: "체크포인트 완료 단위 건너뜀",
        },
        addPipelineLog
      );
      continue;
    }

    const maxRetries =
      step.retry && typeof step.retry.max === "number" ? step.retry.max : 0;
    const backoffList =
      step.retry && Array.isArray(step.retry.backoffMs)
        ? step.retry.backoffMs
        : [5000, 20000];
    const allowedRetryCodes =
      step.retry && Array.isArray(step.retry.on) ? step.retry.on : null;

    let attempt = 0;
    let stepCompleted = false;
    let lastError = null;

    while (attempt <= maxRetries) {
      attempt += 1;
      await waitWhilePausedOrAborted();

      const t0 = Date.now();
      appendEvent(
        jobDir,
        {
          job: jobId,
          step: step.id,
          attempt,
          type: "start",
        },
        addPipelineLog
      );

      checkpoint.markStepState(jobDir, step.id, {
        status: "running",
        attempts: attempt,
      });

      const ctx = {
        jobId,
        jobDir,
        stepId: step.id,
        attempt,
        newChat: attempt > 1 || Boolean(step.newChat),
        shrinkCards: Boolean(lastError && lastError.code === "E_TRUNCATED"),
        previousError: lastError,
        results,
        deps,
      };

      try {
        const output = await runWithTimeout(
          () => step.run(ctx),
          step.timeoutMs,
          deps.onTimeout
        );
        const v = validateFn(output);
        if (!v || !v.ok) {
          throw new HarnessError(
            v?.code || "E_SCHEMA",
            v?.detail || "단계 출력 형식 검사 실패"
          );
        }

        if (step.saveAs) {
          writeStepArtifact(jobDir, step.saveAs, output);
        }

        const elapsed = Date.now() - t0;
        const chars =
          typeof output === "string"
            ? output.length
            : typeof output?.text === "string"
              ? output.text.length
              : 0;

        checkpoint.markDone(jobDir, step.id, step.saveAs || null, {
          attempts: attempt,
          ms: elapsed,
        });
        results[step.id] = output;

        appendEvent(
          jobDir,
          {
            job: jobId,
            step: step.id,
            attempt,
            type: "end",
            ms: elapsed,
            chars,
          },
          addPipelineLog
        );

        stepCompleted = true;
        break;
      } catch (rawErr) {
        const elapsed = Date.now() - t0;
        const err =
          rawErr instanceof HarnessError
            ? rawErr
            : new HarnessError(
                rawErr && rawErr.code ? rawErr.code : "E_SCHEMA",
                rawErr && rawErr.message ? rawErr.message : String(rawErr)
              );
        lastError = err;

        const canRetry =
          attempt <= maxRetries && isRetryableError(err, allowedRetryCodes);

        if (canRetry) {
          appendEvent(
            jobDir,
            {
              job: jobId,
              step: step.id,
              attempt,
              type: "retry",
              code: err.code,
              ms: elapsed,
              detail: err.detail || err.causeText,
            },
            addPipelineLog
          );

          if (typeof deps.onRecover === "function") {
            await deps.onRecover(err, ctx);
          }

          const waitMs =
            backoffList[Math.min(attempt - 1, backoffList.length - 1)] ?? 0;
          if (waitMs > 0) {
            await sleep(waitMs);
          }
          continue;
        }

        appendEvent(
          jobDir,
          {
            job: jobId,
            step: step.id,
            attempt,
            type: "fail",
            code: err.code,
            ms: elapsed,
            detail: err.detail || err.causeText,
          },
          addPipelineLog
        );
        break;
      }
    }

    if (!stepCompleted) {
      const policy = step.onExhausted || "needs-human";
      if (policy === "skip-with-flag") {
        const skipRecord = {
          skipped: true,
          stepId: step.id,
          code: lastError ? lastError.code : "E_UNKNOWN",
          detail: lastError ? lastError.causeText : "건너뜀",
        };
        checkpoint.markStepState(jobDir, step.id, {
          status: "skipped-with-flag",
          code: skipRecord.code,
          detail: skipRecord.detail,
          attempts: attempt,
        });
        results[step.id] = skipRecord;
        continue;
      }

      const finalStatus =
        policy === "abort" || (lastError && lastError.code === "E_ABORTED")
          ? "aborted"
          : "needs-human";

      checkpoint.markStepState(jobDir, step.id, {
        status: finalStatus,
        code: lastError ? lastError.code : "E_UNKNOWN",
        detail: lastError ? lastError.toUserMessage() : "",
        attempts: attempt,
      });

      const curState = checkpoint.load(jobDir);
      curState.status = finalStatus;
      curState.failedStep = step.id;
      curState.lastError = lastError
        ? {
            code: lastError.code,
            cause: lastError.causeText,
            action: lastError.actionText,
          }
        : null;
      checkpoint.save(jobDir, curState);

      return {
        status: finalStatus,
        failedStep: step.id,
        error: lastError,
        results,
      };
    }
  }

  const finalState = checkpoint.load(jobDir);
  finalState.status = "done";
  finalState.failedStep = null;
  finalState.lastError = null;
  checkpoint.save(jobDir, finalState);

  return {
    status: "done",
    results,
  };
}

module.exports = {
  runSteps,
  writeStepArtifact,
};
