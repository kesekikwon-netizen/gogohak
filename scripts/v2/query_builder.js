'use strict';

/**
 * 사실 카드(FactCard[])와 사용자 초점(userFocus)으로부터
 * DBpia AI 학술 검색 질의를 최대 maxQueries(기본 8)개 생성한다.
 * - 특정 유적명·지역명을 코드에 하드코딩하지 않으며, 오직 입력된 카드와 userFocus에서만 용어를 추출한다.
 * - 부록 A의 기존 buildMultiFacetedDbpiaQueries도 함께 활용하여 최대 8개까지 중복 없이 반환한다.
 */
function buildQueriesFromFacts(facts = [], docType = 'report', userFocus = '', options = {}) {
  const maxQueries = Number.isInteger(options.maxQueries) ? options.maxQueries : 8;
  const validFacts = Array.isArray(facts) ? facts.filter((f) => f && typeof f === 'object') : [];

  const features = new Set();
  const artifacts = new Set();
  const periods = new Set();
  const sites = new Set();
  const regions = new Set();

  for (const f of validFacts) {
    const subj = String(f.subject || '').trim();
    const attr = String(f.attribute || '').trim();
    const val = String(f.value || '').trim();
    const quote = String(f.quote || '').trim();

    if (f.kind === 'site') {
      if (val && /유적/.test(val)) sites.add(val);
      else if (subj && /유적/.test(subj)) sites.add(subj);
      const regMatch = (val || subj || quote).match(/([가-힣]{2,5}(?:시|군|구|도|지역|권역))/);
      if (regMatch) regions.add(regMatch[1]);
    } else if (f.kind === 'feature') {
      const cleaned = subj.replace(/^\d+호\s*/, '').replace(/^제?\d+\s*/, '').trim();
      if (cleaned) features.add(cleaned);
      if (/평면|카마도|부뚜막|주공|구들|아궁이|배수구|벽구|주구|석실|석곽|토광|주거지/.test(`${attr} ${val}`)) {
        const kw = `${attr} ${val}`.match(/(철자형|凸자형|呂자형|방형|장방형|원형|부뚜막|카마도|구들|아궁이|주구|석실|석곽|토광묘|주거지|수혈)/g);
        if (kw) kw.forEach((k) => features.add(k));
      }
    } else if (f.kind === 'artifact') {
      const cleaned = subj.replace(/^\d+호\s*/, '').trim();
      if (cleaned && !/주거지|수혈|구상유구/.test(cleaned)) artifacts.add(cleaned);
      if (val && !/^\d+(\.\d+)?$/.test(val)) {
        val
          .split(/[,·/]+/)
          .map((s) => s.trim())
          .filter((s) => s.length >= 2 && !/^\d+$/.test(s))
          .forEach((s) => artifacts.add(s));
      }
    } else if (f.kind === 'chronology' || f.kind === 'dating') {
      if (val && !/^\d+(\.\d+)?$/.test(val)) {
        periods.add(val);
      }
    }

    const periodMatches = `${subj} ${attr} ${val} ${quote}`.match(
      /(구석기|신석기|청동기|초기철기|원삼국|삼국시대|고구려|백제|신라|가야|통일신라|남북국|고려|조선|전기|중기|후기|\d+세기)/g
    );
    if (periodMatches) {
      periodMatches.forEach((p) => periods.add(p));
    }
  }

  const featList = Array.from(features).slice(0, 4);
  const artList = Array.from(artifacts).slice(0, 4);
  const periodList = Array.from(periods).slice(0, 3);
  const regionList = Array.from(regions).slice(0, 2);
  const siteList = Array.from(sites).slice(0, 1);

  const scopePrefix = [
    ...regionList,
    ...periodList,
  ]
    .filter(Boolean)
    .join(' ')
    .trim();

  const queries = [];
  const pushUnique = (q) => {
    const clean = String(q || '').replace(/\s+/g, ' ').trim();
    if (clean.length >= 4 && !queries.includes(clean)) {
      queries.push(clean);
    }
  };

  const focusClean = String(userFocus || '').replace(/\s+/g, ' ').trim();
  if (focusClean) {
    pushUnique(`${focusClean} 구조적 특징 및 편년 연구`);
  }

  if (featList.length > 0) {
    pushUnique(`${scopePrefix} ${featList.slice(0, 2).join(' ')} 평면 형태와 축조 방식 연구`.trim());
    pushUnique(`${scopePrefix} ${featList.join(' ')} 내부시설 구조와 지역적 비교`.trim());
  }

  if (artList.length > 0) {
    pushUnique(`${scopePrefix} ${artList.slice(0, 3).join(' ')} 형식분류와 상대편년 연구`.trim());
  }

  if (periodList.length > 0 && (featList.length > 0 || artList.length > 0)) {
    pushUnique(`${periodList.join(' ')} ${[...featList.slice(0, 1), ...artList.slice(0, 2)].join(' ')} 절대연대 및 단계 변천`.trim());
  }

  if (docType === 'paper') {
    const topicCore = [scopePrefix, ...featList.slice(0, 2), ...artList.slice(0, 2)].filter(Boolean).join(' ') || focusClean;
    if (topicCore) {
      pushUnique(`${topicCore} 선행연구 쟁점 및 학사적 검토`);
      pushUnique(`${topicCore} 계통과 사회문화적 의의`);
    }
  } else {
    const compCore = [...regionList, ...periodList, ...featList.slice(0, 2)].filter(Boolean).join(' ') || focusClean;
    if (compCore) {
      pushUnique(`${compCore} 주변 비교유적 취락 분포와 성격 고찰`);
    }
  }

  // 기존 buildMultiFacetedDbpiaQueries 래핑(사용 가능한 경우 보조 질의 병합)
  try {
    const server = require('../gemini_mcp_server');
    if (typeof server.buildMultiFacetedDbpiaQueries === 'function' && (validFacts.length > 0 || focusClean)) {
      const syntheticReport = validFacts
        .map((f) => `${f.subject || ''} ${f.attribute || ''} ${f.value || ''}${f.unit || ''} ${f.quote || ''}`)
        .join('\n');
      const legacyQueries = server.buildMultiFacetedDbpiaQueries({
        userFocus: focusClean || siteList[0] || '',
        reportText: syntheticReport,
        docType,
        round: 1,
      });
      if (Array.isArray(legacyQueries)) {
        for (const lq of legacyQueries) {
          if (queries.length >= maxQueries) break;
          pushUnique(lq);
        }
      }
    }
  } catch (_) {
    // 순수 단위 테스트 시 보조 호출 실패 무시
  }

  if (queries.length === 0 && siteList.length > 0) {
    pushUnique(`${siteList[0]} 유구 및 출토유물 고고학적 성격`);
  }

  return queries.slice(0, maxQueries);
}

module.exports = {
  buildQueriesFromFacts,
};
