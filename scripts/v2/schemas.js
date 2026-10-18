"use strict";

const { isFactId, isLitId, isCardId } = require("./ids.js");

const FACT_KINDS = new Set([
  "feature",
  "artifact",
  "stratum",
  "dating",
  "chronology",
  "site",
  "other",
]);

const FACT_CONFIDENCE = new Set(["verified", "unverified"]);
const DOC_TYPES = new Set(["report", "paper"]);
const CLAIM_KINDS = new Set(["fact", "interpretation", "citation"]);
const FINAL_VERDICTS = new Set(["pass", "rework", "needs-human"]);

function validateFactCard(card) {
  const errors = [];
  if (!card || typeof card !== "object") {
    return { ok: false, errors: ["FactCard must be an object"] };
  }
  if (!isFactId(card.id)) {
    errors.push(`Invalid FactCard id: ${card.id}`);
  }
  if (!FACT_KINDS.has(card.kind)) {
    errors.push(`Invalid FactCard kind: ${card.kind}`);
  }
  if (typeof card.subject !== "string" || !card.subject.trim()) {
    errors.push("FactCard.subject must be a non-empty string");
  }
  if (typeof card.attribute !== "string" || !card.attribute.trim()) {
    errors.push("FactCard.attribute must be a non-empty string");
  }
  if (typeof card.value !== "string" || !card.value.trim()) {
    errors.push("FactCard.value must be a non-empty string");
  }
  if (typeof card.unit !== "string") {
    errors.push("FactCard.unit must be a string");
  }
  if (typeof card.quote !== "string" || !card.quote.trim()) {
    errors.push("FactCard.quote must be a non-empty string");
  }
  if (!card.source || typeof card.source !== "object") {
    errors.push("FactCard.source must be an object");
  } else {
    if (typeof card.source.file !== "string" || !card.source.file.trim()) {
      errors.push("FactCard.source.file must be a non-empty string");
    }
    if (
      card.source.page !== null &&
      (!Number.isInteger(card.source.page) || card.source.page <= 0)
    ) {
      errors.push("FactCard.source.page must be a positive integer or null");
    }
    if (typeof card.source.chapter !== "string") {
      errors.push("FactCard.source.chapter must be a string");
    }
  }
  if (!FACT_CONFIDENCE.has(card.confidence)) {
    errors.push(`Invalid FactCard confidence: ${card.confidence}`);
  }
  return { ok: errors.length === 0, errors };
}

function validateLiteratureCard(card) {
  const errors = [];
  if (!card || typeof card !== "object") {
    return { ok: false, errors: ["LiteratureCard must be an object"] };
  }
  if (!isLitId(card.id)) {
    errors.push(`Invalid LiteratureCard id: ${card.id}`);
  }
  if (typeof card.nodeId !== "string") {
    errors.push("LiteratureCard.nodeId must be a string");
  }
  if (!Array.isArray(card.authors)) {
    errors.push("LiteratureCard.authors must be an array");
  }
  if (typeof card.year !== "string") {
    errors.push("LiteratureCard.year must be a string");
  }
  if (typeof card.title !== "string" || !card.title.trim()) {
    errors.push("LiteratureCard.title must be a non-empty string");
  }
  if (typeof card.journal !== "string") {
    errors.push("LiteratureCard.journal must be a string");
  }
  if (!Array.isArray(card.quotes)) {
    errors.push("LiteratureCard.quotes must be an array");
  }
  if (typeof card.summary !== "string") {
    errors.push("LiteratureCard.summary must be a string");
  }
  if (typeof card.query !== "string") {
    errors.push("LiteratureCard.query must be a string");
  }
  if (typeof card.verified !== "boolean") {
    errors.push("LiteratureCard.verified must be a boolean");
  }
  return { ok: errors.length === 0, errors };
}

function validateOutline(outline) {
  const errors = [];
  if (!outline || typeof outline !== "object") {
    return { ok: false, errors: ["Outline must be an object"] };
  }
  if (!DOC_TYPES.has(outline.docType)) {
    errors.push(`Invalid Outline docType: ${outline.docType}`);
  }
  if (typeof outline.title !== "string" || !outline.title.trim()) {
    errors.push("Outline.title must be a non-empty string");
  }
  if (typeof outline.approved !== "boolean") {
    errors.push("Outline.approved must be a boolean");
  }
  if (!Array.isArray(outline.omitted)) {
    errors.push("Outline.omitted must be an array");
  }
  if (!Array.isArray(outline.sections) || outline.sections.length === 0) {
    errors.push("Outline.sections must be a non-empty array");
  } else {
    outline.sections.forEach((sec, idx) => {
      if (!sec || typeof sec !== "object") {
        errors.push(`Section[${idx}] must be an object`);
        return;
      }
      if (!/^S\d+$/.test(String(sec.id || ""))) {
        errors.push(`Section[${idx}].id must match S<number>`);
      }
      if (typeof sec.heading !== "string" || !sec.heading.trim()) {
        errors.push(`Section[${idx}].heading must be a non-empty string`);
      }
      if (!Array.isArray(sec.claims) || sec.claims.length === 0) {
        errors.push(`Section[${idx}].claims must be a non-empty array`);
      } else {
        sec.claims.forEach((cl, cIdx) => {
          if (!cl || typeof cl.text !== "string" || !cl.text.trim()) {
            errors.push(`Section[${idx}].claims[${cIdx}].text must be non-empty`);
          }
          if (
            !Array.isArray(cl?.evidence) ||
            cl.evidence.length === 0 ||
            !cl.evidence.every(isCardId)
          ) {
            errors.push(
              `Section[${idx}].claims[${cIdx}].evidence must be a non-empty array of valid card IDs`
            );
          }
        });
      }
      if (!Array.isArray(sec.visuals)) {
        errors.push(`Section[${idx}].visuals must be an array`);
      }
      if (typeof sec.targetChars !== "number" || sec.targetChars <= 0) {
        errors.push(`Section[${idx}].targetChars must be a positive number`);
      }
    });
  }
  return { ok: errors.length === 0, errors };
}

function validateClaim(claim) {
  const errors = [];
  if (!claim || typeof claim !== "object") {
    return { ok: false, errors: ["Claim must be an object"] };
  }
  if (typeof claim.id !== "string" || !claim.id.trim()) {
    errors.push("Claim.id must be a non-empty string");
  }
  if (typeof claim.sentence !== "string" || !claim.sentence.trim()) {
    errors.push("Claim.sentence must be a non-empty string");
  }
  if (!Array.isArray(claim.atoms)) {
    errors.push("Claim.atoms must be an array");
  }
  if (!Array.isArray(claim.markers) || !claim.markers.every(isCardId)) {
    errors.push("Claim.markers must be an array of valid card IDs");
  }
  if (!CLAIM_KINDS.has(claim.kind)) {
    errors.push(`Invalid Claim kind: ${claim.kind}`);
  }
  return { ok: errors.length === 0, errors };
}

function validateVerifyMatrix(matrix) {
  const errors = [];
  if (!matrix || typeof matrix !== "object") {
    return { ok: false, errors: ["VerifyMatrix must be an object"] };
  }
  if (!/^S\d+$/.test(String(matrix.sectionId || ""))) {
    errors.push(`Invalid VerifyMatrix.sectionId: ${matrix.sectionId}`);
  }
  if (typeof matrix.round !== "number" || matrix.round < 0) {
    errors.push("VerifyMatrix.round must be a non-negative number");
  }
  if (!Array.isArray(matrix.rows)) {
    errors.push("VerifyMatrix.rows must be an array");
  } else {
    matrix.rows.forEach((row, idx) => {
      if (!row || typeof row.claimId !== "string") {
        errors.push(`rows[${idx}].claimId must be a string`);
      }
      if (typeof row?.sentence !== "string") {
        errors.push(`rows[${idx}].sentence must be a string`);
      }
      if (!FINAL_VERDICTS.has(row?.final)) {
        errors.push(`rows[${idx}].final must be pass | rework | needs-human`);
      }
    });
  }
  if (!matrix.critic || typeof matrix.critic !== "object") {
    errors.push("VerifyMatrix.critic must be an object");
  }
  if (!matrix.summary || typeof matrix.summary !== "object") {
    errors.push("VerifyMatrix.summary must be an object");
  }
  return { ok: errors.length === 0, errors };
}

module.exports = {
  FACT_KINDS,
  FACT_CONFIDENCE,
  DOC_TYPES,
  CLAIM_KINDS,
  FINAL_VERDICTS,
  validateFactCard,
  validateLiteratureCard,
  validateOutline,
  validateClaim,
  validateVerifyMatrix,
};
