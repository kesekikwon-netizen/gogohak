/* @ds-bundle: {"format":4,"namespace":"ArchaeoStudio","components":[{"name":"Icon"},{"name":"Button"},{"name":"Tag"},{"name":"Tooltip"},{"name":"TextField"},{"name":"SearchField"},{"name":"Chip"},{"name":"Segmented"},{"name":"PanelHeader"},{"name":"Menu"},{"name":"Dialog"},{"name":"Ribbon"},{"name":"StatusBar"},{"name":"CollectStrip"},{"name":"StudioStep"},{"name":"PromptField"},{"name":"SourceInput"},{"name":"RunPanel"},{"name":"ProgressBar"},{"name":"LogPanel"},{"name":"DbpiaWindow"},{"name":"ExportDialog"},{"name":"DocPage"},{"name":"PaperTitle"}]} */
(function () {
  'use strict';
  var React = window.React;
  var h = React.createElement;
  var useState = React.useState;

  /* 선 아이콘(24 칸, 선 1.6, currentColor). Trowel 3.0 아이콘과 같은 규칙 */
  var ICONS = {"new": ["0 0 24 24", "<path d=\"M6 3h8l4 4v14H6z M14 3v4h4\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"1.6\" stroke-linejoin=\"round\"/><path d=\"M12 11v6M9 14h6\" stroke=\"currentColor\" stroke-width=\"1.6\"/>"], "open": ["0 0 24 24", "<path d=\"M3 7h6l2 2h10v10H3z\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"1.6\" stroke-linejoin=\"round\"/>"], "save": ["0 0 24 24", "<path d=\"M5 4h11l3 3v13H5z M8 4v5h7V4 M8 20v-6h8v6\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"1.6\" stroke-linejoin=\"round\"/>"], "text": ["0 0 24 24", "<path d=\"M5 5h14M12 5v15M9 20h6\" stroke=\"currentColor\" stroke-width=\"1.8\" fill=\"none\"/>"], "table": ["0 0 24 24", "<rect x=\"3\" y=\"5\" width=\"18\" height=\"14\" rx=\"1\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"1.6\"/><path d=\"M3 9.5h18M3 14.2h18M9 5v14\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"1.6\"/>"], "img": ["0 0 24 24", "<rect x=\"4\" y=\"5\" width=\"16\" height=\"14\" rx=\"1.5\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"1.6\"/><path d=\"M4 16l5-5 4 4 3-3 4 4\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"1.6\"/>"], "footnote": ["0 0 24 24", "<path d=\"M3.5 6.5h11M3.5 10.5h11M3.5 14.5h7\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"1.6\"/><path d=\"M3.5 19h17\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"1.1\"/><path d=\"M17.2 5.2l1.6-1.2v6.5\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"1.4\" stroke-linejoin=\"round\"/>"], "panel": ["0 0 24 24", "<rect x=\"3\" y=\"4.5\" width=\"18\" height=\"15\" rx=\"1.5\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"1.6\"/><path d=\"M9.5 4.5v15M5.2 8.5h2.2M5.2 11.5h2.2M5.2 14.5h2.2\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"1.6\"/>"], "export": ["0 0 24 24", "<path d=\"M12 3v11M8 7l4-4 4 4M5 13v7h14v-7\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"1.6\" stroke-linejoin=\"round\"/>"], "search": ["0 0 24 24", "<circle cx=\"10.5\" cy=\"10.5\" r=\"6\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"1.8\"/><path d=\"M15 15l5 5\" stroke=\"currentColor\" stroke-width=\"1.8\"/>"], "run": ["0 0 24 24", "<path d=\"M8 5v14l11-7z\" fill=\"currentColor\" stroke=\"currentColor\" stroke-width=\"1.2\" stroke-linejoin=\"round\"/>"], "pause": ["0 0 24 24", "<rect x=\"7\" y=\"5\" width=\"3.6\" height=\"14\" rx=\".8\" fill=\"currentColor\"/><rect x=\"13.4\" y=\"5\" width=\"3.6\" height=\"14\" rx=\".8\" fill=\"currentColor\"/>"], "stop": ["0 0 24 24", "<rect x=\"6\" y=\"6\" width=\"12\" height=\"12\" rx=\"1.5\" fill=\"currentColor\"/>"], "key": ["0 0 24 24", "<circle cx=\"8\" cy=\"15.5\" r=\"4\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"1.6\"/><path d=\"M11 12.5l8.5-8.5M16 8l2.5 2.5M13.8 10.2l2 2\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"1.6\" stroke-linecap=\"round\"/>"]};

  function cx() {
    var out = [];
    for (var i = 0; i < arguments.length; i++) { if (arguments[i]) out.push(arguments[i]); }
    return out.join(' ');
  }
  function omit(o, keys) {
    var r = {};
    for (var k in o) { if (Object.prototype.hasOwnProperty.call(o, k) && keys.indexOf(k) < 0) r[k] = o[k]; }
    return r;
  }
  function useControlled(value, def, onChange) {
    var st = useState(def);
    var controlled = value !== undefined;
    return [controlled ? value : st[0], function (n) { if (!controlled) st[1](n); if (onChange) onChange(n); }];
  }
  function optId(o) { return typeof o === 'string' ? o : o.id; }
  function optLabel(o) { return typeof o === 'string' ? o : o.label; }

  /* ───────── 기본 ───────── */

  function Icon(p) {
    var d = ICONS[p.name];
    if (!d) return null;
    var size = p.size || 18;
    return h('svg', {
      className: cx('as-icon', p.className), width: size, height: size, viewBox: d[0],
      role: p.label ? 'img' : undefined, 'aria-label': p.label || undefined, 'aria-hidden': p.label ? undefined : true,
      style: p.style, dangerouslySetInnerHTML: { __html: d[1] }
    });
  }

  function Button(p) {
    var variant = p.variant || 'default', size = p.size || 'md';
    var rest = omit(p, ['variant', 'size', 'icon', 'children', 'className', 'block']);
    return h('button', Object.assign({ type: 'button' }, rest, {
      className: cx('as-btn', 'as-btn--' + variant, 'as-btn--' + size, p.block && 'as-btn--block', p.className)
    }), p.icon ? h(Icon, { name: p.icon, size: size === 'lg' ? 16 : 14 }) : null, p.children);
  }

  function Tag(p) {
    return h('span', { className: cx('as-tag', 'as-tag--' + (p.tone || 'neutral'), p.className) }, p.children);
  }

  function Tooltip(p) { return h('span', { className: 'as-tooltip', role: 'tooltip' }, p.children); }

  function TextField(p) {
    var rest = omit(p, ['multiline', 'rows', 'className']);
    if (p.multiline) return h('textarea', Object.assign({ rows: p.rows || 4 }, rest, { className: cx('as-field', 'as-field--multi', p.className) }));
    return h('input', Object.assign({ type: 'text' }, rest, { className: cx('as-field', p.className) }));
  }

  function SearchField(p) {
    return h('label', { className: cx('as-search', p.className) },
      h(Icon, { name: 'search', size: 15 }),
      h('input', { type: 'search', placeholder: p.placeholder || '명령 찾기', value: p.value, onChange: p.onChange, 'aria-label': p.placeholder || '명령 찾기' }),
      p.shortcut ? h('span', { className: 'as-search__key' }, p.shortcut) : null);
  }

  function Chip(p) {
    return h('button', { type: 'button', className: cx('as-chip', p.on && 'is-on', p.className), 'aria-pressed': !!p.on, onClick: p.onClick }, p.children);
  }

  function ChipGroup(p) {
    var s = useControlled(p.value, p.defaultValue !== undefined ? p.defaultValue : optId(p.options[0]), p.onChange);
    return h('div', { className: 'as-chips', role: 'radiogroup', 'aria-label': p.label }, p.options.map(function (o) {
      var id = optId(o);
      return h(Chip, { key: id, on: s[0] === id, onClick: function () { s[1](id); } }, optLabel(o));
    }));
  }

  function Segmented(p) {
    var s = useControlled(p.value, p.defaultValue !== undefined ? p.defaultValue : optId(p.options[0]), p.onChange);
    return h('div', { className: cx('as-seg', 'as-seg--' + (p.size || 'md'), p.className), role: 'radiogroup', 'aria-label': p.label },
      p.options.map(function (o) {
        var id = optId(o);
        return h('button', { key: id, type: 'button', role: 'radio', 'aria-checked': s[0] === id, disabled: p.disabled, className: cx('as-seg__item', s[0] === id && 'is-on'), onClick: function () { s[1](id); } }, optLabel(o));
      }));
  }

  function PanelHeader(p) {
    return h('div', { className: cx('as-ph', p.className) },
      h('span', { className: 'as-ph__title' }, p.title),
      p.sub ? h('span', { className: 'as-ph__sub' }, p.sub) : null,
      p.actions ? h('span', { className: 'as-ph__actions' }, p.actions) : null);
  }

  function Menu(p) {
    return h('div', { className: 'as-menu', role: 'menu', style: { width: p.width || 250 } },
      p.items.map(function (it, i) {
        if (it.separator) return h('hr', { key: 's' + i });
        return h('div', { key: i, role: 'menuitem', className: cx('as-menu__item', it.on && 'is-on', it.disabled && 'is-disabled') },
          h('span', null, it.label), h('span', { className: 'as-menu__key' }, it.shortcut || ''));
      }));
  }

  function Dialog(p) {
    return h('div', { className: 'as-dialog', role: 'dialog', 'aria-label': p.title, style: p.width ? { width: p.width, maxWidth: '100%' } : undefined },
      h('div', { className: 'as-ph as-dialog__head' }, h('span', { className: 'as-ph__title' }, p.title), p.sub ? h('span', { className: 'as-ph__sub' }, p.sub) : null),
      h('div', { className: 'as-dialog__body' }, p.children),
      p.footer ? h('div', { className: 'as-dialog__foot' }, p.footer) : null);
  }

  function FieldRow(p) {
    return h('div', { className: 'as-frow' }, h('span', { className: 'as-frow__label' }, p.label), h('div', { className: 'as-frow__ctl' }, p.children));
  }

  /* ───────── 화면 틀 ───────── */

  function RibbonTile(p) {
    return h('button', { type: 'button', className: cx('as-tile', p.state === 'selected' && 'is-sel'), 'aria-pressed': p.state === 'selected' ? true : undefined, disabled: p.disabled, onClick: p.onClick },
      h('span', { className: 'as-tile__ic' }, h(Icon, { name: p.icon, size: 24 })),
      h('span', { className: 'as-tile__label' }, p.label, p.dropdown ? h('span', { className: 'as-tile__dd' }, '▾') : null));
  }
  function RibbonGroup(p) {
    return h('div', { className: 'as-rg', role: 'group', 'aria-label': p.caption },
      h('div', { className: 'as-rg__cap' }, p.caption), h('div', { className: 'as-rg__tiles' }, p.children));
  }
  function Ribbon(p) {
    return h('div', { className: 'as-ribbon' }, p.children, p.search ? h('div', { className: 'as-ribbon__search' }, p.search) : null);
  }

  /* WordCraft 리본: 파일 · 홈·서식 · 삽입 · 스튜디오 · 내보내기 */
  function StudioRibbon(p) {
    return h(Ribbon, { search: h(SearchField, { placeholder: '명령 찾기', shortcut: 'Ctrl K' }) },
      h(RibbonGroup, { caption: '파일' }, h(RibbonTile, { icon: 'new', label: '새 문서' }), h(RibbonTile, { icon: 'open', label: '열기' }), h(RibbonTile, { icon: 'save', label: '저장' })),
      h(RibbonGroup, { caption: '홈·서식' }, h(RibbonTile, { icon: 'text', label: '글자', dropdown: true })),
      h(RibbonGroup, { caption: '삽입' }, h(RibbonTile, { icon: 'table', label: '표' }), h(RibbonTile, { icon: 'img', label: '도면' }), h(RibbonTile, { icon: 'footnote', label: '각주' })),
      h(RibbonGroup, { caption: '스튜디오' }, h(RibbonTile, { icon: 'panel', label: '스튜디오', state: p.studioOpen === false ? undefined : 'selected' })),
      h(RibbonGroup, { caption: '내보내기' }, h(RibbonTile, { icon: 'export', label: 'PDF 변환' })));
  }

  function StatusBar(p) {
    return h('div', { className: 'as-status', role: 'status' },
      h('span', { className: cx('as-status__hint', p.error && 'is-error') }, p.hint),
      h('div', { className: 'as-status__r' }, (p.items || []).map(function (it, i) {
        return h('span', { key: i, className: cx('as-status__item', it.mono && 'is-mono') }, it.label);
      })));
  }

  function CollectStrip(p) {
    var st = p.state || 'running';
    var left;
    if (st === 'idle') {
      left = [h('span', { key: 'a', className: 'as-collect__item' }, h('i', { className: 'as-collect__dot is-hollow' }), p.idleText || '실행 전')];
    } else if (st === 'done') {
      left = [
        h('span', { key: 'a', className: 'as-collect__item' }, h('i', { className: 'as-collect__dot is-ok' }), 'DBpia 논문 수집 끝'),
        h('span', { key: 'b', className: 'as-collect__num' }, (p.found || 0) + '건'),
        h('span', { key: 'c', className: 'as-collect__item' }, '본문 작성 끝'),
        p.footnotes != null ? h('span', { key: 'd', className: 'as-collect__num' }, '각주 ' + p.footnotes + ' · 참고문헌 ' + (p.refs || 0)) : null];
    } else {
      left = [
        h('span', { key: 'a', className: 'as-collect__item' }, h('i', { className: 'as-collect__dot' }), 'DBpia 논문 수집 중'),
        p.found != null ? h('span', { key: 'b', className: 'as-collect__num' }, p.found + '건') : null,
        p.progress != null ? h('span', { key: 'c', className: 'as-collect__item' }, h('span', { className: 'as-collect__mini' }, h('span', { style: { width: p.progress + '%' } })), (st === 'paused' ? '잠시 멈춤 ' : '본문 작성 ') + p.progress + '%') : null];
    }
    return h('div', { className: 'as-collect', role: 'status' }, left, p.right ? h('span', { className: 'as-collect__r' }, p.right) : null);
  }

  /* ───────── 스튜디오 패널 ───────── */

  function StudioStep(p) {
    return h('section', { className: 'as-step', 'aria-label': p.title },
      h('div', { className: 'as-step__head' },
        p.n != null ? h('span', { className: 'as-step__n' }, String(p.n)) : null,
        h('span', { className: 'as-step__title' }, p.title),
        p.aside ? h('span', { className: 'as-step__aside' }, p.aside) : null),
      p.help ? h('p', { className: 'as-step__help' }, p.help) : null,
      h('div', { className: 'as-step__body' }, p.children));
  }

  function PromptField(p) {
    var open = useControlled(p.showProcessed, false, p.onToggleProcessed);
    return h('div', { className: 'as-prompt' },
      h(TextField, { multiline: true, rows: p.rows || 3, value: p.value, defaultValue: p.value === undefined ? p.defaultValue : undefined, onChange: p.onChange, placeholder: p.placeholder || '무엇을 쓸지 적어 주세요. 예: 기안동 유적 주거지의 구조와 편년을 정리해 줘', 'aria-label': '요청' }),
      h('div', { className: 'as-prompt__note' },
        h(Tag, { tone: 'new' }, '1차 가공'),
        h('span', null, '입력하신 요청은 고고학 학술 양식에 맞춰 1차 가공되어 전달됩니다.')),
      p.processed ? h('button', { type: 'button', className: 'as-linkbtn', 'aria-expanded': open[0], onClick: function () { open[1](!open[0]); } }, (open[0] ? '▾ ' : '▸ ') + '가공된 요청 보기') : null,
      p.processed && open[0] ? h('div', { className: 'as-prompt__processed' }, p.processed) : null);
  }

  function FileChip(p) {
    return h('span', { className: 'as-file' },
      h('span', { className: 'as-file__name' }, p.name),
      p.size ? h('span', { className: 'as-file__size' }, p.size) : null,
      h('button', { type: 'button', className: 'as-file__x', 'aria-label': p.name + ' 빼기', onClick: p.onRemove }, '✕'));
  }

  function SourceInput(p) {
    var files = p.files || [];
    var empty = !p.value && !p.defaultValue && files.length === 0;
    return h('div', { className: 'as-source' },
      h(TextField, { multiline: true, rows: p.rows || 4, value: p.value, defaultValue: p.value === undefined ? p.defaultValue : undefined, onChange: p.onChange, placeholder: p.placeholder, 'aria-label': p.label || '자료' }),
      h('div', { className: 'as-source__bar' },
        h(Button, { icon: 'open', onClick: p.onPick }, '파일 불러오기'),
        h('span', { className: 'as-source__types' }, p.accept || '.docx .pdf .txt .md .csv'),
        empty && p.optional ? h(Tag, { tone: 'neutral', className: 'as-source__empty' }, '비워 둠') : null),
      files.length ? h('div', { className: 'as-source__files' }, files.map(function (f, i) { return h(FileChip, { key: i, name: f.name, size: f.size, onRemove: f.onRemove }); })) : null,
      p.note ? h('p', { className: 'as-source__note' }, p.note) : null);
  }

  function ProgressBar(p) {
    var v = Math.max(0, Math.min(100, p.value || 0));
    return h('div', { className: 'as-progress' },
      p.label ? h('div', { className: 'as-progress__row' }, h('span', null, p.label), h('span', { className: 'as-progress__pct' }, Math.round(v) + '%')) : null,
      h('div', { className: 'as-progress__track', role: 'progressbar', 'aria-valuenow': Math.round(v), 'aria-valuemin': 0, 'aria-valuemax': 100, 'aria-label': p.label || '진행' },
        h('div', { className: cx('as-progress__fill', p.done && 'is-done'), style: { width: v + '%' } })));
  }

  var LOG_KIND = { collect: '수집', write: '작성', note: '알림', warn: '확인', error: '오류', done: '완료' };
  function LogPanel(p) {
    var entries = p.entries || [];
    return h('div', { className: 'as-log' },
      (p.title !== false || p.progress != null) ? h('div', { className: 'as-log__head' },
        p.title === false ? null : h('span', { className: 'as-ph__title' }, p.title || '진행 기록'),
        p.progress != null ? h(ProgressBar, { value: p.progress, label: p.progressLabel || '본문 작성', done: p.progress >= 100 }) : null) : null,
      entries.length ? h('ol', { className: 'as-log__list', 'aria-live': 'polite' }, entries.map(function (e, i) {
        return h('li', { key: i, className: cx('as-log__row', 'is-' + e.kind) },
          h('span', { className: 'as-log__time' }, e.time),
          h('span', { className: 'as-log__kind' }, h('i', null), LOG_KIND[e.kind] || e.kind),
          h('span', { className: 'as-log__text' }, e.text));
      })) : h('p', { className: 'as-log__empty' }, p.empty || '실행하면 진행 기록이 여기에 쌓입니다.'));
  }

  function RunPanel(p) {
    var mode = p.mode || 'report';
    var st = p.state || 'idle';
    var running = st === 'running' || st === 'paused';
    function runBtn(id, label) {
      var active = mode === id;
      if (running && active) return h(Button, { variant: 'accent', size: 'lg', block: true, disabled: true, icon: 'run' }, st === 'paused' ? '잠시 멈춤' : '작성 중…');
      return h(Button, { variant: active ? 'accent' : 'default', size: 'lg', block: true, icon: 'run', disabled: running, onClick: function () { if (p.onRun) p.onRun(id); } }, label);
    }
    return h('div', { className: 'as-run' },
      h('div', { className: 'as-run__main' }, runBtn('report', '발굴조사보고서 고찰 실행'), runBtn('paper', '논문 실행')),
      running ? h('div', { className: 'as-run__ctl' },
        h(Button, { icon: st === 'paused' ? 'run' : 'pause', onClick: p.onPause }, st === 'paused' ? '이어서' : '잠시 멈춤'),
        h(Button, { icon: 'stop', onClick: p.onStop }, '그만두기')) : null,
      h('div', { className: 'as-run__sub' },
        h(Button, { icon: 'key', onClick: p.onLogin, className: 'as-btn--brass' }, '디비피아 로그인'),
        p.loggedIn ? h(Tag, { tone: 'brass' }, '로그인됨') : h(Tag, { tone: 'neutral' }, '로그인 필요'),
        h('span', { className: 'as-run__spacer' }),
        h(Button, { variant: st === 'done' ? 'default' : 'quiet', icon: 'export', onClick: p.onExport }, 'PDF로 내보내기')));
  }

  /* DBpia 로그인 창: 앱이 띄우는 창의 틀만. 안쪽은 DBpia 사이트가 그대로 열린다 */
  function DbpiaWindow(p) {
    return h(Dialog, { title: '디비피아 로그인', sub: '학술논문을 찾을 때 쓰는 로그인입니다', width: p.width || 560,
      footer: [
        h('span', { key: 't', className: 'as-dialog__state' }, p.loggedIn ? h(Tag, { tone: 'brass' }, '로그인됨') : h(Tag, { tone: 'neutral' }, '로그인 필요')),
        h(Button, { key: 'c', variant: p.loggedIn ? 'strong' : 'default', onClick: p.onClose }, p.loggedIn ? '닫고 계속하기' : '닫기')] },
      h('div', { className: 'as-web' },
        h('div', { className: 'as-web__bar' }, h(Icon, { name: 'key', size: 13 }), h('span', null, p.address || 'DBpia')),
        h('div', { className: 'as-web__view' },
          h('p', null, '디비피아 사이트의 로그인 화면이 이 칸에 그대로 열립니다.'),
          h('p', { className: 'as-web__sub' }, '개인·학교·기관 로그인 모두 여기서 하고, 끝나면 창을 닫으세요.'))));
  }

  function ExportDialog(p) {
    return h(Dialog, { title: 'PDF로 내보내기', sub: p.source || '기안동 유적 Ⅳ 고찰.docx', width: p.width || 520,
      footer: [h(Button, { key: 'c', onClick: p.onCancel }, '그만두기'), h(Button, { key: 'e', variant: 'strong', icon: 'export', onClick: p.onExport }, '내보내기')] },
      h(FieldRow, { label: '파일 이름' }, h(TextField, { defaultValue: p.fileName || '기안동 유적 Ⅳ 고찰.pdf' })),
      h(FieldRow, { label: '범위' }, h(ChipGroup, { label: '범위', options: ['문서 전체', '지금 쪽'] })),
      h(FieldRow, { label: '내보낸 뒤' }, h(ChipGroup, { label: '내보낸 뒤', options: ['PDF 열기', '폴더만 열기', '아무것도 안 함'] })),
      h('p', { className: 'as-dialog__note' }, '각주는 쪽 아래, 참고문헌은 문서 끝 그대로 옮깁니다. 작업은 계속 .docx에서 합니다.'));
  }

  /* ───────── A4 문서 ───────── */

  function DocPage(p) {
    return h('article', { className: cx('as-page', p.className), style: p.scale ? { zoom: p.scale } : undefined },
      h('div', { className: 'as-page__body' }, p.empty && !p.children ? h('p', { className: 'as-page__empty' }, p.empty) : p.children),
      p.footnotes ? h('div', { className: 'as-page__notes' }, h('hr', null), h('ol', null, p.footnotes.map(function (f, i) {
        return h('li', { key: i }, h('span', { className: 'as-page__fnn' }, (f.n || i + 1) + ')'), h('span', null, f.text));
      }))) : null,
      p.pageNumber != null ? h('div', { className: 'as-page__num' }, '- ' + p.pageNumber + ' -') : null);
  }
  function DocChapter(p) { return h('h1', { className: 'as-doc-chapter' }, p.children); }
  function DocSection(p) { return h('h2', { className: 'as-doc-section' }, p.children); }
  function DocPara(p) { return h('p', { className: 'as-doc-para' }, p.children, p.live ? h('span', { className: 'as-caret', 'aria-hidden': true }) : null); }
  function FootnoteRef(p) { return h('sup', { className: 'as-fnref' }, p.n + ')'); }
  function DocTable(p) {
    return h('figure', { className: 'as-doc-table' },
      p.caption ? h('figcaption', null, p.caption) : null,
      h('table', null,
        h('thead', null, h('tr', null, p.columns.map(function (c, i) { return h('th', { key: i }, c); }))),
        h('tbody', null, p.rows.map(function (r, i) { return h('tr', { key: i }, r.map(function (c, j) { return h('td', { key: j }, c); })); }))));
  }
  function References(p) {
    return h('section', { className: 'as-doc-refs' },
      h('h2', { className: 'as-doc-section' }, p.title || '【참고문헌】'),
      h('ul', null, p.items.map(function (r, i) { return h('li', { key: i }, r); })));
  }
  function PaperTitle(p) {
    return h('header', { className: 'as-paper' },
      h('h1', { className: 'as-paper__title' }, p.title),
      p.subtitle ? h('p', { className: 'as-paper__subtitle' }, p.subtitle) : null,
      p.author ? h('p', { className: 'as-paper__author' }, p.author) : null,
      p.abstract ? h('div', { className: 'as-paper__abstract' },
        h('p', { className: 'as-paper__abs-h' }, p.abstractTitle || '국문초록'),
        h('p', { className: 'as-paper__abs-t' }, p.abstract),
        p.keywords ? h('p', { className: 'as-paper__kw' }, h('b', null, '주제어'), ' : ' + p.keywords.join(', ')) : null) : null);
  }

  /* ───────── 화면 한 장 ───────── */

  var REPORT_DOC = function (live) {
    return [
      h(DocChapter, { key: 1 }, 'Ⅳ. 고  찰'),
      h(DocSection, { key: 2 }, '1. 유구 구조와 편년적 위치'),
      h(DocPara, { key: 3 }, '기안동 유적에서 확인된 원삼국~백제 한성기 주거지는 평면 방형계를 유지하며 부뚜막과 외줄구들을 갖춘 구조가 중심을 이룬다', h(FootnoteRef, { n: 1 }), '. 1호 주거지는 장축 4.8m, 단축 4.2m 규모로 경기 남부 일대 같은 시기 주거지의 평균 범위에 든다.'),
      h(DocTable, { key: 4, caption: '표 1. 기안동 유적 주거지 속성', columns: ['유구', '평면형태', '규모(m)', '출토유물'], rows: [['1호', '방형', '4.8×4.2', '타날문단경호'], ['2호', '방형', '5.1×4.6', '경질무문토기'], ['3호', '말각방형', '4.4×4.0', '시루편']] }),
      h(DocPara, { key: 5, live: live }, '출토유물의 구성으로 보아 이들 주거지는 대체로 3세기 후반에서 4세기 전반에 걸쳐 조성된 것으로 판단되며', live ? '' : ', 부뚜막 구조의 변화는 한성기 백제 주거지의 일반적 흐름과 같다', live ? null : h(FootnoteRef, { n: 2 }), live ? '' : '.'),
      live ? null : h(DocPara, { key: 6 }, '연대분석 자료가 없으므로 이 판단은 출토유물의 형식과 선행 연구의 편년안에 기댄 것이다.'),
      h(References, { key: 7, items: live ? ['김고고, 2023, 「경기 남부 원삼국시대 주거지 구조 연구」, 『한국고고학보』 128, 한국고고학회.'] : ['김고고, 2023, 「경기 남부 원삼국시대 주거지 구조 연구」, 『한국고고학보』 128, 한국고고학회.', '박발굴, 2021, 「한성기 백제 주거지의 부뚜막 구조」, 『고고학』 20-2, 중부고고학회.'] })
    ];
  };
  var FN1 = { text: '김고고, 2023, 「경기 남부 원삼국시대 주거지 구조 연구」, 『한국고고학보』 128, 한국고고학회, 45~48쪽.' };
  var FN2 = { text: '박발굴, 2021, 「한성기 백제 주거지의 부뚜막 구조」, 『고고학』 20-2, 중부고고학회, 112~115쪽.' };

  var LOG_RUNNING = [
    { time: '14:31:02', kind: 'collect', text: 'DBpia에서 「원삼국 주거지 편년」 관련 논문 12건을 찾았습니다' },
    { time: '14:31:40', kind: 'write', text: '1절 「유구 구조와 편년적 위치」를 쓰고 있습니다' },
    { time: '14:32:05', kind: 'note', text: '연대분석 자료가 비어 있어 연대 수치는 쓰지 않습니다' },
    { time: '14:32:18', kind: 'collect', text: '김고고 2023 『한국고고학보』 128 — 각주 1)에 넣었습니다' }
  ];
  var LOG_PAPER = [
    { time: '14:31:02', kind: 'collect', text: 'DBpia에서 「원삼국 주거지 편년」 관련 논문 12건을 찾았습니다' },
    { time: '14:31:25', kind: 'write', text: '제목과 국문초록을 썼습니다' },
    { time: '14:31:48', kind: 'write', text: 'Ⅰ. 머리말을 쓰고 있습니다' },
    { time: '14:32:05', kind: 'note', text: '연대분석 자료가 비어 있어 절대연대는 다루지 않습니다' }
  ];
  var LOG_DONE = [
    { time: '14:38:51', kind: 'warn', text: '논문 1건은 원문을 열 수 없어 빼고 썼습니다' },
    { time: '14:39:30', kind: 'write', text: '3절 「유적의 성격」을 마쳤습니다' },
    { time: '14:40:02', kind: 'done', text: '작성이 끝났습니다 · 각주 14개, 참고문헌 12건' }
  ];

  function StudioScreen(p) {
    var st = p.state || 'running';
    var mode = useState(p.mode || 'report');
    var paper = mode[0] === 'paper';
    var ready = st === 'ready';
    var hints = {
      ready: '요청과 자료를 넣고 실행하세요. 비워 둔 자료는 빼고 씁니다.',
      running: '제미나이가 ' + (paper ? 'Ⅰ. 머리말' : '1절') + '을 쓰고 있습니다. 쓰는 동안에도 문서를 고칠 수 있습니다.',
      done: '작성이 끝났습니다. 바로 고칠 수 있고, 필요하면 PDF로 내보내세요.'
    };
    var doc;
    if (ready) doc = h(DocPage, { empty: '실행하면 이 자리에 ' + (paper ? '논문' : '『Ⅳ. 고찰』') + '이 작성됩니다. 직접 써 넣으셔도 됩니다.', pageNumber: 1 });
    else if (paper) doc = h(DocPage, { pageNumber: 1, footnotes: [FN1] },
      h(PaperTitle, { title: '기안동 유적 원삼국~한성기 주거지의 구조와 편년', author: '홍길동 (○○문화유산연구원)', abstract: '이 글은 기안동 유적에서 조사된 원삼국~백제 한성기 주거지의 평면 형태와 부뚜막 구조를 정리하고, 출토유물과 선행 연구를 바탕으로 그 편년적 위치를 검토하였다. 연대분석 자료가 없어 절대연대는 다루지 않았다.', keywords: ['원삼국', '한성기', '주거지', '부뚜막', '편년'] }),
      h(DocChapter, null, 'Ⅰ. 머리말'),
      h(DocPara, { live: st === 'running' }, '경기 남부 지역의 원삼국~한성기 주거지는 최근 조사 사례가 크게 늘었다', h(FootnoteRef, { n: 1 }), '. 이 글에서는 기안동 유적 주거지의 구조를 정리하고'));
    else doc = h(DocPage, { pageNumber: 87, footnotes: st === 'done' ? [FN1, FN2] : [FN1] }, REPORT_DOC(st === 'running'));

    var stripProps = ready ? { state: 'idle', idleText: p.loggedIn === false || p.overlay === 'login' ? '실행 전 · 디비피아 로그인 필요' : '실행 전', right: '저장됨 14:20' }
      : st === 'done' ? { state: 'done', found: 18, footnotes: 14, refs: 12, right: '자동 저장 14:40' }
      : { state: 'running', found: 12, progress: 62, right: '자동 저장 14:32' };

    var screen = h('div', { className: 'as-screen' },
      h('div', { className: 'as-screen__title' }, h('span', null, (paper ? '기안동 주거지 논문.docx' : '기안동 유적 Ⅳ 고찰.docx') + ' - ARCHAEO STUDIO'), h('span', { className: 'as-screen__win' }, h('span', null, '—'), h('span', null, '❐'), h('span', null, '✕'))),
      h(StudioRibbon, null),
      h('div', { className: 'as-screen__body' },
        h('aside', { className: 'as-screen__panel', 'aria-label': '제미나이 학술 스튜디오' },
          h('div', { className: 'as-screen__steps' },
            h(StudioStep, { n: 1, title: '문서 종류' }, h(Segmented, { size: 'lg', label: '문서 종류', value: mode[0], onChange: mode[1], disabled: st === 'running', options: [{ id: 'report', label: '발굴조사보고서 고찰' }, { id: 'paper', label: '논문' }] })),
            h(StudioStep, { n: 2, title: '요청' }, h(PromptField, { rows: 2, defaultValue: paper ? '기안동 유적 주거지의 구조와 편년을 다룬 학술논문 초고를 써 줘' : '기안동 유적 원삼국~백제 한성기 주거지의 구조와 편년을 정리해 줘', showProcessed: p.showProcessed, processed: paper ? '[학술논문] 제목 · 국문초록 · 주제어 · Ⅰ. 머리말 ~ Ⅴ. 맺음말 · 참고문헌 · 서지: 저자, 연도, 「제목」, 『학술지』 권호, 쪽 · 입력되지 않은 분석 자료는 쓰지 않음' : '[발굴조사보고서 Ⅳ. 고찰] 1. 유구 구조와 편년적 위치 2. 출토유물 검토 3. 유적의 성격 · 서지: 저자, 연도, 「제목」, 『학술지』 권호, 쪽 · 입력되지 않은 분석 자료는 쓰지 않음' })),
            h(StudioStep, { n: 3, title: '내 보고서·조사 자료' }, h(SourceInput, { rows: 3, label: '내 보고서·조사 자료', placeholder: '보고서 초안, 유구·유물 설명, 수치표를 붙여 넣으세요', files: [{ name: '기안동_유구설명.docx', size: '84 KB' }, { name: '주거지_계측표.csv', size: '6 KB' }] })),
            h(StudioStep, { n: 4, title: '연대분석·보유 자료', aside: h(Tag, { tone: 'neutral' }, '선택') }, h(SourceInput, { rows: 2, label: '연대분석·보유 자료', optional: true, placeholder: 'AMS 연대, 자연과학분석, 실측치 등', note: '보유하지 않은 자료는 비워 두세요. 없는 자료는 제외하고 입력된 자료만으로 결과가 작성됩니다.' }))),
          h('div', { className: 'as-screen__dock' },
            h(StudioStep, { n: 5, title: '실행', aside: st === 'done' ? h(Tag, { tone: 'ok' }, '작성 끝 · 14:40') : null }, h(RunPanel, { mode: mode[0], state: st === 'ready' ? 'idle' : st, loggedIn: !(ready && (p.loggedIn === false || p.overlay === 'login')) })),
            h(StudioStep, { n: 6, title: '진행 기록' }, h(LogPanel, { title: false, progress: ready ? null : (st === 'done' ? 100 : 62), entries: ready ? [] : st === 'done' ? LOG_DONE : paper ? LOG_PAPER : LOG_RUNNING })))),
        h('main', { className: 'as-screen__canvas' },
          h(CollectStrip, stripProps),
          h('div', { className: 'as-screen__scroll' }, doc))),
      h(StatusBar, { hint: p.overlay === 'login' ? '디비피아 로그인 창이 열려 있습니다. 로그인이 끝나면 창을 닫으세요.' : p.overlay === 'export' ? 'PDF로 내보낼 설정을 고르세요.' : hints[st], items: [{ label: ready ? '쪽 1 / 1' : '쪽 1 / 3' }, { label: ready ? '글자 0' : '글자 ' + (st === 'done' ? '6,820' : '2,431'), mono: true }, { label: '100% ▾', mono: true }] }),
      p.overlay ? h('div', { className: 'as-screen__scrim' }, p.overlay === 'login' ? h(DbpiaWindow, { loggedIn: false }) : h(ExportDialog, null)) : null);
    return screen;
  }

  var api = {
    Icon: Icon, Button: Button, Tag: Tag, Tooltip: Tooltip, TextField: TextField, SearchField: SearchField, Chip: Chip, ChipGroup: ChipGroup,
    Segmented: Segmented, PanelHeader: PanelHeader, Menu: Menu, Dialog: Dialog, FieldRow: FieldRow,
    Ribbon: Ribbon, RibbonGroup: RibbonGroup, RibbonTile: RibbonTile, StudioRibbon: StudioRibbon, StatusBar: StatusBar, CollectStrip: CollectStrip,
    StudioStep: StudioStep, PromptField: PromptField, FileChip: FileChip, SourceInput: SourceInput, ProgressBar: ProgressBar, LogPanel: LogPanel, RunPanel: RunPanel,
    DbpiaWindow: DbpiaWindow, ExportDialog: ExportDialog,
    DocPage: DocPage, DocChapter: DocChapter, DocSection: DocSection, DocPara: DocPara, FootnoteRef: FootnoteRef, DocTable: DocTable, References: References, PaperTitle: PaperTitle,
    StudioScreen: StudioScreen,
    ICON_NAMES: Object.keys(ICONS)
  };
  window.ArchaeoStudio = Object.assign(window.ArchaeoStudio || {}, api);
})();
