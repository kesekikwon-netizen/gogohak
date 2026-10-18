"use strict";

const MARKER_COMBINED_REGEX =
  /(?:⟦((?:[FL]\d+)(?:\s*,\s*[FL]\d+)*)⟧|\[\[((?:[FL]\d+)(?:\s*,\s*[FL]\d+)*)\]\])/g;

function splitMarkerIdList(rawInner) {
  return String(rawInner || "")
    .split(",")
    .map((s) => s.trim())
    .filter((s) => /^[FL]\d+$/.test(s));
}

function parseMarkers(text = "") {
  const src = String(text || "");
  const matches = [];
  const allIds = [];
  const factIds = [];
  const litIds = [];

  const rx = new RegExp(MARKER_COMBINED_REGEX.source, "g");
  let m;
  while ((m = rx.exec(src)) !== null) {
    const inner = m[1] || m[2] || "";
    const ids = splitMarkerIdList(inner);
    const fIds = ids.filter((id) => id.startsWith("F"));
    const lIds = ids.filter((id) => id.startsWith("L"));
    matches.push({
      raw: m[0],
      ids,
      factIds: fIds,
      litIds: lIds,
      index: m.index,
    });
    for (const id of ids) {
      if (!allIds.includes(id)) allIds.push(id);
    }
    for (const f of fIds) {
      if (!factIds.includes(f)) factIds.push(f);
    }
    for (const l of lIds) {
      if (!litIds.includes(l)) litIds.push(l);
    }
  }

  return {
    matches,
    allIds,
    factIds,
    litIds,
  };
}

function normalizePunctuationSpacing(str) {
  return String(str || "")
    .replace(/[ \t]+([.,;:!?])/g, "$1")
    .replace(/([.])\1+/g, ".");
}

function stripFactMarkers(text = "") {
  const rx = new RegExp(MARKER_COMBINED_REGEX.source, "g");
  const replaced = String(text || "").replace(rx, (full, g1, g2) => {
    const ids = splitMarkerIdList(g1 || g2 || "");
    const litOnly = ids.filter((id) => id.startsWith("L"));
    if (litOnly.length === 0) {
      return "";
    }
    return `⟦${litOnly.join(",")}⟧`;
  });
  return normalizePunctuationSpacing(replaced);
}

function stripAllMarkers(text = "") {
  const rx = new RegExp(MARKER_COMBINED_REGEX.source, "g");
  return normalizePunctuationSpacing(String(text || "").replace(rx, ""));
}

function defaultFormatCitation(card) {
  if (!card) return "";
  const authorStr = Array.isArray(card.authors)
    ? card.authors.filter(Boolean).join("·")
    : String(card.author || "");
  const yearStr = String(card.year || "").trim();
  const titleStr = String(card.title || "").trim();
  const journalStr = String(card.journal || "").trim();
  const volIssue = [
    card.volume ? `제${card.volume}권` : "",
    card.issue ? `제${card.issue}호` : "",
  ]
    .filter(Boolean)
    .join(" ");
  const journalFull = [journalStr, volIssue].filter(Boolean).join(" ");
  const pagesStr = card.pages
    ? `${String(card.pages).replace(/^pp?\.\s*/i, "").replace(/-/g, "~").replace(/쪽$/, "")}쪽`
    : "";
  return [
    authorStr,
    yearStr,
    titleStr ? `「${titleStr}」` : "",
    journalFull ? `『${journalFull}』` : "",
    pagesStr,
  ]
    .filter(Boolean)
    .join(", ") + ".";
}

function markersToFootnotes(markdown = "", litCards = [], formatCitationFn = null) {
  const litMap = new Map();
  for (const c of litCards || []) {
    if (c && c.id) litMap.set(c.id, c);
  }
  const formatter =
    typeof formatCitationFn === "function" ? formatCitationFn : defaultFormatCitation;

  const assignedNumByLitId = new Map();
  const footnotes = [];
  let nextNum = 1;

  const rx = new RegExp(MARKER_COMBINED_REGEX.source, "g");
  const converted = String(markdown || "").replace(rx, (full, g1, g2) => {
    const ids = splitMarkerIdList(g1 || g2 || "");
    const lIds = ids.filter((id) => id.startsWith("L"));
    if (lIds.length === 0) {
      return "";
    }
    const fnMarks = [];
    for (const lid of lIds) {
      let num = assignedNumByLitId.get(lid);
      if (!num) {
        num = nextNum++;
        assignedNumByLitId.set(lid, num);
        const card = litMap.get(lid) || { id: lid, authors: [], year: "", title: lid, journal: "" };
        footnotes.push({
          num,
          litId: lid,
          card,
          text: formatter(card),
        });
      }
      fnMarks.push(`${num})`);
    }
    return ` ${fnMarks.join(" ")}`;
  });

  const cleanMarkdown = normalizePunctuationSpacing(converted);

  const bibliography = [...footnotes]
    .map((fn) => {
      const bibLine = fn.text.replace(/,\s*\d+(?:~\d+)?쪽\.?$/, ".");
      return {
        litId: fn.litId,
        authorKey: Array.isArray(fn.card?.authors)
          ? fn.card.authors.join("·")
          : String(fn.card?.author || fn.text),
        text: bibLine,
      };
    })
    .sort((a, b) => a.authorKey.localeCompare(b.authorKey, "ko-KR"));

  return {
    markdown: cleanMarkdown,
    footnotes,
    bibliography,
    assignedNumByLitId,
  };
}

module.exports = {
  MARKER_COMBINED_REGEX,
  parseMarkers,
  stripFactMarkers,
  stripAllMarkers,
  defaultFormatCitation,
  markersToFootnotes,
};
