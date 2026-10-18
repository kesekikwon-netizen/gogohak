"use strict";

const fs = require("node:fs");
const path = require("node:path");

function splitTextByPageMarkers(rawText = "") {
  const src = String(rawText || "");
  const pageRx = /\[\[\s*PAGE\s+(\d+)\s*\]\]/gi;
  const matches = [];
  let m;
  while ((m = pageRx.exec(src)) !== null) {
    matches.push({
      page: Number(m[1]),
      start: m.index,
      contentStart: m.index + m[0].length,
    });
  }

  if (matches.length === 0) {
    return [{ page: null, text: src.trim() }];
  }

  const pages = [];
  for (let i = 0; i < matches.length; i++) {
    const cur = matches[i];
    const nextStart = i + 1 < matches.length ? matches[i + 1].start : src.length;
    const slice = src.slice(cur.contentStart, nextStart).trim();
    pages.push({
      page: cur.page,
      text: slice,
    });
  }
  return pages;
}

async function extractPages(filePathOrOptions, craftDriver = null) {
  if (typeof filePathOrOptions === "object" && filePathOrOptions !== null) {
    const { filePath, rawText } = filePathOrOptions;
    if (rawText && String(rawText).trim()) {
      return splitTextByPageMarkers(rawText);
    }
    if (filePath) {
      return await extractPagesFromFile(filePath, craftDriver);
    }
    return [];
  }

  const str = String(filePathOrOptions || "");
  if (fs.existsSync(str) && fs.statSync(str).isFile()) {
    return await extractPagesFromFile(str, craftDriver);
  }
  return splitTextByPageMarkers(str);
}

async function extractPagesFromFile(filePath, craftDriver = null) {
  const ext = path.extname(filePath).toLowerCase();

  if (ext === ".pdf" && craftDriver) {
    const info = craftDriver.pdfInfo(filePath);
    const pageCount = Number(info?.pageCount || 0);
    if (pageCount > 0) {
      const pages = [];
      for (let p = 1; p <= pageCount; p++) {
        const pageText = craftDriver.pdfPageText(filePath, p);
        pages.push({
          page: p,
          text: String(pageText || "").trim(),
        });
      }
      return pages;
    }
    const fullPdfText = craftDriver.pdfPageText(filePath, null);
    return splitTextByPageMarkers(fullPdfText);
  }

  const content = fs.readFileSync(filePath, "utf8");
  return splitTextByPageMarkers(content);
}

module.exports = {
  splitTextByPageMarkers,
  extractPages,
  extractPagesFromFile,
};
