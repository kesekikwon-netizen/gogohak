"use strict";

const fs = require("node:fs");
const path = require("node:path");
const os = require("node:os");
const { HarnessError } = require("./errors.js");

function getRunJsonPath(jobDir) {
  return path.join(jobDir, "run.json");
}

function defaultLockPath(baseDir) {
  const root = baseDir || path.join(os.homedir(), "archaeology_reports");
  return path.join(root, ".lock");
}

function isPidAlive(pid, customCheck = null) {
  if (typeof customCheck === "function") {
    return Boolean(customCheck(pid));
  }
  const num = Number(pid);
  if (!Number.isInteger(num) || num <= 0) return false;
  try {
    process.kill(num, 0);
    return true;
  } catch (err) {
    return err && err.code === "EPERM";
  }
}

function acquireLock(lockPath, jobId, options = {}) {
  const targetLock = lockPath || defaultLockPath(options.baseDir);
  fs.mkdirSync(path.dirname(targetLock), { recursive: true });
  const currentPid = options.pid || process.pid;

  if (fs.existsSync(targetLock)) {
    try {
      const existing = JSON.parse(fs.readFileSync(targetLock, "utf8"));
      if (
        existing &&
        existing.pid &&
        existing.pid !== currentPid &&
        isPidAlive(existing.pid, options.isPidAlive)
      ) {
        throw new HarnessError(
          "E_LOCKED",
          `실행 중인 작업ID=${existing.jobId || "unknown"}, PID=${existing.pid}`
        );
      }
    } catch (err) {
      if (err instanceof HarnessError) throw err;
      // corrupted lock file -> overwrite
    }
  }

  const lockData = {
    jobId: String(jobId || "v2-job"),
    pid: currentPid,
    startedAt: new Date().toISOString(),
  };
  const tmpLock = `${targetLock}.${currentPid}.tmp`;
  fs.writeFileSync(tmpLock, JSON.stringify(lockData, null, 2), "utf8");
  fs.renameSync(tmpLock, targetLock);
  return { lockPath: targetLock, ...lockData };
}

function releaseLock(lockPath, jobId = null) {
  if (!lockPath || !fs.existsSync(lockPath)) return false;
  try {
    if (jobId) {
      const existing = JSON.parse(fs.readFileSync(lockPath, "utf8"));
      if (existing && existing.jobId && existing.jobId !== jobId) {
        return false;
      }
    }
    fs.unlinkSync(lockPath);
    return true;
  } catch {
    return false;
  }
}

function load(jobDir) {
  const runPath = getRunJsonPath(jobDir);
  if (!fs.existsSync(runPath)) {
    return {
      jobId: path.basename(jobDir),
      createdAt: new Date().toISOString(),
      updatedAt: new Date().toISOString(),
      status: "idle",
      steps: {},
    };
  }
  try {
    const parsed = JSON.parse(fs.readFileSync(runPath, "utf8"));
    if (!parsed.steps || typeof parsed.steps !== "object") {
      parsed.steps = {};
    }
    return parsed;
  } catch {
    return {
      jobId: path.basename(jobDir),
      createdAt: new Date().toISOString(),
      updatedAt: new Date().toISOString(),
      status: "idle",
      steps: {},
    };
  }
}

function save(jobDir, state) {
  fs.mkdirSync(jobDir, { recursive: true });
  const runPath = getRunJsonPath(jobDir);
  const next = {
    ...state,
    updatedAt: new Date().toISOString(),
  };
  const tmpPath = `${runPath}.${process.pid}.${Date.now()}.tmp`;
  fs.writeFileSync(tmpPath, JSON.stringify(next, null, 2), "utf8");
  fs.renameSync(tmpPath, runPath);
  return next;
}

function markStepState(jobDir, stepId, stepPatch) {
  const state = load(jobDir);
  state.steps[stepId] = {
    ...(state.steps[stepId] || {}),
    ...stepPatch,
    updatedAt: new Date().toISOString(),
  };
  return save(jobDir, state);
}

function markDone(jobDir, stepId, saveAsRelPath = null, meta = {}) {
  return markStepState(jobDir, stepId, {
    status: "done",
    file: saveAsRelPath || null,
    ...meta,
  });
}

function readSavedArtifact(jobDir, saveAsRelPath) {
  const absPath = path.join(jobDir, saveAsRelPath);
  if (!fs.existsSync(absPath)) return { exists: false, value: null };
  const raw = fs.readFileSync(absPath, "utf8");
  if (saveAsRelPath.endsWith(".json")) {
    try {
      return { exists: true, value: JSON.parse(raw) };
    } catch {
      return { exists: false, corrupted: true, value: null };
    }
  }
  return { exists: true, value: raw };
}

function isDone(jobDir, stepId, validateFn = null) {
  const state = load(jobDir);
  const entry = state.steps && state.steps[stepId];
  if (!entry || entry.status !== "done") return false;

  if (entry.file) {
    const loaded = readSavedArtifact(jobDir, entry.file);
    if (!loaded.exists) return false;
    if (typeof validateFn === "function") {
      try {
        const check = validateFn(loaded.value);
        if (!check || !check.ok) return false;
      } catch {
        return false;
      }
    }
  }
  return true;
}

module.exports = {
  defaultLockPath,
  isPidAlive,
  acquireLock,
  releaseLock,
  load,
  save,
  markStepState,
  markDone,
  readSavedArtifact,
  isDone,
};
