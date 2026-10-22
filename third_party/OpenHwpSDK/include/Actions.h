#pragma once
 
#define ACTIONGROUP01 0
#define ACTIONGROUP02 4000
#define ACTIONGROUP03 8000
 
namespace HWPSDK {
    /**
    * @file ations.h
    * @brief HwpSDK Actions
    *
    * 한글 SDK에서 사용 가능한 Action(기능) 설명
    */
 
    /**
    * @defgroup action_list
    * @{
    */
 
    /**
    * @enum action list
    */
    enum ActionGroup
    {
        Print,/**< 인쇄 */
        FileSaveAsImage,/**< 그림 형식으로 저장하기 */
        FileSaveAsImageOption,/**< 그림 형식으로 저장할 때 옵션 설정하기 */
        FileSaveAsPdfOption,/**< PDF 파일 저장 옵션 */
        PrintToImage,/**< 그림으로 저장하기 */
        MoveLeft,/**< 캐럿을 (논리적 개념의) 왼쪽으로 이동 */
        MoveRight,/**< 캐럿을 (논리적 개념의) 오른쪽으로 이동 */
        MoveUp,/**< 캐럿을 (논리적 개념의) 위로 이동 */
        MoveDown,/**< 캐럿을 (논리적 개념의) 아래로 이동 */
        MoveSelLeft,/**< 셀렉션: 캐럿을 (논리적 방향) 왼쪽으로 이동 */
        MoveSelRight,/**< 셀렉션: 캐럿을 (논리적 방향) 오른쪽으로 이동 */
        MoveSelUp,/**< 셀렉션: 캐럿을 (논리적 방향) 위로 이동 */
        MoveSelDown,/**< 셀렉션: 캐럿을 (논리적 방향) 아래로 이동 */
        MovePrevChar,/**< 한 글자 앞 이동. 현재 리스트만을 대상으로 동작 */
        MoveNextChar,/**< 한 글자 뒤로 이동. 현재 리스트만을 대상으로 동작 */
        MovePrevPos,/**< 한 글자 앞으로 이동. 하위 목록으로 이동 가능 */
        MoveNextPos,/**< 한 글자 뒤로 이동. 하위 목록으로 이동 가능 */
        MovePrevPosEx,/**< 한 글자 앞으로 이동. 하위 목록으로 이동 가능(머리말, 꼬리말, 각주, 미주, 글상자 포함) */
        MoveNextPosEx,/**< 한 글자 뒤로 이동. 하위 목록으로 이동 가능(머리말, 꼬리말, 각주, 미주, 글상자 포함) */
        MovePrevWord,/**< 한 단어 앞으로 이동. 현재 목록만 대상으로 동작 */
        MoveNextWord,/**< 한 단어 뒤로 이동. 현재 목록만 대상으로 동작 */
        MoveWordBegin,/**< 현재 위치한 단어의 시작으로 이동. 현재 목록만 대상으로 동작 */
        MoveWordEnd,/**< 현재 위치한 단어의 끝으로 이동. 현재 목록만 대상으로 동작 */
        MoveDocBegin,/**< 문서의 시작으로 이동 */
        MoveDocEnd,/**< 문서의 끝으로 이동 */
        MoveLineBegin,/**< 현재 위치한 줄의 시작/끝으로 이동 */
        MoveLineEnd,/**< 현재 위치한 줄의 시작/끝으로 이동 */
        MoveParaBegin,/**< 현재 위치한 문단의 시작/끝으로 이동 */
        MoveParaEnd,/**< 현재 위치한 문단의 시작/끝으로 이동 */
        MovePageUp,/**< 뒤 페이지의 시작으로 이동. 현재 최상위 목록이 아니면 최상위 목록으로 이동 */
        MovePageDown,/**< 앞 페이지의 시작으로 이동. 현재 최상위 목록이 아니면 최상위 목록으로 이동 */
        MovePageBegin,/**< 현재 페이지의 시작점으로 이동 */
        MovePageEnd,/**< 현재 페이지의 끝점으로 이동 */
        MoveListBegin,/**< 현재 목록의 시작으로 이동 */
        MoveListEnd,/**< 현재 목록의 끝으로 이동 */
        MoveTopLevelBegin,/**< 최상위 목록의 시작으로 이동 */
        MoveTopLevelEnd,/**< 최상위 목록의 끝으로 이동 */
        MovePrevParaEnd,/**< 앞 문단의 끝으로 이동. 현재 목록만 대상으로 동작 */
        MoveNextParaBegin,/**< 다음 문단의 시작으로 이동. 현재 목록만 대상으로 동작 */
        MovePrevParaBegin,/**< 앞 문단의 시작으로 이동. 현재 목록만 대상으로 동작 */
        MoveSectionUp,/**< 앞 섹션으로 이동. 현재 루트 목록이 아니면 루트 목록으로 이동 */
        MoveSectionDown,/**< 뒤 섹션으로 이동. 현재 루트 목록이 아니면 루트 목록으로 이동 */
        MoveParentList,/**< 한 레벨 상위/최상위/루트 목록으로 이동 */
        MoveTopLevelList,/**< 한 레벨 상위/최상위/루트 목록으로 이동 */
        MoveRootList,/**< 한 레벨 상위/최상위/루트 목록으로 이동 */
        MovePrevList,/**< 이전 목록으로 이동 */
        MoveNextList,/**< 다음 목록으로 이동 */
        MoveSelPrevChar,/**< 셀렉션: 이전 글자 */
        MoveSelNextChar,/**< 셀렉션: 다음 글자 */
        MoveSelPrevPos,/**< 셀렉션: 이전 위치 */
        MoveSelNextPos,/**< 셀렉션: 다음 위치 */
        MoveSelPrevWord,/**< 셀렉션: 이전 단어 */
        MoveSelNextWord,/**< 셀렉션: 다음 단어 */
        MoveSelWordBegin,/**< 셀렉션: 단어 처음 */
        MoveSelWordEnd,/**< 셀렉션: 단어 끝 */
        MoveSelDocBegin,/**< 셀렉션: 문서 처음 */
        MoveSelDocEnd,/**< 셀렉션: 문서 끝 */
        MoveSelLineBegin,/**< 셀렉션: 줄 처음 */
        MoveSelLineEnd,/**< 셀렉션: 줄 끝 */
        MoveSelParaBegin,/**< 셀렉션: 문단 처음 */
        MoveSelParaEnd,/**< 셀렉션: 문단 끝 */
        MoveSelPageUp,/**< 셀렉션: 페이지 위로 */
        MoveSelPageDown,/**< 셀렉션: 페이지 아래로 */
        MoveSelListBegin,/**< 셀렉션: 목록 처음 */
        MoveSelListEnd,/**< 셀렉션: 목록 끝 */
        MoveSelTopLevelBegin,/**< 셀렉션: 처음 */
        MoveSelTopLevelEnd,/**< 셀렉션: 끝 */
        MoveSelPrevParaEnd,/**< 셀렉션: 이전 문단 끝 */
        MoveSelNextParaBegin,/**< 셀렉션: 다음 문단 처음 */
        MoveSelPrevParaBegin,/**< 셀렉션: 이전 문단 시작 */
        MoveLineUp,/**< 한 줄 위로 이동 */
        MoveLineDown,/**< 한 줄 아래로 이동 */
        MoveViewUp,/**< 현재 뷰의 크기만큼 위로 이동. PgUp 키의 기능임 */
        MoveViewDown,/**< 현재 뷰의 크기만큼 아래로 이동. PgDn 키의 기능임 */
        MoveViewBegin,/**< 현재 뷰의 시작에 위치한 곳으로 이동 */
        MoveViewEnd,/**< 현재 뷰의 끝에 위치한 곳으로 이동 */
        MoveColumnBegin,/**< 단의 시작점으로 이동. 단이 없을 경우에는 동작하지 않으며 해당 목록 안에서만 동작 */
        MoveColumnEnd,/**< 단의 끝점으로 이동. 단이 없을 경우에는 동작하지 않으며 해당 목록 안에서만 동작 */
        MoveScrollPrev,/**< 이전 방향으로 스크롤하면서 이동 */
        MoveScrollNext,/**< 다음 방향으로 스크롤하면서 이동 */
        MoveScrollUp,/**< 위 방향으로 스크롤하면서 이동 */
        MoveScrollDown,/**< 아래 방향으로 스크롤하면서 이동 */
        MoveSelLineUp,/**< 셀렉션: 한 줄 위 */
        MoveSelLineDown,/**< 셀렉션: 한 줄 아래 */
        MoveSelViewUp,/**< 셀렉션: 위 */
        MoveSelViewDown,/**< 셀렉션: 아래 */
        Close,/**< 현재 목록을 닫고 상위 목록으로 이동 */
        CloseEx,/**< 현재 목록을 닫고 상위 목록으로 이동 */
        MovePrevColumn,/**<  앞 단으로 이동 */
        MoveNextColumn,/**< 뒤 단으로 이동 */
        MoveCaretBack,/**< 캐럿 위치 이동 */
        MoveCaretPrev,/**< 캐럿 위치 이동 */
        SelectAll,/**< 모두 선택 */
        Select,/**< 선택(F3 Key를 누른 효과) */
        SelectColumn,/**< 칸 블록 선택(F4 Key를 누른 효과) */
        Cancel,/**< Esc */
        DocSummaryInfo,/**< 문서 정보 */
        Copy,/**< 복사하기 */
        Cut,/**< 오려두기 */
        FindDlg,/**< 찾기 */
        FindAll,/**< 모두 찾기 */
        RepeatFind,/**< 다시 찾기 */
        ReverseFind,/**< 거꾸로 찾기 */
        ForwardFind,/**< 앞으로 찾기 */
        BackwardFind,/**< 뒤로 찾기 */
        DocFindInit,/**< 문서 찾기 초기화 */
        DocFindNext,/**< 문서 찾기 계속 */
        DocFindEnd,/**< 문서 찾기 종료 */
        Goto,/**< 찾아가기 */
        FindForeBackFind,/**< 앞뒤로 찾아가기: 찾기 */
        FindForeBackPage,/**< 앞뒤로 찾아가기: 페이지 */
        FindForeBackSection,/**< 앞뒤로 찾아가기: 구역 */
        FindForeBackLine,/**< 앞뒤로 찾아가기: 줄 */
        FindForeBackStyle,/**< 앞뒤로 찾아가기: 스타일 */
        FindForeBackCtrl,/**< 앞뒤로 찾아가기: 조판 부호 */
        FindForeBackBookmark,/**< 앞뒤로 찾아가기  책갈피 */
        HyperlinkJump,/**< 하이퍼링크 이동 */
        ViewOptionPaper,/**< 쪽 윤곽 보기 */
        ViewOptionGuideLine,/**< 안내선 */
        ViewOptionPicture,/**< 그림 보이기/숨기기([보기-그림] 메뉴와 동일) */
        ViewOptionMemo,/**< 메모 보이기/숨기기([보기-메모-메모 보이기/숨기기] 메뉴와 동일) */
        ViewOptionMemoGuideline,/**< 메모 안내선 표시([보기-메모-메모 안내선 표시] 메뉴와 동일) */
        ViewOptionRevision,/**< 교정 부호 보이기/숨기기([보기-교정 부호] 메뉴와 동일) */
        ViewOptionParaMark,/**< 문단 부호 */
        ViewOptionCtrlMark,/**< 조판 부호 */
        ViewOptionGray,/**< 회색조 보기 */
        ViewOptionColor,/**< 컬러로 보기(회색조 보기 되돌리기 액션) */
        ViewOptionColorCustom,/**< 사용자 색 보기 */
        ViewOptionColorCustomOption,/**< 사용자 색 설정 */
        ViewZoom,/**< 화면 확대(Ribbon) */
        ViewZoomNormal,/**< 화면 확대: 정상 */
        ViewZoomFitPage,/**< 화면 확대: 페이지에 맞춤 */
        ViewZoomFitWidth,/**< 화면 확대: 폭에 맞춤 */
        ShapeObjGuideLine,/**< 그리기 개체 안내선 */
        ShapeObjShowGuideLine,/**< 그리기 개체 안내선 */
        ShapeObjShowGuideLineBase,/**< 그리기 안내선 */
        ViewGridOption,/**< 격자 */
        ViewShowGrid,/**< 격자 보이기 */
        DocumentInfo,/**< 현재 문서에 대한 정보 */
        LinkDocument,/**< 문서 연결([파일-문서 연결] 메뉴와 동일) */
        ASaveBlockAction,/**< 블록 영역 저장 */
        FileDiffResultFile,/**< 문서를 비교하여 저장 */
        Undo,/**< 되돌리기 */
        Redo,/**< 다시 실행 */
        Paste,/**< 붙이기 */
        PasteExceptField,/**< 붙이기 특정 필드 제외 */
        Delete,/**< Delete */
        DeleteBack,/**< Backspace */
        DeleteLine,/**< Ctrl-Y(한줄 지우기) */
        DeleteLineEnd,/**< Alt-Y(현재 커서에서 줄 끝까지 지우기) */
        DeleteWord,/**< 단어 지우기(Ctrl-T) */
        DeleteWordBack,/**< Ctrl-BS(Back Space) */
        ToggleOverwrite,/**< 덮어쓰기 설정/해제 */
        DeleteField,/**< 누름틀/메모 지우기 */
        CancelSelection,/**< 선택 취소 */
        EditParaUp,/**< 문단 위로 이동 */
        EditParaDown,/**< 문단 아래로 이동 */
        DeleteCtrls,/**< 조판 부호 지우기 */
        ExecReplace,/**< 바꾸기(실행) */
        AllReplace,/**< 모두 바꾸기 */
        ShapeCopyPaste,/**< 모양 복사 */
        ShapeCopyPasteClear,/**< 모양 복사 지우기 */
        ReturnPrevPos,/**< 직전 위치로 돌아가기 */
        SelectCtrlReverse,/**< 개체 선택 역방향 */
        SelectCtrlFront,/**< 개체 선택 정방향 */
        ModifySecTextVertAll,/**< 세로 쓰기(영문 세움) */
        ModifySecTextVert,/**< 세로 쓰기(영문 눕힘) */
        ModifySecTextHorz,/**< 가로 쓰기 */
        DutmalChars,/**< 덧말 넣기 */
        DeleteDutmal,/**< 덧말 지우기 */
        ModifyDutmal,/**< 고치기 - 덧말 */
        ComposeChars,/**< 글자 겹침 */
        ComposeCharsEdit,/**< 글자 겹치기 테두리 변환 */
        ModifyComposeChars,/**< 고치기 - 글자 겹침 */
        TableLeftCell,/**< 셀 이동: 셀 왼쪽 */
        TableRightCell,/**< 셀 이동: 셀 오른쪽 */
        TableRightCellAppend,/**< 셀 이동: 셀 오른쪽에 이어서 */
        TableUpperCell,/**< 셀 이동: 셀 위 */
        TableLowerCell,/**< 셀 이동: 셀 아래 */
        TableColBegin,/**< 셀 이동: 열 시작 */
        TableColEnd,/**< 셀 이동: 열 끝 */
        TableColPageUp,/**< 셀 이동: 페이지 위로 */
        TableColPageDown,/**< 셀 이동: 페이지 아래로 */
        TableResizeLeft,/**< 셀 크기 변경 */
        TableResizeRight,/**< 셀 크기 변경 */
        TableResizeUp,/**< 셀 크기 변경 */
        TableResizeDown,/**< 셀 크기 변경 */
        TableResizeCellLeft,/**< 셀 크기 변경: 셀 왼쪽 */
        TableResizeCellRight,/**< 셀 크기 변경: 셀 오른쪽 */
        TableResizeCellUp,/**< 셀 크기 변경: 셀 위 */
        TableResizeCellDown,/**< 셀 크기 변경: 셀 아래 */
        TableResizeLineLeft,/**< 셀 크기 변경: 선 왼쪽 */
        TableResizeLineRight,/**< 셀 크기 변경: 선 오른쪽 */
        TableResizeLineUp,/**< 셀 크기 변경: 선 위 */
        TableResizeLineDown,/**< 셀 크기 변경: 선 아래 */
        CharShape,/**< 글자 모양 */
        CharShapeNormal,/**< 보통 모양 Alt+Shift+C */
        CharShapeBold,/**< 진하게 Alt+L */
        CharShapeItalic,/**< 기울임 Alt+Shift+I */
        CharShapeUnderline,/**< 밑줄 Alt+Shift+U */
        CharShapeCenterline,/**< 취소선(CenterLine) */
        CharShapeOutline,/**< 외곽선 */
        CharShapeShadow,/**< 그림자 */
        CharShapeEmboss,/**< 양각 */
        CharShapeEngrave,/**< 음각 */
        CharShapeSuperscript,/**< 위 첨자 Alt+Shift+P */
        CharShapeSubscript,/**< 아래 첨자 Alt+Shift+S */
        CharShapeSuperSubscript,/**< "위 첨자 -> 아래 첨자 -> 보통" 반복 */
        CharShapeHeightIncrease,/**< 크기 크게 Alt+Shift+E */
        CharShapeHeightDecrease,/**< 크기 작게 Alt+Shift+R */
        CharShapeWidthIncrease,/**< 장평 넓게 Alt+Shift+K */
        CharShapeWidthDecrease,/**< 장평 좁게 Alt+Shift+J */
        CharShapeSpacingIncrease,/**< 자간 넓게 Alt+Shift+W */
        CharShapeSpacingDecrease,/**< 자간 좁게 Alt+Shift+N */
        CharShapeNextFaceName,/**< 다음 글꼴 Alt+Shift+F */
        CharShapePrevFaceName,/**< 이전 글꼴 Alt+Shift+G */
        ParagraphShape,/**< 문단 모양 */
        ParagraphShapeAlignJustify,/**< 양쪽 정렬 */
        ParagraphShapeAlignLeft,/**< 왼쪽 정렬 */
        ParagraphShapeAlignRight,/**< 오른쪽 정렬 */
        ParagraphShapeAlignCenter,/**< 가운데 정렬 */
        ParagraphShapeAlignDistribute,/**< 배분 정렬 */
        ParagraphShapeAlignDivision,/**< 나눔 정렬 */
        ParagraphShapeWithNext,/**< 다음 문단과 함께 */
        ParagraphShapeProtect,/**< 문단 보호 */
        ParagraphShapeIncreaseLeftMargin,/**< 왼쪽 여백 늘리기 */
        ParagraphShapeDecreaseLeftMargin,/**< 왼쪽 여백 줄이기 */
        ParagraphShapeDecreaseRightMargin,/**< 오른쪽 여백 줄이기 */
        ParagraphShapeIncreaseRightMargin,/**< 오른쪽 여백 늘리기 */
        ParagraphShapeIndentNegative,/**< 첫 줄을 한 글자 내어 씀 */
        ParagraphShapeIndentPositive,/**< 첫 줄을 한 글자 들여 씀 */
        ParagraphShapeDecreaseLineSpacing,/**< 줄 간격을 점점 좁힘 */
        ParagraphShapeIncreaseLineSpacing,/**< 줄 간격을 점점 넓힘 */
        ParagraphShapeIncreaseMargin,/**< 왼쪽-오른쪽 여백 늘리기 */
        ParagraphShapeDecreaseMargin,/**< 왼쪽-오른쪽 여백 줄이기 */
        ParagraphShapeSingleRow,/**< 한 줄로 입력 */
        CharShapeTextColorBlack,/**< 글자색을 검정 */
        CharShapeTextColorRed,/**< 글자색을 빨강 */
        CharShapeTextColorBlue,/**< 글자색을 파랑 */
        CharShapeTextColorViolet,/**< 글자색을 자주 */
        CharShapeTextColorGreen,/**< 글자색을 초록 */
        CharShapeTextColorYellow,/**< 글자색을 노랑 */
        CharShapeTextColorBluish,/**< 글자색을 청록 */
        CharShapeTextColorWhite,/**< 글자색을 흰색 */
        MarkPenShape,/**< 형광펜. 선택된 Text 영역의 배경을 형광펜으로 표시함 */
        MarkPenNext,/**< 형광펜 다음으로 이동 */
        MarkPenPrev,/**< 형광펜 이전으로 이동 */
        MarkPenDelete,/**< 형광펜 삭제 */
        ParaNumberBullet,/**< 문단 번호/글머리표 */
        OutlineNumber,/**< 개요 번호 */
        ParaNumberBulletLevelUp,/**< 문단 번호/글머리표 한 수준 위로 */
        ParaNumberBulletLevelDown,/**< 문단 번호/글머리표 한 수준 아래로 */
        PutParaNumber,/**< 문단 번호 달기 */
        PutNewParaNumber,/**< 새 번호로 문단 번호 시작 */
        PutBullet,/**< 글머리표 적용 */
        PutOutlineNumber,/**< 개요 번호 적용 */
        GetDefaultParaNumber,/**< 문단 번호 기본 값 */
        GetDefaultBullet,/**< 글머리표 기본 값 */
        DropCap,/**< 문단 첫 글자 장식 */
        StyleEx,/**< 스타일(한글 2007) */
        StyleDelete,/**< 스타일 제거 */
        Style,/**< 스타일(한글 2005 이하 버전) */
        StyleShortcut1,/**< 스타일 단축키 <Ctrl + 0> */
        StyleShortcut2,/**< 스타일 단축키 <Ctrl + 2> */
        StyleShortcut3,/**< 스타일 단축키 <Ctrl + 3> */
        StyleShortcut4,/**< 스타일 단축키 <Ctrl + 4> */
        StyleShortcut5,/**< 스타일 단축키 <Ctrl + 5> */
        StyleShortcut6,/**< 스타일 단축키 <Ctrl + 6> */
        StyleShortcut7,/**< 스타일 단축키 <Ctrl + 7> */
        StyleShortcut8,/**< 스타일 단축키 <Ctrl + 8> */
        StyleShortcut9,/**< 스타일 단축키 <Ctrl + 9> */
        StyleShortcut10,/**< n번째 스타일 적용 */
        StyleShortcut11,/**< n번째 스타일 적용 */
        StyleShortcut12,/**< n번째 스타일 적용 */
        StyleShortcut13,/**< n번째 스타일 적용 */
        StyleShortcut14,/**< n번째 스타일 적용 */
        StyleShortcut15,/**< n번째 스타일 적용 */
        StyleShortcut16,/**< n번째 스타일 적용 */
        StyleShortcut17,/**< n번째 스타일 적용 */
        StyleShortcut18,/**< n번째 스타일 적용 */
        StyleShortcut19,/**< n번째 스타일 적용 */
        StyleShortcut20,/**< n번째 스타일 적용 */
        StyleClearCharStyle,/**< 글자 스타일 해제 */
        StyleTemplate,/**< 스타일 마당 */
        StyleChangeToCurrentShape,/**< 스타일 현재 모양으로 바꾸기 */
        Bookmark,/**< 책갈피 */
        ModifyBookmark,/**< 책갈피 고치기 */
        InsertCrossReference,/**< 상호 참조 만들기(삽입) */
        ModifyCrossReference,/**< 상호 참조 고치기 */
        EquationCreate,/**< 수식 만들기 */
        EquationModify,/**< 수식 편집 */
        TextArtCreate,/**< 글맵시 */
        InsertFootnote,/**< 각주 입력 */
        InsertEndnote,/**< 미주 입력 */
        InputCodeChange,/**< 문자/코드 변환 */
        InsertTab,/**< 탭 삽입 */
        InsertSoftHyphen,/**< 하이픈 삽입 */
        InsertText,/**< 텍스트 삽입 */
        InsertSpace,/**< 공백 삽입 */
        InsertNonBreakingSpace,/**< 묶음 빈 칸 삽입 */
        InsertFixedWidthSpace,/**< 고정 폭 빈 칸 삽입 */
        RightShiftBlock,/**< 텍스트 블록 상태에서 블록이 문단의 시작 위치에서 시작할 경우 블록 왼쪽에 탭을 삽입 */
        LeftShiftBlock,/**< 텍스트 블록 상태에서 블록 왼쪽에 있는 탭(또는 공백)을 지움 */
        IndexMark,/**< 찾아보기 표시 */
        MakeIndex,/**< 찾아보기 만들기 */
        MakeContents,/**< 차례 만들기 */
        IndexMarkModify,/**< 찾아보기 표시 고치기 */
        UpdateAllContents,/**< 모든 차례 업데이트 */
        UpdateCurContents,/**< 현재 차례 업데이트 */
        MarkTitle,/**< 제목 차례 표시([도구-차례/찾아보기-제목 차례 표시] 메뉴와 같음) */
        HideTitle,/**< 차례 숨기기([도구-차례/찾아보기-차례 숨기기] 메뉴와 같음) */
        Sort,/**< 정렬 */
        Sum,/**< 블록 합계 */
        Average,/**< 블록 평균 */
        InputDateStyle,/**< 날짜/시간 형식 지정해서 추가([입력-날짜/시간-날짜/시간 형식] 메뉴와 동일) */
        InsertFieldDateTime,/**< 날짜/시간 코드로 넣기([입력-날짜/시간-날짜/시간 코드] 메뉴와 동일) */
        InsertStringDateTime,/**< 날짜/시간 넣기 - 문자열로 넣기([입력-날짜/시간-날짜/시간 문자열] 메뉴와 동일) */
        InsertDateCode,/**< 상용구 코드 넣기(만든 날짜) */
        InsertUserName,/**< 상용구 코드 넣기(만든 사람) */
        InsertFileName,/**< 상용구 코드 넣기(파일 이름만) */
        InsertFilePath,/**< 상용구 코드 넣기(파일 이름과 경로) */
        InsertDocTitle,/**< 상용구 코드 넣기(문서 제목) */
        InsertCpNo,/**< 상용구 코드 넣기(현재 쪽 번호) */
        InsertTpNo,/**< 상용구 코드 넣기(전체 쪽 번호) */
        InsertCpTpNo,/**< 상용구 코드 넣기(현재 쪽/전체 쪽) */
        InsertDocInfo,/**< 상용구 코드 넣기(만든 사람, 현재 쪽, 만든 날짜) */
        InsertLastSaveDate,/**< 상용구 코드 넣기(마지막 저장한 날짜) */
        InsertLastSaveBy,/**< 상용구 코드 넣기(마지막 저장한 사람) */
        InsertLastPrintDate,/**< 상용구 코드 넣기(마지막 인쇄한 날짜) */
        RecalcPageCount,/**< 현재 페이지의 쪽 번호 재계산 */
        ReturnKeyInField,/**< 캐럿이 필드 안에 위치한 상태에서 Return Key에 대한 액션 분기 */
        InsertDocumentProperty,/**< 상호 참조 넣기 */
        InsertFieldTemplate,/**< 문서마당 정보 */
        InsertFieldCtrl,/**< 필드 컨트롤(누름틀, 개인 정보, 문서 요약 등)을 추가 */
        InsertFieldFileName,/**< 파일 이름 넣기([입력-날짜/시간-파일 이름] 메뉴와 동일) */
        ModifyFieldClickhere,/**< 누름틀 정보 고치기 */
        ModifyFieldSummary,/**< 문서 요약 필드 고치기 */
        ModifyFieldPath,/**< 문서 경로 필드 고치기 */
        InsertFieldMemo,/**< 메모 넣기([입력-메모-메모 넣기] 메뉴와 동일) */
        InsertFieldCitation,/**< 인용 */
        InsertFieldBibliography,/**< 인용 넣기 */
        UpdataeFieldBibStyle,/**< 인용 업데이트 */
        InsertFile,/**< 끼워 넣기 */
        Hyperlink,/**< 캐럿이 필드 안에 위치했는지 여부에 따라 Insert 또는 Modify 하이퍼링크 액션 */
        InsertHyperlink,/**< 하이퍼링크 만들기 */
        DeleteHyperlink,/**< 하이퍼링크 삭제 */
        InsertRevisionHyperlink,/**< 교정 부호 넣기: 자료 연결 */
        ModifyRevisionHyperlink,/**< 자료 연결(교정 부호) 고치기 */
        InsertRevisionSpace,/**< 교정 부호 넣기: 띄움표 */
        InsertRevisionLineInsert,/**< 교정 부호 넣기: 줄 비움표 */
        InsertFieldRevisionChagne,/**< 메모 고침표 넣기(현재 한글 메뉴에 없음, 메모와 동일한 기능) */
        InsertRevisionDelete,/**< 교정 부호 넣기: 지움표 */
        InsertRevisionAttach,/**< 교정 부호 넣기: 붙임표 */
        InsertRevisionClipping,/**< 교정 부호 넣기: 뺌표 */
        InsertRevisionLineLink,/**< 교정 부호 넣기: 줄 이음표 */
        InsertRevisionLineAttach,/**< 교정 부호 넣기: 줄 붙임표 */
        InsertRevisionSawTooth,/**< 교정 부호 넣기: 톱니표 */
        InsertRevisionThinking,/**< 교정 부호 넣기: 생각표 */
        InsertRevisionPraise,/**< 교정 부호 넣기: 칭찬표 */
        InsertRevisionLine,/**< 교정 부호 넣기: 줄표 */
        InsertRevision,/**< 교정 부호 넣기([입력-교정 부호-교정 부호 넣기] 메뉴와 동일) */
        InsertRevisionSymbol,/**< 교정 부호 넣기: 부호 넣음표 */
        InsertRevisionInsert,/**< 교정 부호 넣기: 넣음표 */
        InsertRevisionSimpleChange,/**< 교정 부호 넣기: 고침표 */
        InsertRevisionRightMove,/**< 교정 부호 넣기: 오른자리 옮김표 */
        InsertRevisionLeftMove,/**< 교정 부호 넣기: 왼자리 옮김표 */
        InsertRevisionLineTransfer,/**< 교정 부호 넣기: 줄 서로 바꿈표 */
        InsertRevisionTransferSplit,/**< 교정 부호 넣기: 자리 바꿈 나눔표(내부용) */
        InsertRevisionLineTransferSplit,/**< 교정 부호 넣기: 줄 서로 바꿈 나눔표(내부용) */
        QuickMarkInsert0,/**< 쉬운 책갈피 - 삽입 */
        QuickMarkInsert1,/**< 쉬운 책갈피 - 삽입 */
        QuickMarkInsert2,/**< 쉬운 책갈피 - 삽입 */
        QuickMarkInsert3,/**< 쉬운 책갈피 - 삽입 */
        QuickMarkInsert4,/**< 쉬운 책갈피 - 삽입 */
        QuickMarkInsert5,/**< 쉬운 책갈피 - 삽입 */
        QuickMarkInsert6,/**< 쉬운 책갈피 - 삽입 */
        QuickMarkInsert7,/**< 쉬운 책갈피 - 삽입 */
        QuickMarkInsert8,/**< 쉬운 책갈피 - 삽입 */
        QuickMarkInsert9,/**< 쉬운 책갈피 - 삽입 */
        QuickMarkMove0,/**< 쉬운 책갈피 - 이동 */
        QuickMarkMove1,/**< 쉬운 책갈피 - 이동 */
        QuickMarkMove2,/**< 쉬운 책갈피 - 이동 */
        QuickMarkMove3,/**< 쉬운 책갈피 - 이동 */
        QuickMarkMove4,/**< 쉬운 책갈피 - 이동 */
        QuickMarkMove5,/**< 쉬운 책갈피 - 이동 */
        QuickMarkMove6,/**< 쉬운 책갈피 - 이동 */
        QuickMarkMove7,/**< 쉬운 책갈피 - 이동 */
        QuickMarkMove8,/**< 쉬운 책갈피 - 이동 */
        QuickMarkMove9,/**< 쉬운 책갈피 - 이동 */
        Comment,/**< 숨은 설명 */
        CommentModify,/**< 숨은 설명 고치기 */
        CommentDelete,/**< 숨은 설명 지우기 */
        DrawObjCreatorTextBox,/**< 글상자 */
        DrawObjCreatorHorzTextBox,/**< 가로 글상자 만들기 */
        DrawObjCreatorVertTextBox,/**< 세로 글상자 만들기 */
        DrawObjCreatorVertAllTextBox,/**< 세로 글상자 만들기 - 영문 세움 */
        DrawObjCreatorCanvas,/**< 캔버스 그리기 */
        InsertLine,/**< 선 넣기 */
        ShapeObjAttachCaption,/**< 캡션 넣기 */
        ShapeObjInsertCaptionNum,/**< 캡션 번호 넣기 */
        ShapeObjDetachCaption,/**< 캡션 없애기 */
        EqCaptionPosTop,/**< 수식 캡션 위치 - 위 */
        EqCaptionPosBottom,/**< 수식 캡션 위치 - 아래 */
        EqCaptionPosLeftTop,/**< 수식 캡션 위치 - 왼쪽 위 */
        EqCaptionPosLeftCenter,/**< 수식 캡션 위치 - 왼쪽 가운데 */
        EqCaptionPosLeftBottom,/**< 수식 캡션 위치 - 왼쪽 아래 */
        EqCaptionPosRightTop,/**< 수식 캡션 위치 - 오른쪽 위 */
        EqCaptionPosRightCenter,/**< 수식 캡션 위치 - 오른쪽 가운데 */
        EqCaptionPosRightBottom,/**< 수식 캡션 위치 - 오른쪽 아래 */
        PictureNoBrightness,/**< 그림 밝기 효과 없음 */
        PictureNoContrast,/**< 그림 대비 효과 없음 */
        PictureNoShadow,/**< 그림 그림자 효과 없음 */
        PictureNoReflection,/**< 그림 반사 효과 없음 */
        PictureNoGlow,/**< 그림 네온 효과 없음 */
        PictureNoSofeEdge,/**< 그림 부드러운 가장자리 효과 없음 */
        PictureNoStyle,/**< 그림 스타일 효과 없음 */
        PictureChange,/**< 그림 바꾸기 */
        PictureEffect1,/**< 그림-그레이 스케일 */
        PictureEffect2,/**< 그림-흑백으로 */
        PictureEffect3,/**< 그림-워터마크 */
        PictureEffect4,/**< 그림-효과 없음 */
        PictureToOriginal,/**< 그림 - 원래 그림으로 */
        PictureEffect5,/**< 그림 밝기 증가 */
        PictureEffect6,/**< 그림 밝기 감소 */
        PictureEffect7,/**< 그림 명암 증가 */
        PictureEffect8,/**< 그림 명암 감소 */
        PictureSave,/**< 그림 빼내기 */
        PictureLinkedToEmbedded,/**< 연결된 그림을 모두 삽입 그림으로 */
        PictureScissor,/**< 그림 자르기 */
        PictureSaveAsOption,/**< 바이너리 그림을 다른 형태로 저장하는 옵션을 설정 */
        PictureSaveAs,/**< 이미지를 다른 이름으로 저장 */
        PictureSaveAsAll,/**< 삽입된 바이너리 그림을 다른 형태로 저장 */
        LabelAdd,/**< 라벨 새 쪽 추가하기 */
        LabelTemplate,/**< 라벨 문서 만들기 */
        ManuScriptTemplate,/**< 원고지 쓰기 */
        PageBorder,/**< 쪽 테두리/배경 */
        PageBorderTab,/**< 쪽 테두리/배경(항상 테두리 탭이 선택되어 보임) */
        PageFillTab,/**< 쪽 테두리/배경(항상 테두리 탭이 선택되어 보임) */
        PageHiding,/**< 감추기 */
        PageHidingModify,/**< 감추기 고치기 */
        InsertPageNum,/**< 쪽 번호 넣기 */
        CopyPage,/**< 쪽 복사하기 */
        PastePage,/**< 쪽 붙여넣기 */
        BreakPage,/**< 쪽 나누기 */
        BreakColumn,/**< 단 나누기 */
        BreakSection,/**< 구역 나누기 */
        BreakLine,/**< 줄 바꿈 */
        BreakPara,/**< 문단 나누기 */
        BreakColDef,/**< 단 정의 삽입 */
        MultiColumn,/**< 다단 */
        PageLandscape,/**< 용지 넓게 */
        PagePortrait,/**< 용지 좁게 */
        PageSetup,/**< 편집 용지 */
        PageSetupDL,/**< 편집 용지(쪽 여백 설정) */
        PageMarginSetup,/**< 편집 용지(쪽 여백 설정) */
        ModifySection,/**< 구역 */
        VerticalText,/**< 세로 쓰기 */
        DeletePage,/**< 쪽 지우기 */
        MasterPage,/**< 바탕쪽 */
        NewNumber,/**< 새 번호로 시작 */
        NewNumberModify,/**< 새 번호 고치기 */
        ShowLineNumbers,/**< 줄 번호 넣기 */
        SuppressLineNumbers,/**< 줄 번호 넣기 */
        SetLineNumbers,/**< 줄 번호 넣기 */
        FootnoteOption,/**< 각주/미주 모양 */
        ExchangeFootnoteEndnote,/**< 각주/미주 변환 */
        FootnoteToEndnote,/**< 모든 각주를 미주로 */
        EndnoteToFootnote,/**< 모든 미주를 각주로 */
        ExchangeFootnoteEndnote,/**< 각주/미주 변환 */
        NoteNoSuperscript,/**< 주석 번호 보통(위 첨자 사용 안 함) */
        NoteSuperscript,/**< 주석 번호 작게(위 첨자) */
        FootnoteNoBeneathText,/**< 각주–꼬리말 바로 위 */
        FootnoteBeneathText,/**< 각주–본문 아래 */
        EndnoteEndOfSection,/**< 미주–구역의 끝 */
        EndnoteEndOfDocument,/**< 미주–문서의 끝 */
        FootnoteBottomOfEachColumn,/**< 다단 각주–각 단 아래 */
        FootnoteBottomOfMultiColumn,/**< 다단 각주–전 단 */
        NoteDelete,/**< 주석 지우기 */
        InsertAutoNum,/**< 번호 다시 넣기 */
        SaveFootnote,/**< 주석 저장 */
        TextArtModify,/**< 글맵시 고치기 */
        TextArtShadow,/**< 글맵시 그림자 넣기/빼기 */
        TextArtShadowMoveToUp,/**< 글맵시 그림자 위치 이동-위로 */
        TextArtShadowMoveToDown,/**< 글맵시 그림자 위치 이동-아래로 */
        TextArtShadowMoveToLeft,/**< 글맵시 그림자 위치 이동-왼쪽으로 */
        TextArtShadowMoveToRight,/**< 글맵시 그림자 위치 이동-오른쪽으로 */
        NoneTextArtShadow,/**< 글맵시 그림자 없음 */
        MemoToPrev,/**< 이전 메모 */
        MemoToNext,/**< 다음 메모 */
        MemoShape,/**< 메모 모양([입력-메모-메모 모양] 메뉴와 동일함) */
        DeleteFieldMemo,/**< 메모 지우기 */
        EditFieldMemo,/**< 메모 내용 편집 */
        ReplyMemo,/**< 메모 회신 */
        HeaderFooter,/**< 머리말/꼬리말 */
        HeaderFooterInsField,/**< 코드 넣기 */
        HeaderFooterToPrev,/**< 이후 머리말 */
        HeaderFooterToNext,/**< 이전 머리말 */
        HeaderFooterDelete,/**< 머리말 지우기 */
        HeaderFooterModify,/**< 머리말/꼬리말 고치기 */
        SelectPageNumShape,/**< 쪽 번호 모양 */
        PageNumPos,/**< 쪽 번호 매기기 */
        PageNumPosModify,/**< 쪽 번호 매기기 */
        ShapeObjTableSelCell,/**< 테이블 선택 상태에서 첫 번째 셀 선택 */
        TableAutoDrawPenStyleWidthDlg,/**< 표 그리기 선 모양 */
        TableCellShadeInc,/**< 셀 배경의 음영 어둡게 */
        TableCellShadeDec,/**< 셀 배경의 음영 밝게 */
        NoneTableCellShade,/**< 셀 음영 없음  */
        TableCellTextVert,/**< 셀 문자 방향-세로 쓰기-영문 눕힘 */
        TableCellTextVertAll,/**< 셀 문자 방향-세로 쓰기-영문 세움 */
        TableCellTextHorz,/**< 셀 문자 방향-가로 쓰기 */
        TableCellToggleDirection,/**< 표 문자 방향- 설정/해제 */
        TableDeleteCell,/**< 셀 삭제 */
        TableCreate,/**< 표 만들기 */
        TableDrawPen,/**< 표 그리기 */
        TableEraser,/**< 표 지우개 */
        TableFormula,/**< 계산식 */
        TableFormulaSumHor,/**< 가로 합계 */
        TableFormulaSumVer,/**< 세로 합계 */
        TableFormulaAvgHor,/**< 가로 평균 */
        TableFormulaAvgVer,/**< 세로 평균 */
        TableFormulaProHor,/**< 가로 곱 */
        TableFormulaProVer,/**< 세로 곱 */
        TableFormulaSumAuto,/**< 블록 합계 */
        TableFormulaAvgAuto,/**< 블록 평균 */
        TableFormulaProAuto,/**< 블록 곱 */
        TablePropertyDialog,/**< 표 고치기 */
        TableCaptionPosTop,/**< 테이블 캡션 위치-위 */
        TableCaptionPosBottom,/**< 테이블 캡션 위치-아래 */
        TableCaptionPosLeftTop,/**< 테이블 캡션 위치–왼쪽 위 */
        TableCaptionPosLeftCenter,/**< 테이블 캡션 위치–왼쪽 가운데 */
        TableCaptionPosLeftBottom,/**< 테이블 캡션 위치–왼쪽 아래 */
        TableCaptionPosRightTop,/**< 테이블 캡션 위치–오른쪽 위 */
        TableCaptionPosRightCenter,/**< 테이블 캡션 위치–오른쪽 가운데 */
        TableCaptionPosRightBottom,/**< 테이블 캡션 위치–오른쪽 아래 */
        TableTemplate,/**< 표 마당 */
        TableTemplateOptBorder,/**< 표 템플릿 스타일 적용 서식: 테두리 */
        TableTemplateOptAlign,/**< 표 템플릿 스타일 적용 서식: 글자 모양과 문단 모양 */
        TableTemplateOptCell,/**< 표 템플릿 스타일 적용 서식: 셀 배경 */
        TableTemplateOptGreyScale,/**< 표 템플릿 스타일 적용 서식: 그레이 스케일로 */
        TableTemplateOptTitleRow,/**< 표 템플릿 스타일 적용 대상: 제목 줄 */
        TableTemplateOptEndRow,/**< 표 템플릿 스타일 적용 대상: 마지막 줄 */
        TableTemplateOptLeftCol,/**< 표 템플릿 스타일 적용 대상: 첫째 칸 */
        TableTemplateOptRightCol,/**< 표 템플릿 스타일 적용 대상: 마지막 칸 */
        TableTemplateOptDefault,/**< 표 템플릿 스타일 적용 서식: 기본 값으로 */
        TableCellBlock,/**< 셀 블록 */
        TableCellBlockExtend,/**< 셀 블록 연장(F5 + F5) */
        TableCellBlockExtendAbs,/**< 셀 블록 연장(Shift + F5) */
        TableCellBlockCol,/**< 셀 블록(칸) */
        TableCellBlockRow,/**< 셀 블록(줄) */
        TableMergeCell,/**< 셀 합치기 */
        TableSplitCell,/**< 셀 나누기 */
        TableSplitCellRow2,/**< 셀 칸 나누기 */
        TableSplitCellCol2,/**< 셀 줄 나누기 */
        TableVAlignTop,/**< 셀 세로 정렬 위 */
        TableVAlignCenter,/**< 셀 세로 정렬 가운데 */
        TableVAlignBottom,/**< 셀 세로 정렬 아래 */
        TableDistributeCellWidth,/**< 셀 너비를 같게 */
        TableDistributeCellHeight,/**< 셀 높이를 같게 */
        TableDeleteRowColumn,/**< 줄-칸 지우기 */
        TableDeleteColumn,/**< 칸 지우기 */
        TableDeleteRow,/**< 줄 지우기 */
        TableInsertRowColumn,/**< 줄-칸 삽입 */
        TableInsertLeftColumn,/**< 왼쪽 칸 삽입 */
        TableInsertRightColumn,/**< 오른쪽 칸 삽입 */
        TableInsertUpperRow,/**< 위쪽 줄 삽입 */
        TableInsertLowerRow,/**< 아래쪽 줄 삽입 */
        TableAppendRow,/**< 줄 추가 */
        TableSubtractRow,/**< 표 줄 삭제 */
        CellBorderFill,/**< 셀 테두리 */
        CellBorder,/**< 셀 테두리 */
        CellFill,/**< 셀 배경 */
        CellZoneBorderFill,/**< 셀 테두리(여러 셀에 걸쳐 적용) */
        CellZoneBorder,/**< 셀 테두리(여러 셀에 걸쳐 적용) */
        CellZoneFill,/**< 셀 배경(여러 셀에 걸쳐 적용) */
        TableSplitTable,/**< 표 나누기 */
        TableMergeTable,/**< 표 붙이기 */
        TableTableToString,/**< 표를 문자열로 */
        TableStringToTable,/**< 문자열을 표로 */
        TableBreak,/**< 표 쪽 경계에서(나누지 않음) */
        TableBreakCell,/**< 표 쪽 경계에서(나눔) */
        TableBreakNone,/**< 표 쪽 경계에서(셀 단위로 나눔) */
        TableCellAlignLeftTop,/**< para align + cell valign: 표의 셀 안에서만 동작 */
        TableCellAlignLeftCenter,/**< para align + cell valign: 표의 셀 안에서만 동작 */
        TableCellAlignLeftBottom,/**< para align + cell valign :표의 셀 안에서만 동작 */
        TableCellAlignCenterTop,/**< para align + cell valign :표의 셀 안에서만 동작 */
        TableCellAlignCenterCenter,/**< para align + cell valign :표의 셀 안에서만 동작 */
        TableCellAlignCenterBottom,/**< para align + cell valign :표의 셀 안에서만 동작 */
        TableCellAlignRightTop,/**< para align + cell valign :표의 셀 안에서만 동작 */
        TableCellAlignRightCenter,/**< para align + cell valign :표의 셀 안에서만 동작 */
        TableCellAlignRightBottom,/**< para align + cell valign :표의 셀 안에서만 동작 */
        TableCellBorderOutside,/**< 바깥 셀 테두리 설정/해제. 셀 블록 상태일 경우에만 동작 */
        TableCellBorderAll,/**< 모든 셀 테두리 설정/해제. 셀 블록 상태일 경우에만 동작 */
        TableCellBorderInside,/**< 모든 안쪽 셀 테두리 설정/해제. 셀 블록 상태일 경우에만 동작 */
        TableCellBorderNo,/**< 모든 셀 테두리 지움. 셀 블록 상태일 경우에만 동작 */
        TableCellBorderTop,/**< 가장 위의 셀 테두리 설정/해제. 셀 블록 상태일 경우에만 동작 */
        TableCellBorderBottom,/**< 가장 아래 셀 테두리 설정/해제. 셀 블록 상태일 경우에만 동작 */
        TableCellBorderLeft,/**< 가장 왼쪽의 셀 테두리 설정/해제. 셀 블록 상태일 경우에만 동작 */
        TableCellBorderRight,/**< 가장 오른쪽의 셀 테두리 설정/해제. 셀 블록 상태일 경우에만 동작 */
        TableCellBorderInsideHorz,/**< 모든 안쪽 가로 셀 테두리 설정/해제. 셀 블록 상태일 경우에만 동작 */
        TableCellBorderInsideVert,/**< 모든 안쪽 세로 셀 테두리 설정/해제. 셀 블록 상태일 경우에만 동작 */
        TableCellBorderDiagonalDown,/**< 대각선(⍂) 셀 테두리 설정/해제. 셀 블록 상태일 경우에만 동작 */
        TableCellBorderDiagonalUp,/**< 대각선(⍁) 셀 테두리 설정/해제. 셀 블록 상태일 경우에만 동작 */
        TableSwap,/**< 표 뒤집기 */
        TableInsertComma,/**< 세 자리마다 자리점 넣기 */
        TableDeleteComma,/**< 세 자리마다 자리점 빼기 */
        TableResizeExLeft,/**< 셀 크기 변경: 셀 왼쪽 */
        TableResizeExRight,/**< 셀 크기 변경: 셀 오른쪽 */
        TableResizeExUp,/**< 셀 크기 변경: 셀 위 */
        TableResizeExDown,/**< 셀 크기 변경: 셀 아래 */
        ImageFindPath,/**< 그림 경로 찾기 */
        FillColorShadeInc,/**< 면 색 음영 비율 증가 */
        FillColorShadeDec,/**< 면 색 음영 비율 감소 */
        LinkTextBox,/**< 글상자 연결 */
        UnlinkTextBox,/**< 글상자 연결 끊기 */
        PrevTextBoxLinked,/**< 연결된 글상자의 이전 글상자로 이동 */
        NextTextBoxLinked,/**< 연결된 글상자의 다음 글상자로 이동 */
        TextBoxTextVert,/**< 글상자 문자 방향–세로 쓰기–영문 눕힘 */
        TextBoxTextVertAll,/**< 글상자 문자 방향–세로 쓰기–영문 세움 */
        TextBoxTextHorz,/**< 글상자 문자 방향–가로 쓰기 */
        TextBoxToggleDirection,/**< 글상자 문자 방향–세로/가로 전환 */
        TextBoxVAlignTop,/**< 글상자 세로 정렬-위 */
        TextBoxVAlignCenter,/**< 글상자 세로 정렬-가운데 */
        TextBoxVAlignBottom,/**< 글상자 세로 정렬-아래 */
        TextBoxAlignLeftTop,/**< 글상자 정렬 */
        TextBoxAlignLeftCenter,/**< 글상자 정렬 */
        TextBoxAlignLeftBottom,/**< 글상자 정렬 */
        TextBoxAlignCenterTop,/**< 글상자 정렬 */
        TextBoxAlignCenterCenter,/**< 글상자 정렬 */
        TextBoxAlignCenterTop,/**< 글상자 정렬 */
        TextBoxAlignRightTop,/**< 글상자 정렬 */
        TextBoxAlignRightCenter,/**< 글상자 정렬 */
        TextBoxAlignRightBottom,/**< 글상자 정렬 */
        DrawShapeObjShadow,/**< 그리기 개체 그림자 만들기/지우기 */
        ShapeObjShadowParellelLeftTop,/**< 그리기 개체 그림자를 원본 개체와 동일한 크기로 왼쪽 상단에 생성 */
        ShapeObjShadowParellelRightTop,/**< 그리기 개체 그림자를 원본 개체와 동일한 크기로 오른쪽 상단에 생성 */
        ShapeObjShadowParellelLeftBottom,/**< 그리기 개체 그림자를 원본 개체와 동일한 크기로 왼쪽 하단에 생성 */
        ShapeObjShadowParellelRightBottom,/**< 그리기 개체 그림자를 원본 개체와 동일한 크기로 오른쪽 하단에 생성 */
        ShapeObjShadowNarrow,/**< 그리기 개체 그림자를 작은 형태로 변경. 그림자가 없으면 생성 */
        ShapeObjShadowEnlarge,/**< 그리기 개체 그림자를 큰 형태로 변경. 그림자가 없으면 생성 */
        ShapeObjShadowShearLeftTop,/**< 그리기 개체 그림자를 왼쪽 앞부분으로 눕혀서 생성 */
        ShapeObjShadowShearRightTop,/**< 그리기 개체 그림자를 오른쪽 앞부분으로 눕혀서 생성 */
        ShapeObjShadowShearLeftBottom,/**< 그리기 개체 그림자를 왼쪽 뒷부분으로 눕혀서 생성 */
        ShapeObjShadowShearRightBottom,/**< 그리기 개체 그림자를 오른쪽 뒷부분으로 눕혀서 생성 */
        ShapeObjShadowMoveUp,/**< 그리기 개체 그림자를 위로 이동. 그림자가 없으면 생성 */
        ShapeObjShadowMoveDown,/**< 그리기 개체 그림자를 밑으로 이동. 그림자가 없으면 생성 */
        ShapeObjShadowMoveLeft,/**< 그리기 개체 그림자를 왼쪽으로 이동. 그림자가 없으면 생성 */
        ShapeObjShadowMoveRight,/**< 그리기 개체 그림자를 오른쪽으로 이동. 그림자가 없으면 생성 */
        ShapeObjShadowMoveOrginal,/**< 그리기 개체 그림자 위치 원점으로(offset 제거) */
        CaptionPosTop,/**< 캡션 위치-위 */
        CaptionPosBottom,/**< 캡션 위치-아래 */
        CaptionPosLeftTop,/**< 캡션 위치–왼쪽 위 */
        CaptionPosLeftCenter,/**< 캡션 위치–왼쪽 가운데 */
        CaptionPosLeftBottom,/**< 캡션 위치-왼쪽 아래 */
        CaptionPosRightTop,/**< 캡션 위치–오른쪽 위 */
        CaptionPosRightCenter,/**< 캡션 위치–오른쪽 가운데 */
        CaptionPosRightBottom,/**< 캡션 위치–오른쪽 아래 */
        ShapeObjNoShadow,/**< 그림자 없음 */
        ShapeObjNoShade,/**< 채우기 색 음영 없음 */
        ShapeObjectPaste,/**< 그리기 모양 붙여넣기 */
        ShapeObjectCopy,/**< 그리기 모양 복사 */
        ShapeObjAttrDialog,/**< 틀 속성 환경 설정 */
        ShapeObjLineProperty,/**< 고치기 대화 상자 중 [선] 탭 */
        ShapeObjLineStyleOhter,/**< 다른 선 종류 */
        ShapeObjLineWidthOhter,/**< 다른 선 굵기 */
        DrawObjCreatorLine,/**< 선 그리기 */
        DrawObjCreatorRectangle,/**< 사각형 그리기 */
        DrawObjCreatorEllipse,/**< 원 그리기 */
        DrawObjCreatorArc,/**< 호 그리기 */
        DrawObjCreatorPolygon,/**< 다각형 그리기 */
        DrawObjCreatorCurve,/**< 곡선 그리기 */
        DrawObjCreatorFreeDrawing,/**< 펜 */
        DrawObjCreatorObject,/**< 그리기 개체 */
        DrawObjCreatorMultiLine,/**< 반복해서 선 그리기 */
        DrawObjCreatorMultiRectangle,/**< 반복해서 사각형 그리기 */
        DrawObjCreatorMultiEllipse,/**< 반복해서 원 그리기 */
        DrawObjCreatorMultiArc,/**< 반복해서 호 그리기 */
        DrawObjCreatorMultiPolygon,/**< 반복해서 다각형 그리기 */
        DrawObjCreatorMultiCurve,/**< 반복해서 곡선 그리기 */
        DrawObjCreatorMultiFreeDrawing,/**< 반복해서 펜 그리기 */
        InsertConnectLineStraightNoArrow,/**< 개체 연결선(직선 연결선) */
        InsertConnectLineStraightOneWay,/**< 개체 연결선(직선 화살표 연결선) */
        InsertConnectLineStraightBoth,/**< 개체 연결선(직선 양쪽 화살표 연결선) */
        InsertConnectLineStrokeNoArrow,/**< 개체 연결선(꺾인 연결선) */
        InsertConnectLineStrokeOneWay,/**< 개체 연결선(꺾인 화살표 연결선) */
        InsertConnectLineStrokeBoth,/**< 개체 연결선(꺾인 양쪽 화살표 연결선) */
        InsertConnectLineArcNoArrow,/**< 개체 연결선(구부러진 연결선) */
        InsertConnectLineArcOneWay,/**< 개체 연결선(구부러진 화살표 연결선) */
        InsertConnectLineArcBoth,/**< 개체 연결선(구부러진 양쪽 화살표 연결선) */
        InsertConnectLineMultiStraightNoArrow,/**< 개체 연결선 반복해서 그리기(직선 연결선) */
        InsertConnectLineMultiStraightOneWay,/**< 개체 연결선 반복해서 그리기(직선 화살표 연결선) */
        InsertConnectLineMultiStraightBoth,/**< 개체 연결선 반복해서 그리기(직선 양쪽 화살표 연결선) */
        InsertConnectLineMultiStrokeNoArrow,/**< 개체 연결선 반복해서 그리기(꺾인 연결선) */
        InsertConnectLineMultiStrokeOneWay,/**< 개체 연결선 반복해서 그리기(꺾인 화살표 연결선) */
        InsertConnectLineMultiStrokeBoth,/**< ShapeObjGroup */
        InsertConnectLineMultiArcNoArrow,/**< 개체 연결선 반복해서 그리기(구부러진 양쪽 화살표 연결선) */
        InsertConnectLineMultiArcOneWay,/**< 개체 연결선 반복해서 그리기 */
        DrawObjCreatorMultiCanvas,/**< 반복해서 캔버스 그리기 */
        DrawObjCancelOneStep,/**< 다각형(곡선) 그리는 중 이전 선 지우기 */
        ShapeObjRotater,/**< 자유각 회전(회전 중심 고정) */
        ShapeObjRightAngleRotater,/**< 90도 회전 */
        ShapeObjRightAngleRotaterAnticlockwise,/**< -90도 회전 */
        ShapeObjShear,/**< 그리기 개체 기울이기 */
        ShapeObjComment,/**< 개체 설명문 수정 */
        DrawObjEditDetail,/**< 그리기 개체 편집 */
        ShapeObjSelect,/**< 틀 선택 도구 */
        ShapeObjNorm,/**< 기본 도형 설정 */
        ShapeObjGroup,/**< 틀 묶기 */
        ShapeObjUngroup,/**< 틀 풀기 */
        ShapeObjBringToFront,/**< 맨 앞으로 */
        ShapeObjSendToBack,/**< 맨 뒤로 */
        ShapeObjBringForward,/**< 앞으로 */
        ShapeObjSendBack,/**< 뒤로 */
        ShapeObjBringInFrontOfText,/**< 글 앞으로 */
        ShapeObjCtrlSendBehindText,/**< 글 뒤로 */
        ShapeObjWrapTopAndBottom,/**< 자리 차지 */
        ShapeObjWrapSquare,/**< 직사각형 */
        ShapeObjTreatAsChar,/**< 글자처럼 취급 */
        ShapeObjProtectSize,/**< 그리기 개체 크기 고정 */
        TableTreatAsChar,/**< 표 글자처럼 취급 */
        EqTreatAsChar,/**< 수식 글자처럼 취급 */
        ShapeObjPrevObject,/**< 이전 개체로 이동(Shift + Tab 키) */
        ShapeObjNextObject,/**< 이후 개체로 이동(Tab 키) */
        ShapeObjHorzFlip,/**< 그리기 개체 좌우 뒤집기 */
        ShapeObjVertFlip,/**< 그리기 개체 상하 뒤집기 */
        ShapeObjHorzFlipOrgState,/**< 그리기 개체 좌우 뒤집기 원래 상태로 되돌리기 */
        ShapeObjVertFlipOrgState,/**< 그리기 개체 상하 뒤집기 원래 상태로 되돌리기 */
        ShapeObjLock,/**< 개체 잠금 */
        ShapeObjUnlockAll,/**< 개체 모두 잠금 해제 */
        ShapeObjAlignTop,/**< 위로 정렬 */
        ShapeObjAlignMiddle,/**< 중간 정렬 */
        ShapeObjAlignBottom,/**< 아래로 정렬 */
        ShapeObjAlignLeft,/**< 왼쪽으로 정렬 */
        ShapeObjAlignCenter,/**< 가운데로 정렬 */
        ShapeObjAlignRight,/**< 오른쪽으로 정렬 */
        ShapeObjAlignVertSpacing,/**< 위/아래 일정한 비율로 정렬 */
        ShapeObjAlignHorzSpacing,/**< 왼쪽/오른쪽 일정한 비율로 정렬 */
        ShapeObjAlignWidth,/**< 폭 맞춤 */
        ShapeObjAlignHeight,/**< 높이 맞춤 */
        ShapeObjAlignSize,/**< 폭/높이 맞춤 */
        ShapeObjMoveLeft,/**< 키로 움직이기(왼쪽) */
        ShapeObjMoveRight,/**< 키로 움직이기(오른쪽) */
        ShapeObjMoveUp,/**< 키로 움직이기(위) */
        ShapeObjMoveDown,/**< 키로 움직이기(아래) */
        ShapeObjResizeLeft,/**< 키로 크기 조절(Shift + 왼쪽) */
        ShapeObjResizeRight,/**< 키로 크기 조절(Shift + 오른쪽) */
        ShapeObjResizeUp,/**< 키로 크기 조절(Shift + 위) */
        ShapeObjResizeDown,/**< 키로 크기 조절(Shift + 아래) */
        ShapeObjToggleTextBox,/**< 도형 글 상자로 만들기-설정/해제 */
        ShapeObjAttachTextBox,/**< 글 상자로 만들기 */
        ShapeObjDetachTextBox,/**< 글상자 속성 없애기 */
        ShapeObjTextBoxEdit,/**< 글상자 선택 상태에서 편집 모드로 전환 */
        ShapeObjSaveAsPicture,/**< 그리기 개체를 그림으로 저장하기 */
        DrawObjOpenClosePolygon,/**< 다각형 열기/닫기 */
        TrackChangeApply,/**< 변경 내용 추적: 변경 내용 적용 */
        TrackChangeCancel,/**< 변경 내용 추적: 변경 내용 취소 */
        TrackChangePrev,/**< 변경 내용 추적 이전 */
        TrackChangeNext,/**< 변변경 내용 추적 다음 */
        TrackChangeApplyNext,/**< 변경 내용 추적: 적용 후 다음으로 이동 */
        TrackChangeApplyPrev,/**< 변경 내용 추적: 적용 후 이전으로 이동 */
        TrackChangeAuthor,/**< 변경 내용 추적: 사용자 이름 변경 */
        TrackChangeProtection,/**< 변경 내용 추적: 변경 내용 보호 */
        TrackChangeOption,/**< 변경 내용 추적: 변경 추적 옵션 */
        TrackChangeApplyAll,/**< 변경 내용 추적: 문서에서 변경 내용 모두 적용 */
        TrackChangeApplyViewAll,/**< 변경 내용 추적: 표시된 변경 내용 모두 적용 */
        TrackChangeCancelAll,/**< 변경 내용 추적: 문서에서 변경 내용 모두 취소 */
        TrackChangeCancelViewAll,/**< 변경 내용 추적: 표시된 변경 내용 모두 취소 */
        ViewOptionTrackChange,/**< 변경 내용 추적 보기 */
        ViewOptionTrackChangeOriginalMemo,/**<  메모 및 변경 내용 원본 */
        ViewOptionTrackChangeFinalMemo,/**< 메모 및 변경 내용 최종본 */
        ViewOptionTrackChangeOriginal,/**< 원본 보기 */
        ViewOptionTrackChangeFinal,/**< 최종본 보기 */
        ViewOptionTrackChangeInsertDelete,/**< 삽입 및 삭제 */
        ViewOptionTrackChangeShape,/**< 서식 */
        ViewOptionTrackChangeInline,/**< 안내문에 표시 */
        ViewOptionTrackChangeInfo,/**< 변경 내용 보기 */
 
    };
 
    /**
    * @}
    */
};