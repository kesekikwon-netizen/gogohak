"use strict";

function makeFactId(index) {
  return `F${Number(index)}`;
}

function makeLitId(index) {
  return `L${Number(index)}`;
}

function makeSectionId(index) {
  return `S${Number(index)}`;
}

function makeClaimId(sectionId, round, index) {
  return `${sectionId}.r${Number(round)}.K${Number(index)}`;
}

function makeJobId(date = new Date()) {
  const pad = (n) => String(n).padStart(2, "0");
  const yyyy = date.getFullYear();
  const mm = pad(date.getMonth() + 1);
  const dd = pad(date.getDate());
  const hh = pad(date.getHours());
  const mi = pad(date.getMinutes());
  const ss = pad(date.getSeconds());
  return `${yyyy}${mm}${dd}-${hh}${mi}${ss}`;
}

function isFactId(id) {
  return /^F\d+$/.test(String(id || "").trim());
}

function isLitId(id) {
  return /^L\d+$/.test(String(id || "").trim());
}

function isCardId(id) {
  return /^[FL]\d+$/.test(String(id || "").trim());
}

module.exports = {
  makeFactId,
  makeLitId,
  makeSectionId,
  makeClaimId,
  makeJobId,
  isFactId,
  isLitId,
  isCardId,
};
