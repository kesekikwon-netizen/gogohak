'use strict';

const {
  normalizeWcDiagramSpec,
  renderNanoBananaArchaeologySvg,
  NANO_BANANA_ARCHAEO_THEME,
} = require('../archaeology_visuals_enricher');

function parseNumericValue(val) {
  const m = String(val || '').replace(/,/g, '').match(/-?\d+(?:\.\d+)?/);
  return m ? Number(m[0]) : null;
}

/**
 * 10장 표·차트·모식도 규칙:
 * 개수 목표 대신 조건으로 만든다. 못 채우면 만들지 않고 skipped에 이유를 남긴다.
 * 모든 값 칸과 차트·모식도 블록은 근거 카드 번호(evidence / ⟦F...⟧ / ⟦L...⟧)를 가진다(V8).
 * 디자인은 나노바나나 최신버전 고고학 전문가 수준(Nano Banana Pro Archaeology Editorial Design)을 적용한다.
 */
function planVisuals(facts = [], lit = [], outline = null) {
  const validFacts = (facts || []).filter((f) => f && typeof f === 'object');
  const verifiedLit = (lit || []).filter((l) => l && l.verified === true);

  const specs = [];
  const skipped = [];

  // 1. 유구 제원 일람표: 크기 값이 있는 유구 카드 2개 이상 (출처: F)
  const featureSizeCards = validFacts.filter(
    (f) =>
      f.kind === 'feature' &&
      (parseNumericValue(f.value) !== null || /방형|장방형|원형|타원형|철자형|凸자형|呂자형/.test(String(f.value || '')))
  );
  if (featureSizeCards.length >= 2) {
    const bySubject = new Map();
    for (const fc of featureSizeCards) {
      const subj = String(fc.subject || '유구').trim();
      if (!bySubject.has(subj)) {
        bySubject.set(subj, { subject: subj, attrs: [], ids: [] });
      }
      const entry = bySubject.get(subj);
      entry.attrs.push(`${fc.attribute}: ${fc.value}${fc.unit || ''} ⟦${fc.id}⟧`);
      if (!entry.ids.includes(fc.id)) entry.ids.push(fc.id);
    }

    const rows = Array.from(bySubject.values()).map(
      (item, idx) =>
        `| ${idx + 1} | **${item.subject}** | ${item.attrs.join(', ')} | ${item.ids.map((id) => `⟦${id}⟧`).join(' ')} |`
    );
    const markdown = [
      '**<표> 확인 유구 제원 및 구조 속성 일람표**',
      '',
      '| 연번 | 유구명 | 측정 제원 및 구조 속성 | 근거 카드 |',
      '| :---: | :--- | :--- | :---: |',
      ...rows,
    ].join('\n');

    specs.push({
      id: 'vis_feature_table',
      category: 'feature_table',
      type: 'table',
      title: '확인 유구 제원 및 구조 속성 일람표',
      evidence: featureSizeCards.map((c) => c.id),
      markdown,
    });
  } else {
    skipped.push({
      category: 'feature_table',
      title: '유구 제원 일람표',
      reason: `크기·형태 값이 있는 유구 카드가 ${featureSizeCards.length}개로 기준(2개 이상) 미달`,
    });
  }

  // 2. 유물 속성 비교표: 유물 카드 3개 이상 (출처: F)
  const artifactCards = validFacts.filter((f) => f.kind === 'artifact');
  if (artifactCards.length >= 3) {
    const rows = artifactCards.map(
      (ac, idx) =>
        `| ${idx + 1} | **${ac.subject}** | ${ac.attribute} | ${ac.value}${ac.unit || ''} ⟦${ac.id}⟧ | p.${ac.source?.page ?? '미상'} ⟦${ac.id}⟧ |`
    );
    const markdown = [
      '**<표> 출토유물 기종별 속성 및 수량 비교표**',
      '',
      '| 연번 | 출토 맥락 / 기종 | 속성 구분 | 확인 내용 및 수량 | 출처 쪽수(근거) |',
      '| :---: | :--- | :--- | :--- | :---: |',
      ...rows,
    ].join('\n');

    specs.push({
      id: 'vis_artifact_table',
      category: 'artifact_table',
      type: 'table',
      title: '출토유물 기종별 속성 및 수량 비교표',
      evidence: artifactCards.map((c) => c.id),
      markdown,
    });
  } else {
    skipped.push({
      category: 'artifact_table',
      title: '유물 속성 비교표',
      reason: `유물(artifact) 카드가 ${artifactCards.length}개로 기준(3개 이상) 미달`,
    });
  }

  // 3. 주변 유적 대비표: 다른 유적/지역을 다룬 L 카드 2개 이상 (출처: L)
  const comparativeLit = verifiedLit.filter((l) =>
    /유적|지역|유역|권역|분포|취락|고분|비교|계통/.test(`${l.title || ''} ${l.summary || ''} ${(l.quotes || []).join(' ')}`)
  );
  if (comparativeLit.length >= 2) {
    const rows = comparativeLit.map((lc, idx) => {
      const authorYear = `${(lc.authors || []).join('·')}(${lc.year})`;
      const summaryClean = String(lc.summary || lc.quotes?.[0] || lc.title).replace(/\|/g, '/').slice(0, 80);
      return `| ${idx + 1} | **${authorYear}** | 「${lc.title}」 | ${summaryClean} ⟦${lc.id}⟧ |`;
    });
    const markdown = [
      '**<표> 주변 유적 및 동일 문화권 비교 연구 대비표**',
      '',
      '| 연번 | 연구자(연도) | 비교 연구 논문명 | 핵심 비교 소견 및 근거 |',
      '| :---: | :--- | :--- | :--- |',
      ...rows,
    ].join('\n');

    specs.push({
      id: 'vis_comparative_site_table',
      category: 'comparative_site_table',
      type: 'table',
      title: '주변 유적 및 동일 문화권 비교 연구 대비표',
      evidence: comparativeLit.map((c) => c.id),
      markdown,
    });
  } else {
    skipped.push({
      category: 'comparative_site_table',
      title: '주변 유적 대비표',
      reason: `주변 유적·지역 비교 문헌 카드가 ${comparativeLit.length}개로 기준(2개 이상) 미달`,
    });
  }

  // 4. 선행연구 쟁점표: verified L 카드 3개 이상 (출처: L)
  if (verifiedLit.length >= 3) {
    const rows = verifiedLit.map((lc, idx) => {
      const authorYear = `${(lc.authors || []).join('·')}(${lc.year})`;
      const quoteOrSum = String(lc.quotes?.[0] || lc.summary || lc.title).replace(/\|/g, '/').slice(0, 85);
      return `| ${idx + 1} | **${authorYear}** | 『${lc.journal || '학술지'}』 | ${quoteOrSum} ⟦${lc.id}⟧ |`;
    });
    const markdown = [
      '**<표> 선행연구 핵심 쟁점 및 학술적 논거 종합표**',
      '',
      '| 연번 | 연구자(발행연도) | 수록 학술지 | 핵심 논거 및 인용 근거 |',
      '| :---: | :--- | :--- | :--- |',
      ...rows,
    ].join('\n');

    specs.push({
      id: 'vis_literature_issue_table',
      category: 'literature_issue_table',
      type: 'table',
      title: '선행연구 핵심 쟁점 및 학술적 논거 종합표',
      evidence: verifiedLit.map((c) => c.id),
      markdown,
    });
  } else {
    skipped.push({
      category: 'literature_issue_table',
      title: '선행연구 쟁점표',
      reason: `검증된(verified) 문헌 카드가 ${verifiedLit.length}개로 기준(3개 이상) 미달`,
    });
  }

  // 5. 유구 규모 차트: 같은 속성(또는 길이/면적 등 측정치) 숫자 값 3개 이상 (출처: F)
  const numericFeatureCards = validFacts.filter(
    (f) => f.kind === 'feature' && parseNumericValue(f.value) !== null
  );
  const byAttr = new Map();
  for (const fc of numericFeatureCards) {
    const key = String(fc.attribute || '규모').trim();
    if (!byAttr.has(key)) byAttr.set(key, []);
    byAttr.get(key).push(fc);
  }
  let chosenScaleCards = [];
  let chosenAttrName = '규모';
  for (const [attr, list] of byAttr.entries()) {
    if (list.length >= 3 && list.length > chosenScaleCards.length) {
      chosenScaleCards = list;
      chosenAttrName = attr;
    }
  }
  if (chosenScaleCards.length < 3 && numericFeatureCards.length >= 3) {
    chosenScaleCards = numericFeatureCards.slice(0, 6);
    chosenAttrName = '유구 측정 제원';
  }

  if (chosenScaleCards.length >= 3) {
    const unit = chosenScaleCards[0].unit || 'm';
    const chartSpec = {
      type: 'column',
      title: `확인 유구별 ${chosenAttrName}(${unit}) 정량 비교 [${chosenScaleCards.map((c) => c.id).join(', ')}]`,
      legend: 'bottom',
      dataLabels: true,
      designSystem: NANO_BANANA_ARCHAEO_THEME.id,
      palette: NANO_BANANA_ARCHAEO_THEME.chartPalette,
      evidence: chosenScaleCards.map((c) => c.id),
      data: {
        categories: chosenScaleCards.map((c) => `${c.subject}(${c.id})`),
        series: [
          {
            name: `${chosenAttrName}(${unit})`,
            values: chosenScaleCards.map((c) => parseNumericValue(c.value)),
          },
        ],
      },
    };
    const svg = renderNanoBananaArchaeologySvg({ kind: 'chart', spec: chartSpec });
    specs.push({
      id: 'vis_feature_scale_chart',
      category: 'feature_scale_chart',
      type: 'chart',
      title: chartSpec.title,
      evidence: chartSpec.evidence,
      spec: chartSpec,
      svg,
      markdown: `\`\`\`wc-chart\n${JSON.stringify(chartSpec)}\n\`\`\``,
    });
  } else {
    skipped.push({
      category: 'feature_scale_chart',
      title: '유구 규모 차트',
      reason: `유구 수치 카드가 ${numericFeatureCards.length}개로 기준(3개 이상) 미달`,
    });
  }

  // 6. 연대 차트: 연대(dating) 카드 2개 이상 (없으면 절대 만들지 않음!) (출처: F)
  const datingCards = validFacts.filter(
    (f) => f.kind === 'dating' && parseNumericValue(f.value) !== null
  );
  if (datingCards.length >= 2) {
    const unit = datingCards[0].unit || 'BP';
    const chartSpec = {
      type: 'column',
      title: `절대연대 측정치(${unit}) 분포 비교 [${datingCards.map((c) => c.id).join(', ')}]`,
      legend: 'bottom',
      dataLabels: true,
      designSystem: NANO_BANANA_ARCHAEO_THEME.id,
      palette: NANO_BANANA_ARCHAEO_THEME.datingPalette,
      evidence: datingCards.map((c) => c.id),
      data: {
        categories: datingCards.map((c) => `${c.subject}(${c.id})`),
        series: [
          {
            name: `측정 연대(${unit})`,
            values: datingCards.map((c) => parseNumericValue(c.value)),
          },
        ],
      },
    };
    const svg = renderNanoBananaArchaeologySvg({ kind: 'dating_chart', spec: chartSpec, cards: datingCards });
    specs.push({
      id: 'vis_dating_chart',
      category: 'dating_chart',
      type: 'chart',
      title: chartSpec.title,
      evidence: chartSpec.evidence,
      spec: chartSpec,
      svg,
      markdown: `\`\`\`wc-chart\n${JSON.stringify(chartSpec)}\n\`\`\``,
    });
  } else {
    skipped.push({
      category: 'dating_chart',
      title: '연대 차트',
      reason: `연대(dating) 카드가 ${datingCards.length}개로 기준(2개 이상) 미달 — 생성 금지 규칙 적용`,
    });
  }

  // 7. 층위 모식도: 층위(stratum) 카드 2개 이상 (출처: F)
  const stratumCards = validFacts.filter((f) => f.kind === 'stratum');
  if (stratumCards.length >= 2) {
    const rawItems = stratumCards.flatMap((sc) => [
      { text: `${sc.subject} (${sc.id})`, level: 0 },
      { text: `${sc.attribute}: ${sc.value}${sc.unit || ''}`, level: 1 },
    ]);
    const norm = normalizeWcDiagramSpec({
      layout: 'process',
      colors: 'accent1',
      title: `조사구역 표준 퇴적 층위(Harris Matrix) 모식도 [${stratumCards.map((c) => c.id).join(', ')}]`,
      items: rawItems,
    });
    const diagSpec = {
      ...norm,
      designSystem: NANO_BANANA_ARCHAEO_THEME.id,
      diagramStyle: 'stratigraphy_matrix',
      evidence: stratumCards.map((c) => c.id),
    };
    const svg = renderNanoBananaArchaeologySvg({ kind: 'stratigraphy_diagram', spec: diagSpec, cards: stratumCards });
    specs.push({
      id: 'vis_stratum_diagram',
      category: 'stratum_diagram',
      type: 'diagram',
      title: diagSpec.title,
      evidence: diagSpec.evidence,
      spec: diagSpec,
      svg,
      markdown: `\`\`\`wc-diagram\n${JSON.stringify(diagSpec)}\n\`\`\``,
    });
  } else {
    skipped.push({
      category: 'stratum_diagram',
      title: '층위 모식도',
      reason: `층위(stratum) 카드가 ${stratumCards.length}개로 기준(2개 이상) 미달`,
    });
  }

  // 8. 단계 변천 모식도: 중복관계·편년 단계(chronology) 카드 2개 이상 (출처: F)
  const chronologyCards = validFacts.filter((f) => f.kind === 'chronology');
  if (chronologyCards.length >= 2) {
    const rawItems = chronologyCards.flatMap((cc) => [
      { text: `${cc.subject} (${cc.id})`, level: 0 },
      { text: `${cc.attribute}: ${cc.value}${cc.unit || ''}`, level: 1 },
    ]);
    const norm = normalizeWcDiagramSpec({
      layout: 'process',
      colors: 'accent2',
      title: `유구 중복관계 및 단계별 통시적 편년 변천 모식도 [${chronologyCards.map((c) => c.id).join(', ')}]`,
      items: rawItems,
    });
    const diagSpec = {
      ...norm,
      designSystem: NANO_BANANA_ARCHAEO_THEME.id,
      diagramStyle: 'chronology_phase',
      evidence: chronologyCards.map((c) => c.id),
    };
    const svg = renderNanoBananaArchaeologySvg({ kind: 'chronology_diagram', spec: diagSpec, cards: chronologyCards });
    specs.push({
      id: 'vis_chronology_diagram',
      category: 'chronology_diagram',
      type: 'diagram',
      title: diagSpec.title,
      evidence: diagSpec.evidence,
      spec: diagSpec,
      svg,
      markdown: `\`\`\`wc-diagram\n${JSON.stringify(diagSpec)}\n\`\`\``,
    });
  } else {
    skipped.push({
      category: 'chronology_diagram',
      title: '단계 변천 모식도',
      reason: `편년·단계(chronology) 카드가 ${chronologyCards.length}개로 기준(2개 이상) 미달`,
    });
  }

  return {
    specs,
    skipped,
  };
}

module.exports = {
  planVisuals,
};
