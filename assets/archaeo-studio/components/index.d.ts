import type * as React from 'react';

/* ARCHAEO STUDIO — window.ArchaeoStudio. React 18이 페이지에 먼저 있어야 합니다. */

export type IconName = 'new' | 'open' | 'save' | 'text' | 'table' | 'img' | 'footnote' | 'panel' | 'export' | 'search' | 'run' | 'pause' | 'stop' | 'key';
type Option = string | { id: string; label: React.ReactNode };
export type DocMode = 'report' | 'paper';
export type RunState = 'idle' | 'running' | 'paused' | 'done';

/* 기본 */
export interface IconProps { name: IconName; size?: number; label?: string; className?: string; style?: React.CSSProperties }
export declare function Icon(props: IconProps): React.ReactElement | null;
export interface ButtonProps extends React.ButtonHTMLAttributes<HTMLButtonElement> {
  /** default: tab 바탕 · quiet: 투명 · strong: 먹색(대화상자 확정) · accent: 흙색(한 화면에 하나, 고른 실행 단추) */
  variant?: 'default' | 'quiet' | 'strong' | 'accent';
  /** md 28 · lg 36(실행 단추) */
  size?: 'md' | 'lg';
  icon?: IconName;
  block?: boolean;
}
export declare function Button(props: ButtonProps): React.ReactElement;
export interface TagProps { tone?: 'neutral' | 'new' | 'brass' | 'ok' | 'warn' | 'bad'; className?: string; children?: React.ReactNode }
export declare function Tag(props: TagProps): React.ReactElement;
export interface TooltipProps { children?: React.ReactNode }
export declare function Tooltip(props: TooltipProps): React.ReactElement;
export interface TextFieldProps extends React.InputHTMLAttributes<HTMLInputElement> { multiline?: boolean; rows?: number }
export declare function TextField(props: TextFieldProps): React.ReactElement;
export interface SearchFieldProps { placeholder?: string; shortcut?: string; value?: string; onChange?: React.ChangeEventHandler<HTMLInputElement>; className?: string }
export declare function SearchField(props: SearchFieldProps): React.ReactElement;
export interface ChipProps { on?: boolean; onClick?: () => void; className?: string; children?: React.ReactNode }
export declare function Chip(props: ChipProps): React.ReactElement;
export interface ChipGroupProps { options: Option[]; value?: string; defaultValue?: string; onChange?: (id: string) => void; label?: string }
export declare function ChipGroup(props: ChipGroupProps): React.ReactElement;
export interface SegmentedProps { options: Option[]; value?: string; defaultValue?: string; onChange?: (id: string) => void; size?: 'md' | 'lg'; disabled?: boolean; label?: string; className?: string }
export declare function Segmented(props: SegmentedProps): React.ReactElement;
export interface PanelHeaderProps { title: React.ReactNode; sub?: React.ReactNode; actions?: React.ReactNode; className?: string }
export declare function PanelHeader(props: PanelHeaderProps): React.ReactElement;
export interface MenuItem { label?: React.ReactNode; shortcut?: string; on?: boolean; disabled?: boolean; separator?: boolean }
export interface MenuProps { items: MenuItem[]; width?: number }
export declare function Menu(props: MenuProps): React.ReactElement;
export interface DialogProps { title: string; sub?: React.ReactNode; footer?: React.ReactNode; width?: number; children?: React.ReactNode }
export declare function Dialog(props: DialogProps): React.ReactElement;
export interface FieldRowProps { label: React.ReactNode; children?: React.ReactNode }
export declare function FieldRow(props: FieldRowProps): React.ReactElement;

/* 화면 틀 */
export interface RibbonProps { search?: React.ReactNode; children?: React.ReactNode }
export declare function Ribbon(props: RibbonProps): React.ReactElement;
export interface RibbonGroupProps { caption: string; children?: React.ReactNode }
export declare function RibbonGroup(props: RibbonGroupProps): React.ReactElement;
export interface RibbonTileProps { icon: IconName; label: React.ReactNode; state?: 'default' | 'selected'; dropdown?: boolean; disabled?: boolean; onClick?: () => void }
export declare function RibbonTile(props: RibbonTileProps): React.ReactElement;
export interface StudioRibbonProps { studioOpen?: boolean }
export declare function StudioRibbon(props: StudioRibbonProps): React.ReactElement;
export interface StatusBarProps { hint: React.ReactNode; error?: boolean; items?: { label: React.ReactNode; mono?: boolean }[] }
export declare function StatusBar(props: StatusBarProps): React.ReactElement;
export interface CollectStripProps { state?: 'idle' | 'running' | 'paused' | 'done'; idleText?: string; found?: number; progress?: number; footnotes?: number; refs?: number; right?: React.ReactNode }
export declare function CollectStrip(props: CollectStripProps): React.ReactElement;

/* 스튜디오 패널 */
export interface StudioStepProps { n?: number; title: string; help?: React.ReactNode; aside?: React.ReactNode; children?: React.ReactNode }
export declare function StudioStep(props: StudioStepProps): React.ReactElement;
export interface PromptFieldProps { value?: string; defaultValue?: string; onChange?: React.ChangeEventHandler<HTMLTextAreaElement>; placeholder?: string; rows?: number; processed?: React.ReactNode; showProcessed?: boolean; onToggleProcessed?: (open: boolean) => void }
export declare function PromptField(props: PromptFieldProps): React.ReactElement;
export interface FileChipProps { name: string; size?: string; onRemove?: () => void }
export declare function FileChip(props: FileChipProps): React.ReactElement;
export interface SourceInputProps { label?: string; placeholder?: string; value?: string; defaultValue?: string; onChange?: React.ChangeEventHandler<HTMLTextAreaElement>; rows?: number; files?: FileChipProps[]; onPick?: () => void; accept?: string; optional?: boolean; note?: React.ReactNode }
export declare function SourceInput(props: SourceInputProps): React.ReactElement;
export interface RunPanelProps { mode?: DocMode; state?: RunState; loggedIn?: boolean; onRun?: (mode: DocMode) => void; onPause?: () => void; onStop?: () => void; onLogin?: () => void; onExport?: () => void }
export declare function RunPanel(props: RunPanelProps): React.ReactElement;
export interface ProgressBarProps { value: number; label?: React.ReactNode; done?: boolean }
export declare function ProgressBar(props: ProgressBarProps): React.ReactElement;
export interface LogEntry { time: string; kind: 'collect' | 'write' | 'note' | 'warn' | 'error' | 'done'; text: React.ReactNode }
export interface LogPanelProps { title?: string | false; progress?: number | null; progressLabel?: string; entries?: LogEntry[]; empty?: string }
export declare function LogPanel(props: LogPanelProps): React.ReactElement;
export interface DbpiaWindowProps { loggedIn?: boolean; address?: string; onClose?: () => void; width?: number }
export declare function DbpiaWindow(props: DbpiaWindowProps): React.ReactElement;
export interface ExportDialogProps { source?: string; fileName?: string; onCancel?: () => void; onExport?: () => void; width?: number }
export declare function ExportDialog(props: ExportDialogProps): React.ReactElement;

/* A4 문서 */
export interface DocPageProps { footnotes?: { n?: number; text: React.ReactNode }[]; pageNumber?: number; empty?: string; scale?: number; className?: string; children?: React.ReactNode }
export declare function DocPage(props: DocPageProps): React.ReactElement;
export declare function DocChapter(props: { children?: React.ReactNode }): React.ReactElement;
export declare function DocSection(props: { children?: React.ReactNode }): React.ReactElement;
export declare function DocPara(props: { live?: boolean; children?: React.ReactNode }): React.ReactElement;
export declare function FootnoteRef(props: { n: number }): React.ReactElement;
export interface DocTableProps { caption?: React.ReactNode; columns: React.ReactNode[]; rows: React.ReactNode[][] }
export declare function DocTable(props: DocTableProps): React.ReactElement;
export interface ReferencesProps { title?: string; items: React.ReactNode[] }
export declare function References(props: ReferencesProps): React.ReactElement;
export interface PaperTitleProps { title: React.ReactNode; subtitle?: React.ReactNode; author?: React.ReactNode; abstract?: React.ReactNode; abstractTitle?: string; keywords?: string[] }
export declare function PaperTitle(props: PaperTitleProps): React.ReactElement;

/* 화면 한 장 (시안용) */
export interface StudioScreenProps { state?: 'ready' | 'running' | 'done'; mode?: DocMode; overlay?: 'login' | 'export'; loggedIn?: boolean; showProcessed?: boolean }
export declare function StudioScreen(props: StudioScreenProps): React.ReactElement;

declare global {
  interface Window {
    ArchaeoStudio: {
      Icon: typeof Icon; Button: typeof Button; Tag: typeof Tag; Tooltip: typeof Tooltip; TextField: typeof TextField; SearchField: typeof SearchField;
      Chip: typeof Chip; ChipGroup: typeof ChipGroup; Segmented: typeof Segmented; PanelHeader: typeof PanelHeader; Menu: typeof Menu; Dialog: typeof Dialog; FieldRow: typeof FieldRow;
      Ribbon: typeof Ribbon; RibbonGroup: typeof RibbonGroup; RibbonTile: typeof RibbonTile; StudioRibbon: typeof StudioRibbon; StatusBar: typeof StatusBar; CollectStrip: typeof CollectStrip;
      StudioStep: typeof StudioStep; PromptField: typeof PromptField; FileChip: typeof FileChip; SourceInput: typeof SourceInput; RunPanel: typeof RunPanel; ProgressBar: typeof ProgressBar;
      LogPanel: typeof LogPanel; DbpiaWindow: typeof DbpiaWindow; ExportDialog: typeof ExportDialog;
      DocPage: typeof DocPage; DocChapter: typeof DocChapter; DocSection: typeof DocSection; DocPara: typeof DocPara; FootnoteRef: typeof FootnoteRef; DocTable: typeof DocTable;
      References: typeof References; PaperTitle: typeof PaperTitle; StudioScreen: typeof StudioScreen;
      ICON_NAMES: IconName[];
    };
  }
}
