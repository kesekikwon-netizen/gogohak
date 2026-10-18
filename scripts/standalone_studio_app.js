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
<title>기안동 유적 Ⅳ 고찰.docx - ARCHAEO STUDIO</title>
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

/* A4 문서 */
.as-page { box-sizing: border-box; width: var(--size-a4-w); min-height: var(--size-a4-h); padding: 25mm 20mm 18mm; background: var(--page); color: var(--page-ink); box-shadow: var(--shadow-page); font-family: var(--font-serif); display: flex; flex-direction: column; margin: 0 auto; outline: none; }
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
.as-page__num { text-align: center; font-size: 10.67px; margin-top: 1.2em; color: var(--page-ink-muted); }
.as-caret { display: inline-block; width: 1.5px; height: 1.05em; background: var(--clay); vertical-align: -0.15em; margin-left: 1px; animation: as-blink 1s steps(1) infinite; }
@keyframes as-blink { 50% { opacity: 0; } }

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
    <span id="win-doc-title" style="font-weight:600;">기안동 유적 Ⅳ 고찰.docx - ARCHAEO STUDIO</span>
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
              <textarea id="input-focus" class="as-field as-field--multi" rows="3" placeholder="무엇을 쓸지 적어 주세요. 예: 기안동 유적 주거지의 구조와 편년을 정리해 줘" aria-label="요청" oninput="refreshEngineeredPrompt(true)"></textarea>
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

        <section class="as-step" aria-label="진행 기록">
          <div class="as-step__head">
            <span class="as-step__n">6</span>
            <span class="as-step__title">진행 기록</span>
          </div>
          <div class="as-step__body">
            <div class="as-log">
              <div class="as-log__head" id="log-progress-head" style="display:none;">
                <div class="as-progress">
                  <div class="as-progress__row">
                    <span id="log-progress-label">본문 작성</span>
                    <span class="as-progress__pct" id="log-progress-pct">0%</span>
                  </div>
                  <div class="as-progress__track" role="progressbar">
                    <div class="as-progress__fill" id="log-progress-fill" style="width:0%"></div>
                  </div>
                </div>
              </div>
              <ol class="as-log__list" id="log-list" aria-live="polite">
                <p class="as-log__empty" id="log-empty">실행하면 진행 기록이 여기에 쌓입니다.</p>
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
          <span id="collect-right-time">WordCraft .docx 대기</span>
          <button type="button" class="as-btn" style="height:22px;padding:0 10px;font-size:11px;" onclick="openInWordCraft()">WordCraft 창 열기</button>
        </span>
      </div>

      <div class="as-screen__scroll" id="doc-scroll">
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
        <span class="as-ph__sub" id="export-source-label">기안동 유적 Ⅳ 고찰.docx</span>
      </div>
      <div class="as-dialog__body">
        <div class="as-frow">
          <span class="as-frow__label">파일 이름</span>
          <div class="as-frow__ctl">
            <input type="text" class="as-field" id="export-filename-input" value="기안동 유적 Ⅳ 고찰.pdf">
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
  document.getElementById('doc-page').style.zoom = String(z);
  document.getElementById('status-zoom').textContent = Math.round(z * 100) + '% ▾';
}

function togglePageFont() {
  const p = document.getElementById('doc-page');
  const isSans = p.style.fontFamily && p.style.fontFamily.includes('Sans');
  p.style.fontFamily = isSans ? 'var(--font-serif)' : 'var(--font-sans)';
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

  const docName = isPaper ? '기안동 주거지 논문.docx' : '기안동 유적 Ⅳ 고찰.docx';
  document.getElementById('win-doc-title').textContent = docName + ' - ARCHAEO STUDIO';
  document.title = docName + ' - ARCHAEO STUDIO';
  document.getElementById('export-source-label').textContent = docName;
  document.getElementById('export-filename-input').value = docName.replace('.docx', '.pdf');

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
  const docLabel = isPaper ? '학술논문' : '발굴조사보고서 Ⅳ. 고찰';
  const topic = userFocus || (reportText ? '입력된 조사 자료 기반 유구·유물 성격 및 편년 검토' : '조사 유적의 유구·출토유물 양상과 주변 비교유적 편년 검토');
  return [
    '[' + docLabel + '] ' + topic,
    isPaper
      ? '구성: 제목 · 국문초록 · 주제어 · Ⅰ. 머리말 ~ Ⅴ. 맺음말 · 영문초록 · 각주 · 참고문헌'
      : '구성: Ⅳ. 고찰 (1. 머리말 · 2. 유구 구조와 성격 · 3. 출토유물 및 편년 · 4. 주변 유적 비교 및 맺음말) · 각주 · 참고문헌',
    '표·도면: 표 1(유구·유물 속성표) · 표 2(주변유적 대비표) · 제미나이 차트 및 단계별 변천 모식도 포함',
    '서지: 저자, 연도, 「제목」, 『학술지』 권호, 00~00쪽 (각주 ↔ 참고문헌 1:1 일치)',
    '원칙: ' + (datingText ? '입력된 연대분석 자료 교차검증 반영' : '입력되지 않은 연대분석 자료는 쓰지 않음')
  ].join(' · ');
}

async function refreshEngineeredPrompt() {
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

async function startExecution(mode) {
  if (mode) selectDocMode(mode);
  await refreshEngineeredPrompt(false);
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
  document.getElementById('doc-page-body').innerHTML =
    '<p class="as-page__empty" id="doc-page-empty">실행하면 이 자리에 ' +
    (currentDocType === 'paper' ? '논문' : '『Ⅳ. 고찰』') +
    '이 작성됩니다. 직접 써 넣으셔도 됩니다.</p>';
  document.getElementById('doc-page-notes').style.display = 'none';
  lastRenderedText = '';
}

function insertSampleTable() {
  const body = document.getElementById('doc-page-body');
  const empty = document.getElementById('doc-page-empty');
  if (empty) empty.remove();
  body.insertAdjacentHTML('beforeend',
    '<figure class="as-doc-table"><figcaption>표. 유구 속성 일람표</figcaption>' +
    '<table><thead><tr><th>유구명</th><th>평면형태</th><th>규모(m)</th><th>출토유물</th></tr></thead>' +
    '<tbody><tr><td>1호 주거지</td><td>방형</td><td>4.8×4.2</td><td>타날문단경호</td></tr></tbody></table></figure>'
  );
}

function insertSampleDiagram() {
  const body = document.getElementById('doc-page-body');
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
  const notesBox = document.getElementById('doc-page-notes');
  const list = document.getElementById('doc-page-notes-list');
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
 * 마크다운 원고를 ARCHAEO STUDIO A4 양식(DocChapter, DocSection, DocPara, FootnoteRef, DocTable, 차트/모식도, References, 쪽 아래 각주)으로 실시간 조판
 */
function renderManuscriptOntoA4Page(rawMd, isRunning) {
  const bodyEl = document.getElementById('doc-page-body');
  const notesWrap = document.getElementById('doc-page-notes');
  const notesList = document.getElementById('doc-page-notes-list');
  if (!rawMd || !rawMd.trim()) return { footnotesCount: 0, refsCount: 0 };

  let md = String(rawMd);

  // 1) 차트 및 모식도 코드블록을 A4 도면 박스로 변환
  const chartBlocks = [];
  md = md.replace(/\`\`\`wc-chart\\s*([\\s\\S]*?)\`\`\`/gi, (_, j) => {
    try {
      const spec = JSON.parse(j.trim());
      const idx = chartBlocks.length;
      const cats = (spec.data && spec.data.categories) || [];
      const series = (spec.data && spec.data.series) || [];
      let rowsHtml = '';
      cats.forEach((c, cIdx) => {
        const vals = series.map(s => (s.values && s.values[cIdx] != null ? s.values[cIdx] : '-'));
        rowsHtml += '<tr><td>' + escapeHtml(c) + '</td>' + vals.map(v => '<td>' + escapeHtml(String(v)) + '</td>').join('') + '</tr>';
      });
      chartBlocks.push(
        '<figure class="as-doc-table"><figcaption>차트. ' + escapeHtml(spec.title || '유구·유물 분포 차트') + '</figcaption>' +
        '<table><thead><tr><th>구분</th>' + series.map(s => '<th>' + escapeHtml(s.name || '') + '</th>').join('') + '</tr></thead>' +
        '<tbody>' + rowsHtml + '</tbody></table></figure>'
      );
      return '\\n@@AS_CHART_' + idx + '@@\\n';
    } catch {
      return '';
    }
  });

  const diagBlocks = [];
  md = md.replace(/\`\`\`wc-diagram\\s*([\\s\\S]*?)\`\`\`/gi, (_, j) => {
    try {
      const spec = JSON.parse(j.trim());
      const items = Array.isArray(spec.items) ? spec.items : [];
      const groups = [];
      items.forEach(it => {
        if ((it.level || 0) === 0) groups.push({ title: it.text || '', subs: [] });
        else if (groups.length > 0) groups[groups.length - 1].subs.push(it.text || '');
      });
      const idx = diagBlocks.length;
      diagBlocks.push(
        '<div class="as-doc-diagram"><div class="as-doc-diagram__title">모식도. 단계별 유구·출토유물 변천 모식도</div>' +
        '<div class="as-doc-diagram__steps">' +
        groups.map(g => '<div class="as-doc-diagram__step"><strong>' + escapeHtml(g.title) + '</strong>' + escapeHtml(g.subs.join(' · ')) + '</div>').join('') +
        '</div></div>'
      );
      return '\\n@@AS_DIAG_' + idx + '@@\\n';
    } catch {
      return '';
    }
  });

  // 2) 【각주】 추출하여 페이지 하단 각주(footnotes)로 분리
  const fnRegex = /(?:^|\\n)\\s*(?:#{1,6}\\s*)?(?:\\*{1,2}\\s*)?(?:【\\s*각\\s*주\\s*】|\\[\\s*각\\s*주\\s*\\]|각\\s*주)(?:\\s*\\*{1,2})?\\s*\\n([\\s\\S]*?)(?=(?:\\n\\s*(?:#{1,6}\\s*)?(?:\\*{1,2}\\s*)?(?:【\\s*참고\\s*문헌\\s*】|\\[\\s*참고\\s*문헌\\s*\\]|참고\\s*문헌|참고\\s*논문))|$)/i;
  const fnMatch = fnRegex.exec(md);
  const footnotes = [];
  if (fnMatch && fnMatch[1]) {
    fnMatch[1].split(/\\r?\\n/).forEach(rawLine => {
      const l = rawLine.trim().replace(/^[-*•]\\s*/, '').replace(/^\\*{1,2}|\\*{1,2}$/g, '');
      const m = /^(?:\\[\\^?(\\d+)\\]:?|\\(?(\\d+)\\)|(\\d+)\\.)\\s*(.+)$/.exec(l);
      if (m) {
        const n = Number(m[1] || m[2] || m[3]);
        const txt = (m[4] || '').trim().replace(/^\\d+\\)\\s*/, '');
        if (n > 0 && txt) footnotes.push({ n, text: txt });
      }
    });
    md = md.slice(0, fnMatch.index) + '\\n' + md.slice(fnMatch.index + fnMatch[0].length);
  }

  // 3) 줄 단위 파싱 (제목, 절, 표, 참고문헌, 본문 문단 + 위첨자 각주 1), 2))
  const lines = md.split(/\\r?\\n/);
  let htmlParts = [];
  let inTable = false;
  let tableRows = [];
  let inRefs = false;
  let refItems = [];

  function formatInline(s) {
    let out = escapeHtml(s);
    out = out.replace(/\\*\\*(.+?)\\*\\*/g, '<strong>$1</strong>');
    out = out.replace(/\\[\\^(\\d+)\\]/g, '<sup class="as-fnref">$1)</sup>');
    out = out.replace(/([.다됨함음임”"'\\])])\\s*(\\d+)\\)/g, '$1<sup class="as-fnref">$2)</sup>');
    return out;
  }

  function flushTable() {
    if (!inTable || tableRows.length === 0) return;
    inTable = false;
    const header = tableRows[0];
    const bodyRows = tableRows.slice(1).filter(r => !r.every(c => /^[:\\-\\s]+$/.test(c)));
    htmlParts.push(
      '<figure class="as-doc-table"><table><thead><tr>' +
      header.map(c => '<th>' + formatInline(c) + '</th>').join('') +
      '</tr></thead><tbody>' +
      bodyRows.map(r => '<tr>' + r.map(c => '<td>' + formatInline(c) + '</td>').join('') + '</tr>').join('') +
      '</tbody></table></figure>'
    );
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
      if (chartBlocks[idx]) htmlParts.push(chartBlocks[idx]);
      continue;
    }
    if (raw.startsWith('@@AS_DIAG_')) {
      flushTable();
      const idx = Number(raw.replace(/@@AS_DIAG_|@@/g, ''));
      if (diagBlocks[idx]) htmlParts.push(diagBlocks[idx]);
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
      htmlParts.push('<h1 class="as-doc-chapter">' + formatInline(raw.replace(/^#\\s+/, '')) + '</h1>');
    } else if (/^##\\s+/.test(raw)) {
      htmlParts.push('<h2 class="as-doc-section">' + formatInline(raw.replace(/^##\\s+/, '')) + '</h2>');
    } else if (/^#{3,6}\\s+/.test(raw)) {
      htmlParts.push('<h3 class="as-doc-subsec">' + formatInline(raw.replace(/^#{3,6}\\s+/, '')) + '</h3>');
    } else {
      htmlParts.push('<p class="as-doc-para">' + formatInline(raw) + '</p>');
    }
  }
  flushTable();

  if (isRunning && htmlParts.length > 0) {
    const lastIdx = htmlParts.length - 1;
    htmlParts[lastIdx] = htmlParts[lastIdx].replace(/<\\/p>$/, '<span class="as-caret" aria-hidden="true"></span></p>');
  }

  if (refItems.length > 0) {
    htmlParts.push(
      '<section class="as-doc-refs"><h2 class="as-doc-section">【참고문헌】</h2><ul>' +
      refItems.map(r => '<li>' + r + '</li>').join('') +
      '</ul></section>'
    );
  }

  bodyEl.innerHTML = htmlParts.join('\\n');

  if (footnotes.length > 0) {
    notesWrap.style.display = 'block';
    notesList.innerHTML = footnotes.map(f =>
      '<li><span class="as-page__fnn">' + f.n + ')</span><span>' + escapeHtml(f.text) + '</span></li>'
    ).join('');
  } else {
    notesWrap.style.display = 'none';
  }

  return { footnotesCount: footnotes.length, refsCount: refItems.length };
}

function classifyLogEntry(rawLine) {
  const s = String(rawLine || '');
  const timeMatch = /^\\[(\\d{2}:\\d{2}:\\d{2})\\]\\s*(.*)$/.exec(s);
  const time = timeMatch ? timeMatch[1] : new Date().toLocaleTimeString('ko-KR', { hour12: false });
  const text = (timeMatch ? timeMatch[2] : s).replace(/^\\[[A-Z_]+\\]\\s*/, '');
  let kind = 'write';
  if (/DBpia|수집|논문|검색/i.test(text)) kind = 'collect';
  else if (/완료|끝|완성/i.test(text)) kind = 'done';
  else if (/오류|실패|중단/i.test(text)) kind = 'error';
  else if (/참고|비어|제외/i.test(text)) kind = 'note';
  return { time, kind, text };
}

const LOG_LABELS = { collect: '수집', write: '작성', note: '알림', warn: '확인', error: '오류', done: '완료' };

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
        lblReport.textContent = paused ? '잠시 멈춤' : '작성 중…';
      } else {
        lblPaper.textContent = paused ? '잠시 멈춤' : '작성 중…';
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
      doneTagEl.innerHTML = '<span class="as-tag as-tag--ok">작성 끝 · ' + nowHm + '</span>';
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

    // 4) A4 문서 실시간 조판
    const liveMd = (p.latestManuscript && p.latestManuscript.trim()) || (p.liveStreamText && p.liveStreamText.trim()) || '';
    let fnCount = (p.wordcraftInfo && p.wordcraftInfo.footnotesCount) || 0;
    let refCount = papersCount;
    if (liveMd && liveMd !== lastRenderedText) {
      lastRenderedText = liveMd;
      const counts = renderManuscriptOntoA4Page(liveMd, running);
      if (counts.footnotesCount) fnCount = counts.footnotesCount;
      if (counts.refsCount) refCount = counts.refsCount;
    }

    // 5) 수집 띠 (CollectStrip)
    const collectLeft = document.getElementById('collect-left');
    const collectRightTime = document.getElementById('collect-right-time');
    if (!running && !done && !liveMd) {
      collectLeft.innerHTML = '<i class="as-collect__dot is-hollow"></i><span>실행 전</span>';
    } else if (done) {
      collectLeft.innerHTML =
        '<span class="as-collect__item"><i class="as-collect__dot is-ok"></i>DBpia 논문 수집 끝</span>' +
        '<span class="as-collect__num" style="margin-left:8px;">' + papersCount + '건</span>' +
        '<span class="as-collect__item" style="margin-left:12px;">본문 작성 끝</span>' +
        '<span class="as-collect__num" style="margin-left:8px;">각주 ' + fnCount + ' · 참고문헌 ' + refCount + '</span>';
      collectRightTime.textContent = 'WordCraft .docx 자동 저장됨';
    } else {
      collectLeft.innerHTML =
        '<span class="as-collect__item"><i class="as-collect__dot"></i>DBpia 논문 수집 중</span>' +
        '<span class="as-collect__num" style="margin-left:8px;">' + papersCount + '건</span>' +
        '<span class="as-collect__item" style="margin-left:12px;"><span class="as-collect__mini"><span style="width:' + pct + '%"></span></span>' +
        (paused ? '잠시 멈춤 ' : '본문 작성 ') + pct + '%</span>';
      collectRightTime.textContent = '실시간 작성 중';
    }

    // 6) 상태줄 (StatusBar)
    const hintEl = document.getElementById('status-hint');
    if (!document.getElementById('scrim-dbpia').classList.contains('is-open') &&
        !document.getElementById('scrim-export').classList.contains('is-open')) {
      if (running) {
        hintEl.textContent = '제미나이가 ' + (currentDocType === 'paper' ? '논문' : '『Ⅳ. 고찰』') + ' 본문을 쓰고 있습니다. 쓰는 동안에도 문서를 고칠 수 있습니다.';
      } else if (done) {
        hintEl.textContent = '작성이 끝났습니다. 바로 고칠 수 있고, 필요하면 PDF로 내보내세요.';
      } else {
        hintEl.textContent = '요청과 자료를 넣고 실행하세요. 비워 둔 자료는 빼고 씁니다.';
      }
    }

    const totalPages = (p.wordcraftInfo && p.wordcraftInfo.pages) || (liveMd ? Math.max(1, Math.ceil(liveMd.length / 1800)) : 1);
    const totalChars = liveMd ? liveMd.length : Number(p.totalCharsWritten || 0);
    document.getElementById('status-pages').textContent = '쪽 1 / ' + totalPages;
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
