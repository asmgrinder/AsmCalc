; win32.inc for yasm, only used by this program definitions defined
; This file is provided as is with no warranty of any kind. Use at your own risk.

%xdefine null ebx
%xdefine _0 ebx
%xdefine false ebx

word.size	equ 2
dword.size	equ 4
tword.size	equ 10

%macro import 2-*
	%if 0 != (%0 % 2)
		%error	"macro import takes even number of parameters"
	%else
		%rep %0 / 2
			extern %2
			%1 equ %2
			%rotate 2
		%endrep
	%endif
%endm

%macro unicode 1-*
	%rep %0
		%ifstr %1
			%strlen    %%charcnt    %1
			%xdefine    %%index 1
			%rep %%charcnt
				%substr %%mychar %1 %%index
				dw %%mychar
				%xdefine %%index (%%index+1)
			%endrep
		%else
			dw %1
		%endif
		%rotate 1
	%endrep
%endm

%macro wcstbl 2-*
	%push wcstbl
	%assign %$index 1
	%rep %0-1
		%rotate 1
		%$_%{$index}:	unicode %1, 0
        %assign %$index (%$index+1)
    %endrep
    %rotate 1
    align 4
    %1:
	%assign %$index 1
	%rep %0-1
		dd %$_%{$index}
        %assign %$index (%$index+1)
    %endrep
    %1.length equ %0-1
    %pop
%endm

%macro push 1-*
	%rep %0
		push	%1
		%rotate 1
	%endrep
%endm

%macro pop 1-*
	%rep %0
		pop		%1
		%rotate 1
	%endrep
%endm

%macro push_unicode_string 1+
	[section .rdata]
	%%text:	unicode %1, 0
	[section .text]
	push dword %%text
%endm

%macro stdcall 1-*
	%rep %0 - 1
		%rotate -1
		%ifstr %1
			push_unicode_string %1
		%else
			push dword %1
		%endif
	%endrep
	%rotate -1
	call %1
%endm

%macro invoke 1-*
	%rep %0 - 1
		%rotate -1
		%ifstr %1
			push_unicode_string %1
		%else
			push dword %1
		%endif
	%endrep
	%rotate -1
	call %1
%endm

%macro proc 1-*
	%1:
	%push %1
	%xdefine %$paramcount (%0 - 1)
	%if %$paramcount > 0
		absolute ebp+2*dword.size
		%rep %$paramcount
			%rotate 1
			%1:	resd 1
		%endrep
		[section .text]
	%endif
%endm

%macro enter 0-*
	%if (%$paramcount > 0 || %0 > 0)
		push	ebp
		mov		ebp, esp
		%if %0 > 0
			absolute 0
			%rep %0
				%rotate 1
				%$%{1}
			%endrep
			%%stackSize:
			[section .text]
			sub		esp, %%stackSize;%0 * dword.size
			absolute ebp - %%stackSize;%0 * dword.size
			%rep %0
				%rotate -1
				%1;~ %1:	resd 1
			%endrep
			[section .text]
		%endif
	%endif
%endm

%macro leave 0
	mov		esp, ebp
	pop		ebp
%endm

%macro return 0
	%if %$paramcount > 0
		ret		%$paramcount * dword.size
	%else
		ret
	%endif
%endm

%macro endp	0
	%pop
%endm

import 	MessageBox, _MessageBoxW@16,\
		ExitProcess, _ExitProcess@4,\
		LoadLibrary, _LoadLibraryW@4,\
		FreeLibrary, _FreeLibrary@4,\
		GetProcAddress, _GetProcAddress@8,\
		InitCommonControlsEx, _InitCommonControlsEx@4,\
		InitCommonControls, _InitCommonControls@0,\
		LoadIcon, _LoadIconW@8,\
		VirtualAlloc, _VirtualAlloc@16,\
		VirtualFree, _VirtualFree@12,\
		GetModuleHandle, _GetModuleHandleW@4,\
		LoadAccelerators, _LoadAcceleratorsW@8,\
		CreateWindowEx, _CreateWindowExW@48,\
		DialogBoxParam, _DialogBoxParamW@20,\
		CreateDialogParam, _CreateDialogParamW@20,\
		DestroyWindow, _DestroyWindow@4,\
		GetMessage, _GetMessageW@16,\
		TranslateAccelerator, _TranslateAcceleratorW@12,\
		IsDialogMessage, _IsDialogMessageW@8,\
		TranslateMessage, _TranslateMessage@4,\
		DispatchMessage, _DispatchMessageW@4,\
		PostQuitMessage, _PostQuitMessage@4,\
		GetWindowLongPtr, _GetWindowLongW@8,\
		SetWindowLongPtr, _SetWindowLongW@12,\
		GetFocus, _GetFocus@0,\
		CallWindowProc, _CallWindowProcW@20,\
		EndDialog, _EndDialog@8,\
		GetWindowTextLength, _GetWindowTextLengthW@4,\
		GetWindowText, _GetWindowTextW@12,\
		GetWindowTextLengthA, _GetWindowTextLengthA@4,\
		GetWindowTextA, _GetWindowTextA@12,\
		SetWindowText, _SetWindowTextW@8,\
		GetDlgItem, _GetDlgItem@8,\
		SendMessage, _SendMessageW@16,\
		SetFocus, _SetFocus@4,\
		RegCreateKeyEx, _RegCreateKeyExW@36,\
		RegSetValueEx, _RegSetValueExW@24,\
		RegQueryValueEx, _RegQueryValueExW@24,\
		RegCloseKey, _RegCloseKey@4,\
		GlobalAlloc, _GlobalAlloc@8,\
		GlobalFree, _GlobalFree@4,\
		GlobalLock, _GlobalLock@4,\
		GlobalUnlock, _GlobalUnlock@4,\
		OpenClipboard, _OpenClipboard@4,\
		EmptyClipboard, _EmptyClipboard@0,\
		SetClipboardData, _SetClipboardData@8,\
		CloseClipboard, _CloseClipboard@0,\
		lstrlen, _lstrlenW@4,\
		lstrcpy, _lstrcpyW@8

%xdefine TOOLTIPS_CLASS "tooltips_class32"

CW_USEDEFAULT			equ 0x80000000

WS_POPUP				equ 0x80000000

TTS_ALWAYSTIP			equ 1
TTS_NOPREFIX			equ 2
TTS_NOANIMATE			equ 0x10
TTS_NOFADE				equ 0x20
TTS_BALLOON				equ 0x40
TTS_CLOSE				equ 0x80

TTF_IDISHWND			equ 1
TTF_CENTERTIP			equ 2
TTF_RTLREADING			equ 4
TTF_SUBCLASS			equ 16
TTF_TRACK				equ 0x0020
TTF_ABSOLUTE			equ 0x0080
TTF_TRANSPARENT			equ 0x0100
TTF_PARSELINKS 			equ 0x1000
TTF_DI_SETITEM 			equ 0x8000

WM_DESTROY				equ 2
WM_CLOSE				equ 16
WM_SETICON				equ 128
WM_GETDLGCODE			equ	135
WM_CHAR					equ 258
WM_INITDIALOG			equ 272
WM_COMMAND				equ 273
WM_PASTE				equ 770
WM_USER					equ 1024

TTM_ADDTOOLW			equ (WM_USER+50)
TTM_ADDTOOL				equ TTM_ADDTOOLW

HKEY_CURRENT_USER		equ 80000001h
;REG_OPTION_NON_VOLATILE	equ	0
KEY_ALL_ACCESS			equ 0f003fh
KEY_READ				equ 20019h
KEY_WRITE				equ 20006h
KEY_QUERY_VALUE			equ 1
KEY_SET_VALUE 			equ 2
KEY_CREATE_SUB_KEY		equ 4
KEY_ENUMERATE_SUB_KEYS	equ 8

REG_NONE						equ 0
REG_SZ							equ 1
REG_EXPAND_SZ					equ 2
REG_BINARY						equ 3
REG_DWORD_LITTLE_ENDIAN			equ 4
REG_DWORD						equ 4
REG_DWORD_BIG_ENDIAN			equ 5
REG_LINK						equ 6
REG_MULTI_SZ					equ 7
REG_RESOURCE_LIST				equ 8
REG_FULL_RESOURCE_DESCRIPTOR	equ 9
REG_RESOURCE_REQUIREMENTS_LIST	equ 10
REG_QWORD_LITTLE_ENDIAN			equ 11
REG_QWORD						equ 11

ERROR_MORE_DATA					equ 234

GMEM_MOVEABLE			equ 2
CF_TEXT					equ 1
CF_UNICODETEXT			equ 13

IDI_APPLICATION			equ 32512
ICON_SMALL				equ 0
ICON_BIG				equ 1

MEM_COMMIT				equ 0x1000
MEM_RELEASE				equ 0x8000
PAGE_READWRITE			equ 0x0004

GWLP_WNDPROC			equ -4
GWLP_USERDATA			equ -21

EM_GETSEL				equ 176
EM_SETSEL				equ 177
EM_REPLACESEL			equ 194

CB_ADDSTRING			equ 323
CB_DELETESTRING			equ 324
CB_GETCURSEL			equ 327
CB_GETLBTEXT			equ 328
CB_GETLBTEXTLEN			equ 329
CB_INSERTSTRING			equ 330
CB_SETCURSEL			equ 334
CB_GETITEMDATA			equ 336
CB_SETITEMDATA			equ 337

CBN_SELCHANGE			equ 1
CBN_CLOSEUP				equ 8

BM_CLICK				equ 245
BM_GETCHECK				equ 240
BM_GETIMAGE				equ 246
BM_GETSTATE				equ 242
BM_SETCHECK				equ 241
BM_SETIMAGE				equ 247
BM_SETSTATE				equ 243
BM_SETSTYLE				equ 244

BN_CLICKED				equ 0
BN_DBLCLK				equ 5
BN_DISABLE				equ 4
BN_DOUBLECLICKED		equ 5
BN_HILITE				equ 2
BN_KILLFOCUS			equ 7
BN_PAINT				equ 1
BN_PUSHED				equ 2
BN_SETFOCUS				equ 6
BN_UNHILITE				equ 3
BN_UNPUSHED				equ 3

EN_CHANGE				equ 768

BST_CHECKED				equ 1
BST_INDETERMINATE		equ 2
BST_UNCHECKED			equ 0
BST_FOCUS				equ 8
BST_PUSHED				equ 4

IDOK					equ 1
IDCANCEL				equ 2
IDABORT					equ 3
IDRETRY					equ 4
IDIGNORE				equ 5
IDYES					equ 6
IDNO					equ 7

MB_USERICON				equ 128
MB_ICONASTERISK			equ 64
MB_ICONEXCLAMATION		equ 0x30
MB_ICONWARNING			equ 0x30
MB_ICONERROR			equ 16
MB_ICONHAND				equ 16
MB_ICONQUESTION			equ 32
MB_OK					equ 0
MB_ABORTRETRYIGNORE		equ 2
MB_APPLMODAL			equ 0
MB_DEFAULT_DESKTOP_ONLY	equ 0x20000
MB_HELP					equ 0x4000
MB_RIGHT				equ 0x80000
MB_RTLREADING			equ 0x100000
MB_TOPMOST				equ 0x40000
MB_DEFBUTTON1			equ 0
MB_DEFBUTTON2			equ 256
MB_DEFBUTTON3			equ 512
MB_DEFBUTTON4			equ 0x300
MB_ICONINFORMATION		equ 64
MB_ICONSTOP				equ 16
MB_OKCANCEL				equ 1
MB_RETRYCANCEL			equ 5

DLGC_WANTARROWS			equ 1
DLGC_WANTTAB			equ 2
DLGC_WANTALLKEYS		equ 4
DLGC_WANTMESSAGE		equ 4
DLGC_HASSETSEL			equ 8
DLGC_DEFPUSHBUTTON		equ 16
DLGC_UNDEFPUSHBUTTON	equ 32
DLGC_RADIOBUTTON		equ 64
DLGC_WANTCHARS			equ 128
DLGC_STATIC				equ 256
DLGC_BUTTON				equ 0x2000

ICC_WIN95_CLASSES		equ 255

STAP_ALLOW_NONCLIENT	equ 0x00000001
STAP_ALLOW_CONTROLS		equ 0x00000002
STAP_ALLOW_WEBCONTENT	equ 0x00000004

struc POINT
	.x:		resd 1
	.y:		resd 1
	.size:
endstruc

struc MSG
	.hwnd:		resd 1
	.message:	resd 1
	.wParam:	resd 1
	.lParam:	resd 1
	.time:		resd 1
	.pt:		resb POINT.size
	.size:
endstruc

struc INITCOMMONCONTROLSEX
	.dwSize:	resd 1
	.dwICC:		resd 1
	.size:
endstruc

struc TOOLINFO
	.cbSize:		resd 1
	.uFlags:		resd 1
	.hwnd:			resd 1
	.uId:			resd 1
	.rect:			resd 4
	.hinst:			resd 1
	.lpszText:		resd 1
	.lParam:		resd 1
	.size:
endstruc
