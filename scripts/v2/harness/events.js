"use strict";

const fs = require("node:fs");
const path = require("node:path");

function appendEvent(jobDir, eventObj, addPipelineLog = null) {
  if (!jobDir) return null;
  fs.mkdirSync(jobDir, { recursive: true });
  const eventsPath = path.join(jobDir, "events.jsonl");
  const record = {
    t: eventObj.t || new Date().toISOString(),
    job: eventObj.job || path.basename(jobDir),
    step: eventObj.step || "",
    attempt: typeof eventObj.attempt === "number" ? eventObj.attempt : 1,
    type: eventObj.type || "info",
    ...(eventObj.code ? { code: eventObj.code } : {}),
    ...(typeof eventObj.ms === "number" ? { ms: eventObj.ms } : {}),
    ...(typeof eventObj.chars === "number" ? { chars: eventObj.chars } : {}),
    ...(eventObj.rawPath ? { rawPath: eventObj.rawPath } : {}),
    ...(eventObj.detail ? { detail: String(eventObj.detail) } : {}),
  };

  fs.appendFileSync(eventsPath, JSON.stringify(record) + "\n", "utf8");

  if (typeof addPipelineLog === "function") {
    const tag = `[v2:${record.step || "job"}#${record.attempt}]`;
    const codePart = record.code ? ` (${record.code})` : "";
    const msPart = typeof record.ms === "number" ? ` ${record.ms}ms` : "";
    const detailPart = record.detail ? ` — ${record.detail}` : "";
    addPipelineLog(`${tag} ${record.type}${codePart}${msPart}${detailPart}`);
  }

  return record;
}

function readEvents(jobDir) {
  const eventsPath = path.join(jobDir, "events.jsonl");
  if (!fs.existsSync(eventsPath)) return [];
  const raw = fs.readFileSync(eventsPath, "utf8");
  const list = [];
  for (const line of raw.split(/\r?\n/)) {
    const trimmed = line.trim();
    if (!trimmed) continue;
    try {
      list.push(JSON.parse(trimmed));
    } catch {
      // skip malformed line
    }
  }
  return list;
}

function computeMetrics(jobDir) {
  const events = readEvents(jobDir);
  const stepMs = {};
  const errorCounts = {};
  let totalRetries = 0;
  let geminiCalls = 0;
  let totalChars = 0;

  for (const ev of events) {
    if (ev.type === "retry") {
      totalRetries += 1;
    }
    if (ev.code) {
      errorCounts[ev.code] = (errorCounts[ev.code] || 0) + 1;
    }
    if (ev.type === "end" && typeof ev.ms === "number") {
      stepMs[ev.step] = (stepMs[ev.step] || 0) + ev.ms;
    }
    if (ev.type === "end" && typeof ev.chars === "number" && ev.chars > 0) {
      geminiCalls += 1;
      totalChars += ev.chars;
    }
  }

  return {
    jobId: path.basename(jobDir),
    totalEvents: events.length,
    stepMs,
    totalRetries,
    errorCounts,
    geminiCalls,
    totalChars,
  };
}

module.exports = {
  appendEvent,
  readEvents,
  computeMetrics,
};
