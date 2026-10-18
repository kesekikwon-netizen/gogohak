//! ARCHAEO STUDIO docked left panel inside WordCraft.
//!
//! Connects the native WordCraft editor to the local Gemini × DBpia AI bridge
//! (`http://127.0.0.1:18787`) so the user can configure Steps 1–6 on the left
//! while editing the live `.docx` manuscript on the right.

use std::io::{Read, Write};
use std::net::{SocketAddr, TcpStream};
use std::sync::mpsc::{Receiver, channel};
use std::time::Duration;

use egui::{Align2, Color32, CornerRadius, Margin, Rect, RichText, ScrollArea, Sense, Stroke, StrokeKind, Ui, pos2, vec2};
use serde_json::{Value, json};

use crate::WordApp;
use crate::theme::{medium, regular, semibold};

/// Message returned by a background HTTP call to the local bridge (`127.0.0.1:18787`).
pub enum BridgeReply {
    FirstStage { engineered: String },
    DbpiaLogin { message: String },
    DbpiaCollect { answer: String, references: String },
    Generated { docx_path: Option<String>, pdf_path: Option<String>, message: String },
    Exported { docx_path: Option<String>, pdf_path: Option<String>, message: String },
    Error(String),
}

/// State of the docked ARCHAEO STUDIO left panel.
pub struct ArchaeoStudioPane {
    pub open: bool,
    pub doc_mode: String,
    pub site_name: String,
    pub focus_topic: String,
    pub raw_data: String,
    pub dating_data: String,
    pub dbpia_queries: String,
    pub first_stage_prompt: String,
    pub dbpia_answer: String,
    pub dbpia_refs: String,
    pub max_rounds: u32,
    pub include_charts: bool,
    pub include_stratigraphy: bool,
    pub status_line: String,
    pub busy: bool,
    pub step_open: [bool; 6],
    rx: Option<Receiver<BridgeReply>>,
}

impl Default for ArchaeoStudioPane {
    fn default() -> Self {
        Self {
            open: false,
            doc_mode: "report".to_string(),
            site_name: "경산 임당동·조영동 유적".to_string(),
            focus_topic: "묘제 변천과 출토 토기·위세품을 통해 본 압독국 지배집단의 위계구조".to_string(),
            raw_data: "1. 조사개요: 경산 임당동·조영동 구릉 일대 주부식 목곽묘 18기, 수혈식 석곽묘 9기, 옹관묘 5기 조사\n2. 유구 중복관계: 조영동 EⅢ-2호 주부식 목곽묘가 선행 원삼국시대 목관묘(EⅢ-1호)를 파괴·중복하며 축조됨. 5세기 중엽 수혈식 석곽묘(EⅢ-7호)가 목곽묘 호석을 일부 절단함.\n3. 주요 출토유물: 주부식 목곽묘 주곽에서 금동관모편, 은제관식, 환두대도, 철모·철촉 다량 출토. 부곽에서 고배·장경호·파수부완·단경호 등 도질토기 다량 부장.".to_string(),
            dating_data: "조영동 EⅢ-2호 목곽묘 목탄 AMS: 1680±30 BP (보정연대 AD 330~410, 95.4%)\nEⅢ-7호 석곽묘 바닥 목탄 AMS: 1560±35 BP (보정연대 AD 420~540, 95.4%)".to_string(),
            dbpia_queries: "경산 임당동 조영동 목곽묘 위계\n영남지역 삼국시대 주부식 목곽묘 토기 편년".to_string(),
            first_stage_prompt: String::new(),
            dbpia_answer: String::new(),
            dbpia_refs: String::new(),
            max_rounds: 2,
            include_charts: true,
            include_stratigraphy: true,
            status_line: "대기 중 · Gemini '작업' 모드 연동 준비".to_string(),
            busy: false,
            step_open: [true, true, true, false, false, true],
            rx: None,
        }
    }
}

/// ARCHAEO STUDIO warm academic ink & terracotta palette (`tokens.json`).
struct StudioPalette {
    bg_pane: Color32,
    bg_card: Color32,
    bg_elev: Color32,
    border: Color32,
    ink_1: Color32,
    ink_2: Color32,
    ink_3: Color32,
    accent: Color32,
    accent_soft: Color32,
    amber: Color32,
    ok: Color32,
}

impl StudioPalette {
    fn for_theme(dark: bool) -> Self {
        if dark {
            Self {
                bg_pane: Color32::from_rgb(28, 25, 22),
                bg_card: Color32::from_rgb(35, 31, 27),
                bg_elev: Color32::from_rgb(43, 38, 33),
                border: Color32::from_rgb(62, 54, 46),
                ink_1: Color32::from_rgb(242, 236, 225),
                ink_2: Color32::from_rgb(199, 190, 174),
                ink_3: Color32::from_rgb(154, 144, 128),
                accent: Color32::from_rgb(208, 106, 76),
                accent_soft: Color32::from_rgb(66, 39, 31),
                amber: Color32::from_rgb(216, 155, 60),
                ok: Color32::from_rgb(73, 160, 111),
            }
        } else {
            Self {
                bg_pane: Color32::from_rgb(245, 241, 232),
                bg_card: Color32::from_rgb(251, 248, 242),
                bg_elev: Color32::from_rgb(238, 232, 219),
                border: Color32::from_rgb(216, 207, 190),
                ink_1: Color32::from_rgb(31, 26, 20),
                ink_2: Color32::from_rgb(74, 66, 56),
                ink_3: Color32::from_rgb(122, 112, 99),
                accent: Color32::from_rgb(158, 62, 36),
                accent_soft: Color32::from_rgb(246, 229, 221),
                amber: Color32::from_rgb(184, 116, 26),
                ok: Color32::from_rgb(31, 107, 69),
            }
        }
    }
}

/// Top-bar pill button in WordCraft's title bar to toggle the ARCHAEO STUDIO left panel.
pub fn title_bar_button(ui: &mut Ui, app: &mut WordApp) {
    let dark = app.ui_is_dark();
    let p = StudioPalette::for_theme(dark);
    let active = app.archaeo_studio.open;
    let (r, resp) = ui.allocate_exact_size(vec2(126.0, 22.0), Sense::click());
    let fill = if active || resp.hovered() { p.accent } else { p.accent_soft };
    let fg = if active || resp.hovered() { Color32::WHITE } else { p.accent };
    ui.painter().rect(
        r,
        CornerRadius::same(11),
        fill,
        Stroke::new(1.0, p.accent),
        StrokeKind::Inside,
    );
    ui.painter().text(
        pos2(r.center().x, r.center().y),
        Align2::CENTER_CENTER,
        "ARCHAEO STUDIO",
        semibold(10.5),
        fg,
    );
    if resp.on_hover_text("ARCHAEO STUDIO — 한국 고고학 발굴조사보고서 『Ⅳ. 고찰』 · 학술논문 스튜디오 패널 열기/닫기").clicked() {
        app.archaeo_studio.open = !app.archaeo_studio.open;
        app.want_system_cjk = true;
    }
}

/// Poll background responses from the local bridge and show the docked left panel.
pub fn show(app: &mut WordApp, ui: &mut Ui) {
    poll_bridge(app, ui.ctx());
    if !app.archaeo_studio.open {
        return;
    }
    app.want_system_cjk = true;
    let dark = app.ui_is_dark();
    let p = StudioPalette::for_theme(dark);

    egui::Panel::left("archaeo_studio_pane")
        .default_size(390.0)
        .size_range(330.0..=540.0)
        .resizable(true)
        .frame(
            egui::Frame::NONE
                .fill(p.bg_pane)
                .inner_margin(Margin::same(10))
                .stroke(Stroke::new(1.0, p.border)),
        )
        .show(ui, |ui| {
            // Top Studio Header
            ui.horizontal(|ui| {
                let (badge_r, _) = ui.allocate_exact_size(vec2(24.0, 24.0), Sense::hover());
                ui.painter().rect_filled(badge_r, CornerRadius::same(5), p.accent);
                ui.painter().text(badge_r.center(), Align2::CENTER_CENTER, "고", semibold(12.0), Color32::WHITE);
                ui.vertical(|ui| {
                    ui.spacing_mut().item_spacing.y = 1.0;
                    ui.label(RichText::new("ARCHAEO STUDIO").font(semibold(13.0)).color(p.ink_1));
                    ui.label(RichText::new("발굴조사보고서 Ⅳ.고찰 · 논문 워크스테이션").font(regular(10.5)).color(p.ink_3));
                });
                ui.with_layout(egui::Layout::right_to_left(egui::Align::Center), |ui| {
                    if ui.add(egui::Button::new(RichText::new("✕").font(semibold(11.0)).color(p.ink_2)).frame(false)).on_hover_text("패널 닫기").clicked() {
                        app.archaeo_studio.open = false;
                    }
                    if ui.add(egui::Button::new(RichText::new("웹 스튜디오").font(medium(10.5)).color(p.accent))).on_hover_text("브라우저에서 풀스크린 ARCHAEO STUDIO 열기 (http://127.0.0.1:18787/studio)").clicked() {
                        app.canvas.open_url = Some("http://127.0.0.1:18787/studio".to_string());
                    }
                });
            });

            ui.add_space(6.0);

            // Document Mode Segment Switch
            egui::Frame::NONE
                .fill(p.bg_elev)
                .corner_radius(CornerRadius::same(6))
                .inner_margin(Margin::same(3))
                .stroke(Stroke::new(1.0, p.border))
                .show(ui, |ui| {
                    ui.columns(2, |cols| {
                        let is_report = app.archaeo_studio.doc_mode == "report";
                        let b1 = egui::Button::new(
                            RichText::new("발굴조사보고서 (Ⅳ.고찰)")
                                .font(semibold(11.0))
                                .color(if is_report { Color32::WHITE } else { p.ink_2 }),
                        )
                        .fill(if is_report { p.accent } else { Color32::TRANSPARENT })
                        .corner_radius(CornerRadius::same(4));
                        if cols[0].add_sized([cols[0].available_width(), 24.0], b1).clicked() {
                            app.archaeo_studio.doc_mode = "report".to_string();
                        }

                        let is_paper = app.archaeo_studio.doc_mode == "paper";
                        let b2 = egui::Button::new(
                            RichText::new("학술논문 (학회지 투고)")
                                .font(semibold(11.0))
                                .color(if is_paper { Color32::WHITE } else { p.ink_2 }),
                        )
                        .fill(if is_paper { p.accent } else { Color32::TRANSPARENT })
                        .corner_radius(CornerRadius::same(4));
                        if cols[1].add_sized([cols[1].available_width(), 24.0], b2).clicked() {
                            app.archaeo_studio.doc_mode = "paper".to_string();
                        }
                    });
                });

            ui.add_space(6.0);

            // Live Status Banner
            egui::Frame::NONE
                .fill(p.accent_soft)
                .corner_radius(CornerRadius::same(5))
                .inner_margin(Margin { left: 8, right: 8, top: 5, bottom: 5 })
                .stroke(Stroke::new(1.0, p.border))
                .show(ui, |ui| {
                    ui.horizontal(|ui| {
                        let dot_col = if app.archaeo_studio.busy { p.amber } else { p.ok };
                        let (dr, _) = ui.allocate_exact_size(vec2(8.0, 8.0), Sense::hover());
                        ui.painter().circle_filled(dr.center(), 3.5, dot_col);
                        ui.label(RichText::new(&app.archaeo_studio.status_line).font(medium(11.0)).color(p.ink_1));
                    });
                });

            ui.add_space(6.0);

            // Scrollable 6-step pipeline
            ScrollArea::vertical().id_salt("archaeo_steps_scroll").show(ui, |ui| {
                step_card(ui, app, &p, 0, "01", "조사개요 · 기초자료 입력", "유적명, 유구·출토유물, 절대연대", |ui, app| {
                    ui.label(RichText::new("유적명 / 논문 대주제").font(semibold(11.0)).color(p.ink_2));
                    ui.add(egui::TextEdit::singleline(&mut app.archaeo_studio.site_name).desired_width(f32::INFINITY));
                    ui.add_space(4.0);
                    ui.label(RichText::new("핵심 고찰 주제 (쟁점 · 편년 · 성격)").font(semibold(11.0)).color(p.ink_2));
                    ui.add(egui::TextEdit::multiline(&mut app.archaeo_studio.focus_topic).desired_rows(2).desired_width(f32::INFINITY));
                    ui.add_space(4.0);
                    ui.label(RichText::new("발굴조사 유구·유물 현황 및 층위·중복관계").font(semibold(11.0)).color(p.ink_2));
                    ui.add(egui::TextEdit::multiline(&mut app.archaeo_studio.raw_data).desired_rows(5).desired_width(f32::INFINITY));
                    ui.add_space(4.0);
                    ui.label(RichText::new("자연과학분석 · AMS 절대연대 (없으면 비워둠 — 날조 금지)").font(semibold(11.0)).color(p.ink_2));
                    ui.add(egui::TextEdit::multiline(&mut app.archaeo_studio.dating_data).desired_rows(2).desired_width(f32::INFINITY));
                });

                ui.add_space(6.0);

                step_card(ui, app, &p, 1, "02", "1차 가공 · 프롬프트 엔지니어링", "고고학 전문 논증 지시문 자동 설계", |ui, app| {
                    ui.horizontal(|ui| {
                        ui.checkbox(&mut app.archaeo_studio.include_charts, RichText::new("차트·모식도 자동 생성").font(regular(11.0)).color(p.ink_1));
                        ui.checkbox(&mut app.archaeo_studio.include_stratigraphy, RichText::new("층위·중복 최우선").font(regular(11.0)).color(p.ink_1));
                    });
                    ui.add_space(4.0);
                    let btn = egui::Button::new(RichText::new("1차 가공 (고찰 프롬프트 자동 설계)").font(semibold(11.5)).color(Color32::WHITE))
                        .fill(p.accent)
                        .corner_radius(CornerRadius::same(5));
                    if ui.add_enabled(!app.archaeo_studio.busy, btn).clicked() {
                        trigger_first_stage(app, ui.ctx().clone());
                    }
                    if !app.archaeo_studio.first_stage_prompt.is_empty() {
                        ui.add_space(4.0);
                        ui.label(RichText::new("1차 가공 완료 지시문 (수정 가능)").font(semibold(10.5)).color(p.ok));
                        ui.add(egui::TextEdit::multiline(&mut app.archaeo_studio.first_stage_prompt).desired_rows(4).desired_width(f32::INFINITY));
                    }
                });

                ui.add_space(6.0);

                step_card(ui, app, &p, 2, "03", "DBpia AI 국내 학술논문 수집", "실제 학술지·발굴보고서 인용문헌 확보", |ui, app| {
                    ui.label(RichText::new("DBpia 검색 질의어 (줄바꿈 구분)").font(semibold(11.0)).color(p.ink_2));
                    ui.add(egui::TextEdit::multiline(&mut app.archaeo_studio.dbpia_queries).desired_rows(2).desired_width(f32::INFINITY));
                    ui.add_space(4.0);
                    ui.horizontal(|ui| {
                        let login_btn = egui::Button::new(RichText::new("DBpia 로그인 창").font(medium(11.0)).color(p.ink_1))
                            .fill(p.bg_elev)
                            .corner_radius(CornerRadius::same(4));
                        if ui.add_enabled(!app.archaeo_studio.busy, login_btn).clicked() {
                            trigger_dbpia_login(app, ui.ctx().clone());
                        }
                        let ask_btn = egui::Button::new(RichText::new("DBpia 선행연구 수집").font(semibold(11.0)).color(Color32::WHITE))
                            .fill(p.amber)
                            .corner_radius(CornerRadius::same(4));
                        if ui.add_enabled(!app.archaeo_studio.busy, ask_btn).clicked() {
                            trigger_dbpia_collect(app, ui.ctx().clone());
                        }
                    });
                    if !app.archaeo_studio.dbpia_refs.is_empty() {
                        ui.add_space(4.0);
                        ui.label(RichText::new("수집된 참고문헌 목록").font(semibold(10.5)).color(p.ok));
                        ui.add(egui::TextEdit::multiline(&mut app.archaeo_studio.dbpia_refs).desired_rows(3).desired_width(f32::INFINITY));
                    }
                });

                ui.add_space(6.0);

                step_card(ui, app, &p, 3, "04", "유구·유물 속성표 · 병행유적 대비표", "<표 1> · <표 2> 및 분기/계통 모식도", |ui, _app| {
                    ui.label(
                        RichText::new("• <표 1> 주요 유구·출토유물 속성 일람표 자동 편성\n• <표 2> 주변 병행유적 비교 대비표 자동 편성\n• Gemini 직접 생성 네이티브 차트(wc-chart) 및 분기·편년 모식도(wc-diagram)가 우측 WordCraft 본문에 편집 가능한 개체로 즉시 삽입됩니다.")
                            .font(regular(11.0))
                            .color(p.ink_2),
                    );
                });

                ui.add_space(6.0);

                step_card(ui, app, &p, 4, "05", "각주 1) 2) · 【참고문헌】 1:1 검증", "한국고고학보 · 문화재청 보고서 규격", |ui, _app| {
                    ui.label(
                        RichText::new("• 본문 반괄호 각주 1), 2), 3) → WordCraft 페이지 하단 진짜 각주 자동 변환\n• 『보고서/학술지』 · 「논문」 · 00~00쪽 부호 표준화\n• 본문 인용 각주와 말미 【참고문헌】 1:1 누락 검증")
                            .font(regular(11.0))
                            .color(p.ink_2),
                    );
                });

                ui.add_space(6.0);

                step_card(ui, app, &p, 5, "06", "Gemini '작업' 모드 집필 · 내보내기", "WordCraft(.docx) 직결 · 필요시 PDF 변환", |ui, app| {
                    ui.horizontal(|ui| {
                        ui.label(RichText::new("심층 고찰 라운드 수:").font(medium(11.0)).color(p.ink_2));
                        for r in [1_u32, 2, 3] {
                            if ui.selectable_label(app.archaeo_studio.max_rounds == r, format!("{r}회")).clicked() {
                                app.archaeo_studio.max_rounds = r;
                            }
                        }
                    });
                    ui.add_space(6.0);
                    let run_label = if app.archaeo_studio.doc_mode == "paper" {
                        "✨ 학술논문 전체 집필 → 우측 WordCraft 반영"
                    } else {
                        "✨ 보고서 『Ⅳ. 고찰』 집필 → 우측 WordCraft 반영"
                    };
                    let run_btn = egui::Button::new(RichText::new(run_label).font(semibold(12.0)).color(Color32::WHITE))
                        .fill(p.accent)
                        .corner_radius(CornerRadius::same(6));
                    if ui.add_sized([ui.available_width(), 32.0], run_btn).clicked() && !app.archaeo_studio.busy {
                        trigger_full_generation(app, ui.ctx().clone());
                    }

                    ui.add_space(6.0);
                    ui.columns(2, |cols| {
                        let save_btn = egui::Button::new(RichText::new("Word(.docx) 저장").font(semibold(11.0)).color(p.ink_1))
                            .fill(p.bg_elev)
                            .corner_radius(CornerRadius::same(5));
                        if cols[0].add_sized([cols[0].available_width(), 26.0], save_btn).clicked() {
                            let _ = app.run("file.save", json!({}));
                        }
                        let pdf_btn = egui::Button::new(RichText::new("PDF로 내보내기").font(semibold(11.0)).color(p.accent))
                            .fill(p.accent_soft)
                            .corner_radius(CornerRadius::same(5));
                        if cols[1].add_sized([cols[1].available_width(), 26.0], pdf_btn).clicked() {
                            let _ = app.run("file.exportPdf", json!({}));
                        }
                    });
                });
            });
        });
}

fn step_card(
    ui: &mut Ui,
    app: &mut WordApp,
    p: &StudioPalette,
    idx: usize,
    num: &str,
    title: &str,
    subtitle: &str,
    body: impl FnOnce(&mut Ui, &mut WordApp),
) {
    let mut is_open = app.archaeo_studio.step_open.get(idx).copied().unwrap_or(true);
    egui::Frame::NONE
        .fill(p.bg_card)
        .corner_radius(CornerRadius::same(6))
        .inner_margin(Margin::same(8))
        .stroke(Stroke::new(1.0, p.border))
        .show(ui, |ui| {
            let (hdr_r, hdr_resp) = ui.allocate_exact_size(vec2(ui.available_width(), 28.0), Sense::click());
            if hdr_resp.clicked() {
                is_open = !is_open;
                if let Some(slot) = app.archaeo_studio.step_open.get_mut(idx) {
                    *slot = is_open;
                }
            }
            let badge_r = Rect::from_min_size(hdr_r.min, vec2(22.0, 22.0));
            ui.painter().rect_filled(badge_r, CornerRadius::same(4), p.accent_soft);
            ui.painter().text(badge_r.center(), Align2::CENTER_CENTER, num, semibold(10.5), p.accent);
            ui.painter().text(
                pos2(badge_r.max.x + 8.0, hdr_r.min.y + 7.0),
                Align2::LEFT_CENTER,
                title,
                semibold(11.5),
                p.ink_1,
            );
            ui.painter().text(
                pos2(badge_r.max.x + 8.0, hdr_r.min.y + 21.0),
                Align2::LEFT_CENTER,
                subtitle,
                regular(10.0),
                p.ink_3,
            );
            let arrow = if is_open { "▾" } else { "▸" };
            ui.painter().text(
                pos2(hdr_r.max.x - 4.0, hdr_r.center().y),
                Align2::RIGHT_CENTER,
                arrow,
                semibold(11.0),
                p.ink_3,
            );
            if is_open {
                ui.add_space(6.0);
                ui.separator();
                ui.add_space(4.0);
                body(ui, app);
            }
        });
}

fn poll_bridge(app: &mut WordApp, ctx: &egui::Context) {
    let Some(rx) = &app.archaeo_studio.rx else { return };
    let Ok(reply) = rx.try_recv() else { return };
    app.archaeo_studio.busy = false;
    app.archaeo_studio.rx = None;
    match reply {
        BridgeReply::FirstStage { engineered } => {
            app.archaeo_studio.first_stage_prompt = engineered;
            app.archaeo_studio.status_line = "1차 가공 프롬프트 설계 완료".to_string();
            app.archaeo_studio.step_open[1] = true;
        }
        BridgeReply::DbpiaLogin { message } => {
            app.archaeo_studio.status_line = message;
        }
        BridgeReply::DbpiaCollect { answer, references } => {
            app.archaeo_studio.dbpia_answer = answer;
            app.archaeo_studio.dbpia_refs = references;
            app.archaeo_studio.status_line = "DBpia 국내 선행연구 및 참고문헌 수집 완료".to_string();
        }
        BridgeReply::Generated { docx_path, pdf_path: _, message }
        | BridgeReply::Exported { docx_path, pdf_path: _, message } => {
            app.archaeo_studio.status_line = message.clone();
            if let Some(path) = docx_path
                && !path.is_empty()
                && std::path::Path::new(&path).exists()
            {
                let _ = app.execute("file.open", json!({ "path": path }));
                app.status(format!("ARCHAEO STUDIO 원고 반영 완료: {path}"));
            }
        }
        BridgeReply::Error(err) => {
            app.archaeo_studio.status_line = format!("오류: {err}");
        }
    }
    ctx.request_repaint();
}

fn build_combined_topic(app: &WordApp) -> String {
    let mut parts = Vec::new();
    parts.push(format!("[유적명/대주제] {}", app.archaeo_studio.site_name.trim()));
    parts.push(format!("[핵심 고찰 쟁점] {}", app.archaeo_studio.focus_topic.trim()));
    if !app.archaeo_studio.raw_data.trim().is_empty() {
        parts.push(format!("[발굴조사 유구·유물 및 중복·층위 현황]\n{}", app.archaeo_studio.raw_data.trim()));
    }
    if !app.archaeo_studio.dating_data.trim().is_empty() {
        parts.push(format!("[자연과학분석·절대연대 자료]\n{}", app.archaeo_studio.dating_data.trim()));
    } else {
        parts.push("[자연과학분석·절대연대 자료] 해당 자료 없음 (절대 임의 수치 날조 금지 — 없는 자료는 없는 대로 상대편년·형식학으로만 논증할 것)".to_string());
    }
    parts.join("\n\n")
}

fn trigger_first_stage(app: &mut WordApp, ctx: egui::Context) {
    let topic = build_combined_topic(app);
    let doc_mode = app.archaeo_studio.doc_mode.clone();
    let payload = json!({
        "topic": topic,
        "docMode": doc_mode,
        "includeCharts": app.archaeo_studio.include_charts,
        "includeStratigraphy": app.archaeo_studio.include_stratigraphy,
    });
    app.archaeo_studio.busy = true;
    app.archaeo_studio.status_line = "1차 가공 프롬프트 엔지니어링 생성 중…".to_string();
    let (tx, rx) = channel();
    app.archaeo_studio.rx = Some(rx);
    std::thread::spawn(move || {
        let res = post_json("/first-stage-prompt", &payload, Duration::from_secs(30));
        let reply = match res {
            Ok(v) => {
                let engineered = v
                    .get("engineeredPrompt")
                    .or_else(|| v.get("prompt"))
                    .and_then(Value::as_str)
                    .unwrap_or("")
                    .to_string();
                BridgeReply::FirstStage { engineered }
            }
            Err(e) => BridgeReply::Error(e),
        };
        let _ = tx.send(reply);
        ctx.request_repaint();
    });
}

fn trigger_dbpia_login(app: &mut WordApp, ctx: egui::Context) {
    app.archaeo_studio.busy = true;
    app.archaeo_studio.status_line = "DBpia 로그인 창 호출 중…".to_string();
    let (tx, rx) = channel();
    app.archaeo_studio.rx = Some(rx);
    std::thread::spawn(move || {
        let res = post_json("/dbpia/login", &json!({}), Duration::from_secs(20));
        let reply = match res {
            Ok(_) => BridgeReply::DbpiaLogin { message: "DBpia 창이 열렸습니다. 로그인 후 선행연구 수집을 누르세요.".to_string() },
            Err(e) => BridgeReply::Error(e),
        };
        let _ = tx.send(reply);
        ctx.request_repaint();
    });
}

fn trigger_dbpia_collect(app: &mut WordApp, ctx: egui::Context) {
    let first_query = app
        .archaeo_studio
        .dbpia_queries
        .lines()
        .map(str::trim)
        .find(|l| !l.is_empty())
        .unwrap_or(&app.archaeo_studio.site_name)
        .to_string();
    let payload = json!({ "prompt": first_query });
    app.archaeo_studio.busy = true;
    app.archaeo_studio.status_line = format!("DBpia AI 검색 중: {first_query}…");
    let (tx, rx) = channel();
    app.archaeo_studio.rx = Some(rx);
    std::thread::spawn(move || {
        let res = post_json("/dbpia/ask", &payload, Duration::from_secs(120));
        let reply = match res {
            Ok(v) => {
                let answer = v.get("answer").and_then(Value::as_str).unwrap_or("").to_string();
                let refs = v
                    .get("references")
                    .and_then(Value::as_array)
                    .map(|arr| {
                        arr.iter()
                            .filter_map(|item| {
                                let t = item.get("title").and_then(Value::as_str).unwrap_or("");
                                let a = item.get("authors").and_then(Value::as_str).unwrap_or("");
                                if t.is_empty() { None } else { Some(format!("• {a} — {t}")) }
                            })
                            .collect::<Vec<_>>()
                            .join("\n")
                    })
                    .unwrap_or_default();
                BridgeReply::DbpiaCollect { answer, references: refs }
            }
            Err(e) => BridgeReply::Error(e),
        };
        let _ = tx.send(reply);
        ctx.request_repaint();
    });
}

fn trigger_full_generation(app: &mut WordApp, ctx: egui::Context) {
    let topic = build_combined_topic(app);
    let queries: Vec<String> = app
        .archaeo_studio
        .dbpia_queries
        .lines()
        .map(str::trim)
        .filter(|l| !l.is_empty())
        .map(str::to_string)
        .collect();
    let payload = json!({
        "topic": topic,
        "docMode": app.archaeo_studio.doc_mode,
        "firstStagePrompt": app.archaeo_studio.first_stage_prompt,
        "dbpiaQueries": queries,
        "maxRounds": app.archaeo_studio.max_rounds,
        "exportFormat": "docx",
        "openApps": false,
    });
    app.archaeo_studio.busy = true;
    app.archaeo_studio.status_line = "Gemini '작업' 모드 × DBpia 고찰 원고 집필 및 WordCraft(.docx) 조판 중…".to_string();
    let (tx, rx) = channel();
    app.archaeo_studio.rx = Some(rx);
    std::thread::spawn(move || {
        let res = post_json("/archaeology/discuss", &payload, Duration::from_secs(600));
        let reply = match res {
            Ok(v) => {
                let docx_path = v
                    .get("wordcraft")
                    .and_then(|w| w.get("docxPath"))
                    .and_then(Value::as_str)
                    .map(str::to_string);
                let pdf_path = v
                    .get("wordcraft")
                    .and_then(|w| w.get("pdfPath"))
                    .and_then(Value::as_str)
                    .map(str::to_string);
                BridgeReply::Generated {
                    docx_path,
                    pdf_path,
                    message: "집필 완료 · 우측 WordCraft 캔버스에 원고·표·각주·차트가 반영되었습니다.".to_string(),
                }
            }
            Err(e) => BridgeReply::Error(e),
        };
        let _ = tx.send(reply);
        ctx.request_repaint();
    });
}

fn post_json(path: &str, body: &Value, timeout: Duration) -> Result<Value, String> {
    let payload = serde_json::to_string(body).map_err(|e| e.to_string())?;
    let addr = SocketAddr::from(([127, 0, 0, 1], 18787));
    let mut stream = TcpStream::connect_timeout(&addr, Duration::from_secs(3))
        .map_err(|_| "ARCHAEO 브리지(127.0.0.1:18787)에 연결할 수 없습니다. 브리지가 실행 중인지 확인하세요.".to_string())?;
    let _ = stream.set_read_timeout(Some(timeout));
    let _ = stream.set_write_timeout(Some(Duration::from_secs(10)));
    let req = format!(
        "POST {path} HTTP/1.1\r\nHost: 127.0.0.1:18787\r\nContent-Type: application/json; charset=utf-8\r\nContent-Length: {}\r\nConnection: close\r\n\r\n{}",
        payload.len(),
        payload
    );
    stream.write_all(req.as_bytes()).map_err(|e| e.to_string())?;
    let mut buf = Vec::new();
    stream.read_to_end(&mut buf).map_err(|e| e.to_string())?;
    let raw = String::from_utf8_lossy(&buf);
    let json_part = raw.split("\r\n\r\n").nth(1).unwrap_or(&raw).trim();
    // Handle chunked or plain body by locating first '{' and last '}'
    let start = json_part.find('{').ok_or_else(|| "유효한 JSON 응답이 아닙니다.".to_string())?;
    let end = json_part.rfind('}').ok_or_else(|| "유효한 JSON 응답이 아닙니다.".to_string())?;
    let slice = json_part.get(start..=end).ok_or_else(|| "JSON 슬라이스 오류".to_string())?;
    let parsed: Value = serde_json::from_str(slice).map_err(|e| e.to_string())?;
    if parsed.get("ok").and_then(Value::as_bool) == Some(false) {
        let err = parsed.get("error").and_then(Value::as_str).unwrap_or("요청 실패");
        return Err(err.to_string());
    }
    Ok(parsed)
}
