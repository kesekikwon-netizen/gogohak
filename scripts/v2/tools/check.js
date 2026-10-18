#!/usr/bin/env node
'use strict';

const fs = require('node:fs');
const path = require('node:path');
const { execSync, spawnSync } = require('node:child_process');

const ROOT_DIR = path.resolve(__dirname, '..', '..', '..');
const BASE_COMMIT = '9c54e8f';

const PROTECTED_PREFIXES = ['bin/', 'patches/', 'CLAUDE.md', '.claude/', 'factory/'];

const FORBIDDEN_RULES = [
  {
    id: 'FORBIDDEN_GOOGLE_WORKSPACE',
    regex: new RegExp(
      ['@' + 'Google\\s*Workspace', '@' + 'Gmail', '@' + 'Google\\s*Drive'].join('|'),
      'i'
    ),
    reason: '변경된 코드/프롬프트에 금지된 워크스페이스/메일/드라이브 멘션 키워드가 포함되어 있습니다.',
    fix: '프롬프트 및 코드에서 해당 멘션 키워드를 제거하십시오 (명세서 0.1-4).',
  },
  {
    id: 'FORBIDDEN_INNER_HTML',
    regex: new RegExp('innerHTML' + '\\s*=', 'g'),
    reason: '변경된 코드에 금지된 innerHTML 직접 대입이 포함되어 있습니다.',
    fix: 'textContent, DOM API 또는 injectTextIntoGeminiEditor() 경로를 사용하십시오 (명세서 0.1-5).',
  },
  {
    id: 'FORBIDDEN_USER_PATH',
    regex: new RegExp(['C:\\\\' + 'Users\\\\', '/' + 'home/'].join('|'), 'i'),
    reason: '변경된 파일에 사용자 절대경로가 하드코딩되어 있습니다.',
    fix: 'os.homedir(), __dirname, %USERPROFILE%, %~dp0 동적 경로로 교체하십시오 (명세서 0.1-2).',
  },
];

function failOneLine(ruleOrStep, reason, fix) {
  console.error(`[v6-check 실패: ${ruleOrStep}] 이유: ${reason} | 고칠 방법: ${fix}`);
  process.exit(1);
}

function getChangedFiles(isStaged) {
  const files = new Set();
  try {
    if (isStaged) {
      const out = execSync('git diff --cached --name-only --diff-filter=ACMR', {
        cwd: ROOT_DIR,
        encoding: 'utf8',
      });
      out
        .split(/\r?\n/)
        .map((s) => s.trim())
        .filter(Boolean)
        .forEach((f) => files.add(f.replace(/\\/g, '/')));
    } else {
      let baseRef = BASE_COMMIT;
      try {
        execSync(`git cat-file -e ${BASE_COMMIT}^{commit}`, { cwd: ROOT_DIR, stdio: 'ignore' });
      } catch (_) {
        baseRef = 'HEAD~1';
      }
      const diffOut = execSync(`git diff --name-only --diff-filter=ACMR ${baseRef}..HEAD`, {
        cwd: ROOT_DIR,
        encoding: 'utf8',
      });
      diffOut
        .split(/\r?\n/)
        .map((s) => s.trim())
        .filter(Boolean)
        .forEach((f) => files.add(f.replace(/\\/g, '/')));

      const statusOut = execSync('git status --porcelain', {
        cwd: ROOT_DIR,
        encoding: 'utf8',
      });
      statusOut
        .split(/\r?\n/)
        .map((line) => line.slice(3).trim())
        .filter(Boolean)
        .forEach((f) => files.add(f.replace(/\\/g, '/')));
    }
  } catch (_) {}
  return Array.from(files);
}

function getAddedLinesForFile(relPath, isStaged) {
  try {
    const cmd = isStaged
      ? `git diff --cached -U0 -- "${relPath}"`
      : `git diff -U0 ${BASE_COMMIT} -- "${relPath}"`;
    const diff = execSync(cmd, { cwd: ROOT_DIR, encoding: 'utf8' });
    const added = diff
      .split(/\r?\n/)
      .filter((line) => line.startsWith('+') && !line.startsWith('+++'))
      .map((line) => line.slice(1));
    if (added.length > 0) {
      return added.join('\n');
    }
  } catch (_) {}
  const abs = path.join(ROOT_DIR, relPath);
  if (fs.existsSync(abs) && fs.statSync(abs).isFile()) {
    return fs.readFileSync(abs, 'utf8');
  }
  return '';
}

function runCommonCommands() {
  const syntaxTargets = [
    'scripts/gemini_mcp_server.js',
    'scripts/standalone_studio_app.js',
    'scripts/archaeology_visuals_enricher.js',
  ];

  for (const rel of syntaxTargets) {
    const r = spawnSync(process.execPath, ['--check', rel], {
      cwd: ROOT_DIR,
      encoding: 'utf8',
    });
    if (r.status !== 0) {
      failOneLine(
        `node --check ${rel}`,
        (r.stderr || r.stdout || '구문 오류').trim().replace(/\r?\n/g, ' '),
        `${rel}의 자바스크립트 문법 오류를 수정하십시오.`
      );
    }
  }

  const testRun = spawnSync(process.execPath, ['--test', 'tests/'], {
    cwd: ROOT_DIR,
    encoding: 'utf8',
  });
  if (testRun.status !== 0) {
    failOneLine(
      'node --test tests/',
      '단위/통합 테스트 중 실패한 항목이 있습니다.',
      '`node --test tests/`를 실행해 실패한 테스트를 확인하고 수정하십시오.'
    );
  }

  // CI 인라인 node -e 검사 1 (gemini_mcp_server.js 핵심 심볼 유지 확인)
  const serverSrc = fs.readFileSync(path.join(ROOT_DIR, 'scripts/gemini_mcp_server.js'), 'utf8');
  const requiredServerSymbols = [
    'WORDCRAFT_EXE',
    'PDFCRAFT_EXE',
    'exportManuscriptToWordCraftAndPdfCraft',
    'combinePdfReportsWithPdfCraft',
    'ensureRichArchaeologyVisualsAndTables',
  ];
  for (const sym of requiredServerSymbols) {
    if (!serverSrc.includes(sym)) {
      failOneLine(
        'CI-inline-1',
        `scripts/gemini_mcp_server.js에 필수 심볼(${sym})이 누락되었습니다.`,
        `기존 v1 호환 심볼 ${sym}을 복원하십시오 (명세서 0.2-1).`
      );
    }
  }

  // CI 인라인 node -e 검사 2 (standalone_studio_app.js 핵심 UI 문자열 유지 확인)
  const studioMod = require(path.join(ROOT_DIR, 'scripts/standalone_studio_app.js'));
  const html = studioMod.getStandaloneStudioHtml();
  const requiredUiTokens = [
    'ARCHAEO STUDIO',
    '발굴조사보고서 고찰',
    '논문',
    '디비피아 로그인',
    'PDF로 내보내기',
  ];
  for (const tok of requiredUiTokens) {
    if (!html.includes(tok)) {
      failOneLine(
        'CI-inline-2',
        `scripts/standalone_studio_app.js 출력 HTML에 필수 토큰(${tok})이 누락되었습니다.`,
        `기존 UI 호환 토큰 "${tok}"을 유지하십시오 (명세서 0.2-1).`
      );
    }
  }
}

function main() {
  const isStaged = process.argv.includes('--staged');
  const changedFiles = getChangedFiles(isStaged);

  // 1. 보호 경로 검사
  for (const rel of changedFiles) {
    for (const prefix of PROTECTED_PREFIXES) {
      if (rel === prefix || rel.startsWith(prefix)) {
        failOneLine(
          'PROTECTED_PATH',
          `보호 경로(${rel})가 변경 목록에 포함되어 있습니다.`,
          `\`git checkout -- ${rel}\`로 보호 경로 변경을 되돌리십시오 (명세서 0.2-6, 14.3).`
        );
      }
    }
  }

  // 2. 금지 패턴 검사 (규칙을 설명하는 명세서·워크플로·작업기록 문서는 제외하고 실제 코드·프롬프트·설정 변경 줄을 검사)
  const docExemptRe =
    /^(?:ARCHAEO_STUDIO_6\.1.*\.md|1_ARCHAEO_STUDIO.*\.md|README\.md|docs\/v6_jobs\/.*|\.agent\/workflows\/.*|skills(?:_backup)?\/.*)$/;
  for (const rel of changedFiles) {
    if (rel === 'scripts/v2/tools/check.js') continue;
    if (docExemptRe.test(rel)) continue;
    const addedText = getAddedLinesForFile(rel, isStaged);
    for (const rule of FORBIDDEN_RULES) {
      if (rule.regex.test(addedText)) {
        failOneLine(rule.id, `${rel}: ${rule.reason}`, rule.fix);
      }
    }
  }

  // 3. SKILL.md 동기화 검사
  const skillMain = path.join(ROOT_DIR, 'skills/archaeo-studio-pro/SKILL.md');
  const skillBackup = path.join(ROOT_DIR, 'skills_backup/archaeo-studio-pro/SKILL.md');
  if (fs.existsSync(skillMain) && fs.existsSync(skillBackup)) {
    const c1 = fs.readFileSync(skillMain, 'utf8').replace(/\r\n/g, '\n');
    const c2 = fs.readFileSync(skillBackup, 'utf8').replace(/\r\n/g, '\n');
    if (c1 !== c2) {
      failOneLine(
        'SKILL_SYNC',
        'skills/archaeo-studio-pro/SKILL.md와 skills_backup/archaeo-studio-pro/SKILL.md의 내용이 다릅니다.',
        '두 파일의 내용을 동일하게 맞추십시오 (명세서 0.2-6, 14.3).'
      );
    }
  }

  // 4. 11장 공통 검증 4개 명령 + CI 인라인 검사 2개 실행
  runCommonCommands();

  console.log('[v6-check 통과] 보호 경로, 금지 패턴, SKILL 동기화, 구문 검사 3종, 단위 테스트, CI 인라인 검사 2종 모두 통과했습니다.');
}

main();
