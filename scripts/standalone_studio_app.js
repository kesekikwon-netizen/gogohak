/**
 * ARCHAEO STUDIO — WordCraft × Gemini 고고학 『Ⅳ. 고찰』 · 학술논문 작성 스튜디오
 *
 * 의뢰 디자인 패키지(archaeo-studio: tokens.json, tokens.css, components/bundle.css,
 * components/bundle.js, README.md)를 100% 그대로 반영하고 실시간 백엔드(Gemini '작업' 모드,
 * DBpia AI, WordCraft .docx, PdfCraft .pdf)와 직결한 프로덕션 모듈입니다.
 */

function getStandaloneStudioHtml() {
  return `<!DOCTYPE html>
<html lang="ko" data-theme="light">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>발굴조사보고서 Ⅳ 고찰.docx - ARCHAEO STUDIO</title>
<link rel="preconnect" href="https://fonts.googleapis.com">
<link rel="preconnect" href="https://fonts.gstatic.com" crossorigin>
<link href="https://fonts.googleapis.com/css2?family=Noto+Sans+KR:wght@400;500;600;700&family=Noto+Serif+KR:wght@400;600;700&family=IBM+Plex+Mono:wght@400;500&display=swap" rel="stylesheet">
<style>
/* ARCHAEO STUDIO — generated from tokens.json */
:root, [data-theme="light"] { --bg: #FAF9F5; --tab: #F0EEE6; --input: #FFFFFF; --scrim: rgba(20,20,19,.28); --hover: #E8E6DC; --oat: #E3DACC; --canvas: #ECEAE2; --page: #FFFFFF; --line: #DEDCD1; --line-soft: #EEECE5; --line-strong: #C2C0B6; --ink: #141413; --ink-muted: #5E5D59; --ink-faint: #87867F; --ink-disabled: #9A988F; --icon: #30302E; --page-ink: #141413; --page-ink-muted: #5E5D59; --clay: #B5573A; --clay-deep: #9C4A2F; --on-clay: #FFFFFF; --on-ink: #FFFFFF; --ok: #3F6B31; --ok-soft: #E6EDDF; --warn: #7A5A00; --warn-soft: #F7EFD6; --bad: #A33B3B; --bad-soft: #F5E3DF; --brass: #9A7B38; --brass-soft: #F3EDDD; --progress: #2E6B4E; --shadow-page: 0 1px 3px rgba(0,0,0,.18); --shadow-menu: 0 4px 14px rgba(0,0,0,.14); --focus-ring: 0 0 0 2px #FAF9F5, 0 0 0 4px #B5573A; }
[data-theme="dark"] { --bg: #1E1D1A; --tab: #2D2B27; --input: #262521; --scrim: rgba(0,0,0,.5); --hover: #33312C; --oat: #4A4235; --canvas: #161513; --page: #FFFFFF; --line: #3D3B36; --line-soft: #2F2D29; --line-strong: #55524B; --ink: #EEECE5; --ink-muted: #B4B1A8; --ink-faint: #8D8A82; --ink-disabled: #6F6C65; --icon: #D9D6CE; --page-ink: #141413; --page-ink-muted: #5E5D59; --clay: #E08A6A; --clay-deep: #E8A487; --on-clay: #1E1D1A; --on-ink: #1E1D1A; --ok: #9DC48A; --ok-soft: #27331F; --warn: #E3C26A; --warn-soft: #3A3220; --bad: #E58C80; --bad-soft: #3E2622; --brass: #C9A85E; --brass-soft: #3A3222; --progress: #6FB592; --shadow-page: 0 1px 3px rgba(0,0,0,.6); --shadow-menu: 0 4px 14px rgba(0,0,0,.55); --focus-ring: 0 0 0 2px #1E1D1A, 0 0 0 4px #E08A6A; }
:root { --space-2: 2px; --space-4: 4px; --space-6: 6px; --space-8: 8px; --space-10: 10px; --space-12: 12px; --space-14: 14px; --space-16: 16px; --radius-4: 4px; --radius-6: 6px; --radius-7: 7px; --radius-8: 8px; --radius-9: 9px; --radius-10: 10px; --radius-full: 999px; --size-title-bar: 32px; --size-ribbon: 86px; --size-status-bar: 28px; --size-panel-header: 36px; --size-control: 28px; --size-search: 32px; --size-tile: 58px; --size-tile-icon-w: 46px; --size-tile-icon-h: 40px; --size-studio-panel-min: 360px; --size-studio-panel: 392px; --size-studio-panel-max: 420px; --size-a4-w: 210mm; --size-a4-h: 297mm; --font-sans: "Noto Sans KR", "Malgun Gothic", "Source Sans 3", system-ui, sans-serif; --font-serif: Batang, "바탕", "Noto Serif KR", "Malgun Gothic", serif; --font-mono: "IBM Plex Mono", ui-monospace, Consolas, monospace; }

* { box-sizing: border-box; }
html, body { margin: 0; height: 100%; background: var(--bg); color: var(--ink); font: 13px/1.3 var(--font-sans); overflow: hidden; }
.as-icon { display: block; flex: none; }
button { font: inherit; color: inherit; }
:where(.as-btn, .as-chip, .as-seg__item, .as-tile, .as-linkbtn, .as-file__x):focus-visible { outline: none; box-shadow: var(--focus-ring); }

/* 단추 */
.as-btn { height: var(--size-control); display: inline-flex; align-items: center; justify-content: center; gap: var(--space-6); padding: 0 var(--space-14); border: 1px solid var(--line-strong); border-radius: var(--radius-7); background: var(--tab); color: var(--ink); font-size: 12px; white-space: nowrap; cursor: pointer; }
.as-btn:hover:not(:disabled) { background: var(--hover); }
.as-btn--quiet { background: transparent; }
.as-btn--strong { background: var(--ink); border-color: var(--ink); color: var(--on-ink); }
.as-btn--strong:hover:not(:disabled) { background: var(--icon); }
.as-btn--accent { background: var(--clay); border-color: var(--clay); color: var(--on-clay); }
.as-btn--accent:hover:not(:disabled) { background: var(--clay-deep); border-color: var(--clay-deep); }
.as-btn--lg { height: var(--size-panel-header); font-size: 13px; font-weight: 600; padding: 0 var(--space-16); }
.as-btn--block { display: flex; width: 100%; }
.as-btn:disabled { color: var(--ink-disabled); cursor: default; }
.as-btn--accent:disabled { background: var(--clay); border-color: var(--clay); color: var(--on-clay); opacity: .72; }
.as-btn--brass .as-icon { color: var(--brass); }

/* 태그 */
.as-tag { display: inline-block; font-size: 11.5px; font-weight: 500; line-height: 1.5; padding: 1px 7px; border-radius: var(--radius-full); white-space: nowrap; background: var(--tab); color: var(--ink-muted); border: 1px solid transparent; }
.as-tag--ok { background: var(--ok-soft); color: var(--ok); }
.as-tag--warn { background: var(--warn-soft); color: var(--warn); }
.as-tag--bad { background: var(--bad-soft); color: var(--bad); }
.as-tag--new { background: var(--oat); color: var(--ink); }
.as-tag--brass { background: var(--brass-soft); color: var(--ink); border-color: var(--brass); }
.as-tag--brass::before { content: ""; display: inline-block; width: 6px; height: 6px; border-radius: 50%; background: var(--brass); margin-right: 5px; vertical-align: 1px; }

/* 입력 */
.as-field { box-sizing: border-box; width: 100%; height: var(--size-control); border: 1px solid var(--line-strong); border-radius: var(--radius-7); background: var(--input); color: var(--ink); font: 12.5px/1.3 var(--font-sans); padding: 0 var(--space-8); }
.as-field--multi { height: auto; line-height: 1.5; padding: var(--space-6) var(--space-8); resize: vertical; display: block; }
.as-field::placeholder { color: var(--ink-disabled); }
.as-field:focus { outline: none; border-color: var(--clay); box-shadow: 0 0 0 1px var(--clay); }
.as-search { box-sizing: border-box; display: flex; align-items: center; width: 210px; height: var(--size-search); border: 1px solid var(--line-strong); border-radius: var(--radius-8); padding: 0 var(--space-10); gap: var(--space-8); background: var(--input); color: var(--ink-muted); font-size: 13px; }
.as-search input { flex: 1; min-width: 0; border: 0; background: transparent; color: var(--ink); font: inherit; outline: none; padding: 0; }
.as-search input::placeholder { color: var(--ink-muted); }
.as-search:focus-within { border-color: var(--clay); box-shadow: 0 0 0 1px var(--clay); }
.as-search__key { font: 11px var(--font-mono); color: var(--ink-muted); }

/* 칩·세그먼트 */
.as-chips { display: flex; flex-wrap: wrap; gap: var(--space-4); }
.as-chip { font-size: 11.5px; padding: 2px var(--space-8); border-radius: var(--radius-full); border: 1px solid var(--line-strong); color: var(--ink-muted); background: var(--input); cursor: pointer; white-space: nowrap; }
.as-chip:hover:not(.is-on) { background: var(--hover); }
.as-chip.is-on { background: var(--oat); color: var(--ink); border: 2px solid var(--ink); padding: 1px 7px; }
.as-seg { display: flex; border: 1px solid var(--line-strong); border-radius: var(--radius-6); overflow: hidden; background: var(--input); }
.as-seg__item { flex: 1; text-align: center; font-size: 11.5px; padding: var(--space-4) 0; border: 0; border-right: 1px solid var(--line); background: transparent; color: var(--ink); cursor: pointer; }
.as-seg__item:last-child { border-right: 0; }
.as-seg__item:hover:not(.is-on):not(:disabled) { background: var(--hover); }
.as-seg__item.is-on { background: var(--oat); font-weight: 600; }
.as-seg__item:disabled { cursor: default; }
.as-seg__item:disabled:not(.is-on) { color: var(--ink-disabled); }
.as-seg--lg .as-seg__item { font-size: 13px; padding: var(--space-8) var(--space-6); }

/* 패널 머리·대화상자 */
.as-ph { display: flex; align-items: center; gap: var(--space-6); padding: 0 var(--space-12); height: var(--size-panel-header); border-bottom: 1px solid var(--line); font-size: 13.5px; font-weight: 600; color: var(--ink); }
.as-ph__sub { font-weight: 400; font-size: 12px; color: var(--ink-muted); }
.as-ph__actions { margin-left: auto; display: flex; gap: var(--space-10); color: var(--ink-muted); font-weight: 400; }
.as-dialog { box-sizing: border-box; width: 520px; max-width: 92vw; background: var(--bg); color: var(--ink); border: 1px solid var(--line-strong); border-radius: var(--radius-10); overflow: hidden; font-size: 13px; box-shadow: var(--shadow-menu); }
.as-dialog__body { padding: var(--space-14) var(--space-16); display: flex; flex-direction: column; gap: var(--space-10); }
.as-dialog__foot { display: flex; gap: var(--space-8); justify-content: flex-end; align-items: center; padding: 0 var(--space-16) var(--space-14); }
.as-dialog__state { margin-right: auto; }
.as-dialog__note { margin: 0; font-size: 11.5px; line-height: 1.5; color: var(--ink-muted); }
.as-frow { display: grid; grid-template-columns: 70px 1fr; align-items: center; gap: var(--space-8); font-size: 12.5px; color: var(--ink-muted); }
.as-web { border: 1px solid var(--line); border-radius: var(--radius-7); overflow: hidden; background: var(--input); }
.as-web__bar { display: flex; align-items: center; gap: var(--space-6); height: 28px; padding: 0 var(--space-10); border-bottom: 1px solid var(--line-soft); font: 11.5px var(--font-mono); color: var(--ink-muted); }
.as-web__bar .as-icon { color: var(--brass); }
.as-web__view { height: 220px; display: flex; flex-direction: column; align-items: center; justify-content: center; gap: var(--space-6); background: var(--canvas); text-align: center; padding: 0 var(--space-16); }
.as-web__view p { margin: 0; font-size: 12.5px; color: var(--ink); line-height: 1.5; }
.as-web__view .as-web__sub { font-size: 11.5px; color: var(--ink-muted); }

/* 리본 */
.as-ribbon { display: flex; align-items: stretch; height: var(--size-ribbon); box-sizing: border-box; padding: var(--space-6) var(--space-10) var(--space-4); border-bottom: 1px solid var(--line); background: var(--bg); }
.as-ribbon__search { margin-left: auto; align-self: center; display: flex; align-items: center; gap: var(--space-8); }
.as-rg { display: flex; flex-direction: column; padding: 0 var(--space-10); border-right: 1px solid var(--line); }
.as-rg:last-of-type { border-right: 0; }
.as-rg__cap { font-size: 11.5px; color: var(--ink-muted); margin: 0 0 var(--space-4) 2px; }
.as-rg__tiles { display: flex; gap: var(--space-4); }
.as-tile { min-width: var(--size-tile); display: flex; flex-direction: column; align-items: center; gap: 3px; font-size: 12px; color: var(--ink); background: transparent; border: 0; padding: 0 4px; cursor: pointer; border-radius: var(--radius-9); }
.as-tile__ic { width: var(--size-tile-icon-w); height: var(--size-tile-icon-h); border-radius: var(--radius-9); display: grid; place-items: center; color: var(--icon); }
.as-tile:hover .as-tile__ic { background: var(--hover); }
.as-tile.is-sel .as-tile__ic { background: var(--oat); }
.as-tile:disabled { color: var(--ink-disabled); cursor: default; }
.as-tile:disabled .as-tile__ic { color: var(--ink-disabled); background: transparent; }
.as-tile__dd { font-size: 9px; color: var(--ink-muted); margin-left: 1px; }

/* 상태줄·수집 띠 */
.as-status { display: flex; align-items: center; height: var(--size-status-bar); border-top: 1px solid var(--line); font-size: 12px; color: var(--ink-muted); background: var(--bg); }
.as-status__hint { padding-left: var(--space-12); color: var(--ink); white-space: nowrap; overflow: hidden; text-overflow: ellipsis; min-width: 0; }
.as-status__hint.is-error { color: var(--bad); }
.as-status__r { margin-left: auto; display: flex; height: 100%; flex: none; }
.as-status__item { display: flex; align-items: center; padding: 0 var(--space-12); border-left: 1px solid var(--line); color: var(--ink-muted); white-space: nowrap; }
.as-status__item.is-mono { font: 11.5px var(--font-mono); color: var(--ink); }
.as-collect { display: flex; align-items: center; gap: var(--space-12); height: var(--size-status-bar); padding: 0 var(--space-12); background: var(--bg); border-bottom: 1px solid var(--line); font-size: 12px; color: var(--ink); white-space: nowrap; }
.as-collect__item { display: inline-flex; align-items: center; gap: var(--space-6); }
.as-collect__dot { width: 7px; height: 7px; border-radius: 50%; background: var(--brass); box-sizing: border-box; }
.as-collect__dot.is-hollow { background: transparent; border: 1.5px solid var(--brass); }
.as-collect__dot.is-ok { background: var(--ok); }
.as-collect__num { font: 11.5px var(--font-mono); color: var(--ink-muted); }
.as-collect__mini { width: 56px; height: 4px; border-radius: var(--radius-full); background: var(--tab); box-shadow: inset 0 0 0 1px var(--line); overflow: hidden; display: inline-block; }
.as-collect__mini span { display: block; height: 100%; background: var(--progress); }
.as-collect__r { margin-left: auto; color: var(--ink-muted); display: inline-flex; align-items: center; gap: var(--space-8); }

/* 스튜디오 패널 */
.as-step { padding: var(--space-12) var(--space-16); border-bottom: 1px solid var(--line); }
.as-step__head { display: flex; align-items: center; gap: var(--space-8); margin-bottom: var(--space-8); }
.as-step__n { width: 18px; height: 18px; border-radius: 50%; background: var(--ink); color: var(--on-ink); font: 600 11px/18px var(--font-mono); text-align: center; flex: none; }
.as-step__title { font-size: 13.5px; font-weight: 600; }
.as-step__aside { margin-left: auto; }
.as-step__help { margin: calc(-1 * var(--space-4)) 0 var(--space-8); font-size: 11px; color: var(--ink-muted); }
.as-step__body { display: flex; flex-direction: column; gap: var(--space-8); }
.as-prompt { display: flex; flex-direction: column; gap: var(--space-6); }
.as-prompt__note { display: flex; align-items: baseline; gap: var(--space-6); font-size: 11px; color: var(--ink-muted); line-height: 1.45; }
.as-prompt__note .as-tag { flex: none; }
.as-linkbtn { align-self: flex-start; border: 0; background: transparent; padding: 0; color: var(--clay-deep); font-size: 11.5px; cursor: pointer; border-radius: var(--radius-4); }
.as-prompt__processed { background: var(--tab); border-radius: var(--radius-7); padding: var(--space-6) var(--space-8); font-size: 11.5px; line-height: 1.55; color: var(--ink-muted); max-height: 130px; overflow: auto; white-space: pre-wrap; }
.as-source { display: flex; flex-direction: column; gap: var(--space-6); }
.as-source__bar { display: flex; align-items: center; gap: var(--space-8); }
.as-source__types { font: 10.5px var(--font-mono); color: var(--ink-muted); }
.as-source__empty { margin-left: auto; }
.as-source__files { display: flex; flex-wrap: wrap; gap: var(--space-4); }
.as-source__note { margin: 0; background: var(--tab); border-radius: var(--radius-7); padding: var(--space-6) var(--space-8); font-size: 11.5px; line-height: 1.5; color: var(--ink); }
.as-file { display: inline-flex; align-items: center; gap: var(--space-6); height: 24px; padding: 0 var(--space-4) 0 var(--space-8); border: 1px solid var(--line); border-radius: var(--radius-7); background: var(--input); font-size: 11.5px; max-width: 100%; box-sizing: border-box; }
.as-file__name { white-space: nowrap; overflow: hidden; text-overflow: ellipsis; max-width: 210px; }
.as-file__size { font: 10.5px var(--font-mono); color: var(--ink-muted); flex: none; }
.as-file__x { border: 0; background: transparent; color: var(--ink-muted); cursor: pointer; width: 18px; height: 18px; border-radius: var(--radius-4); padding: 0; font-size: 10px; flex: none; }
.as-file__x:hover { background: var(--hover); color: var(--ink); }
.as-run { display: flex; flex-direction: column; gap: var(--space-8); }
.as-run__main { display: flex; flex-direction: column; gap: var(--space-6); }
.as-run__ctl { display: flex; gap: var(--space-6); }
.as-run__ctl .as-btn { flex: 1; }
.as-run__sub { display: flex; align-items: center; gap: var(--space-6); flex-wrap: wrap; }
.as-run__spacer { flex: 1; }
.as-progress { display: flex; flex-direction: column; gap: var(--space-4); min-width: 120px; }
.as-progress__row { display: flex; justify-content: space-between; font-size: 11.5px; color: var(--ink-muted); }
.as-progress__pct { font: 11.5px var(--font-mono); color: var(--ink); }
.as-progress__track { height: 6px; border-radius: var(--radius-full); background: var(--tab); overflow: hidden; box-shadow: inset 0 0 0 1px var(--line); }
.as-progress__fill { height: 100%; background: var(--progress); border-radius: var(--radius-full); transition: width .3s ease; }
.as-progress__fill.is-done { background: var(--ok); }
.as-log { border: 1px solid var(--line); border-radius: var(--radius-7); background: var(--input); overflow: hidden; }
.as-log__head { display: flex; flex-direction: column; gap: var(--space-6); padding: var(--space-8) var(--space-10); border-bottom: 1px solid var(--line-soft); }
.as-log__head .as-ph__title { font-size: 12.5px; font-weight: 600; }
.as-log__list { list-style: none; margin: 0; padding: var(--space-4) 0; max-height: 92px; overflow: auto; }
.as-log__row { display: grid; grid-template-columns: 54px 40px 1fr; gap: var(--space-6); padding: 3px var(--space-10); font-size: 12px; line-height: 1.45; align-items: baseline; }
.as-log__time { font: 10.5px var(--font-mono); color: var(--ink-muted); }
.as-log__kind { display: inline-flex; align-items: center; gap: var(--space-4); font-size: 11px; color: var(--ink-muted); white-space: nowrap; }
.as-log__kind i { width: 6px; height: 6px; border-radius: 50%; background: var(--ink-muted); flex: none; }
.as-log__row.is-collect .as-log__kind i { background: var(--brass); }
.as-log__row.is-write .as-log__kind i { background: var(--progress); }
.as-log__row.is-done .as-log__kind i { background: var(--ok); }
.as-log__row.is-warn .as-log__kind i { background: var(--warn); }
.as-log__row.is-warn .as-log__kind { color: var(--warn); }
.as-log__row.is-error .as-log__kind i { background: var(--bad); }
.as-log__row.is-error .as-log__text, .as-log__row.is-error .as-log__kind { color: var(--bad); }
.as-log__empty { margin: 0; padding: var(--space-10); font-size: 11.5px; color: var(--ink-muted); }
.as-goal-box { border: 1px solid var(--line); border-radius: var(--radius-7); background: var(--input); padding: var(--space-8) var(--space-10); margin-bottom: var(--space-6); }
.as-goal-head { display: flex; align-items: center; justify-content: space-between; gap: var(--space-6); font-size: 11.5px; font-weight: 600; color: var(--ink); margin-bottom: var(--space-6); }
.as-goal-grid { display: grid; grid-template-columns: 1fr 1fr; gap: 4px 8px; list-style: none; margin: 0; padding: 0; }
.as-goal-item { display: flex; align-items: center; justify-content: space-between; gap: 4px; font-size: 11px; color: var(--ink-muted); padding: 2px 6px; border-radius: var(--radius-4); background: var(--tab); overflow: hidden; }
.as-goal-item.is-pass { background: var(--ok-soft); color: var(--ok); font-weight: 500; }
.as-goal-item__name { white-space: nowrap; overflow: hidden; text-overflow: ellipsis; }
.as-goal-item__badge { font: 600 10px var(--font-mono); flex: none; }

/* A4 문서 (다중 페이지 분리 지원) */
.as-pages-wrap { display: flex; flex-direction: column; gap: 22px; align-items: center; }
.as-page { box-sizing: border-box; width: var(--size-a4-w); min-height: var(--size-a4-h); padding: 25mm 20mm 18mm; background: var(--page); color: var(--page-ink); box-shadow: var(--shadow-page); font-family: var(--font-serif); display: flex; flex-direction: column; margin: 0 auto; outline: none; position: relative; }
.as-page__head-running { font-size: 10px; color: var(--page-ink-muted); border-bottom: 0.5px solid #D5D3C9; padding-bottom: 4px; margin-bottom: 14px; display: flex; justify-content: space-between; font-family: var(--font-sans); }
.as-page__body { flex: 1; outline: none; }
.as-page__empty { margin: 60mm 0 0; text-align: center; font-size: 13.33px; line-height: 1.7; color: var(--page-ink-muted); font-family: var(--font-sans); }
.as-doc-chapter { font-size: 21.33px; font-weight: 700; line-height: 1.4; text-align: center; margin: 0 0 1.2em; white-space: pre-wrap; }
.as-doc-section { font-size: 16px; font-weight: 700; line-height: 1.5; margin: 1.1em 0 .5em; }
.as-doc-subsec { font-size: 14px; font-weight: 700; line-height: 1.5; margin: .8em 0 .35em; }
.as-doc-para { font-size: 13.33px; line-height: 1.7; text-indent: 1em; text-align: justify; margin: 0 0 .5em; word-break: keep-all; }
.as-fnref { font-size: .68em; line-height: 0; vertical-align: super; margin-left: 1px; color: var(--clay-deep); font-weight: 600; }
.as-doc-table { margin: 1em 0 1.1em; }
.as-doc-table figcaption { font-size: 12px; font-weight: 700; text-align: center; margin-bottom: .45em; }
.as-doc-table table { width: 100%; border-collapse: collapse; font-size: 12px; line-height: 1.4; }
.as-doc-table th, .as-doc-table td { border: .75px solid var(--page-ink); padding: 4px 6px; text-align: center; }
.as-doc-table thead th { font-weight: 700; border-top-width: 1.5px; background: #F7F6F2; }
.as-doc-table tbody tr:last-child td { border-bottom-width: 1.5px; }
.as-doc-refs { margin-top: 1.4em; }
.as-doc-refs ul { list-style: none; margin: 0; padding: 0; }
.as-doc-refs li { font-size: 12px; line-height: 1.6; padding-left: 2em; text-indent: -2em; margin-bottom: 3px; }
.as-doc-diagram { margin: 1em 0; padding: 10px 12px; border: 1px solid #C2C0B6; background: #FAF9F5; font-family: var(--font-sans); }
.as-doc-diagram__title { font-size: 12px; font-weight: 700; text-align: center; margin-bottom: 8px; color: #141413; }
.as-doc-diagram__steps { display: flex; gap: 8px; align-items: stretch; justify-content: space-between; flex-wrap: wrap; }
.as-doc-diagram__step { flex: 1; min-width: 130px; background: #FFFFFF; border: 1px solid #B5573A; border-radius: 6px; padding: 8px; font-size: 11.5px; line-height: 1.45; }
.as-doc-diagram__step strong { display: block; color: #9C4A2F; margin-bottom: 4px; font-size: 12px; }
.as-paper { text-align: center; margin-bottom: 1.6em; }
.as-paper__title { font-size: 24px; font-weight: 700; line-height: 1.4; margin: 0 0 .3em; word-break: keep-all; }
.as-paper__subtitle { font-size: 13.33px; margin: 0 0 .6em; }
.as-paper__author { font-size: 13.33px; margin: .4em 0 1.4em; }
.as-paper__abstract { text-align: left; border-top: .75px solid var(--page-ink); border-bottom: .75px solid var(--page-ink); padding: .7em .2em; font-size: 12px; line-height: 1.6; }
.as-paper__abstract p { margin: 0; }
.as-paper__abs-h { font-weight: 700; text-align: center; margin-bottom: .4em !important; }
.as-paper__abs-t { text-indent: 1em; text-align: justify; word-break: keep-all; }
.as-paper__kw { margin-top: .5em !important; }
.as-page__notes { margin-top: 1.4em; font-size: 10.67px; line-height: 1.5; }
.as-page__notes hr { width: 33%; margin: 0 0 .5em; border: 0; border-top: .75px solid var(--page-ink); }
.as-page__notes ol { list-style: none; margin: 0; padding: 0; }
.as-page__notes li { display: flex; gap: .4em; margin-bottom: 2px; }
.as-page__fnn { flex: none; font-weight: 600; }
.as-page__num { text-align: center; font-size: 10.67px; margin-top: 1.2em; color: var(--page-ink-muted); font-family: var(--font-sans); }
.as-caret { display: inline-block; width: 1.5px; height: 1.05em; background: var(--clay); vertical-align: -0.15em; margin-left: 1px; animation: as-blink 1s steps(1) infinite; }
@keyframes as-blink { 50% { opacity: 0; } }

/* DBpia AI 실시간 다각도 검색 요점 & 실존 논문 수집 보드 */
.as-dbpia-board { width: calc(var(--size-a4-w) * 0.9); max-width: 92%; margin: 0 auto 18px; background: var(--bg); border: 1px solid var(--brass-line); border-radius: var(--radius-7); box-shadow: 0 2px 10px rgba(20,20,19,0.08); overflow: hidden; font-family: var(--font-sans); }
.as-dbpia-board__head { display: flex; align-items: center; justify-content: space-between; gap: 8px; padding: 9px 14px; background: var(--brass-soft); border-bottom: 1px solid var(--brass-line); font-size: 12px; font-weight: 600; color: var(--ink); }
.as-dbpia-board__body { padding: 12px 14px; max-height: 440px; overflow-y: auto; display: flex; flex-direction: column; gap: 12px; }
.as-dbpia-feed-card { border: 1px solid var(--line); border-radius: var(--radius-5); background: var(--input); padding: 10px 12px; }
.as-dbpia-feed-card__q { font-size: 12px; font-weight: 700; color: var(--brass); margin-bottom: 6px; display: flex; align-items: center; justify-content: space-between; }
.as-dbpia-feed-card__sum { font-size: 11.8px; line-height: 1.6; color: var(--ink); white-space: pre-wrap; max-height: 180px; overflow-y: auto; padding-right: 4px; }
.as-dbpia-papers-grid { display: grid; grid-template-columns: 1fr 1fr; gap: 8px; }
.as-dbpia-paper-item { border: 1px solid var(--line); border-radius: var(--radius-5); background: var(--tab); padding: 8px 10px; font-size: 11.2px; line-height: 1.45; }
.as-dbpia-paper-item__cite { font-weight: 600; color: var(--ink); margin-bottom: 4px; }
.as-dbpia-paper-item__quote { color: var(--ink-muted); font-size: 10.8px; display: -webkit-box; -webkit-line-clamp: 3; -webkit-box-orient: vertical; overflow: hidden; }

/* 전체 화면 틀 */
.as-screen { position: relative; width: 100vw; height: 100vh; display: grid; grid-template-rows: var(--size-title-bar) var(--size-ribbon) 1fr var(--size-status-bar); background: var(--bg); color: var(--ink); overflow: hidden; }
.as-screen__title { display: flex; align-items: center; gap: var(--space-10); padding: 0 var(--space-12); border-bottom: 1px solid var(--line); font-size: 13px; }
.as-screen__win { margin-left: auto; display: flex; align-items: center; gap: var(--space-6); }
.as-screen__body { display: grid; grid-template-columns: var(--size-studio-panel) 1fr; min-height: 0; transition: grid-template-columns .18s ease; }
.as-screen__body.is-collapsed { grid-template-columns: 0px 1fr; }
.as-screen__panel { border-right: 1px solid var(--line); background: var(--bg); display: grid; grid-template-rows: 1fr auto; min-height: 0; overflow: hidden; }
.as-screen__steps { overflow: auto; min-height: 0; }
.as-screen__dock { border-top: 1px solid var(--line-strong); background: var(--bg); }
.as-screen__dock .as-step:last-child { border-bottom: 0; }
.as-screen__canvas { display: grid; grid-template-rows: auto 1fr; min-width: 0; min-height: 0; background: var(--canvas); }
.as-screen__scroll { overflow: auto; padding: var(--space-16) 0 40px; }
.as-screen__scroll .as-page { zoom: .9; }
.as-screen__scrim { position: fixed; inset: 0; background: var(--scrim); display: none; place-items: center; z-index: 999; }
.as-screen__scrim.is-open { display: grid; }
</style>
</head>
<body>

<div class="as-screen">
  <!-- 1. 제목줄 -->
  <div class="as-screen__title">
    <span id="win-doc-title" style="font-weight:600;">발굴조사보고서 Ⅳ 고찰.docx - ARCHAEO STUDIO</span>
    <span class="as-tag as-tag--new" id="top-mode-badge">WordCraft 통합</span>
    <div class="as-screen__win">
      <button type="button" class="as-btn" style="height:22px;padding:0 10px;font-size:11px;" onclick="openInWordCraft()" title="우측 문서를 네이티브 WordCraft 데스크톱 창으로 열기">WordCraft 데스크톱 열기 (.docx)</button>
      <button type="button" class="as-btn as-btn--quiet" style="height:22px;padding:0 8px;font-size:11px;" onclick="toggleTheme()" id="theme-toggle-btn">어두운 화면</button>
    </div>
  </div>

  <!-- 2. 리본 (StudioRibbon: 파일 · 홈·서식 · 삽입 · 스튜디오 · 내보내기) -->
  <div class="as-ribbon">
    <div class="as-rg" role="group" aria-label="파일">
      <div class="as-rg__cap">파일</div>
      <div class="as-rg__tiles">
        <button type="button" class="as-tile" onclick="newDocument()">
          <span class="as-tile__ic"><svg class="as-icon" width="24" height="24" viewBox="0 0 24 24"><path d="M6 3h8l4 4v14H6z M14 3v4h4" fill="none" stroke="currentColor" stroke-width="1.6" stroke-linejoin="round"/><path d="M12 11v6M9 14h6" stroke="currentColor" stroke-width="1.6"/></svg></span>
          <span class="as-tile__label">새 문서</span>
        </button>
        <button type="button" class="as-tile" onclick="document.getElementById('file-input-report').click()">
          <span class="as-tile__ic"><svg class="as-icon" width="24" height="24" viewBox="0 0 24 24"><path d="M3 7h6l2 2h10v10H3z" fill="none" stroke="currentColor" stroke-width="1.6" stroke-linejoin="round"/></svg></span>
          <span class="as-tile__label">열기</span>
        </button>
        <button type="button" class="as-tile" onclick="saveAndOpenWordCraft()">
          <span class="as-tile__ic"><svg class="as-icon" width="24" height="24" viewBox="0 0 24 24"><path d="M5 4h11l3 3v13H5z M8 4v5h7V4 M8 20v-6h8v6" fill="none" stroke="currentColor" stroke-width="1.6" stroke-linejoin="round"/></svg></span>
          <span class="as-tile__label">저장</span>
        </button>
      </div>
    </div>

    <div class="as-rg" role="group" aria-label="홈·서식">
      <div class="as-rg__cap">홈·서식</div>
      <div class="as-rg__tiles">
        <button type="button" class="as-tile" onclick="togglePageFont()">
          <span class="as-tile__ic"><svg class="as-icon" width="24" height="24" viewBox="0 0 24 24"><path d="M5 5h14M12 5v15M9 20h6" stroke="currentColor" stroke-width="1.8" fill="none"/></svg></span>
          <span class="as-tile__label">글자<span class="as-tile__dd">▾</span></span>
        </button>
      </div>
    </div>

    <div class="as-rg" role="group" aria-label="삽입">
      <div class="as-rg__cap">삽입</div>
      <div class="as-rg__tiles">
        <button type="button" class="as-tile" onclick="insertSampleTable()">
          <span class="as-tile__ic"><svg class="as-icon" width="24" height="24" viewBox="0 0 24 24"><rect x="3" y="5" width="18" height="14" rx="1" fill="none" stroke="currentColor" stroke-width="1.6"/><path d="M3 9.5h18M3 14.2h18M9 5v14" fill="none" stroke="currentColor" stroke-width="1.6"/></svg></span>
          <span class="as-tile__label">표</span>
        </button>
        <button type="button" class="as-tile" onclick="insertSampleDiagram()">
          <span class="as-tile__ic"><svg class="as-icon" width="24" height="24" viewBox="0 0 24 24"><rect x="4" y="5" width="16" height="14" rx="1.5" fill="none" stroke="currentColor" stroke-width="1.6"/><path d="M4 16l5-5 4 4 3-3 4 4" fill="none" stroke="currentColor" stroke-width="1.6"/></svg></span>
          <span class="as-tile__label">도면</span>
        </button>
        <button type="button" class="as-tile" onclick="insertSampleFootnote()">
          <span class="as-tile__ic"><svg class="as-icon" width="24" height="24" viewBox="0 0 24 24"><path d="M3.5 6.5h11M3.5 10.5h11M3.5 14.5h7" fill="none" stroke="currentColor" stroke-width="1.6"/><path d="M3.5 19h17" fill="none" stroke="currentColor" stroke-width="1.1"/><path d="M17.2 5.2l1.6-1.2v6.5" fill="none" stroke="currentColor" stroke-width="1.4" stroke-linejoin="round"/></svg></span>
          <span class="as-tile__label">각주</span>
        </button>
      </div>
    </div>

    <div class="as-rg" role="group" aria-label="스튜디오">
      <div class="as-rg__cap">스튜디오</div>
      <div class="as-rg__tiles">
        <button type="button" class="as-tile is-sel" id="ribbon-studio-tile" onclick="toggleStudioPanel()">
          <span class="as-tile__ic"><svg class="as-icon" width="24" height="24" viewBox="0 0 24 24"><rect x="3" y="4.5" width="18" height="15" rx="1.5" fill="none" stroke="currentColor" stroke-width="1.6"/><path d="M9.5 4.5v15M5.2 8.5h2.2M5.2 11.5h2.2M5.2 14.5h2.2" fill="none" stroke="currentColor" stroke-width="1.6"/></svg></span>
          <span class="as-tile__label">스튜디오</span>
        </button>
      </div>
    </div>

    <div class="as-rg" role="group" aria-label="내보내기">
      <div class="as-rg__cap">내보내기</div>
      <div class="as-rg__tiles">
        <button type="button" class="as-tile" onclick="openExportDialog()">
          <span class="as-tile__ic"><svg class="as-icon" width="24" height="24" viewBox="0 0 24 24"><path d="M12 3v11M8 7l4-4 4 4M5 13v7h14v-7" fill="none" stroke="currentColor" stroke-width="1.6" stroke-linejoin="round"/></svg></span>
          <span class="as-tile__label">PDF 변환</span>
        </button>
      </div>
    </div>

    <div class="as-ribbon__search">
      <label class="as-search">
        <svg class="as-icon" width="15" height="15" viewBox="0 0 24 24"><circle cx="10.5" cy="10.5" r="6" fill="none" stroke="currentColor" stroke-width="1.8"/><path d="M15 15l5 5" stroke="currentColor" stroke-width="1.8"/></svg>
        <input type="search" id="cmd-search-input" placeholder="명령 찾기" aria-label="명령 찾기" onkeydown="if(event.key==='Enter')findInDoc(this.value)">
        <span class="as-search__key">Ctrl K</span>
      </label>
    </div>
  </div>

  <!-- 3. 몸통 (좌측 스튜디오 패널 392px + 우측 WordCraft A4 편집 화면) -->
  <div class="as-screen__body" id="screen-body">
    <aside class="as-screen__panel" aria-label="제미나이 학술 스튜디오">
      <div class="as-screen__steps">
        <!-- 단계 1: 문서 종류 -->
        <section class="as-step" aria-label="문서 종류">
          <div class="as-step__head">
            <span class="as-step__n">1</span>
            <span class="as-step__title">문서 종류</span>
          </div>
          <div class="as-step__body">
            <div class="as-seg as-seg--lg" role="radiogroup" aria-label="문서 종류">
              <button type="button" role="radio" id="seg-report" class="as-seg__item is-on" aria-checked="true" onclick="selectDocMode('report')">발굴조사보고서 고찰</button>
              <button type="button" role="radio" id="seg-paper" class="as-seg__item" aria-checked="false" onclick="selectDocMode('paper')">논문</button>
            </div>
          </div>
        </section>

        <!-- 단계 2: 요청 -->
        <section class="as-step" aria-label="요청">
          <div class="as-step__head">
            <span class="as-step__n">2</span>
            <span class="as-step__title">요청</span>
          </div>
          <div class="as-step__body">
            <div class="as-prompt">
              <textarea id="input-focus" class="as-field as-field--multi" rows="3" placeholder="무엇을 쓸지 적어 주세요. 예: 조사 유적 확인 유구의 구조적 특징과 출토유물 편년 및 주변 유적과의 비교" aria-label="요청" oninput="refreshEngineeredPrompt(true)"></textarea>
              <div class="as-prompt__note">
                <span class="as-tag as-tag--new">1차 가공</span>
                <span>입력하신 요청은 고고학 학술 양식에 맞춰 1차 가공되어 전달됩니다.</span>
              </div>
              <button type="button" class="as-linkbtn" id="toggle-processed-btn" aria-expanded="false" onclick="toggleProcessedPrompt()">▸ 가공된 요청 보기</button>
              <div class="as-prompt__processed" id="processed-prompt-box" style="display:none;"></div>
            </div>
          </div>
        </section>

        <!-- 단계 3: 내 보고서·조사 자료 -->
        <section class="as-step" aria-label="내 보고서·조사 자료">
          <div class="as-step__head">
            <span class="as-step__n">3</span>
            <span class="as-step__title">내 보고서·조사 자료</span>
          </div>
          <div class="as-step__body">
            <div class="as-source">
              <textarea id="input-report" class="as-field as-field--multi" rows="3" placeholder="보고서 초안, 유구·유물 설명, 수치표를 붙여 넣으세요" aria-label="내 보고서·조사 자료" oninput="refreshEngineeredPrompt(true)"></textarea>
              <div class="as-source__bar">
                <button type="button" class="as-btn" onclick="document.getElementById('file-input-report').click()">
                  <svg class="as-icon" width="14" height="14" viewBox="0 0 24 24"><path d="M3 7h6l2 2h10v10H3z" fill="none" stroke="currentColor" stroke-width="1.6" stroke-linejoin="round"/></svg>
                  파일 불러오기
                </button>
                <span class="as-source__types">.docx .pdf .txt .md .csv</span>
                <input type="file" id="file-input-report" multiple accept=".pdf,.docx,.doc,.odt,.rtf,.tex,.txt,.md,.csv,.json" style="display:none" onchange="handleFileUpload(this, 'report')">
              </div>
              <div class="as-source__files" id="report-file-chips"></div>
            </div>
          </div>
        </section>

        <!-- 단계 4: 연대분석·보유 자료 -->
        <section class="as-step" aria-label="연대분석·보유 자료">
          <div class="as-step__head">
            <span class="as-step__n">4</span>
            <span class="as-step__title">연대분석·보유 자료</span>
            <span class="as-step__aside"><span class="as-tag">선택</span></span>
          </div>
          <div class="as-step__body">
            <div class="as-source">
              <textarea id="input-dating" class="as-field as-field--multi" rows="2" placeholder="AMS 연대, 자연과학분석, 실측치 등" aria-label="연대분석·보유 자료" oninput="updateDatingEmptyTag();refreshEngineeredPrompt(true)"></textarea>
              <div class="as-source__bar">
                <button type="button" class="as-btn" onclick="document.getElementById('file-input-dating').click()">
                  <svg class="as-icon" width="14" height="14" viewBox="0 0 24 24"><path d="M3 7h6l2 2h10v10H3z" fill="none" stroke="currentColor" stroke-width="1.6" stroke-linejoin="round"/></svg>
                  파일 불러오기
                </button>
                <span class="as-source__types">.docx .pdf .txt .md .csv</span>
                <span class="as-tag as-source__empty" id="dating-empty-tag">비워 둠</span>
                <input type="file" id="file-input-dating" multiple accept=".pdf,.docx,.doc,.odt,.rtf,.tex,.txt,.md,.csv,.json" style="display:none" onchange="handleFileUpload(this, 'dating')">
              </div>
              <div class="as-source__files" id="dating-file-chips"></div>
              <p class="as-source__note">보유하지 않은 자료는 비워 두세요. 없는 자료는 제외하고 입력된 자료만으로 결과가 작성됩니다.</p>
            </div>
          </div>
        </section>
      </div>

      <!-- 하단 고정 도킹: 단계 5 (실행) + 단계 6 (진행 기록) -->
      <div class="as-screen__dock">
        <section class="as-step" aria-label="실행">
          <div class="as-step__head">
            <span class="as-step__n">5</span>
            <span class="as-step__title">실행</span>
            <span class="as-step__aside" id="run-done-tag"></span>
          </div>
          <div class="as-step__body">
            <div class="as-run">
              <div class="as-run__sub" style="margin-bottom:4px;">
                <div class="as-seg" role="radiogroup" aria-label="엔진 선택" style="min-width:150px;">
                  <button type="button" id="seg-engine-v1" class="as-seg__item is-on" onclick="selectEngineMode('v1')">v1 기본</button>
                  <button type="button" id="seg-engine-v2" class="as-seg__item" onclick="selectEngineMode('v2')">v2 하네스·교차검증</button>
                </div>
                <button type="button" class="as-btn" onclick="runV2Selftest()" title="H0 사전 자가진단 실행">자가진단</button>
                <button type="button" class="as-btn" id="btn-v2-resume" onclick="resumeV2Execution()" title="체크포인트부터 이어서 실행">이어서 실행</button>
              </div>
              <div class="as-run__sub" id="v2-extra-controls" style="display:none;margin-bottom:4px;">
                <label style="font-size:11.5px;color:var(--ink-muted);display:inline-flex;align-items:center;gap:4px;cursor:pointer;">
                  <input type="checkbox" id="chk-v2-dbpia-review" checked> 최종 DBpia 검수(X4)
                </label>
                <button type="button" class="as-btn" onclick="approveV2Outline()">목차 승인</button>
                <button type="button" class="as-btn" onclick="exportV2Handoff()">2차 다듬기용 내보내기</button>
                <button type="button" class="as-btn" onclick="importV2Handoff()">2차 원고 불러오기</button>
              </div>
              <div id="v2-gate-banner" style="display:none;padding:6px 8px;border:1px solid var(--line-strong);border-radius:var(--radius-6);background:var(--warn-soft);color:var(--warn);font-size:11.5px;">
                <div id="v2-gate-banner-text" style="margin-bottom:4px;font-weight:600;">게이트 확인 대기 중</div>
                <div class="as-run__sub">
                  <button type="button" class="as-btn" onclick="continueV2Gate()">경고 무시하고 계속</button>
                  <button type="button" class="as-btn" onclick="resolveG7Review('export')">그대로 내보내기(G7)</button>
                  <button type="button" class="as-btn" onclick="resolveG7Review('rework')">해당 절 재작성(G7)</button>
                  <button type="button" class="as-btn" onclick="resolveG7Review('abort')">중단</button>
                </div>
              </div>
              <div class="as-run__main">
                <button type="button" id="btn-run-report" class="as-btn as-btn--accent as-btn--lg as-btn--block" onclick="startExecution('report')">
                  <svg class="as-icon" width="16" height="16" viewBox="0 0 24 24"><path d="M8 5v14l11-7z" fill="currentColor" stroke="currentColor" stroke-width="1.2" stroke-linejoin="round"/></svg>
                  <span id="btn-run-report-label">발굴조사보고서 고찰 실행</span>
                </button>
                <button type="button" id="btn-run-paper" class="as-btn as-btn--default as-btn--lg as-btn--block" onclick="startExecution('paper')">
                  <svg class="as-icon" width="16" height="16" viewBox="0 0 24 24"><path d="M8 5v14l11-7z" fill="currentColor" stroke="currentColor" stroke-width="1.2" stroke-linejoin="round"/></svg>
                  <span id="btn-run-paper-label">논문 실행</span>
                </button>
              </div>
              <div class="as-run__ctl" id="run-ctl-row" style="display:none;">
                <button type="button" class="as-btn" id="btn-pause" onclick="pauseExecution()">
                  <svg class="as-icon" width="14" height="14" viewBox="0 0 24 24"><rect x="7" y="5" width="3.6" height="14" rx=".8" fill="currentColor"/><rect x="13.4" y="5" width="3.6" height="14" rx=".8" fill="currentColor"/></svg>
                  <span id="btn-pause-label">잠시 멈춤</span>
                </button>
                <button type="button" class="as-btn" id="btn-stop" onclick="stopExecution()">
                  <svg class="as-icon" width="14" height="14" viewBox="0 0 24 24"><rect x="6" y="6" width="12" height="12" rx="1.5" fill="currentColor"/></svg>
                  <span>그만두기</span>
                </button>
              </div>
              <div class="as-run__sub">
                <button type="button" class="as-btn as-btn--brass" onclick="openDbpiaDialog()">
                  <svg class="as-icon" width="14" height="14" viewBox="0 0 24 24"><circle cx="8" cy="15.5" r="4" fill="none" stroke="currentColor" stroke-width="1.6"/><path d="M11 12.5l8.5-8.5M16 8l2.5 2.5M13.8 10.2l2 2" fill="none" stroke="currentColor" stroke-width="1.6" stroke-linecap="round"/></svg>
                  디비피아 로그인
                </button>
                <span class="as-tag as-tag--brass" id="dbpia-status-tag">로그인됨</span>
                <span class="as-run__spacer"></span>
                <button type="button" class="as-btn as-btn--quiet" id="btn-export-pdf" onclick="openExportDialog()">
                  <svg class="as-icon" width="14" height="14" viewBox="0 0 24 24"><path d="M12 3v11M8 7l4-4 4 4M5 13v7h14v-7" fill="none" stroke="currentColor" stroke-width="1.6" stroke-linejoin="round"/></svg>
                  PDF로 내보내기
                </button>
              </div>
            </div>
          </div>
        </section>

        <section class="as-step" aria-label="진행 기록 및 /goal 교차검증">
          <div class="as-step__head">
            <span class="as-step__n">6</span>
            <span class="as-step__title">진행 기록 · /goal 교차검증</span>
            <span class="as-step__aside" id="goal-score-badge"><span class="as-tag">8대 기준</span></span>
          </div>
          <div class="as-step__body">
            <div class="as-goal-box" id="goal-scorecard-box">
              <div class="as-goal-head">
                <span id="goal-standard-title">문화재발굴조사보고서 『Ⅳ. 고찰』 8대 판단 기준</span>
                <span id="goal-standard-summary" style="font:11px var(--font-mono);color:var(--ink-muted);">계획 수립됨</span>
              </div>
              <ul class="as-goal-grid" id="goal-criteria-grid">
                <li class="as-goal-item"><span class="as-goal-item__name">1. DBpia 다각도 논문</span><span class="as-goal-item__badge">대기</span></li>
                <li class="as-goal-item"><span class="as-goal-item__name">2. 필수 장·절 완결성</span><span class="as-goal-item__badge">대기</span></li>
                <li class="as-goal-item"><span class="as-goal-item__name">3. 학술 논증·분량</span><span class="as-goal-item__badge">대기</span></li>
                <li class="as-goal-item"><span class="as-goal-item__name">4. 정량 학술 표 2종</span><span class="as-goal-item__badge">대기</span></li>
                <li class="as-goal-item"><span class="as-goal-item__name">5. 차트·모식도 수록</span><span class="as-goal-item__badge">대기</span></li>
                <li class="as-goal-item"><span class="as-goal-item__name">6. 실전 각주·실저자</span><span class="as-goal-item__badge">대기</span></li>
                <li class="as-goal-item"><span class="as-goal-item__name">7. 참고문헌 1:1 일치</span><span class="as-goal-item__badge">대기</span></li>
                <li class="as-goal-item"><span class="as-goal-item__name">8. 원자료 정합·완결</span><span class="as-goal-item__badge">대기</span></li>
              </ul>
            </div>
            <div class="as-log">
              <div class="as-log__head" id="log-progress-head" style="display:none;">
                <div class="as-progress">
                  <div class="as-progress__row">
                    <span id="log-progress-label">본문 작성 및 교차검증</span>
                    <span class="as-progress__pct" id="log-progress-pct">0%</span>
                  </div>
                  <div class="as-progress__track" role="progressbar">
                    <div class="as-progress__fill" id="log-progress-fill" style="width:0%"></div>
                  </div>
                </div>
              </div>
              <ol class="as-log__list" id="log-list" aria-live="polite">
                <p class="as-log__empty" id="log-empty">실행하면 /goal 계획·DBpia 수집·교차검증 기록이 여기에 쌓입니다.</p>
              </ol>
            </div>
          </div>
        </section>
      </div>
    </aside>

    <!-- 4. 우측 넓은 창: WordCraft A4 편집 화면 -->
    <main class="as-screen__canvas">
      <div class="as-collect" role="status" id="collect-strip">
        <span class="as-collect__item" id="collect-left">
          <i class="as-collect__dot is-hollow"></i>
          <span>실행 전</span>
        </span>
        <span class="as-collect__r">
          <button type="button" class="as-btn as-btn--brass" id="btn-toggle-dbpia-board" style="height:22px;padding:0 10px;font-size:11px;display:none;" onclick="toggleDbpiaBoard()">DBpia 검색 요점 · 수집 논문 보기 ▾</button>
          <span id="collect-right-time">WordCraft .docx 대기</span>
          <button type="button" class="as-btn" style="height:22px;padding:0 10px;font-size:11px;" onclick="openInWordCraft()">WordCraft 창 열기</button>
        </span>
      </div>

      <div class="as-screen__scroll" id="doc-scroll">
        <!-- DBpia AI 실시간 다각도 검색 요점 & 실존 논문 수집 현황 보드 -->
        <section class="as-dbpia-board" id="dbpia-live-board" style="display:none;" aria-label="DBpia AI 실시간 다각도 검색 요점 및 수집 논문">
          <div class="as-dbpia-board__head">
            <span id="dbpia-board-title">🔍 DBpia AI 다각도 검색 요점(vetter_conversations) 및 실존 논문 서지·인용문(hovers) 실시간 수집 현황</span>
            <button type="button" class="as-btn as-btn--quiet" style="height:20px;padding:0 8px;font-size:11px;" onclick="toggleDbpiaBoard(false)">접기 ▴</button>
          </div>
          <div class="as-dbpia-board__body" id="dbpia-board-body"></div>
        </section>

        <!-- A4 다중 페이지 컨테이너 (1쪽, 2쪽, 3쪽... 자동 분할) -->
        <div class="as-pages-wrap" id="doc-pages-wrap">
          <article class="as-page" id="doc-page" contenteditable="true" spellcheck="false">
            <div class="as-page__body" id="doc-page-body">
              <p class="as-page__empty" id="doc-page-empty">실행하면 이 자리에 『Ⅳ. 고찰』이 작성됩니다. 직접 써 넣으셔도 됩니다.</p>
            </div>
            <div class="as-page__notes" id="doc-page-notes" style="display:none;">
              <hr>
              <ol id="doc-page-notes-list"></ol>
            </div>
            <div class="as-page__num" id="doc-page-num">- 1 -</div>
          </article>
        </div>
      </div>
    </main>
  </div>

  <!-- 5. 상태줄 (StatusBar) -->
  <div class="as-status" role="status">
    <span class="as-status__hint" id="status-hint">요청과 자료를 넣고 실행하세요. 비워 둔 자료는 빼고 씁니다.</span>
    <div class="as-status__r">
      <span class="as-status__item" id="status-pages">쪽 1 / 1</span>
      <span class="as-status__item is-mono" id="status-chars">글자 0</span>
      <span class="as-status__item is-mono" style="cursor:pointer;" onclick="cycleZoom()" id="status-zoom">90% ▾</span>
    </div>
  </div>

  <!-- 디비피아 로그인 대화상자 (DbpiaWindow) -->
  <div class="as-screen__scrim" id="scrim-dbpia">
    <div class="as-dialog" role="dialog" aria-label="디비피아 로그인" style="width:560px;">
      <div class="as-ph as-dialog__head">
        <span class="as-ph__title">디비피아 로그인</span>
        <span class="as-ph__sub">학술논문을 찾을 때 쓰는 로그인입니다</span>
      </div>
      <div class="as-dialog__body">
        <div class="as-web">
          <div class="as-web__bar">
            <svg class="as-icon" width="13" height="13" viewBox="0 0 24 24"><circle cx="8" cy="15.5" r="4" fill="none" stroke="currentColor" stroke-width="1.6"/><path d="M11 12.5l8.5-8.5M16 8l2.5 2.5M13.8 10.2l2 2" fill="none" stroke="currentColor" stroke-width="1.6" stroke-linecap="round"/></svg>
            <span>https://ai.dbpia.co.kr</span>
          </div>
          <div class="as-web__view">
            <p>디비피아 사이트의 로그인 화면을 안티그라비티 내장 창에 열었습니다.</p>
            <p class="as-web__sub">개인·학교·기관 로그인 모두 진행하신 뒤, 끝나면 아래 단추로 창을 닫으세요.</p>
            <button type="button" class="as-btn as-btn--brass" style="margin-top:6px;" onclick="triggerBackendDbpiaLogin()">디비피아 로그인 창 다시 띄우기</button>
          </div>
        </div>
      </div>
      <div class="as-dialog__foot">
        <span class="as-dialog__state"><span class="as-tag as-tag--brass">로그인 연동됨</span></span>
        <button type="button" class="as-btn as-btn--strong" onclick="closeDbpiaDialog()">닫고 계속하기</button>
      </div>
    </div>
  </div>

  <!-- PDF로 내보내기 대화상자 (ExportDialog) -->
  <div class="as-screen__scrim" id="scrim-export">
    <div class="as-dialog" role="dialog" aria-label="PDF로 내보내기" style="width:520px;">
      <div class="as-ph as-dialog__head">
        <span class="as-ph__title">PDF로 내보내기</span>
        <span class="as-ph__sub" id="export-source-label">발굴조사보고서 Ⅳ 고찰.docx</span>
      </div>
      <div class="as-dialog__body">
        <div class="as-frow">
          <span class="as-frow__label">파일 이름</span>
          <div class="as-frow__ctl">
            <input type="text" class="as-field" id="export-filename-input" value="발굴조사보고서 Ⅳ 고찰.pdf">
          </div>
        </div>
        <div class="as-frow">
          <span class="as-frow__label">범위</span>
          <div class="as-frow__ctl">
            <div class="as-chips">
              <button type="button" class="as-chip is-on">문서 전체</button>
              <button type="button" class="as-chip" onclick="this.parentElement.querySelectorAll('.as-chip').forEach(c=>c.classList.remove('is-on'));this.classList.add('is-on');">지금 쪽</button>
            </div>
          </div>
        </div>
        <div class="as-frow">
          <span class="as-frow__label">내보낸 뒤</span>
          <div class="as-frow__ctl">
            <div class="as-chips" id="export-after-chips">
              <button type="button" class="as-chip is-on" data-after="open-pdf" onclick="selectExportAfter(this)">PDF 열기</button>
              <button type="button" class="as-chip" data-after="open-folder" onclick="selectExportAfter(this)">폴더만 열기</button>
              <button type="button" class="as-chip" data-after="none" onclick="selectExportAfter(this)">아무것도 안 함</button>
            </div>
          </div>
        </div>
        <p class="as-dialog__note">각주는 쪽 아래, 참고문헌은 문서 끝 그대로 옮깁니다. 작업은 계속 .docx에서 합니다.</p>
      </div>
      <div class="as-dialog__foot">
        <button type="button" class="as-btn" onclick="closeExportDialog()">그만두기</button>
        <button type="button" class="as-btn as-btn--strong" onclick="confirmExportPdf()">
          <svg class="as-icon" width="14" height="14" viewBox="0 0 24 24"><path d="M12 3v11M8 7l4-4 4 4M5 13v7h14v-7" fill="none" stroke="currentColor" stroke-width="1.6" stroke-linejoin="round"/></svg>
          내보내기
        </button>
      </div>
    </div>
  </div>
</div>

<script>
let currentDocType = 'report';
let studioOpen = true;
let showProcessed = false;
let currentEngineeredPrompt = '';
let lastRenderedText = '';
let lastRenderedPagesCount = 1;
let dbpiaBoardManualOverride = null;
let lastDbpiaSignature = '';
let uploadedReportFiles = [];
let uploadedDatingFiles = [];
let exportAfterAction = 'open-pdf';
let currentZoomIdx = 0;
const ZOOMS = [0.9, 1.0, 1.1];

function escapeHtml(str) {
  return String(str || '')
    .replace(/&/g, '&amp;')
    .replace(/</g, '&lt;')
    .replace(/>/g, '&gt;');
}

function toggleDbpiaBoard(forceState) {
  const board = document.getElementById('dbpia-live-board');
  if (!board) return;
  const next = typeof forceState === 'boolean' ? forceState : (board.style.display === 'none');
  dbpiaBoardManualOverride = next;
  board.style.display = next ? 'block' : 'none';
  const btn = document.getElementById('btn-toggle-dbpia-board');
  if (btn) {
    btn.textContent = next ? 'DBpia 검색 요점 · 수집 논문 닫기 ▴' : 'DBpia 검색 요점 · 수집 논문 보기 ▾';
  }
}

function toggleTheme() {
  const html = document.documentElement;
  const next = html.getAttribute('data-theme') === 'dark' ? 'light' : 'dark';
  html.setAttribute('data-theme', next);
  document.getElementById('theme-toggle-btn').textContent = next === 'dark' ? '밝은 화면' : '어두운 화면';
}

function toggleStudioPanel() {
  studioOpen = !studioOpen;
  const body = document.getElementById('screen-body');
  const tile = document.getElementById('ribbon-studio-tile');
  body.classList.toggle('is-collapsed', !studioOpen);
  tile.classList.toggle('is-sel', studioOpen);
}

function cycleZoom() {
  currentZoomIdx = (currentZoomIdx + 1) % ZOOMS.length;
  const z = ZOOMS[currentZoomIdx];
  document.querySelectorAll('.as-page').forEach(p => { p.style.zoom = String(z); });
  document.getElementById('status-zoom').textContent = Math.round(z * 100) + '% ▾';
}

function togglePageFont() {
  const pages = document.querySelectorAll('.as-page');
  if (!pages.length) return;
  const isSans = pages[0].style.fontFamily && pages[0].style.fontFamily.includes('Sans');
  const nextFont = isSans ? 'var(--font-serif)' : 'var(--font-sans)';
  pages.forEach(p => { p.style.fontFamily = nextFont; });
}

function inferActiveDocumentBaseName() {
  const isPaper = currentDocType === 'paper';
  const focusVal = (document.getElementById('input-focus')?.value || '').trim();
  if (focusVal) {
    const cleaned = focusVal.replace(/[\\/:*?"<>|]/g, ' ').replace(/\s+/g, ' ').trim().slice(0, 28);
    if (cleaned) {
      return isPaper
        ? (cleaned.includes('논문') ? cleaned : cleaned + ' 논문')
        : (cleaned.includes('고찰') ? cleaned : cleaned + ' Ⅳ 고찰');
    }
  }
  if (uploadedReportFiles.length > 0 && uploadedReportFiles[0].name) {
    const base = String(uploadedReportFiles[0].name).replace(/\.[^.]+$/, '').trim().slice(0, 28);
    if (base) {
      return isPaper ? (base + ' 논문') : (base + ' Ⅳ 고찰');
    }
  }
  return isPaper ? '학술논문' : '발굴조사보고서 Ⅳ 고찰';
}

function syncDynamicDocumentTitle() {
  const docName = inferActiveDocumentBaseName() + '.docx';
  const winTitleEl = document.getElementById('win-doc-title');
  if (winTitleEl) winTitleEl.textContent = docName + ' - ARCHAEO STUDIO';
  document.title = docName + ' - ARCHAEO STUDIO';
  const exportLabel = document.getElementById('export-source-label');
  if (exportLabel) exportLabel.textContent = docName;
  const exportInput = document.getElementById('export-filename-input');
  if (exportInput) exportInput.value = docName.replace('.docx', '.pdf');
}

function selectDocMode(mode) {
  currentDocType = mode === 'paper' ? 'paper' : 'report';
  const isPaper = currentDocType === 'paper';
  const segReport = document.getElementById('seg-report');
  const segPaper = document.getElementById('seg-paper');
  segReport.classList.toggle('is-on', !isPaper);
  segReport.setAttribute('aria-checked', String(!isPaper));
  segPaper.classList.toggle('is-on', isPaper);
  segPaper.setAttribute('aria-checked', String(isPaper));

  // 원칙 3: 흙색(clay) 단추는 고른 문서 종류의 실행 단추 하나뿐
  const btnReport = document.getElementById('btn-run-report');
  const btnPaper = document.getElementById('btn-run-paper');
  btnReport.className = 'as-btn as-btn--lg as-btn--block ' + (!isPaper ? 'as-btn--accent' : 'as-btn--default');
  btnPaper.className = 'as-btn as-btn--lg as-btn--block ' + (isPaper ? 'as-btn--accent' : 'as-btn--default');

  syncDynamicDocumentTitle();

  const emptyEl = document.getElementById('doc-page-empty');
  if (emptyEl) {
    emptyEl.textContent = '실행하면 이 자리에 ' + (isPaper ? '논문' : '『Ⅳ. 고찰』') + '이 작성됩니다. 직접 써 넣으셔도 됩니다.';
  }
  refreshEngineeredPrompt(true);
}

function toggleProcessedPrompt() {
  showProcessed = !showProcessed;
  const btn = document.getElementById('toggle-processed-btn');
  const box = document.getElementById('processed-prompt-box');
  btn.setAttribute('aria-expanded', String(showProcessed));
  btn.textContent = (showProcessed ? '▾ ' : '▸ ') + '가공된 요청 보기';
  box.style.display = showProcessed ? 'block' : 'none';
  if (showProcessed) refreshEngineeredPrompt(true);
}

function updateDatingEmptyTag() {
  const val = (document.getElementById('input-dating').value || '').trim();
  const tag = document.getElementById('dating-empty-tag');
  if (tag) {
    tag.style.display = (!val && uploadedDatingFiles.length === 0) ? 'inline-block' : 'none';
  }
}

function buildClientSideEngineeredPrompt() {
  const userFocus = (document.getElementById('input-focus').value || '').trim();
  const reportText = (document.getElementById('input-report').value || '').trim();
  const datingText = (document.getElementById('input-dating').value || '').trim();
  const isPaper = currentDocType === 'paper';
  const docLabel = isPaper
    ? '학술논문 (『한국고고학보』·『역사학보』 동료심사 규격 · 문제 구속적·연역적 5단계)'
    : '발굴조사보고서 Ⅳ. 고찰 (「매장유산 보호 및 조사에 관한 법률」 [별표 2] · 공간 구속적·귀납적 4대 영역)';
  const topic = userFocus || (reportText ? '입력된 조사 자료 기반 유구·유물 성격 및 편년 검토' : '조사 유적의 유구·출토유물 양상과 주변 비교유적 편년 검토');
  return [
    '[' + docLabel + '] ' + topic,
    isPaper
      ? '구성: 국문제목 · 국문초록·주제어 · Ⅰ. 머리말(문제 제기+비판적 연구사 정리) · Ⅱ~Ⅳ. 본론(사료 세포[표제어·출처·수집/가공정보] 재분류 및 개념어·논지 도출) · Ⅴ. 맺음말 · 영문초록 · 각주 · 참고문헌'
      : '구성: Ⅳ. 고찰 (1. 머리말[조사경위·학술자문회의·연구사] · 2. 유구 고찰[층위·중복 분기·기초/내부시설·자북 주축·해발고도] · 3. 출토유물 및 편년[일련번호 1:1 매칭·잔존치 ( )·상하한 연대] · 4. 주변 비교유적 및 맺음말[정비·복원 제언]) · 각주 · 참고문헌',
    '표·차트·모식도: <표 1>~<표 4>(유구·유물·주변유적·선행연구 비교표 4종+) · wc-chart 비교차트 3종+ · wc-diagram 모식도 3종+',
    '서지(DBpia AI 하이브리드): vetter_conversations 전체 요점 + hovers 원문 인용구 기반 실제 저자·연도·「논문명」·『학술지』·쪽수 1:1 교차검증 (저자미상 0건)',
    '원칙: ' + (datingText ? '입력된 연대분석·자연과학 자료 융합 고찰 반영' : '미입력 연대분석(BP) 수치 날조 금지 · 상대편년 논증')
  ].join('\\n• ');
}

async function refreshEngineeredPrompt() {
  syncDynamicDocumentTitle();
  const fallback = buildClientSideEngineeredPrompt();
  currentEngineeredPrompt = fallback;
  const box = document.getElementById('processed-prompt-box');
  if (box) box.textContent = fallback;
  try {
    const res = await fetch('/local-action', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({
        action: 'preview-engineered-prompt',
        docType: currentDocType,
        userFocus: document.getElementById('input-focus').value || '',
        reportText: document.getElementById('input-report').value || '',
        datingText: document.getElementById('input-dating').value || ''
      })
    });
    const data = await res.json();
    if (data && data.engineeredPrompt) {
      currentEngineeredPrompt = data.engineeredPrompt;
      if (box) box.textContent = data.engineeredPrompt;
    }
  } catch (e) {}
}

function renderFileChips(slot) {
  const list = slot === 'dating' ? uploadedDatingFiles : uploadedReportFiles;
  const container = document.getElementById(slot === 'dating' ? 'dating-file-chips' : 'report-file-chips');
  if (!container) return;
  container.innerHTML = list.map((f, idx) =>
    '<span class="as-file">' +
      '<span class="as-file__name">' + escapeHtml(f.name) + '</span>' +
      (f.size ? '<span class="as-file__size">' + escapeHtml(f.size) + '</span>' : '') +
      '<button type="button" class="as-file__x" onclick="removeFileChip(\\'' + slot + '\\',' + idx + ')">✕</button>' +
    '</span>'
  ).join('');
  updateDatingEmptyTag();
}

function removeFileChip(slot, idx) {
  if (slot === 'dating') uploadedDatingFiles.splice(idx, 1);
  else uploadedReportFiles.splice(idx, 1);
  renderFileChips(slot);
}

function fileToBase64(file) {
  return new Promise((resolve, reject) => {
    const r = new FileReader();
    r.onload = () => {
      const s = String(r.result || '');
      const i = s.indexOf(',');
      resolve(i >= 0 ? s.slice(i + 1) : s);
    };
    r.onerror = reject;
    r.readAsDataURL(file);
  });
}

async function handleFileUpload(inputEl, targetSlot) {
  const files = Array.from(inputEl.files || []);
  if (!files.length) return;
  const targetTextarea = document.getElementById(targetSlot === 'dating' ? 'input-dating' : 'input-report');
  for (const f of files) {
    const sizeStr = Math.max(1, Math.round(f.size / 1024)) + ' KB';
    try {
      const base64Data = await fileToBase64(f);
      const res = await fetch('/local-action', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({
          action: 'parse-uploaded-file',
          fileName: f.name,
          base64Data,
          targetSlot
        })
      });
      const data = await res.json();
      if (data && data.ok) {
        const block = '[' + f.name + ']\\n' + (data.extractedText || '');
        targetTextarea.value = targetTextarea.value ? (targetTextarea.value + '\\n\\n' + block) : block;
        if (targetSlot === 'dating') uploadedDatingFiles.push({ name: f.name, size: sizeStr });
        else uploadedReportFiles.push({ name: f.name, size: sizeStr });
      }
    } catch (e) {}
  }
  renderFileChips(targetSlot);
  refreshEngineeredPrompt(true);
  inputEl.value = '';
}

let currentEngineMode = 'v1';
let currentV2JobId = '';
let currentV2PendingGate = '';

function selectEngineMode(engine) {
  currentEngineMode = engine === 'v2' ? 'v2' : 'v1';
  const v1Btn = document.getElementById('seg-engine-v1');
  const v2Btn = document.getElementById('seg-engine-v2');
  const v2Controls = document.getElementById('v2-extra-controls');
  if (v1Btn) v1Btn.classList.toggle('is-on', currentEngineMode === 'v1');
  if (v2Btn) v2Btn.classList.toggle('is-on', currentEngineMode === 'v2');
  if (v2Controls) v2Controls.style.display = currentEngineMode === 'v2' ? 'flex' : 'none';
}

async function runV2Selftest() {
  const hintEl = document.getElementById('status-hint');
  if (hintEl) hintEl.textContent = 'v2 사전 자가진단(H0)을 실행 중입니다…';
  try {
    const res = await fetch('/v2/selftest');
    const data = await res.json();
    if (hintEl) {
      hintEl.textContent = data.ok
        ? 'v2 자가진단 통과 (T1~T12 정상)'
        : 'v2 자가진단 확인 필요: ' + ((data.items || []).filter(i => !i.ok).map(i => i.id + ':' + i.detail).join(', '));
    }
  } catch (e) {
    if (hintEl) hintEl.textContent = 'v2 자가진단 요청 실패: ' + e.message;
  }
}

async function resumeV2Execution() {
  try {
    const res = await fetch('/v2/resume', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ jobId: currentV2JobId || undefined })
    });
    const data = await res.json();
    if (data && data.jobId) currentV2JobId = data.jobId;
    if (data && data.manuscriptMd) renderManuscriptOntoA4Page(data.manuscriptMd, false);
    pollStatus();
  } catch (e) {}
}

async function approveV2Outline() {
  try {
    const res = await fetch('/v2/outline/approve', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ jobId: currentV2JobId || undefined })
    });
    const data = await res.json();
    if (data && data.manuscriptMd) renderManuscriptOntoA4Page(data.manuscriptMd, false);
    pollStatus();
  } catch (e) {}
}

async function continueV2Gate() {
  try {
    const res = await fetch('/v2/gate/continue', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ jobId: currentV2JobId || undefined, gate: currentV2PendingGate || 'G2' })
    });
    const data = await res.json();
    if (data && data.manuscriptMd) renderManuscriptOntoA4Page(data.manuscriptMd, false);
    pollStatus();
  } catch (e) {}
}

async function exportV2Handoff() {
  const hintEl = document.getElementById('status-hint');
  try {
    const res = await fetch('/v2/handoff/export', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ jobId: currentV2JobId || undefined })
    });
    const data = await res.json();
    if (hintEl) hintEl.textContent = data.ok ? ('2차 다듬기용 인계 묶음 내보내기 완료: ' + (data.handoffDir || '')) : ('인계 내보내기 실패: ' + (data.error || ''));
  } catch (e) {}
}

async function importV2Handoff() {
  const hintEl = document.getElementById('status-hint');
  try {
    const res = await fetch('/v2/handoff/import', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ jobId: currentV2JobId || undefined })
    });
    const data = await res.json();
    if (data && data.manuscriptMd) renderManuscriptOntoA4Page(data.manuscriptMd, false);
    if (hintEl) hintEl.textContent = data.ok ? '2차 원고 수신 및 V1~V8 재검사 통과' : ('2차 원고 수신 검사 실패: ' + (data.error || ''));
  } catch (e) {}
}

async function resolveG7Review(action) {
  try {
    const res = await fetch('/v2/dbpia-review', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ jobId: currentV2JobId || undefined, action })
    });
    const data = await res.json();
    if (data && data.manuscriptMd) renderManuscriptOntoA4Page(data.manuscriptMd, false);
    pollStatus();
  } catch (e) {}
}

async function startExecution(mode) {
  if (mode) selectDocMode(mode);
  await refreshEngineeredPrompt(false);
  if (currentEngineMode === 'v2') {
    const chkReview = document.getElementById('chk-v2-dbpia-review');
    try {
      const res = await fetch('/v2/run', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({
          docType: currentDocType,
          userFocus: document.getElementById('input-focus').value || '',
          reportText: document.getElementById('input-report').value || '',
          datingText: document.getElementById('input-dating').value || '',
          enableDbpiaReview: chkReview ? Boolean(chkReview.checked) : true
        })
      });
      const data = await res.json();
      if (data && data.jobId) currentV2JobId = data.jobId;
      if (data && data.gate && !data.gate.ok) {
        currentV2PendingGate = data.gate.gate || '';
        const banner = document.getElementById('v2-gate-banner');
        const bannerTxt = document.getElementById('v2-gate-banner-text');
        if (banner) banner.style.display = 'block';
        if (bannerTxt) bannerTxt.textContent = '[' + (data.gate.gate || 'GATE') + '] ' + (data.gate.reason || '');
      }
      if (data && data.manuscriptMd) renderManuscriptOntoA4Page(data.manuscriptMd, false);
      pollStatus();
    } catch (e) {}
    return;
  }
  try {
    await fetch('/local-action', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({
        action: 'start-arch-pipeline',
        docType: currentDocType,
        runMode: 'unified',
        userFocus: document.getElementById('input-focus').value || '',
        engineeredPrompt: currentEngineeredPrompt,
        reportText: document.getElementById('input-report').value || '',
        datingText: document.getElementById('input-dating').value || ''
      })
    });
    pollStatus();
  } catch (e) {}
}

async function pauseExecution() {
  try {
    await fetch('/local-action', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ action: 'pause-arch-pipeline' })
    });
    pollStatus();
  } catch (e) {}
}

async function stopExecution() {
  try {
    await fetch('/local-action', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ action: 'stop-arch-pipeline' })
    });
    pollStatus();
  } catch (e) {}
}

function openDbpiaDialog() {
  document.getElementById('scrim-dbpia').classList.add('is-open');
  document.getElementById('status-hint').textContent = '디비피아 로그인 창이 열려 있습니다. 로그인이 끝나면 창을 닫으세요.';
  triggerBackendDbpiaLogin();
}

function closeDbpiaDialog() {
  document.getElementById('scrim-dbpia').classList.remove('is-open');
  document.getElementById('status-hint').textContent = '요청과 자료를 넣고 실행하세요. 비워 둔 자료는 빼고 씁니다.';
}

async function triggerBackendDbpiaLogin() {
  try {
    await fetch('/local-action', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ action: 'dbpia-open-login' })
    });
  } catch (e) {}
}

function openExportDialog() {
  document.getElementById('scrim-export').classList.add('is-open');
  document.getElementById('status-hint').textContent = 'PDF로 내보낼 설정을 고르세요.';
}

function closeExportDialog() {
  document.getElementById('scrim-export').classList.remove('is-open');
}

function selectExportAfter(btn) {
  document.querySelectorAll('#export-after-chips .as-chip').forEach(c => c.classList.remove('is-on'));
  btn.classList.add('is-on');
  exportAfterAction = btn.getAttribute('data-after') || 'open-pdf';
}

async function confirmExportPdf() {
  closeExportDialog();
  document.getElementById('status-hint').textContent = 'PDF 파일을 내보내고 있습니다…';
  try {
    const res = await fetch('/local-action', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({
        action: exportAfterAction === 'open-pdf' ? 'open-in-pdfcraft' : 'export-to-craft',
        docType: currentDocType,
        exportPdf: true,
        autoOpenPdfCraft: exportAfterAction === 'open-pdf',
        autoOpenWordCraft: false
      })
    });
    await res.json();
    if (exportAfterAction === 'open-folder') {
      await fetch('/local-action', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ action: 'open-output-folder' })
      });
    }
    document.getElementById('status-hint').textContent = 'PDF 내보내기를 마쳤습니다. 작업은 계속 .docx에서 합니다.';
  } catch (e) {}
}

async function openInWordCraft() {
  try {
    await fetch('/local-action', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ action: 'open-in-wordcraft', docType: currentDocType })
    });
  } catch (e) {}
}

async function saveAndOpenWordCraft() {
  try {
    await fetch('/local-action', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({
        action: 'export-to-craft',
        docType: currentDocType,
        exportPdf: false,
        autoOpenWordCraft: true
      })
    });
  } catch (e) {}
}

function newDocument() {
  const wrap = document.getElementById('doc-pages-wrap');
  if (wrap) {
    wrap.innerHTML =
      '<article class="as-page" id="doc-page" contenteditable="true" spellcheck="false">' +
        '<div class="as-page__body" id="doc-page-body">' +
          '<p class="as-page__empty" id="doc-page-empty">실행하면 이 자리에 ' +
          (currentDocType === 'paper' ? '논문' : '『Ⅳ. 고찰』') +
          '이 작성됩니다. 직접 써 넣으셔도 됩니다.</p>' +
        '</div>' +
        '<div class="as-page__notes" id="doc-page-notes" style="display:none;">' +
          '<hr><ol id="doc-page-notes-list"></ol>' +
        '</div>' +
        '<div class="as-page__num" id="doc-page-num">- 1 -</div>' +
      '</article>';
  }
  lastRenderedText = '';
  lastRenderedPagesCount = 1;
}

function getActivePageBody() {
  const bodies = document.querySelectorAll('.as-page__body');
  return bodies.length > 0 ? bodies[bodies.length - 1] : document.getElementById('doc-page-body');
}

function insertSampleTable() {
  const body = getActivePageBody();
  const empty = document.getElementById('doc-page-empty');
  if (empty) empty.remove();
  body.insertAdjacentHTML('beforeend',
    '<figure class="as-doc-table"><figcaption>표. 유구 속성 일람표</figcaption>' +
    '<table><thead><tr><th>유구명</th><th>평면형태</th><th>규모(m)</th><th>출토유물</th></tr></thead>' +
    '<tbody><tr><td>1호 주거지</td><td>방형</td><td>4.8×4.2</td><td>타날문단경호</td></tr></tbody></table></figure>'
  );
}

function insertSampleDiagram() {
  const body = getActivePageBody();
  const empty = document.getElementById('doc-page-empty');
  if (empty) empty.remove();
  body.insertAdjacentHTML('beforeend',
    '<div class="as-doc-diagram"><div class="as-doc-diagram__title">도면. 단계별 유구·출토유물 변천 모식도</div>' +
    '<div class="as-doc-diagram__steps">' +
      '<div class="as-doc-diagram__step"><strong>Ⅰ단계</strong>방형 평면 · 중앙 노지 · 전기 유물 조합</div>' +
      '<div class="as-doc-diagram__step"><strong>Ⅱ단계</strong>방형·장방형 · 부뚜막 등장 · 타날문토기 확산</div>' +
      '<div class="as-doc-diagram__step"><strong>Ⅲ단계</strong>凸자형·구들 정착 · 후기 기종 공반</div>' +
    '</div></div>'
  );
}

function insertSampleFootnote() {
  const notesBoxes = document.querySelectorAll('.as-page__notes');
  const notesBox = notesBoxes.length > 0 ? notesBoxes[notesBoxes.length - 1] : document.getElementById('doc-page-notes');
  if (!notesBox) return;
  const list = notesBox.querySelector('ol');
  notesBox.style.display = 'block';
  const nextNum = list.children.length + 1;
  list.insertAdjacentHTML('beforeend',
    '<li><span class="as-page__fnn">' + nextNum + ')</span><span>저자명, 발행연도, 「논문 제목」, 『학술지명』 권호, 발행기관, 45~48쪽.</span></li>'
  );
}

function findInDoc(q) {
  if (!q) return;
  window.find(q);
}

/**
 * DBpia AI 다각도 검색 요점(vetter_conversations) 및 실존 논문 서지·인용문(hovers) 실시간 현황판 렌더링
 */
function renderDbpiaLiveBoard(p) {
  const board = document.getElementById('dbpia-live-board');
  const bodyEl = document.getElementById('dbpia-board-body');
  const titleEl = document.getElementById('dbpia-board-title');
  const toggleBtn = document.getElementById('btn-toggle-dbpia-board');
  if (!board || !bodyEl) return;

  const feed = Array.isArray(p.liveResearchFeed) ? p.liveResearchFeed : [];
  const papers = Array.isArray(p.collectedPapers) ? p.collectedPapers : [];
  const isDbpiaPhase = Boolean(p.running && p.phase && p.phase.includes('DBPIA'));
  const hasData = feed.length > 0 || papers.length > 0;

  if (toggleBtn) {
    toggleBtn.style.display = (hasData || isDbpiaPhase) ? 'inline-flex' : 'none';
  }

  if (!hasData && !isDbpiaPhase) {
    board.style.display = 'none';
    return;
  }

  // Auto-open during PHASE_2_DBPIA_LOOP unless manually closed; auto-collapse when writing starts unless manually opened
  if (dbpiaBoardManualOverride === null) {
    const liveMd = (p.latestManuscript && p.latestManuscript.trim()) || (p.liveStreamText && p.liveStreamText.trim()) || '';
    const shouldShow = isDbpiaPhase || (hasData && !liveMd);
    board.style.display = shouldShow ? 'block' : 'none';
    if (toggleBtn) {
      toggleBtn.textContent = shouldShow ? 'DBpia 검색 요점 · 수집 논문 닫기 ▴' : 'DBpia 검색 요점 · 수집 논문 보기 (' + papers.length + '편) ▾';
    }
  } else if (toggleBtn) {
    toggleBtn.textContent = board.style.display !== 'none'
      ? 'DBpia 검색 요점 · 수집 논문 닫기 ▴'
      : 'DBpia 검색 요점 · 수집 논문 보기 (' + papers.length + '편) ▾';
  }

  const sig = JSON.stringify({
    phase: p.phase,
    step: p.stepDetail,
    fLen: feed.length,
    pLen: papers.length,
    lastSumLen: feed.length ? (feed[feed.length - 1].summary || '').length : 0
  });
  if (sig === lastDbpiaSignature) return;
  lastDbpiaSignature = sig;

  if (titleEl) {
    titleEl.textContent = isDbpiaPhase
      ? '🔍 [DBpia AI 실시간 다각도 검색 진행 중] 학술 요점(vetter_conversations) 및 실존 논문 서지·인용문(hovers) 수집 현황 (' + papers.length + '편 확보)'
      : '📚 DBpia AI 다각도 검색 요점(' + feed.length + '개 관점) 및 검증 완료 실존 학술논문 (' + papers.length + '편)';
  }

  let html = '';
  if (isDbpiaPhase) {
    html += '<div style="padding:8px 10px;border-radius:5px;background:var(--brass-soft);border:1px solid var(--brass-line);font-size:11.5px;color:var(--ink);">' +
      '<strong>현재 DBpia AI 검색 상태:</strong> ' + escapeHtml(p.stepDetail || '다각도 학술 검색 중...') +
    '</div>';
  }

  if (feed.length > 0) {
    feed.forEach((item, idx) => {
      const sumText = String(item.summary || '').trim();
      html += '<div class="as-dbpia-feed-card">' +
        '<div class="as-dbpia-feed-card__q">' +
          '<span>[관점 ' + (idx + 1) + ' 검색어] "' + escapeHtml(item.query || '') + '"</span>' +
          '<span class="as-tag as-tag--brass">요점 ' + sumText.length.toLocaleString() + '자 · 논문 ' + ((item.refs && item.refs.length) || 0) + '편</span>' +
        '</div>' +
        '<div class="as-dbpia-feed-card__sum">' + escapeHtml(sumText || '(DBpia AI 요점 응답 수신 완료)') + '</div>' +
      '</div>';
    });
  }

  if (papers.length > 0) {
    html += '<div style="font-size:11.5px;font-weight:700;color:var(--ink);margin-top:4px;">✓ 서지 메타데이터 및 본문 인용구(hovers) 교차검증 완료 논문 (' + papers.length + '편)</div>';
    html += '<div class="as-dbpia-papers-grid">';
    papers.forEach((r, idx) => {
      const cite = (r.author || '학술연구자') + ', ' + (r.year || '') + ', 「' + (r.title || '') + '」, 『' + (r.journal || '') + '』' + (r.pages ? ', ' + r.pages : '');
      const quote = r.abstract ? String(r.abstract).trim() : '';
      html += '<div class="as-dbpia-paper-item">' +
        '<div class="as-dbpia-paper-item__cite">' + (idx + 1) + ') ' + escapeHtml(cite) + '</div>' +
        (quote ? '<div class="as-dbpia-paper-item__quote">“' + escapeHtml(quote) + '”</div>' : '') +
      '</div>';
    });
    html += '</div>';
  }

  bodyEl.innerHTML = html;
}

/**
 * 마크다운 원고를 ARCHAEO STUDIO 다중 A4 페이지 양식(- 1 -, - 2 -, - 3 -... 자동 쪽 분할 및 쪽별 하단 각주 배치)으로 실시간 조판
 */
function renderManuscriptOntoA4Page(rawMd, isRunning) {
  const pagesWrap = document.getElementById('doc-pages-wrap');
  if (!pagesWrap || !rawMd || !rawMd.trim()) {
    return { footnotesCount: 0, refsCount: 0, pagesCount: 1 };
  }

  let md = String(rawMd);
  // 한 줄로 병합된 마크다운 표 자동 복원
  (function() {
    const rawLines = md.split(/\\r?\\n/);
    const fixedLines = [];
    let k = 0;
    while (k < rawLines.length) {
      const trm = rawLines[k].trim();
      if (trm.startsWith('|') && trm.endsWith('|')) {
        const nxt = (rawLines[k + 1] || '').trim();
        const isSep = nxt.startsWith('|') && nxt.endsWith('|') && /^[\\s|:\\-]+$/.test(nxt.slice(1, -1));
        if (isSep) {
          const colCnt = nxt.slice(1, -1).split('|').map(c => c.trim()).filter(Boolean).length;
          const fCells = trm.slice(1, -1).split('|').map(c => c.trim());
          if (colCnt >= 2 && fCells.length >= colCnt * 2 && fCells.length % colCnt === 0) {
            const totR = fCells.length / colCnt;
            fixedLines.push('| ' + fCells.slice(0, colCnt).join(' | ') + ' |');
            fixedLines.push('| ' + Array(colCnt).fill('---').join(' | ') + ' |');
            for (let r = 1; r < totR; r++) {
              fixedLines.push('| ' + fCells.slice(r * colCnt, (r + 1) * colCnt).join(' | ') + ' |');
            }
            k += 2;
            while (k < rawLines.length && rawLines[k].trim() === trm) k++;
            continue;
          }
        }
      }
      fixedLines.push(rawLines[k]);
      k++;
    }
    md = fixedLines.join('\\n');
  })();

  // 1) 차트 및 모식도 코드블록을 A4 도면 박스로 변환 (Nano Banana Pro 고고학 에디토리얼 SVG 비교차트 + 데이터표 + 시각적 모식도 렌더링)
  const chartBlocks = [];
  const diagBlocks = [];
  const palette = ['#8C4A27', '#2C5E4F', '#B45309', '#3B4D6B', '#6B5B4D'];
  md = md.replace(/\`\`\`([^\\n]*)\\r?\\n([\\s\\S]*?)\`\`\`/gi, (full, langRaw, j) => {
    const lang = String(langRaw || '').trim().toLowerCase();
    const body = String(j || '').trim();
    if (!body.startsWith('{') || !body.endsWith('}')) return full;
    try {
      const spec = JSON.parse(body);
      if (lang.includes('wc-chart') || (spec.data && Array.isArray(spec.data.series)) || Array.isArray(spec.series)) {
        const idx = chartBlocks.length;
        const cats = (spec.data && spec.data.categories) || spec.categories || [];
        const series = (spec.data && spec.data.series) || spec.series || [];
        let maxVal = 1;
        series.forEach(s => (s.values || []).forEach(v => { if (Number(v) > maxVal) maxVal = Number(v); }));
        const svgW = 620, svgH = 225, padL = 48, padR = 22, padT = 32, padB = 48;
        const plotW = svgW - padL - padR, plotH = svgH - padT - padB;
        let svgParts = [
          '<svg viewBox="0 0 ' + svgW + ' ' + svgH + '" style="width:100%;height:auto;display:block;margin:6px 0 8px 0;background:#FAF7F2;border:1.5px solid #2A2520;border-radius:4px;">',
          '<rect x="3" y="3" width="' + (svgW - 6) + '" height="' + (svgH - 6) + '" fill="none" stroke="#8C4A27" stroke-width="0.6" stroke-dasharray="3,2"/>',
          '<text x="' + (svgW - 12) + '" y="16" text-anchor="end" font-size="7.5" font-weight="700" fill="#8C4A27">NANO BANANA PRO · ARCHAEOLOGY</text>'
        ];
        for (let g = 0; g <= 4; g++) {
          const gy = padT + plotH - (plotH * g / 4);
          const gVal = Math.round((maxVal * g) / 4 * 10) / 10;
          svgParts.push('<line x1="' + padL + '" y1="' + gy + '" x2="' + (svgW - padR) + '" y2="' + gy + '" stroke="' + (g === 0 ? '#2A2520' : '#D8CFC2') + '" stroke-width="' + (g === 0 ? '1.3' : '0.8') + '"' + (g === 0 ? '' : ' stroke-dasharray="3,3"') + '/>');
          svgParts.push('<text x="' + (padL - 6) + '" y="' + (gy + 3) + '" text-anchor="end" font-size="9.5" fill="#5C5349">' + gVal + '</text>');
        }
        const catCount = Math.max(1, cats.length);
        const sCount = Math.max(1, series.length);
        const groupW = plotW / catCount;
        const barW = Math.min(28, Math.max(8, (groupW * 0.68) / sCount));
        cats.forEach((c, cIdx) => {
          const gx = padL + cIdx * groupW + (groupW - barW * sCount) / 2;
          series.forEach((s, sIdx) => {
            const val = Number((s.values && s.values[cIdx]) || 0);
            const bh = Math.max(2, (val / maxVal) * plotH);
            const bx = gx + sIdx * barW;
            const by = padT + plotH - bh;
            const col = palette[sIdx % palette.length];
            svgParts.push('<rect x="' + bx.toFixed(1) + '" y="' + by.toFixed(1) + '" width="' + Math.max(4, barW - 2).toFixed(1) + '" height="' + bh.toFixed(1) + '" fill="' + col + '" stroke="#1C1917" stroke-width="0.6" rx="2"/>');
            if (val > 0) {
              svgParts.push('<text x="' + (bx + (barW - 2) / 2).toFixed(1) + '" y="' + (by - 4).toFixed(1) + '" text-anchor="middle" font-size="8.5" font-weight="700" fill="#1C1917">' + val + '</text>');
            }
          });
          const labelShort = String(c).length > 14 ? String(c).slice(0, 14) + '…' : String(c);
          svgParts.push('<text x="' + (padL + cIdx * groupW + groupW / 2).toFixed(1) + '" y="' + (padT + plotH + 15) + '" text-anchor="middle" font-size="9.5" font-weight="600" fill="#1C1917">' + escapeHtml(labelShort) + '</text>');
        });
        series.forEach((s, sIdx) => {
          const lx = padL + sIdx * 145;
          const ly = svgH - 10;
          const col = palette[sIdx % palette.length];
          svgParts.push('<rect x="' + lx + '" y="' + (ly - 8) + '" width="9" height="9" fill="' + col + '" stroke="#1C1917" stroke-width="0.5" rx="2"/>');
          svgParts.push('<text x="' + (lx + 13) + '" y="' + ly + '" font-size="9.5" font-weight="600" fill="#2A2520">' + escapeHtml(s.name || ('계열 ' + (sIdx + 1))) + '</text>');
        });
        svgParts.push('</svg>');
        let rowsHtml = '';
        cats.forEach((c, cIdx) => {
          const vals = series.map(s => (s.values && s.values[cIdx] != null ? s.values[cIdx] : '-'));
          rowsHtml += '<tr><td>' + escapeHtml(c) + '</td>' + vals.map(v => '<td>' + escapeHtml(String(v)) + '</td>').join('') + '</tr>';
        });
        chartBlocks.push(
          '<figure class="as-doc-table"><figcaption>&lt;그림 ' + (idx + 1) + '&gt; ' + escapeHtml(spec.title || '유구·유물 정량 분포 비교 차트') + '</figcaption>' +
          svgParts.join('') +
          '<table><thead><tr><th>구분</th>' + series.map(s => '<th>' + escapeHtml(s.name || '') + '</th>').join('') + '</tr></thead>' +
          '<tbody>' + rowsHtml + '</tbody></table></figure>'
        );
        return '\\n@@AS_CHART_' + idx + '@@\\n';
      }
      const rawNodes = spec.items || spec.nodes || spec.steps || spec.stages;
      if (lang.includes('wc-diagram') || Array.isArray(rawNodes)) {
        const edges = Array.isArray(spec.edges) ? spec.edges : [];
        const childIds = new Set(edges.map(e => e && (e.to || e.target)).filter(Boolean));
        const parentIds = new Set(edges.map(e => e && (e.from || e.source)).filter(Boolean));
        const items = Array.isArray(rawNodes) ? rawNodes.map((n, nIdx) => {
          if (typeof n === 'string') return { text: n, level: nIdx === 0 ? 0 : 1 };
          const id = String(n.id || '');
          const lbl = String(n.text || n.label || n.name || n.title || '').trim();
          const desc = String(n.description || n.detail || '').trim();
          const txt = desc && !lbl.includes(desc) ? (lbl + ' — ' + desc) : lbl;
          let lv = typeof n.level === 'number' ? n.level : 0;
          if (typeof n.level !== 'number' && edges.length > 0 && id) {
            if (childIds.has(id) && !parentIds.has(id)) lv = 1;
            else if (/^P\\d|^S\\d|^sub/i.test(id)) lv = 1;
          }
          return { text: txt, level: lv };
        }) : [];
        const groups = [];
        items.forEach(it => {
          const itemObj = typeof it === 'string' ? { text: it, level: 0 } : it;
          if ((itemObj.level || 0) === 0) groups.push({ title: itemObj.text || '', subs: [] });
          else if (groups.length > 0) groups[groups.length - 1].subs.push(itemObj.text || '');
          else groups.push({ title: itemObj.text || '', subs: [] });
        });
        const idx = diagBlocks.length;
        const diagTitle = spec.title || '단계별 유구·출토유물 및 학술 논증 구조 모식도';
        const stepsHtml = groups.map((g, gIdx) => {
          const accent = palette[gIdx % palette.length];
          const arrowHtml = gIdx < groups.length - 1
            ? '<div style="text-align:center;color:#8C4A27;font-size:13px;font-weight:800;line-height:1;margin:2px 0;">▼</div>'
            : '';
          const subsList = g.subs.length > 0
            ? '<div style="margin-top:3px;padding-left:8px;border-left:2px solid ' + accent + ';color:#332C26;font-size:9.2pt;">• ' + g.subs.map(s => escapeHtml(s)).join('<br>• ') + '</div>'
            : '';
          return '<div class="as-doc-diagram__step" style="background:#FAF7F2;border:1px solid #C8BFA8;border-left:4px solid ' + accent + ';padding:7px 11px;border-radius:4px;">' +
            '<strong style="color:#1C1917;font-size:9.8pt;">[' + (gIdx + 1) + '단계] ' + escapeHtml(g.title) + '</strong>' + subsList + '</div>' + arrowHtml;
        }).join('');
        diagBlocks.push(
          '<div class="as-doc-diagram"><div class="as-doc-diagram__title">&lt;모식도 ' + (idx + 1) + '&gt; ' + escapeHtml(diagTitle) + '</div>' +
          '<div class="as-doc-diagram__steps">' + stepsHtml + '</div></div>'
        );
        return '\\n@@AS_DIAG_' + idx + '@@\\n';
      }
    } catch {}
    return full;
  });

  // 2) 【각주】 추출하여 페이지 하단 각주(footnotes)로 분리
  const fnRegex = /(?:^|\\n)\\s*(?:#{1,6}\\s*)?(?:\\*{1,2}\\s*)?(?:【\\s*각\\s*주\\s*】|\\[\\s*각\\s*주\\s*\\]|각\\s*주)(?:\\s*\\*{1,2})?\\s*\\n([\\s\\S]*?)(?=(?:\\n\\s*(?:#{1,6}\\s*)?(?:\\*{1,2}\\s*)?(?:【\\s*참고\\s*문헌\\s*】|\\[\\s*참고\\s*문헌\\s*\\]|참고\\s*문헌|참고\\s*논문))|$)/i;
  const fnMatch = fnRegex.exec(md);
  const footnotes = [];
  const fnByNum = new Map();
  if (fnMatch && fnMatch[1]) {
    let autoNum = 0;
    fnMatch[1].split(/\\r?\\n/).forEach(rawLine => {
      const l = rawLine.trim().replace(/^[-*•]\\s*/, '').replace(/^\\*{1,2}|\\*{1,2}$/g, '');
      const m = /^(?:\\[\\^?(\\d+)\\]:?|\\(?(\\d+)\\)|(\\d+)\\.)\\s*(.+)$/.exec(l);
      if (m) {
        const n = Number(m[1] || m[2] || m[3]);
        const txt = (m[4] || '').trim().replace(/^\\d+\\)\\s*/, '');
        if (n > 0 && txt) {
          autoNum = Math.max(autoNum, n);
          footnotes.push({ n, text: txt });
          fnByNum.set(n, txt);
        }
      } else if (l.length > 10 && (l.includes('「') || l.includes('『'))) {
        autoNum++;
        footnotes.push({ n: autoNum, text: l });
        fnByNum.set(autoNum, l);
      }
    });
    md = md.slice(0, fnMatch.index) + '\\n' + md.slice(fnMatch.index + fnMatch[0].length);
  }

  // 3) 줄 단위 파싱 (블록별 가중치 계산하여 A4 다중 페이지 분할)
  const lines = md.split(/\\r?\\n/);
  const blockItems = []; // { html, weight, fnNums }
  let inTable = false;
  let tableRows = [];
  let inRefs = false;
  let refItems = [];

  function extractInlineFnNums(str) {
    const nums = [];
    const rx = /<sup class="as-fnref">(\\d+)\\)<\\/sup>/g;
    let m;
    while ((m = rx.exec(str)) !== null) {
      nums.push(Number(m[1]));
    }
    return nums;
  }

  function formatInline(s) {
    let out = escapeHtml(s);
    out = out.replace(/\\*\\*(.+?)\\*\\*/g, '<strong>$1</strong>');
    out = out.replace(/\\[\\^(\\d+)\\]/g, '<sup class="as-fnref">$1)</sup>');
    out = out.replace(/([.다됨함음임”"'\\])])\\s*(?<![0-9(])(\\d+)\\)/g, '$1<sup class="as-fnref">$2)</sup>');
    return out;
  }

  function pushBlock(html, weight) {
    blockItems.push({
      html,
      weight: Math.max(60, weight),
      fnNums: extractInlineFnNums(html)
    });
  }

  function flushTable() {
    if (!inTable || tableRows.length === 0) return;
    inTable = false;
    const header = tableRows[0];
    const bodyRows = tableRows.slice(1).filter(r => !r.every(c => /^[:\\-\\s]+$/.test(c)));
    const tblHtml =
      '<figure class="as-doc-table"><table><thead><tr>' +
      header.map(c => '<th>' + formatInline(c) + '</th>').join('') +
      '</tr></thead><tbody>' +
      bodyRows.map(r => '<tr>' + r.map(c => '<td>' + formatInline(c) + '</td>').join('') + '</tr>').join('') +
      '</tbody></table></figure>';
    pushBlock(tblHtml, 220 + bodyRows.length * 75);
    tableRows = [];
  }

  for (let i = 0; i < lines.length; i++) {
    const raw = lines[i].trim();
    if (!raw) {
      flushTable();
      continue;
    }
    if (raw.startsWith('@@AS_CHART_')) {
      flushTable();
      const idx = Number(raw.replace(/@@AS_CHART_|@@/g, ''));
      if (chartBlocks[idx]) pushBlock(chartBlocks[idx], 360);
      continue;
    }
    if (raw.startsWith('@@AS_DIAG_')) {
      flushTable();
      const idx = Number(raw.replace(/@@AS_DIAG_|@@/g, ''));
      if (diagBlocks[idx]) pushBlock(diagBlocks[idx], 340);
      continue;
    }
    if (raw.startsWith('|') && raw.endsWith('|')) {
      inTable = true;
      const cells = raw.slice(1, -1).split('|').map(c => c.trim());
      tableRows.push(cells);
      continue;
    } else {
      flushTable();
    }

    if (/^(?:#{1,6}\\s*)?(?:\\*{1,2}\\s*)?(?:【\\s*참고\\s*문헌\\s*】|\\[\\s*참고\\s*문헌\\s*\\]|참고\\s*문헌|참고\\s*논문)/.test(raw)) {
      inRefs = true;
      continue;
    }

    if (inRefs) {
      const cleaned = raw.replace(/^[-*•]\\s*/, '').replace(/^#{1,6}\\s*/, '');
      refItems.push(formatInline(cleaned));
      continue;
    }

    if (/^#\\s+/.test(raw)) {
      pushBlock('<h1 class="as-doc-chapter">' + formatInline(raw.replace(/^#\\s+/, '')) + '</h1>', 180);
    } else if (/^##\\s+/.test(raw)) {
      pushBlock('<h2 class="as-doc-section">' + formatInline(raw.replace(/^##\\s+/, '')) + '</h2>', 140);
    } else if (/^#{3,6}\\s+/.test(raw)) {
      pushBlock('<h3 class="as-doc-subsec">' + formatInline(raw.replace(/^#{3,6}\\s+/, '')) + '</h3>', 110);
    } else {
      pushBlock('<p class="as-doc-para">' + formatInline(raw) + '</p>', raw.length + 45);
    }
  }
  flushTable();

  if (isRunning && blockItems.length > 0) {
    const lastIdx = blockItems.length - 1;
    blockItems[lastIdx].html = blockItems[lastIdx].html.replace(/<\\/p>$/, '<span class="as-caret" aria-hidden="true"></span></p>');
  }

  // 4) A4 페이지 단위 분할 (페이지당 약 1,650 가중치 예산, 각주 수록 시 각주당 95 가중치 반영)
  const MAX_PAGE_BUDGET = 1650;
  const pages = []; // [{ blocks: [], fnNums: Set }]
  let curPage = { blocks: [], fnNums: new Set(), weight: 0 };
  const assignedFnNums = new Set();

  for (let bIdx = 0; bIdx < blockItems.length; bIdx++) {
    const blk = blockItems[bIdx];
    const newFns = blk.fnNums.filter(n => fnByNum.has(n) && !assignedFnNums.has(n));
    const fnWeight = newFns.length * 95;
    if (curPage.blocks.length > 0 && (curPage.weight + blk.weight + fnWeight > MAX_PAGE_BUDGET)) {
      pages.push(curPage);
      curPage = { blocks: [], fnNums: new Set(), weight: 0 };
    }
    curPage.blocks.push(blk.html);
    curPage.weight += blk.weight + fnWeight;
    newFns.forEach(n => {
      curPage.fnNums.add(n);
      assignedFnNums.add(n);
    });
  }
  if (curPage.blocks.length > 0 || pages.length === 0) {
    pages.push(curPage);
  }

  // 본문 인라인에 매칭되지 않은 나머지 각주는 마지막 본문 페이지 하단에 배치
  footnotes.forEach(f => {
    if (!assignedFnNums.has(f.n)) {
      pages[pages.length - 1].fnNums.add(f.n);
      assignedFnNums.add(f.n);
    }
  });

  // 【참고문헌】 섹션 배치 (현재 마지막 페이지 여유가 있으면 추가, 없으면 별도 A4 페이지로 분리)
  if (refItems.length > 0) {
    const refHtml =
      '<section class="as-doc-refs"><h2 class="as-doc-section">【참고문헌】</h2><ul>' +
      refItems.map(r => '<li>' + r + '</li>').join('') +
      '</ul></section>';
    const refWeight = 180 + refItems.length * 75;
    const lastP = pages[pages.length - 1];
    if (lastP.weight + refWeight > MAX_PAGE_BUDGET + 250 && lastP.blocks.length > 2) {
      pages.push({ blocks: [refHtml], fnNums: new Set(), weight: refWeight });
    } else {
      lastP.blocks.push(refHtml);
      lastP.weight += refWeight;
    }
  }

  // 5) 다중 A4 페이지 DOM 렌더링 (- 1 -, - 2 -, - 3 -...)
  const currentZoom = String(ZOOMS[currentZoomIdx] || 0.9);
  const runningHeaderTitle = currentDocType === 'paper'
    ? '학술논문 · ARCHAEO STUDIO WordCraft A4 조판'
    : '문화재발굴조사보고서 『Ⅳ. 고찰(考察)』 · ARCHAEO STUDIO WordCraft A4 조판';

  pagesWrap.innerHTML = pages.map((pg, idx) => {
    const pageNum = idx + 1;
    const pageFns = Array.from(pg.fnNums)
      .sort((a, b) => a - b)
      .map(n => ({ n, text: fnByNum.get(n) }))
      .filter(f => f.text);

    const notesHtml = pageFns.length > 0
      ? '<div class="as-page__notes"><hr><ol>' +
          pageFns.map(f => '<li><span class="as-page__fnn">' + f.n + ')</span><span>' + escapeHtml(f.text) + '</span></li>').join('') +
        '</ol></div>'
      : '';

    const runningHead = pageNum > 1
      ? '<div class="as-page__head-running"><span>' + escapeHtml(runningHeaderTitle) + '</span><span>' + pageNum + '쪽</span></div>'
      : '';

    return '<article class="as-page"' + (idx === 0 ? ' id="doc-page"' : '') + ' contenteditable="true" spellcheck="false" style="zoom:' + currentZoom + ';">' +
      runningHead +
      '<div class="as-page__body"' + (idx === 0 ? ' id="doc-page-body"' : '') + '>' +
        pg.blocks.join('\\n') +
      '</div>' +
      notesHtml +
      '<div class="as-page__num">- ' + pageNum + ' -</div>' +
    '</article>';
  }).join('\\n');

  lastRenderedPagesCount = pages.length;
  return { footnotesCount: footnotes.length, refsCount: refItems.length, pagesCount: pages.length };
}

function classifyLogEntry(rawLine) {
  const s = String(rawLine || '');
  const timeMatch = /^\\[(\\d{2}:\\d{2}:\\d{2})\\]\\s*(.*)$/.exec(s);
  const time = timeMatch ? timeMatch[1] : new Date().toLocaleTimeString('ko-KR', { hour12: false });
  const text = (timeMatch ? timeMatch[2] : s).replace(/^\\[[A-Z_]+\\]\\s*/, '');
  let kind = 'write';
  if (/DBpia|수집|논문|검색/i.test(text)) kind = 'collect';
  else if (/교차검증|\\\/goal|판단\\s*기준/i.test(text)) kind = 'warn';
  else if (/완료|끝|완성|통과/i.test(text)) kind = 'done';
  else if (/오류|실패|중단/i.test(text)) kind = 'error';
  else if (/참고|비어|제외/i.test(text)) kind = 'note';
  return { time, kind, text };
}

const LOG_LABELS = { collect: '수집', write: '작성', note: '알림', warn: '검증', error: '오류', done: '완료' };

const SHORT_CRITERIA_NAMES = [
  '1. DBpia 다각도 논문',
  '2. 필수 장·절 완결성',
  '3. 학술 논증·분량',
  '4. 정량 학술 표 2종',
  '5. 차트·모식도 수록',
  '6. 실전 각주·실저자',
  '7. 참고문헌 1:1 일치',
  '8. 원자료 정합·완결'
];

async function pollStatus() {
  try {
    const res = await fetch('/studio/status');
    const data = await res.json();
    const p = data.pipeline || data || {};
    const running = Boolean(p.running);
    const paused = Boolean(p.paused);
    const done = !running && p.phase === 'COMPLETED';
    const pct = Math.min(100, Math.max(0, Number(p.progressPct || 0)));
    const papersCount = Array.isArray(p.collectedPapers) ? p.collectedPapers.length : 0;

    // 0) /goal 8대 판단 기준 및 교차검증 스코어카드 업데이트
    const gv = p.goalVerification || null;
    const gp = p.goalPlan || null;
    const goalTitleEl = document.getElementById('goal-standard-title');
    const goalSummaryEl = document.getElementById('goal-standard-summary');
    const goalBadgeEl = document.getElementById('goal-score-badge');
    const goalGridEl = document.getElementById('goal-criteria-grid');

    if (goalTitleEl) {
      goalTitleEl.textContent = (currentDocType === 'paper' || (gp && gp.docType === 'paper'))
        ? '석사학위 논문 이상·『한국고고학보』 8대 판단 기준'
        : '문화재발굴조사보고서 『Ⅳ. 고찰』 8대 판단 기준';
    }

    const criteriaList = (gv && Array.isArray(gv.criteriaResults))
      ? gv.criteriaResults
      : (gp && Array.isArray(gp.criteria) ? gp.criteria : null);

    if (gv && goalSummaryEl && goalBadgeEl) {
      goalSummaryEl.textContent = gv.passedCount + '/' + gv.totalCriteria + ' 충족 (' + gv.score + '점)';
      goalBadgeEl.innerHTML = gv.passed
        ? '<span class="as-tag as-tag--ok">교차검증 ' + gv.passedCount + '/8 통과</span>'
        : '<span class="as-tag as-tag--warn">교차검증 ' + gv.passedCount + '/8 (' + gv.score + '점)</span>';
    } else if (running && goalSummaryEl && goalBadgeEl) {
      goalSummaryEl.textContent = '실시간 교차검증 진행 중';
      goalBadgeEl.innerHTML = '<span class="as-tag as-tag--brass">검증 루프 활성</span>';
    }

    if (goalGridEl && criteriaList && criteriaList.length > 0) {
      goalGridEl.innerHTML = criteriaList.map((c, idx) => {
        const shortName = SHORT_CRITERIA_NAMES[idx] || c.title;
        const isPass = Boolean(c.passed);
        const badgeTxt = isPass ? '✓ 충족' : (running ? '검증중' : (gv ? '보완' : '대기'));
        return '<li class="as-goal-item ' + (isPass ? 'is-pass' : '') + '" title="' + escapeHtml((c.title || '') + ' — ' + (c.actual || c.target || '')) + '">' +
          '<span class="as-goal-item__name">' + escapeHtml(shortName) + '</span>' +
          '<span class="as-goal-item__badge">' + escapeHtml(badgeTxt) + '</span>' +
        '</li>';
      }).join('');
    }

    // 1) 문서 종류 잠금 (작성 중에는 잠김)
    document.getElementById('seg-report').disabled = running;
    document.getElementById('seg-paper').disabled = running;

    // 2) 실행 단추 및 잠시 멈춤·그만두기 상태
    const btnReport = document.getElementById('btn-run-report');
    const btnPaper = document.getElementById('btn-run-paper');
    const lblReport = document.getElementById('btn-run-report-label');
    const lblPaper = document.getElementById('btn-run-paper-label');
    const ctlRow = document.getElementById('run-ctl-row');

    btnReport.disabled = running;
    btnPaper.disabled = running;
    if (running) {
      ctlRow.style.display = 'flex';
      if (currentDocType === 'report') {
        lblReport.textContent = paused ? '잠시 멈춤' : '교차검증·작성 중…';
      } else {
        lblPaper.textContent = paused ? '잠시 멈춤' : '교차검증·작성 중…';
      }
      document.getElementById('btn-pause-label').textContent = paused ? '이어서' : '잠시 멈춤';
    } else {
      ctlRow.style.display = 'none';
      lblReport.textContent = '발굴조사보고서 고찰 실행';
      lblPaper.textContent = '논문 실행';
    }

    // 작성 끝 태그 및 PDF 내보내기 단추 강조
    const doneTagEl = document.getElementById('run-done-tag');
    const exportBtn = document.getElementById('btn-export-pdf');
    if (done) {
      const nowHm = new Date().toLocaleTimeString('ko-KR', { hour12: false, hour: '2-digit', minute: '2-digit' });
      doneTagEl.innerHTML = '<span class="as-tag as-tag--ok">교차검증 완료 · ' + nowHm + '</span>';
      exportBtn.className = 'as-btn as-btn--default';
    } else if (running) {
      doneTagEl.innerHTML = '';
      exportBtn.className = 'as-btn as-btn--quiet';
    }

    // 3) 진행 기록 (LogPanel)
    const logHead = document.getElementById('log-progress-head');
    const logFill = document.getElementById('log-progress-fill');
    const logPct = document.getElementById('log-progress-pct');
    if (running || done) {
      logHead.style.display = 'flex';
      logPct.textContent = pct + '%';
      logFill.style.width = pct + '%';
      logFill.classList.toggle('is-done', done || pct >= 100);
    }

    const logs = Array.isArray(p.logs) ? p.logs : [];
    const logList = document.getElementById('log-list');
    if (logs.length > 0) {
      const recent = logs.slice(-12).map(classifyLogEntry);
      logList.innerHTML = recent.map(e =>
        '<li class="as-log__row is-' + e.kind + '">' +
          '<span class="as-log__time">' + escapeHtml(e.time) + '</span>' +
          '<span class="as-log__kind"><i></i>' + (LOG_LABELS[e.kind] || '알림') + '</span>' +
          '<span class="as-log__text">' + escapeHtml(e.text) + '</span>' +
        '</li>'
      ).join('');
      logList.scrollTop = logList.scrollHeight;
    }

    // 4) DBpia 실시간 다각도 검색 요점 & 수집 논문 현황 보드 업데이트
    renderDbpiaLiveBoard(p);

    // 5) A4 다중 페이지 문서 실시간 조판
    const liveMd = (p.latestManuscript && p.latestManuscript.trim()) || (p.liveStreamText && p.liveStreamText.trim()) || '';
    let fnCount = (p.wordcraftInfo && p.wordcraftInfo.footnotesCount) || 0;
    let refCount = papersCount;
    if (liveMd && liveMd !== lastRenderedText) {
      lastRenderedText = liveMd;
      const counts = renderManuscriptOntoA4Page(liveMd, running);
      if (counts.footnotesCount) fnCount = counts.footnotesCount;
      if (counts.refsCount) refCount = counts.refsCount;
      if (counts.pagesCount) lastRenderedPagesCount = counts.pagesCount;
    }

    // 6) 수집 띠 (CollectStrip)
    const collectLeft = document.getElementById('collect-left');
    const collectRightTime = document.getElementById('collect-right-time');
    const scoreBadgeText = gv ? (' · 교차검증 ' + gv.passedCount + '/8 (' + gv.score + '점)') : '';
    if (!running && !done && !liveMd && papersCount === 0) {
      collectLeft.innerHTML = '<i class="as-collect__dot is-hollow"></i><span>실행 전 (/goal 8대 판단 기준 대기)</span>';
    } else if (done) {
      collectLeft.innerHTML =
        '<span class="as-collect__item"><i class="as-collect__dot is-ok"></i>DBpia 다각도 수집</span>' +
        '<span class="as-collect__num" style="margin-left:6px;">' + papersCount + '편</span>' +
        '<span class="as-collect__item" style="margin-left:10px;">교차검증 완료</span>' +
        '<span class="as-collect__num" style="margin-left:6px;">' + lastRenderedPagesCount + '쪽 · 각주 ' + fnCount + ' · 참고문헌 ' + refCount + scoreBadgeText + '</span>';
      collectRightTime.textContent = 'WordCraft .docx 자동 저장됨';
    } else {
      collectLeft.innerHTML =
        '<span class="as-collect__item"><i class="as-collect__dot"></i>DBpia 다각도 수집</span>' +
        '<span class="as-collect__num" style="margin-left:6px;">' + papersCount + '편</span>' +
        '<span class="as-collect__item" style="margin-left:10px;"><span class="as-collect__mini"><span style="width:' + pct + '%"></span></span>' +
        (paused ? '잠시 멈춤 ' : '교차검증·작성 ') + pct + '%' + scoreBadgeText + '</span>';
      collectRightTime.textContent = '실시간 교차검증 중';
    }

    // 7) 상태줄 (StatusBar)
    const hintEl = document.getElementById('status-hint');
    if (!document.getElementById('scrim-dbpia').classList.contains('is-open') &&
        !document.getElementById('scrim-export').classList.contains('is-open')) {
      if (running) {
        hintEl.textContent = '[/goal 루프] DBpia 다각도 서지·요점 검증 및 제미나이 8대 기준 교차검증이 진행 중입니다.';
      } else if (done) {
        hintEl.textContent = '8대 기준 교차검증 및 다중 A4 페이지(' + lastRenderedPagesCount + '쪽)·WordCraft(.docx) 생성이 완료되었습니다.';
      } else {
        hintEl.textContent = '요청과 자료를 넣고 실행하면 8대 기준 계획 수립 ➔ DBpia 다각도 수집 ➔ 교차검증 루프가 실행됩니다.';
      }
    }

    const totalPages = lastRenderedPagesCount || (p.wordcraftInfo && p.wordcraftInfo.pages) || (liveMd ? Math.max(1, Math.ceil(liveMd.length / 1650)) : 1);
    const totalChars = liveMd ? liveMd.length : Number(p.totalCharsWritten || 0);
    document.getElementById('status-pages').textContent = '쪽 1 / ' + totalPages + ' (총 ' + totalPages + '쪽)';
    document.getElementById('status-chars').textContent = '글자 ' + totalChars.toLocaleString();
  } catch (e) {}
}

updateDatingEmptyTag();
refreshEngineeredPrompt(true);
pollStatus();
setInterval(pollStatus, 1200);
</script>
</body>
</html>`;
}

module.exports = {
  renderStandaloneStudioHtml: getStandaloneStudioHtml,
  getStandaloneStudioHtml,
};
