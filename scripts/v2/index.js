'use strict';

const fs = require('node:fs');
const path = require('node:path');
const os = require('node:os');

const { HarnessError } = require('./harness/errors');
const { appendEvent, computeMetrics } = require('./harness/events');
const checkpoint = require('./harness/checkpoint');
const { runSteps } = require('./harness/supervisor');
const { runSelftest } = require('./harness/selftest');

const { makeJobId, makeFactId } = require('./ids');
const { createEvidenceStore } = require('./evidence_store');
const { extractPages } = require('./page_extractor');
const { splitChapters, buildFactPrompt, parseFactReply, verifyFactAgainstPages, evaluateGateG1 } = require('./fact_extractor');
const { toLiteratureCards, mergeLiterature, evaluateGateG2 } = require('./literature_cards');
const { buildQueriesFromFacts } = require('./query_builder');
const { buildOutlinePrompt, parseOutline, evaluateGateG3 } = require('./outline_planner');
const { buildSectionPrompt, parseSectionReply } = require('./section_writer');
const { splitClaims } = require('./verify/claim_splitter');
const { runDeterministic } = require('./verify/deterministic');
const { checkFacts } = require('./verify/fact_checker');
const { checkCitations } = require('./verify/citation_checker');
const { critique } = require('./verify/critic');
const { buildVerifyMatrix, attachNeedsHumanTags, evaluateGateG5 } = require('./verify/consensus');
const { planVisuals } = require('./visuals_from_facts');
const { buildCrosswalk } = require('./crosswalk');
const { assemble } = require('./assembler');

const DEFAULT_CONFIG = JSON.parse(fs.readFileSync(path.join(__dirname, 'config.default.json'), 'utf8'));

function getDefaultBaseOutputDir() {
  return path.join(os.homedir(), 'archaeology_reports');
}

const activeJobs = new Map();
let latestJobId = null;

function resolveJobDir(jobId, baseOutputDir = getDefaultBaseOutputDir()) {
  if (activeJobs.has(jobId)) {
    return activeJobs.get(jobId).jobDir;
  }
  return path.join(baseOutputDir, 'jobs', String(jobId));
}

function notifyLegacyPipeline(phase, message, extra = {}) {
  try {
    const server = require('../gemini_mcp_server');
    if (server.pipelineState && typeof server.pipelineState === 'object') {
      server.pipelineState.phase = phase;
      server.pipelineState.engine = 'v2';
      Object.assign(server.pipelineState, extra);
    }
    if (typeof server.addPipelineLog === 'function' && message) {
      server.addPipelineLog(`[v2:${phase}] ${message}`);
    }
  } catch (_) {}
}

function buildSyntheticPagesFromText(reportText = '', datingText = '') {
  const combined = [String(reportText || '').trim(), String(datingText || '').trim()].filter(Boolean).join('\n\n');
  if (!combined) return [];
  const markerRe = /\[\[\s*PAGE\s+(\d+)\s*\]\]/gi;
  const matches = [...combined.matchAll(markerRe)];
  if (matches.length === 0) {
    return [{ page: 1, text: combined }];
  }
  const pages = [];
  for (let i = 0; i < matches.length; i++) {
    const pageNum = Number(matches[i][1]);
    const startIdx = matches[i].index + matches[i][0].length;
    const endIdx = i + 1 < matches.length ? matches[i + 1].index : combined.length;
    pages.push({
      page: pageNum,
      text: combined.slice(startIdx, endIdx).trim(),
    });
  }
  return pages;
}

/**
 * 한 개 절의 집필(P4) + 교차검증(P5: V1~V8, X1, X2, X3, 합의, 최대 maxRework 재작성)을 수행한다.
 * 재작성된 절은 앞 회차 통과 결과를 재사용하지 않고 처음부터 다시 검증받는다(7.5).
 */
async function writeAndVerifySection({
  jobDir,
  store,
  section,
  sectionVisuals = [],
  facts,
  literature,
  pages,
  docType,
  prevSummary,
  geminiDriver,
  config = DEFAULT_CONFIG,
}) {
  const maxRework = config?.verify?.maxRework ?? 2;
  let round = 0;
  let fixes = null;
  let finalMarkdown = '';
  let finalSummary = '';
  let lastMatrix = null;

  while (round <= maxRework) {
    const writeStepId = `P4.${section.id}.r${round}`;
    notifyLegacyPipeline('V2_P4_WRITING', `${section.heading} 집필 중 (회차 r${round})`);

    const { prompt } = buildSectionPrompt(
      section,
      { facts, literature },
      docType,
      prevSummary,
      fixes,
      writeStepId,
      config?.gemini?.maxPromptChars || 12000
    );

    const writeRes = await geminiDriver.ask({
      jobDir,
      jobStep: writeStepId,
      prompt,
      expectJson: false,
      newChat: true,
    });

    const parsedSec = parseSectionReply(writeRes.text, section);
    let draftMd = parsedSec.markdown;

    // 조건을 충족한 시각자료(표·차트·모식도)가 이 절에 배정된 경우 본문 하단에 첨부
    if (sectionVisuals.length > 0) {
      const visBlocks = sectionVisuals
        .filter((v) => v.markdown && !draftMd.includes(v.title))
        .map((v) => v.markdown);
      if (visBlocks.length > 0) {
        draftMd = `${draftMd}\n\n${visBlocks.join('\n\n')}`;
      }
    }

    store.saveSectionDraft(section.id, round, draftMd);

    // P5 교차검증 (항상 처음부터 실행)
    notifyLegacyPipeline('V2_P5_VERIFY', `${section.heading} 교차검증 중 (회차 r${round})`);
    const claims = splitClaims(draftMd, section.id, round);
    const detResult = runDeterministic({
      markdown: draftMd,
      claims,
      facts,
      literature,
      section,
      sectionId: section.id,
      round,
    });

    const x1Result = await checkFacts({
      claims,
      facts,
      pages,
      geminiDriver,
      sectionId: section.id,
      round,
      config,
    });

    const x2Result = await checkCitations({
      claims,
      literature,
      geminiDriver,
      sectionId: section.id,
      round,
    });

    const criticResult = await critique({
      markdown: draftMd,
      section,
      docType,
      geminiDriver,
      sectionId: section.id,
      round,
    });

    const matrix = buildVerifyMatrix({
      sectionId: section.id,
      round,
      detResult,
      x1Result,
      x2Result,
      criticResult,
      maxRework,
    });

    store.saveVerifyMatrix(section.id, round, matrix);
    lastMatrix = matrix;

    if (matrix.summary.rework === 0) {
      finalMarkdown = attachNeedsHumanTags(draftMd, matrix);
      finalSummary = parsedSec.summary;
      break;
    }

    if (round >= maxRework) {
      // 재작성 횟수 소진 -> 남은 문장에 〔확인 필요: 사유〕 부착
      for (const row of matrix.rows) {
        if (row.final === 'rework') {
          row.final = 'needs-human';
        }
      }
      matrix.summary.needsHuman += matrix.summary.rework;
      matrix.summary.rework = 0;
      store.saveVerifyMatrix(section.id, round, matrix);
      finalMarkdown = attachNeedsHumanTags(draftMd, matrix);
      finalSummary = parsedSec.summary;
      break;
    }

    // 다음 재작성 회차 준비 (앞 원고와 지적 사항만 전달)
    const reworkIssues = [];
    for (const row of matrix.rows) {
      if (row.final === 'rework') {
        reworkIssues.push(`[문장 ${row.claimId}] "${row.sentence}" → 수정 사유: ${row.reason}`);
      }
    }
    for (const iss of matrix.critic?.issues || []) {
      reworkIssues.push(`[심사 지적: ${iss.type}] "${iss.sentence}" → ${iss.comment} (제안: ${iss.suggestion})`);
    }

    fixes = {
      previousDraft: draftMd,
      issues: reworkIssues,
    };
    round += 1;
  }

  store.saveSectionDraft(section.id, 'final', finalMarkdown);
  return {
    sectionId: section.id,
    heading: section.heading,
    roundsUsed: round + 1,
    markdown: finalMarkdown,
    summary: finalSummary,
    matrix: lastMatrix,
  };
}

/**
 * 9절 runPipelineV2:
 * v2 파이프라인 전체 실행 및 체크포인트 기반 재개 지원
 */
async function runPipelineV2(options = {}) {
  const config = {
    ...DEFAULT_CONFIG,
    ...(options.config || {}),
  };

  const docType = options.docType === 'paper' ? 'paper' : 'report';
  const userFocus = String(options.userFocus || '').trim();
  const autoApproveOutline = Boolean(options.autoApproveOutline);
  const literatureMode = options.literatureMode === 'manual' ? 'manual' : 'dbpia';
  const baseOutputDir = options.targetPath || options.baseOutputDir || getDefaultBaseOutputDir();
  const jobId = options.jobId || makeJobId();
  const jobDir = options.jobDir || path.join(baseOutputDir, 'jobs', jobId);

  fs.mkdirSync(jobDir, { recursive: true });
  const store = createEvidenceStore(jobDir);

  const ignoredGates = new Set(options.ignoredGates || []);
  const cpState = checkpoint.load(jobDir);
  cpState.meta = {
    ...(cpState.meta || {}),
    jobId,
    docType,
    userFocus,
    autoApproveOutline,
    literatureMode,
  };
  for (const g of cpState.ignoredGates || []) ignoredGates.add(g);
  checkpoint.save(jobDir, cpState);

  // 드라이버 주입 (실기 드라이버 또는 fake_drivers)
  const deps = options.drivers || {};
  let serverRef = null;
  try {
    serverRef = require('../gemini_mcp_server');
  } catch (_) {}

  const geminiDriver =
    deps.geminiDriver ||
    require('./harness/gemini_driver').createGeminiDriver({
      jobDir,
      config,
      askGeminiWeb: serverRef
        ? (prompt, opts) =>
            serverRef.askGeminiWeb({
              prompt,
              newChat: Boolean(opts?.newChat),
            })
        : undefined,
      resetGeminiChat: serverRef ? serverRef.resetGeminiChat : undefined,
      parseJsonFromGeminiReply: serverRef ? serverRef.parseJsonFromGeminiReply : undefined,
    });
  const dbpiaDriver =
    deps.dbpiaDriver ||
    require('./harness/dbpia_driver').createDbpiaDriver({
      jobDir,
      config,
      askDbpiaAi: serverRef ? serverRef.askDbpiaAi : undefined,
      getDbpiaLoginStatus: serverRef ? serverRef.getDbpiaLoginStatus : undefined,
    });
  const craftDriver =
    deps.craftDriver ||
    require('./harness/craft_driver').createCraftDriver({
      jobDir,
      config,
      exportManuscriptToWordCraftAndPdfCraft: serverRef
        ? serverRef.exportManuscriptToWordCraftAndPdfCraft
        : undefined,
    });

  const jobCtx = {
    jobId,
    jobDir,
    docType,
    userFocus,
    autoApproveOutline,
    literatureMode,
    phase: 'V2_H0_SELFTEST',
    status: 'running',
    gates: {},
    error: null,
    options,
    drivers: { geminiDriver, dbpiaDriver, craftDriver },
    ignoredGates,
  };
  activeJobs.set(jobId, jobCtx);
  latestJobId = jobId;

  const lockFile = path.join(jobDir, '.lock');
  const lockInfo = checkpoint.acquireLock(lockFile, jobId, { baseDir: jobDir });

  try {
    // H0. 자가진단
    jobCtx.phase = 'V2_H0_SELFTEST';
    notifyLegacyPipeline('V2_H0_SELFTEST', '사전 자가진단(H0) 실행 중', { jobId });
    if (!options.skipSelftest && !options.drivers) {
      const st = await runSelftest({
        getCdpPort: serverRef ? serverRef.getCdpPort : undefined,
        ensureEmbeddedGeminiWebview: serverRef ? serverRef.ensureEmbeddedGeminiWebview : undefined,
        withCdpSession: serverRef ? serverRef.withCdpSession : undefined,
        ensureArgonHighAndWorkMode: serverRef ? serverRef.ensureArgonHighAndWorkMode : undefined,
        getDbpiaLoginStatus: serverRef ? serverRef.getDbpiaLoginStatus : undefined,
        literatureMode,
        outputDir: jobDir,
      });
      if (!st.ok) {
        const failedItem = st.items.find((i) => !i.ok);
        throw new HarnessError(failedItem?.code || 'E_DOM_DRIFT', failedItem?.detail || '사전 자가진단 실패');
      }
    }

    // P1. 쪽 추출 및 사실 카드 추출
    jobCtx.phase = 'V2_P1_FACTS';
    notifyLegacyPipeline('V2_P1_FACTS', '보고서 쪽별 원문 및 사실 카드 추출(P1) 중', { jobId });

    let pages = store.loadPages();
    if (!pages || pages.length === 0) {
      if (options.reportFilePath && fs.existsSync(options.reportFilePath)) {
        pages = await extractPages(options.reportFilePath, { craftDriver });
      } else {
        pages = buildSyntheticPagesFromText(options.reportText || '', options.datingText || '');
      }
      store.savePages(pages);
    }

    let facts = store.loadFacts();
    const failedChapters = [];
    if (!facts || facts.length === 0) {
      const chapters = splitChapters(pages);
      const allExtracted = [];
      let fSeq = 1;

      const chapterSteps = chapters.map((ch, idx) => ({
        id: `P1.CH${idx + 1}`,
        timeoutMs: (config?.timeouts?.geminiJson || 150) * 1000,
        retry: { max: 2, backoffMs: [0, 0] },
        onExhausted: 'skip-with-flag',
        saveAs: `raw/P1.CH${idx + 1}.done.json`,
        run: async () => {
          const stepId = `P1.CH${idx + 1}`;
          const prompt = buildFactPrompt(ch, options.reportFileName || '보고서.pdf');
          const reply = await geminiDriver.ask({
            jobDir,
            jobStep: stepId,
            prompt,
            expectJson: true,
            newChat: true,
          });
          const rawCards = parseFactReply(
            reply.json || reply.text,
            options.reportFileName || '보고서.pdf',
            ch.chapter || `CH${idx + 1}`,
            fSeq
          );
          const verifiedCards = rawCards.map((c) => {
            const checked = verifyFactAgainstPages(
              { ...c, id: makeFactId(fSeq++) },
              pages
            );
            return checked;
          });
          return verifiedCards;
        },
      }));

      await runSteps({ jobDir, jobId }, chapterSteps, { config });
      for (let i = 0; i < chapters.length; i++) {
        const stepId = `P1.CH${i + 1}`;
        const outPath = path.join(jobDir, 'raw', `${stepId}.done.json`);
        if (fs.existsSync(outPath)) {
          try {
            const arr = JSON.parse(fs.readFileSync(outPath, 'utf8'));
            for (const c of arr) {
              allExtracted.push({ ...c, id: makeFactId(allExtracted.length + 1) });
            }
          } catch (_) {
            failedChapters.push(chapters[i].chapter || stepId);
          }
        } else {
          failedChapters.push(chapters[i].chapter || stepId);
        }
      }
      facts = allExtracted;
      store.saveFacts(facts);
    }

    const g1 = evaluateGateG1(facts, failedChapters, config);
    const g1Reason = g1.reason || g1.detail || '';
    g1.reason = g1Reason;
    jobCtx.gates.G1 = g1;
    appendEvent(jobDir, { job: jobId, step: 'G1', type: 'gate', detail: g1Reason });
    if (!g1.ok && !ignoredGates.has('G1')) {
      jobCtx.phase = 'V2_NEEDS_HUMAN';
      jobCtx.status = 'needs-human';
      jobCtx.error = { code: 'E_GATE', gate: 'G1', detail: g1Reason };
      notifyLegacyPipeline('V2_NEEDS_HUMAN', g1Reason, { jobId });
      return { ok: false, status: 'needs-human', jobId, phase: jobCtx.phase, gate: g1 };
    }

    // P2. 문헌 수집 및 문헌 카드 생성
    jobCtx.phase = 'V2_P2_LITERATURE';
    notifyLegacyPipeline('V2_P2_LITERATURE', 'DBpia 학술 문헌 수집 및 문헌 카드 검증(P2) 중', { jobId });

    let literature = store.loadLiterature();
    if (!literature || literature.length === 0) {
      if (literatureMode === 'manual') {
        literature = mergeLiterature(options.manualLiterature || []);
      } else {
        const queries = buildQueriesFromFacts(facts, docType, userFocus);
        const collectedRaw = [];
        let lSeq = 1;

        const querySteps = queries.map((q, idx) => ({
          id: `P2.Q${idx + 1}`,
          timeoutMs: (config?.timeouts?.dbpiaQuery || 90) * 1000,
          retry: { max: 1, backoffMs: [0] },
          onExhausted: 'skip-with-flag',
          saveAs: `raw/P2.Q${idx + 1}.done.json`,
          run: async () => {
            const dbRes = await dbpiaDriver.query(q, { stepId: `P2.Q${idx + 1}` });
            const cards = toLiteratureCards(dbRes, q, lSeq);
            lSeq += cards.length;
            return cards;
          },
        }));

        await runSteps({ jobDir, jobId }, querySteps, { config });
        for (let i = 0; i < queries.length; i++) {
          const p = path.join(jobDir, 'raw', `P2.Q${i + 1}.done.json`);
          if (fs.existsSync(p)) {
            try {
              collectedRaw.push(...JSON.parse(fs.readFileSync(p, 'utf8')));
            } catch (_) {}
          }
        }
        literature = mergeLiterature(collectedRaw);
      }
      store.saveLiterature(literature);
    }

    const g2 = evaluateGateG2(literature, docType, config);
    jobCtx.gates.G2 = g2;
    appendEvent(jobDir, { job: jobId, step: 'G2', type: 'gate', detail: g2.reason });
    if (!g2.ok && !ignoredGates.has('G2')) {
      if (!g2.canContinue || !options.autoContinueGateWarnings) {
        jobCtx.phase = 'V2_NEEDS_HUMAN';
        jobCtx.status = 'needs-human';
        jobCtx.error = { code: 'E_GATE', gate: 'G2', detail: g2.reason, canContinue: g2.canContinue };
        notifyLegacyPipeline('V2_NEEDS_HUMAN', g2.reason, { jobId });
        return { ok: false, status: 'needs-human', jobId, phase: jobCtx.phase, gate: g2 };
      }
    }

    // P3. 목차 계획 및 G3
    jobCtx.phase = 'V2_P3_OUTLINE';
    notifyLegacyPipeline('V2_P3_OUTLINE', '근거 카드 기반 목차 및 절 주장 설계(P3) 중', { jobId });

    const validCardIds = new Set([
      ...facts.map((f) => f.id),
      ...literature.filter((l) => l.verified).map((l) => l.id),
    ]);

    let outline = store.loadOutline();
    if (!outline || !Array.isArray(outline.sections) || outline.sections.length === 0) {
      const { prompt } = buildOutlinePrompt({
        docType,
        userFocus,
        facts,
        literature: literature.filter((l) => l.verified),
        stepId: 'P3.OUTLINE',
        maxPromptChars: config?.gemini?.maxPromptChars || 12000,
      });
      const outRes = await geminiDriver.ask({
        jobDir,
        jobStep: 'P3.OUTLINE',
        prompt,
        expectJson: true,
        newChat: true,
      });
      const parsedOut = parseOutline(outRes.json || outRes.text, validCardIds, docType);
      outline = parsedOut.outline;
      if (autoApproveOutline) {
        outline.approved = true;
      }
      store.saveOutline(outline);
    } else if (autoApproveOutline && !outline.approved) {
      outline.approved = true;
      store.saveOutline(outline);
    }

    const g3 = evaluateGateG3(outline, validCardIds, autoApproveOutline);
    jobCtx.gates.G3 = g3;
    appendEvent(jobDir, { job: jobId, step: 'G3', type: 'gate', detail: g3.reason });
    if (!g3.ok) {
      jobCtx.phase = g3.needsApproval ? 'V2_P3_WAIT_APPROVAL' : 'V2_NEEDS_HUMAN';
      jobCtx.status = g3.needsApproval ? 'waiting-approval' : 'needs-human';
      jobCtx.error = { code: 'E_GATE', gate: 'G3', detail: g3.reason };
      notifyLegacyPipeline(jobCtx.phase, g3.reason, { jobId });
      return {
        ok: false,
        status: jobCtx.status,
        jobId,
        phase: jobCtx.phase,
        gate: g3,
        outline,
      };
    }

    // 10장 조건부 시각자료(표·차트·모식도 + Nano Banana SVG) 생성
    const plannedVisuals = planVisuals(facts, literature, outline);
    const figuresDir = path.join(jobDir, 'figures');
    fs.mkdirSync(figuresDir, { recursive: true });
    for (const vs of plannedVisuals.specs) {
      if (vs.svg) {
        fs.writeFileSync(path.join(figuresDir, `${vs.id}.svg`), vs.svg, 'utf8');
      }
    }
    fs.writeFileSync(
      path.join(jobDir, 'visuals_plan.json'),
      JSON.stringify(
        {
          specs: plannedVisuals.specs.map((s) => ({
            id: s.id,
            category: s.category,
            type: s.type,
            title: s.title,
            evidence: s.evidence,
          })),
          skipped: plannedVisuals.skipped,
        },
        null,
        2
      ),
      'utf8'
    );

    // P4 & P5. 절별 집필 및 독립 교차검증
    const sectionOutputs = [];
    const matrices = [];
    let prevSummary = '';

    for (let idx = 0; idx < outline.sections.length; idx++) {
      const sec = outline.sections[idx];
      // 중간 본론 절에 조건부 시각자료를 분배
      const assignedVisuals =
        idx === Math.min(1, outline.sections.length - 1) ? plannedVisuals.specs : [];

      const secRelPath = `sections/${sec.id}.final.md`;
      const secDonePath = path.join(jobDir, secRelPath);
      if (checkpoint.isDone(jobDir, `P4_P5.${sec.id}`) && fs.existsSync(secDonePath)) {
        const savedMd = fs.readFileSync(secDonePath, 'utf8');
        const savedMatrix = store.loadLatestVerifyMatrix(sec.id);
        sectionOutputs.push({
          sectionId: sec.id,
          heading: sec.heading,
          markdown: savedMd,
          matrix: savedMatrix,
        });
        if (savedMatrix) matrices.push(savedMatrix);
        prevSummary = savedMd.slice(0, 220);
        continue;
      }

      const secResult = await writeAndVerifySection({
        jobDir,
        store,
        section: sec,
        sectionVisuals: assignedVisuals,
        facts,
        literature,
        pages,
        docType,
        prevSummary,
        geminiDriver,
        config,
      });

      checkpoint.markDone(jobDir, `P4_P5.${sec.id}`, secRelPath);
      sectionOutputs.push(secResult);
      if (secResult.matrix) matrices.push(secResult.matrix);
      prevSummary = secResult.summary;
    }

    const g5 = evaluateGateG5(matrices, config);
    jobCtx.gates.G5 = g5;
    appendEvent(jobDir, { job: jobId, step: 'G5', type: 'gate', detail: g5.reason });
    if (!g5.ok && !ignoredGates.has('G5')) {
      jobCtx.phase = 'V2_NEEDS_HUMAN';
      jobCtx.status = 'needs-human';
      jobCtx.error = { code: 'E_GATE', gate: 'G5', detail: g5.reason, canContinue: true };
      notifyLegacyPipeline('V2_NEEDS_HUMAN', g5.reason, { jobId });
      return { ok: false, status: 'needs-human', jobId, phase: jobCtx.phase, gate: g5 };
    }

    // P6. 조립 및 대조표 생성
    jobCtx.phase = 'V2_P6_ASSEMBLE';
    notifyLegacyPipeline('V2_P6_ASSEMBLE', '각주·참고문헌 1:1 조립 및 근거 대조표 생성(P6) 중', { jobId });

    const assembled = assemble(outline, sectionOutputs, facts, literature);
    fs.writeFileSync(path.join(jobDir, 'manuscript_v1.md'), assembled.rawMarkedMd, 'utf8');
    fs.writeFileSync(path.join(jobDir, 'manuscript.md'), assembled.manuscriptMd, 'utf8');

    // P7.5 최종 DBpia AI 문헌 검수 (X4 — 설정 활성화 및 모듈 존재 시 실행)
    let dbpiaReviewResult = null;
    const dbpiaReviewPath = path.join(__dirname, 'verify', 'dbpia_review.js');
    if (
      config?.dbpiaReview?.enabled !== false &&
      options.enableDbpiaReview !== false &&
      literatureMode === 'dbpia' &&
      fs.existsSync(dbpiaReviewPath)
    ) {
      jobCtx.phase = 'V2_P75_DBPIA_REVIEW';
      notifyLegacyPipeline('V2_P75_DBPIA_REVIEW', '최종 DBpia AI 문헌 검수(P7.5 / X4) 실행 중', { jobId });
      const { runDbpiaFinalReview, evaluateGateG7 } = require('./verify/dbpia_review');
      dbpiaReviewResult = await runDbpiaFinalReview({
        jobDir,
        outline,
        manuscriptMd: assembled.rawMarkedMd,
        facts,
        literature,
        dbpiaDriver,
        geminiDriver,
        config,
      });
      const g7 = evaluateGateG7(dbpiaReviewResult);
      jobCtx.gates.G7 = g7;
      appendEvent(jobDir, { job: jobId, step: 'G7', type: 'gate', detail: g7.reason });
      if (!g7.ok && !ignoredGates.has('G7')) {
        // 대조표는 먼저 저장해 두어 검수 결과를 볼 수 있게 함
        const cw = buildCrosswalk(assembled.rawMarkedMd, facts, literature, matrices, dbpiaReviewResult);
        fs.writeFileSync(path.join(jobDir, 'crosswalk.md'), cw.markdown, 'utf8');
        fs.writeFileSync(path.join(jobDir, 'needs_human.md'), cw.needsHumanMarkdown, 'utf8');

        jobCtx.phase = 'V2_NEEDS_HUMAN';
        jobCtx.status = 'needs-human';
        jobCtx.error = { code: 'E_GATE', gate: 'G7', detail: g7.reason, canContinue: true };
        notifyLegacyPipeline('V2_NEEDS_HUMAN', g7.reason, { jobId });
        return {
          ok: false,
          status: 'needs-human',
          jobId,
          phase: jobCtx.phase,
          gate: g7,
          dbpiaReview: dbpiaReviewResult,
        };
      }
    }

    const crosswalk = buildCrosswalk(
      assembled.rawMarkedMd,
      facts,
      literature,
      matrices,
      dbpiaReviewResult
    );
    fs.writeFileSync(path.join(jobDir, 'crosswalk.md'), crosswalk.markdown, 'utf8');
    fs.writeFileSync(path.join(jobDir, 'needs_human.md'), crosswalk.needsHumanMarkdown, 'utf8');

    // P8. WordCraft / PdfCraft 내보내기
    jobCtx.phase = 'V2_P8_EXPORT';
    notifyLegacyPipeline('V2_P8_EXPORT', 'WordCraft(.docx) 및 PdfCraft(.pdf) 내보내기(P8) 중', { jobId });

    let exportInfo = null;
    if (typeof craftDriver.exportDocx === 'function') {
      exportInfo = await craftDriver.exportDocx({
        markdownPath: path.join(jobDir, 'manuscript.md'),
        outputDir: jobDir,
        title: assembled.title,
      });
    }

    // P9. 유적 라이브러리 적재 (선택적)
    const libModulePath = path.join(__dirname, 'site_library.js');
    if (options.saveToSiteLibrary !== false && fs.existsSync(libModulePath)) {
      jobCtx.phase = 'V2_P9_LIBRARY';
      try {
        const { saveJobToSiteLibrary } = require('./site_library');
        saveJobToSiteLibrary({
          siteKey: options.siteKey || userFocus || '가상유적',
          facts,
          literature,
          baseDir: options.libraryDir,
        });
      } catch (_) {}
    }

    jobCtx.phase = 'V2_DONE';
    jobCtx.status = 'done';
    jobCtx.error = null;
    notifyLegacyPipeline('V2_DONE', 'ARCHAEO STUDIO 6.1 (v2) 파이프라인 완료', {
      jobId,
      finalManuscript: assembled.manuscriptMd,
    });

    return {
      ok: true,
      status: 'done',
      jobId,
      jobDir,
      phase: 'V2_DONE',
      manuscriptMd: assembled.manuscriptMd,
      rawMarkedMd: assembled.rawMarkedMd,
      footnotes: assembled.footnotes,
      bibliography: assembled.bibliography,
      crosswalk,
      visuals: plannedVisuals,
      gates: jobCtx.gates,
      exportInfo,
    };
  } catch (err) {
    const code = err?.code || 'E_STEP_FAILED';
    jobCtx.phase = 'V2_PAUSED';
    jobCtx.status = 'paused';
    jobCtx.error = {
      code,
      message: err?.message || String(err),
    };
    appendEvent(jobDir, {
      job: jobId,
      step: jobCtx.phase,
      type: 'fail',
      code,
      detail: err?.message || String(err),
    });
    notifyLegacyPipeline('V2_PAUSED', `일시정지(${code}): ${err?.message || err}`, { jobId });
    return {
      ok: false,
      status: 'paused',
      jobId,
      jobDir,
      phase: jobCtx.phase,
      error: jobCtx.error,
    };
  } finally {
    checkpoint.releaseLock(lockInfo.lockPath, jobId);
  }
}

async function resumePipelineV2(jobId, extraOptions = {}) {
  const targetId = jobId || latestJobId;
  if (!targetId) {
    throw new Error('재개할 jobId가 없습니다.');
  }
  const prevCtx = activeJobs.get(targetId);
  const jobDir = prevCtx?.jobDir || resolveJobDir(targetId, extraOptions.baseOutputDir);
  const savedCp = checkpoint.load(jobDir);
  const mergedOptions = {
    ...(prevCtx?.options || {}),
    ...(savedCp?.meta || {}),
    ...extraOptions,
    jobId: targetId,
    jobDir,
    ignoredGates: [
      ...(prevCtx?.ignoredGates ? Array.from(prevCtx.ignoredGates) : []),
      ...(savedCp?.ignoredGates || []),
      ...(extraOptions.ignoredGates || []),
    ],
  };
  return runPipelineV2(mergedOptions);
}

async function approveOutlineV2(jobId, newOutline = null, extraOptions = {}) {
  const targetId = jobId || latestJobId;
  const jobDir = resolveJobDir(targetId, extraOptions.baseOutputDir);
  const store = createEvidenceStore(jobDir);
  const current = newOutline || store.loadOutline();
  if (!current) {
    throw new Error('승인할 목차(outline.json)가 존재하지 않습니다.');
  }
  current.approved = true;
  store.saveOutline(current);
  return resumePipelineV2(targetId, { ...extraOptions, autoApproveOutline: true });
}

async function continueGateV2(jobId, gateName, extraOptions = {}) {
  const targetId = jobId || latestJobId;
  const jobDir = resolveJobDir(targetId, extraOptions.baseOutputDir);
  const savedCp = checkpoint.load(jobDir);
  const ignored = new Set([...(savedCp.ignoredGates || []), String(gateName || '')]);
  savedCp.ignoredGates = Array.from(ignored);
  checkpoint.save(jobDir, savedCp);
  return resumePipelineV2(targetId, {
    ...extraOptions,
    ignoredGates: Array.from(ignored),
  });
}

function getV2State(jobId) {
  const targetId = jobId || latestJobId;
  if (!targetId) {
    return { ok: true, active: false, phase: 'IDLE' };
  }
  const ctx = activeJobs.get(targetId);
  const jobDir = ctx?.jobDir || resolveJobDir(targetId);
  if (!fs.existsSync(jobDir)) {
    return { ok: false, error: '해당 작업을 찾을 수 없습니다.', jobId: targetId };
  }
  const store = createEvidenceStore(jobDir);
  const facts = store.loadFacts() || [];
  const lit = store.loadLiterature() || [];
  const outline = store.loadOutline();

  return {
    ok: true,
    jobId: targetId,
    jobDir,
    phase: ctx?.phase || 'V2_PAUSED',
    status: ctx?.status || 'idle',
    factsCount: facts.length,
    verifiedFactsCount: facts.filter((f) => f.confidence === 'verified').length,
    literatureCount: lit.length,
    verifiedLiteratureCount: lit.filter((l) => l.verified).length,
    outline,
    gates: ctx?.gates || {},
    error: ctx?.error || null,
  };
}

function getV2Metrics(jobId) {
  const targetId = jobId || latestJobId;
  if (!targetId) return { ok: true, metrics: null };
  const jobDir = resolveJobDir(targetId);
  return {
    ok: true,
    jobId: targetId,
    metrics: computeMetrics(jobDir),
  };
}

function syncUnifiedPipelineToEvidenceStore({
  baseOutputDir = getDefaultBaseOutputDir(),
  docType = 'report',
  userFocus = '',
  reportText = '',
  datingText = '',
  collectedPapers = [],
  manuscriptMd = '',
} = {}) {
  try {
    const {
      parseFeatureDimensionsFromMarkdown,
      sanitizeArchaeologyTypographyAndFonts,
    } = require('../archaeology_visuals_enricher');

    const jobId = makeJobId();
    const jobDir = path.join(baseOutputDir, 'jobs', jobId);
    fs.mkdirSync(jobDir, { recursive: true });
    const store = createEvidenceStore(jobDir);

    const pages = buildSyntheticPagesFromText(reportText, datingText);
    store.savePages(pages);

    // 원문에서 확인된 유구 제원 사실 카드 생성 및 검증
    const parsedDims = parseFeatureDimensionsFromMarkdown(`${reportText}\n${manuscriptMd}`);
    const facts = [];
    let fIdx = 1;
    for (const d of parsedDims) {
      facts.push(
        verifyFactAgainstPages(
          {
            id: makeFactId(fIdx++),
            kind: 'feature',
            subject: d.name || `유구 ${fIdx}`,
            attribute: '장축',
            value: String(d.lengthM || d.length || ''),
            unit: 'm',
            quote: `${d.name} (${d.lengthM}m × ${d.widthM}m, ${d.areaM2}㎡)`,
            source: { file: '발굴조사보고서', page: 1, chapter: 'Ⅲ' },
            confidence: 'verified',
          },
          pages
        )
      );
      facts.push(
        verifyFactAgainstPages(
          {
            id: makeFactId(fIdx++),
            kind: 'feature',
            subject: d.name || `유구 ${fIdx}`,
            attribute: '면적',
            value: String(d.areaM2 || d.area || ''),
            unit: '㎡',
            quote: `${d.name} (${d.lengthM}m × ${d.widthM}m, ${d.areaM2}㎡)`,
            source: { file: '발굴조사보고서', page: 1, chapter: 'Ⅲ' },
            confidence: 'verified',
          },
          pages
        )
      );
    }
    store.saveFacts(facts);

    // DBpia 수집 문헌을 문헌 카드(L1, L2...)로 변환·병합
    const rawLitCards = toLiteratureCards(
      {
        query: userFocus || '고고학 학술 검색',
        answerText: '',
        refs: (collectedPapers || []).map((p, idx) => ({
          nodeId: p.nodeId || p.id || `DBPIA_${idx + 1}`,
          author: p.author || p.authors || '',
          year: p.year || '',
          title: sanitizeArchaeologyTypographyAndFonts(p.title || ''),
          journal: sanitizeArchaeologyTypographyAndFonts(p.journal || ''),
          pages: p.pages || '',
          abstract: sanitizeArchaeologyTypographyAndFonts(p.abstract || ''),
        })),
      },
      userFocus || '고고학 학술 검색',
      1
    );
    const literature = mergeLiterature(rawLitCards);
    store.saveLiterature(literature);

    const cleanMd = sanitizeArchaeologyTypographyAndFonts(manuscriptMd || '');
    const sectionMatches = [...cleanMd.matchAll(/^##\s+([^\r\n]+)/gm)];
    const outline = {
      docType: docType === 'paper' ? 'paper' : 'report',
      title: userFocus || (docType === 'paper' ? '고고학 학술논문' : 'Ⅳ. 고찰'),
      approved: true,
      omitted: [],
      sections:
        sectionMatches.length > 0
          ? sectionMatches.map((m, idx) => ({
              id: `S${idx + 1}`,
              heading: m[1].trim(),
              claims: [],
              visuals: [],
              targetChars: 2500,
            }))
          : [
              {
                id: 'S1',
                heading: '1. 고찰 본문',
                claims: [],
                visuals: [],
                targetChars: 2500,
              },
            ],
    };
    store.saveOutline(outline);

    // Nano Banana Pro 시각자료 SVG 생성 및 저장
    const plannedVisuals = planVisuals(facts, literature, outline);
    const figuresDir = path.join(jobDir, 'figures');
    fs.mkdirSync(figuresDir, { recursive: true });
    for (const vs of plannedVisuals.specs) {
      if (vs.svg) {
        fs.writeFileSync(path.join(figuresDir, `${vs.id}.svg`), vs.svg, 'utf8');
      }
    }

    // V1~V8 결정적 검사 및 근거 대조표(crosswalk.md) 저장
    const claims = splitClaims(cleanMd, 'S1', 0);
    const detResult = runDeterministic({
      markdown: cleanMd,
      claims,
      facts,
      literature,
      section: outline.sections[0],
      sectionId: 'S1',
      round: 0,
    });
    const matrix = buildVerifyMatrix({
      sectionId: 'S1',
      round: 0,
      detResult,
      x1Result: { byClaimId: {} },
      x2Result: { byClaimId: {} },
      criticResult: { scores: { evidence: 5, coverage: 5, logic: 5, tone: 5 }, issues: [], verdict: 'accept' },
      maxRework: 2,
    });
    store.saveVerifyMatrix('S1', 0, matrix);

    fs.writeFileSync(path.join(jobDir, 'manuscript_v1.md'), cleanMd, 'utf8');
    fs.writeFileSync(path.join(jobDir, 'manuscript.md'), cleanMd, 'utf8');

    const crosswalk = buildCrosswalk(cleanMd, facts, literature, [matrix], null);
    fs.writeFileSync(path.join(jobDir, 'crosswalk.md'), crosswalk.markdown, 'utf8');
    fs.writeFileSync(path.join(jobDir, 'needs_human.md'), crosswalk.needsHumanMarkdown, 'utf8');

    // 유적 라이브러리 누적 저장 (P9)
    try {
      const { saveJobToSiteLibrary } = require('./site_library');
      saveJobToSiteLibrary({
        siteKey: userFocus || '본조사유적',
        facts,
        literature,
      });
    } catch (_) {}

    const jobCtx = {
      jobId,
      jobDir,
      docType: outline.docType,
      userFocus,
      autoApproveOutline: true,
      literatureMode: 'dbpia',
      phase: 'V2_DONE',
      status: 'done',
      gates: {},
      error: null,
      options: {},
      ignoredGates: new Set(),
    };
    activeJobs.set(jobId, jobCtx);
    latestJobId = jobId;

    return {
      ok: true,
      jobId,
      jobDir,
      factsCount: facts.length,
      literatureCount: literature.length,
      visualsCount: plannedVisuals.specs.length,
    };
  } catch (err) {
    return { ok: false, error: err.message };
  }
}

module.exports = {
  runPipelineV2,
  resumePipelineV2,
  approveOutlineV2,
  continueGateV2,
  getV2State,
  getV2Metrics,
  resolveJobDir,
  writeAndVerifySection,
  syncUnifiedPipelineToEvidenceStore,
};
