; Main dialog unit
; This file is provided as is with no warranty of any kind. Use at your own risk.
; definitions from resource.h
IDD_DLGMAIN				equ 104
IDR_ACCELERATOR			equ 101
IDM_SELECTALL       	equ 200
IDM_EXIT           		equ 201
IDC_EDITINPUT			equ 40000
IDC_DEGREES				equ 40001
IDC_RADIANS				equ 40002
IDC_GRADS				equ 40003
IDC_COMBOHISTORY		equ 40004
IDC_EDITRESULT			equ 40005
IDC_BUTTONADDHISTORY	equ 40006
IDC_BUTTON0				equ 40007
IDC_BUTTONMINUS			equ 40008
IDC_BUTTONPLUS			equ 40009
IDC_BUTTONCALC			equ 40010
IDC_BUTTONMULTIPLY		equ 40011
IDC_BUTTONDIVIDE		equ 40012
IDC_BUTTON1				equ 40013
IDC_BUTTON2				equ 40014
IDC_BUTTON3				equ 40015
IDC_BUTTON4				equ 40016
IDC_BUTTON5				equ 40017
IDC_BUTTON6				equ 40018
IDC_BUTTON7				equ 40019
IDC_BUTTON8				equ 40020
IDC_BUTTON9				equ 40021
IDC_BUTTONSIN			equ 40022
IDC_BUTTONCOS			equ 40023
IDC_BUTTONTAN			equ 40024
IDC_BUTTONDIV			equ 40025
IDC_BUTTONABS			equ 40026
IDC_BUTTONXPOWY			equ 40027
IDC_BUTTONLG			equ 40028
IDC_BUTTONLN			equ 40029
IDC_BUTTONEXP			equ 40030
IDC_BUTTONSQRT			equ 40031
IDC_BUTTONPERC			equ 40032
IDC_BUTTONASIN			equ 40033
IDC_BUTTONACOS			equ 40034
IDC_BUTTONATAN			equ 40035
IDC_BUTTONSINH			equ 40036
IDC_BUTTONCOSH			equ 40037
IDC_BUTTONTANH			equ 40038
IDC_BUTTONASINH			equ 40039
IDC_BUTTONACOSH			equ 40040
IDC_BUTTONATANH			equ 40041
IDC_BUTTONROOT			equ 40042
IDC_BUTTONXPOW3			equ 40043
IDC_BUTTONXPOW2			equ 40044
IDC_BUTTONLOGN			equ 40045
IDC_BUTTONLOG2			equ 40046
IDC_BUTTONLOG10			equ 40047
IDC_BUTTONPI			equ 40048
IDC_BUTTONEULER			equ 40049
IDC_BUTTONCLEAR			equ 40050
IDC_BUTTONBRACKET1		equ 40051
IDC_BUTTONBRACKET2		equ 40052
IDC_BUTTONBRACKETS		equ 40053
IDC_BUTTONBIN			equ 40054
IDC_BUTTONOCT			equ 40055
IDC_BUTTONHEX			equ 40056
IDC_BUTTONMOD			equ 40057
IDC_BUTTONCOPY			equ 40058
IDC_BUTTONBITAND		equ 40059
IDC_BUTTONBITOR			equ 40060
IDC_BUTTONBITXOR		equ 40061
IDC_BUTTONBITNOT		equ 40062
IDC_BUTTONBITLEFT		equ 40063
IDC_BUTTONBITRIGHT		equ 40064
IDC_BUTTONPOINT			equ 40065
IDC_BUTTONCOT			equ 40066
IDC_BUTTONACOT			equ 40067
IDC_BUTTONCOTH			equ 40068
IDC_BUTTONACOTH			equ 40069
IDC_BUTTONINT			equ 40070
IDC_BUTTONROUND			equ 40071
IDC_BUTTONCOUNT			equ	(1 + IDC_BUTTONROUND - IDC_BUTTONADDHISTORY)
;IDC_STATICHINT			equ 40072

; main dialog structure
struc MainDialog
	.Instance:		resd 1
	.Window:		resd 1
	.EditInput:		resd 1
	.EditResult:	resd 1
	.RadioDegrees:	resd 1	; these 3 radios should be groupped
	.RadioRadians:	resd 1
	.RadioGrads:	resd 1
	.ComboHistory:	resd 1
	.InputEditProc:	resd 1
	.Evaluated:		resd 1
	.size:
endstruc

MAX_HISTORY_COUNT	equ	20

[section .text]
; ebx == null, edi == this
proc MainDialog_Create, .Instance
	enter	.uxtheme,\
			.accelerator resd 1,\
			.message: resb MSG.size
		invoke	LoadLibrary, "uxtheme.dll"
		test	eax, eax
		je		.noUxTheme
		mov		[.uxtheme], eax
		invoke	GetProcAddress, eax, .SetThemeAppProperties
		test	eax, eax
		je		.noUxTheme
		invoke	eax, STAP_ALLOW_NONCLIENT | STAP_ALLOW_CONTROLS | STAP_ALLOW_WEBCONTENT
	.noUxTheme:
		mov		eax, [.Instance]
		mov		[edi + MainDialog.Instance], eax
		;~ invoke	DialogBoxParam, [.Instance], IDD_DLGMAIN, null, MainDialog_DialogProc, edi
		invoke	LoadAccelerators, eax, IDR_ACCELERATOR
		mov		[.accelerator], eax
		; should be created via CreateDialog for dpi awareness to work
		invoke	CreateDialogParam, [edi + MainDialog.Instance], IDD_DLGMAIN, null, MainDialog_DialogProc, edi
		lea		esi, [.message]
	.messageLoop:
		invoke	GetMessage, esi, null, _0, _0
		test	eax, eax
		je		.quitMessageLoop
			invoke	TranslateAccelerator, [edi + MainDialog.Window], [.accelerator], esi
			test	eax, eax
			jne		.messageLoop
				invoke	IsDialogMessage, [edi + MainDialog.Window], esi
				test	eax, eax
				jne		.messageLoop
					invoke	TranslateMessage, esi
					invoke	DispatchMessage, esi
					jmp		.messageLoop
	.quitMessageLoop:
		invoke	FreeLibrary, [.uxtheme]
	leave
	return
	.SetThemeAppProperties	db "SetThemeAppProperties", 0
endp
; input edit procedure
proc MainDialog_InputEditProc, .Window, .Message, .WordParam, .LongParam
	enter
		cmp		dword[.Message], WM_CHAR
		jne		.notWmChar
			movzx	eax, word[.WordParam]
			cmp		word[.WordParam], '='
			je		.return_0
	.notWmChar:
		stdcall	GetWindowLongPtr, [.Window], GWLP_USERDATA

		leave
		jmp		[eax + MainDialog.InputEditProc]
.return_0:
	leave
	xor		eax, eax
	return
endp
; ebx == null, edi == this
proc MainDialog_onInitDialog
	enter	.toolInfo resb TOOLINFO.size
		invoke	GetDlgItem, esi, IDC_EDITINPUT
		mov		[edi + MainDialog.EditInput], eax
		invoke	GetDlgItem, esi, IDC_EDITRESULT
		mov		[edi + MainDialog.EditResult], eax
		invoke	GetDlgItem, esi, IDC_DEGREES
		mov		[edi + MainDialog.RadioDegrees], eax
		invoke	GetDlgItem, esi, IDC_RADIANS
		mov		[edi + MainDialog.RadioRadians], eax
		invoke	GetDlgItem, esi, IDC_GRADS
		mov		[edi + MainDialog.RadioGrads], eax
		invoke	GetDlgItem, esi, IDC_COMBOHISTORY
		mov		[edi + MainDialog.ComboHistory], eax
		; check degrees radio
		invoke	SendMessage, [edi + MainDialog.RadioDegrees], BM_SETCHECK, BST_CHECKED, _0
		; replace input edit proc
		invoke	SetWindowLongPtr, [edi + MainDialog.EditInput], GWLP_USERDATA, edi
		invoke	SetWindowLongPtr, [edi + MainDialog.EditInput], GWLP_WNDPROC, MainDialog_InputEditProc
		mov		[edi + MainDialog.InputEditProc], eax
		; create tooltips
		mov		edx, .hints
		mov		ecx, .hints.length
	.hintsLoop:
		push	ecx, edx
			push	dword TOOLINFO.size
			pop		dword[.toolInfo + TOOLINFO.cbSize]
			mov		[.toolInfo + TOOLINFO.hwnd], esi
			push	dword (TTF_IDISHWND | TTF_SUBCLASS)
			pop		dword[.toolInfo + TOOLINFO.uFlags]
			push	dword[edx + ecx * dword.size - dword.size]
			pop		dword[.toolInfo + TOOLINFO.lpszText]
			add		ecx, IDC_BUTTON0 - 1
			invoke	GetDlgItem, esi, ecx
			mov		[.toolInfo + TOOLINFO.uId], eax
			invoke	CreateWindowEx, _0, TOOLTIPS_CLASS, null, WS_POPUP | TTS_ALWAYSTIP | TTS_BALLOON,\
					CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, esi, null,\
					dword[edi + MainDialog.Instance], null
			test	eax, eax
			je		.tooltipFail
			lea		ecx, [.toolInfo]
			invoke	SendMessage, eax, TTM_ADDTOOL, _0, ecx;
		.tooltipFail:
		pop		edx, ecx
		loop	.hintsLoop
		; load icon and set it to main window
		invoke	LoadIcon, null, IDI_APPLICATION
		push	eax, eax									; push two 4th parameters on stack
		invoke	SendMessage, esi, WM_SETICON, ICON_SMALL	; 4th param already on stack
		invoke	SendMessage, esi, WM_SETICON, ICON_BIG		; 4th param already on stack
		; load history
		stdcall	MainDialog_LoadHistory
	leave
	return
; depends on controls identifiers
wcstbl	.hints,\
		"0",\
		"-",\
		"+",\
		"calculates expression and replaces it with the result",\
		"*",\
		"/",\
		"1",\
		"2",\
		"3",\
		"4",\
		"5",\
		"6",\
		"7",\
		"8",\
		"9",\
		"sin(angle), depends on angle mode",\
		"cos(angle), depends on angle mode",\
		"tan(angle), depends on angle mode",\
		"div(value1, value2), calculates int(value1 / value2)",\
		"abs(value), absolute value",\
		"x^y, exponentiation",\
		"lg(value), logarithm with base 10, value > 0",\
		"ln(value), logarithm with base 2.71828182845904524, value > 0",\
		"exp(value), euler ^ val",\
		"sqrt(value), square root, val >= 0",\
		"12% = 0.12",\
		"asin(value), depends on angle mode",\
		"acos(value), depends on angle mode",\
		"atan(value), depends on angle mode",\
		"sinh(value), hyperbolic sine",\
		"cosh(value), hyperbolic cosine",\
		"tanh(value), hyperbolic tangent",\
		"asinh(value), area hyperbolic sine",\
		"acosh(value), area hyperbolic cosine",\
		"atanh(value), area hyperbolic tangent",\
		"root(value, power)",\
		"x^3",\
		"x^2",\
		"logN(N, value)",\
		"log2(value), logarithm with base 2",\
		"lg(value), logarithm with base 10, value > 0",\
		{"Archimedes", 27h, " constant, 3.14159265358979324..."},\
		{"Euler", 27h, "s number, 2.71828182845904524..."},\
		"clear expression",\
		"(",\
		")",\
		"()",\
		"bin(32 bit binary number)",\
		"oct(32 bit octal number)",\
		"hex(32 bit hexadecimal number)",\
		"mod(value1, value2), calculates value1 - value2 * int(value1 / value2) ",\
		"copy to clipboard",\
		"bitand(value1, value2), bit AND of two 32 bit integer values",\
		"bitor(value1, value2), bit OR of two 32 bit integer values",\
		"bitxor(value1, value2), bit XOR of two 32 bit integer values",\
		"bitnot(value), bit NOT of 32 bit integer value",\
		"bitleft(value, shift), bit shift left of 32 bit integer value",\
		"bitright(value, shift), bit shift right of 32 bit integer value",\
		"separates integer and fractional parts",\
		"cot(angle), cotangent, depends on angle mode",\
		"acot(value), arccotangent, depends on angle mode",\
		"coth(value), hyperbolic cotangent",\
		"acoth(value), area hyperbolic cotangent",\
		"int(value), integer part",\
		"round(value), rounds number to nearest integer"
endp

; ebx == null, edi == this
proc MainDialog_onExit
		; restore window proc
		invoke	SetWindowLongPtr, [edi + MainDialog.EditInput], GWLP_WNDPROC, [edi + MainDialog.InputEditProc]
		; end dialog
		invoke	DestroyWindow, [edi + MainDialog.Window]
	return
endp

; ebx == null, edi == this
proc MainDialog_onEnChange, .Id
	enter
		mov		eax, [.Id]
		cmp		ax, IDC_EDITINPUT
		jne		.notEditInput
			stdcall	MainDialog_Recalculate
	.notEditInput:
	leave
	return
endp

proc MainDialog_GetAngleMode
	xor		eax, eax
	push	eax
		invoke	SendMessage, [edi + MainDialog.RadioDegrees], BM_GETCHECK, _0, _0
		test	al, BST_CHECKED
	pop		eax
	jne		.angleModeRetrieved
		inc		eax
		push	eax
			invoke	SendMessage, [edi + MainDialog.RadioRadians], BM_GETCHECK, _0, _0
			test	al, BST_CHECKED
		pop		eax
		jne		.angleModeRetrieved
			inc		eax
	.angleModeRetrieved:
	ret
endp

; calculates expression and prints result into result field
proc MainDialog_Recalculate
	enter	.angleMode resd 1,\
			.textLength resd 1,\
			.textSize resd 1,\
			.textPtr resd 1,\
			.result rest 1
		mov		[edi + MainDialog.Evaluated], _0
		; find out angle mode
		stdcall	MainDialog_GetAngleMode
		mov		[.angleMode], eax
		; set result text to empty string
		invoke	SetWindowText, [edi + MainDialog.EditResult], ""
		; get text length from input field
		;mov		esi, [edi + MainDialog.EditInput]	; would save 1 byte, but reduce readability and overwrite esi
		invoke	GetWindowTextLength, [edi + MainDialog.EditInput]
		test	eax, eax
		je		.leave
		mov		[.textLength], eax
		inc		eax		; eax += 1;
		shl		eax, 1	; eax *= word.size;
		mov		[.textSize], eax
		; allocate memory for input expression and result
		invoke	VirtualAlloc, null, eax, MEM_COMMIT, PAGE_READWRITE
		test	eax,eax
		je		.leave
			mov		[.textPtr], eax
			mov		ecx, [.textLength]
			inc		ecx
			; get text length from input field
			invoke	GetWindowText, [edi + MainDialog.EditInput], [.textPtr], ecx
			test	eax, eax
			je		.free_mem
				; evaluate expression
				lea		ecx, [.result]
				stdcall	Evaluate, [.textPtr], [.textLength], [.angleMode], ecx
				je		.noError
					; set result field to error
					invoke	SetWindowText, [edi + MainDialog.EditResult], eax
					jmp		.free_mem
				.noError:
					; if there's not error set result field to result
					fld		tword[.result]
					stdcall	floatToString, [.textPtr]
					invoke	SetWindowText, [edi + MainDialog.EditResult], [.textPtr]
					inc		dword[edi + MainDialog.Evaluated]
	.free_mem:
			; free memory
			invoke	VirtualFree, [.textPtr], _0, MEM_RELEASE	; [.textSize]
.leave:
	leave
	return
endp

[section .rdata]
RegKeyNameStr:	unicode "Software\AsmCalc", 0
Key1NameStr:	unicode "HistoryText", 0
Key1NameLen		equ (($ - Key1NameStr) / word.size - 1)
Key2NameStr:	unicode "HistoryAngleMode", 0
Key2NameLen		equ (($ - Key2NameStr) / word.size - 1)

[section .text]
; ebx == null, edi == this
MAX_KEY_LENGTH			equ 48
MEMORY_ALLOC_INCREMENT	equ	4096
proc MainDialog_LoadHistory
	enter	.key resd 1,\
			.index resd 1,\
			.type resd 1,\
			.item resd 1,\
			.angleMode resd 1,\
			.allocSize resd 1,\
			.bytesRead resd 1,\
			.keyName resw MAX_KEY_LENGTH
	push	esi
		mov		[.index], _0
		lea		eax, [.key]
		invoke	RegCreateKeyEx, HKEY_CURRENT_USER, RegKeyNameStr, _0, null, _0, KEY_READ, null, eax, null
		test	eax, eax				; ERROR_SUCCESS == 0
		jne		.leave
		;
	.queryLoop:
		lea		eax, [.keyName]
		invoke	lstrcpy, eax, Key1NameStr
		push	edi
			lea		edi, [.keyName + Key1NameLen * word.size]
			mov		eax, [.index]
			stdcall	printInteger
		pop		edi

		mov		[.allocSize], _0
		xor		esi, esi
	.tryQuery:
		add		dword[.allocSize], MEMORY_ALLOC_INCREMENT
		invoke	VirtualAlloc, null, [.allocSize], MEM_COMMIT, PAGE_READWRITE
		test	eax, eax
		je		.queryFail
			mov		esi, eax
			lea		eax, [.keyName]
			lea		ecx, [.type]
			push	REG_SZ
			pop		dword[ecx]
			lea		edx, [.bytesRead]
			push	dword[.allocSize]
			pop		dword[edx]
			invoke	RegQueryValueEx, [.key], eax, _0, ecx, esi, edx
			test	eax, eax
			jne		.queryEnd
			; query success
			invoke	SendMessage, [edi + MainDialog.ComboHistory], CB_ADDSTRING, _0, esi
			mov		[.item], eax
			lea		eax, [.keyName]
			invoke	lstrcpy, eax, Key2NameStr
			push	edi
				lea		edi, [.keyName + Key2NameLen * word.size]
				mov		eax, [.index]
				stdcall	printInteger
			pop		edi
			lea		edx, [.bytesRead]
			push	dword.size
			pop		dword[edx]
			push	edx											; place 6th parameter on stack
			lea		edx, [.angleMode]
			lea		eax, [.keyName]
			lea		ecx, [.type]
			push	REG_DWORD
			pop		dword[ecx]
			invoke	RegQueryValueEx, [.key], eax, _0, ecx, edx	; 6th parameter already on stack
			test	eax, eax
			jne		.noAngleMode
				invoke	SendMessage, [edi + MainDialog.ComboHistory], CB_SETITEMDATA, [.item], [.angleMode]
		.noAngleMode:
			xor		eax, eax	; break cycle
		.queryEnd
			push	eax
			invoke	VirtualFree, esi, _0, MEM_RELEASE
			pop		eax
			cmp		eax, ERROR_MORE_DATA
			je		.tryQuery
	.queryFail:
		inc		dword[.index]
		cmp		dword[.index], MAX_HISTORY_COUNT
		jb		.queryLoop
		; close key
		invoke	RegCloseKey, [.key]
.leave:
	pop		esi
	leave
	return
endp

; ebx == null, edi == this
proc MainDialog_SaveHistory
	enter	.key resd 1,\
			.selection resd 1,\
			.textSize resd 1,\
			.angleMode resd 1,\
			.keyName resw MAX_KEY_LENGTH
	push	esi
		lea		eax, [.key]
		invoke	RegCreateKeyEx, HKEY_CURRENT_USER, RegKeyNameStr, _0, null, _0, KEY_WRITE, null, eax, null
		test	eax, eax				; ERROR_SUCCESS == 0
		jne		.leave
		mov		[.selection], _0
	.saveLoop:
		invoke	SendMessage, [edi + MainDialog.ComboHistory], CB_GETLBTEXTLEN, [.selection], _0
		inc		eax
		je		.closeKey
		shl		eax, 1					; eax *= word.size
		mov		[.textSize], eax
		invoke	VirtualAlloc, null, eax, MEM_COMMIT, PAGE_READWRITE
		test	eax, eax
		je		.closeKey
			mov		esi, eax
			invoke	SendMessage, [edi + MainDialog.ComboHistory], CB_GETLBTEXT, [.selection], esi
			inc		eax
			je		.noText
			lea		eax, [.keyName]
			invoke	lstrcpy, eax, Key1NameStr
			push	edi
				lea		edi, [.keyName + Key1NameLen * word.size]
				mov		eax, [.selection]
				stdcall	printInteger
			pop		edi
			lea		eax, [.keyName]
			mov		ecx, [.textSize]
			invoke	RegSetValueEx, [.key], eax, _0, REG_SZ, esi, ecx
			; print second name
			lea		eax, [.keyName]
			invoke	lstrcpy, eax, Key2NameStr
			push	edi
				lea		edi, [.keyName + Key2NameLen * word.size]
				mov		eax, [.selection]
				stdcall	printInteger
			pop		edi
			invoke	SendMessage, [edi + MainDialog.ComboHistory], CB_GETITEMDATA, [.selection], _0
			lea		ecx, [.angleMode]
			mov		[ecx], eax
			inc		eax
			je		.noText
			lea		eax, [.keyName]
			invoke	RegSetValueEx, [.key], eax, _0, REG_DWORD, ecx, dword.size
		.noText:
		.releaseMemory:
			invoke	VirtualFree, esi, _0, MEM_RELEASE
		inc		dword[.selection]
		jmp		.saveLoop
	.closeKey:
		invoke	RegCloseKey, [.key]
.leave:
	pop		esi
	leave
	return
endp

; ebx == null, edi == this
proc MainDialog_onAddHistory
	enter	.textLength resd 1
		cmp		[edi + MainDialog.Evaluated], _0
		je		.notEvaluated
			mov		[.textLength], _0
			invoke	GetWindowTextLength, [edi + MainDialog.EditInput]
			push	eax
			invoke	GetWindowTextLength, [edi + MainDialog.EditResult]
			pop		edx
			lea		eax, [edx + eax + 4]		; add 4 wchars for ' = ' and zero at end
			mov		[.textLength], eax
			shl		eax, 1						; eax *= word.size
			invoke	VirtualAlloc, null, eax, MEM_COMMIT, PAGE_READWRITE
			test	eax, eax
			je		.noMemoryAllocated
				mov		esi, eax				; preserve pointer to memory block
				invoke	GetWindowText, [edi + MainDialog.EditInput], esi, [.textLength]
				;invoke	lstrlen, esi
				lea		edx, [esi + eax * word.size]
				; concatenate ' = ' to result
				mov		dword[edx], 3d0020h		; ' ='
				mov		word[edx + 2 * word.size], ' '
				lea		edx, [edx + 3 * word.size]
				add		eax, 3
				sub		eax, [.textLength]
				neg		eax
				invoke	GetWindowText, [edi + MainDialog.EditResult], edx, eax
				invoke	SendMessage, [edi + MainDialog.ComboHistory], CB_INSERTSTRING, _0, esi
				inc		eax						; CB_ERR = -1
				je		.releaseMemory
				invoke	SendMessage, [edi + MainDialog.ComboHistory], CB_SETCURSEL, _0, _0
				stdcall	MainDialog_GetAngleMode	; attach angle mode to history
				invoke	SendMessage, [edi + MainDialog.ComboHistory], CB_SETITEMDATA, _0, eax
			.releaseMemory:						; release string memory
				invoke	VirtualFree, esi, _0, MEM_RELEASE
				invoke	SendMessage, [edi + MainDialog.ComboHistory], CB_DELETESTRING, MAX_HISTORY_COUNT, _0
		.noMemoryAllocated:
	.notEvaluated
		stdcall	MainDialog_SaveHistory
	leave
	return
endp

; ebx == null, edi == this
proc MainDialog_onCbnSelChange, .Id
	enter	.selection resd 1, .textLength resd 1
		cmp		dword[.Id], IDC_COMBOHISTORY
		jne		.notComboHistory
		invoke	SendMessage, [edi + MainDialog.ComboHistory], CB_GETCURSEL, _0, _0
		mov		[.selection], eax
		inc		eax
		je		.notComboHistory
		invoke	SendMessage, [edi + MainDialog.ComboHistory], CB_GETLBTEXTLEN, [.selection], _0
		inc		eax					; CB_ERR == -1
		je		.notComboHistory	; if eax == CB_ERR, fail
		mov		[.textLength], eax	; text length increased by 1 (terminating zero)
		shl		eax, 1				; eax *= word.size
		invoke	VirtualAlloc, null, eax, MEM_COMMIT, PAGE_READWRITE
		test	eax, eax
		je		.notComboHistory
			mov		esi, eax
			invoke	SendMessage, [edi + MainDialog.ComboHistory], CB_GETLBTEXT, [.selection], esi
			mov		edx, eax
			;~ inc		eax				; CB_ERR == -1
			;~ je		.releaseMemory
			xor		ecx, ecx
		.findEq:
			mov		eax, dword[esi + ecx * 2]
			cmp		eax, 3d0020h	;' ='
			je		.foundEq
			inc		ecx
			cmp		ecx, edx
			jb		.findEq
			jmp		.releaseMemory	; do nothing if not found
		.foundEq:
			mov		[esi + ecx * 2], _0
			stdcall	SendMessage, [edi + MainDialog.EditInput], EM_SETSEL, _0, -1		; 0, -1 - select all text
			stdcall	SendMessage, [edi + MainDialog.EditInput], EM_REPLACESEL, 1, esi	; 1 = replacement can be undone
			; restore angle mode
			stdcall	SendMessage, [edi + MainDialog.ComboHistory], CB_GETITEMDATA, [.selection], _0
			lea		edx, [edi + MainDialog.RadioDegrees]
			movzx	eax, word[edx + eax * 4]
			stdcall	SendMessage, eax, BM_SETCHECK, BST_CHECKED, _0
			stdcall	MainDialog_Recalculate
		.releaseMemory:				; release string memory
			invoke	VirtualFree, esi, _0, MEM_RELEASE
	.notComboHistory:
	leave
	return
endp

MAX_RESULT_LENGTH	equ 48
proc MainDialog_onButtonPress, .Id
	enter	.selStart resd 1,\
			.resultEditLength resd 1,\
			.textLength resd 1,\
			.memoryHandle resd 1,\
			.resultStr resw MAX_RESULT_LENGTH
	push	eax, ecx
		mov		eax, [.Id]
		cmp		eax, IDM_SELECTALL
		jne		.notIdmSelectAll
			invoke	GetFocus
			cmp		eax, [edi + MainDialog.EditInput]
			je		.someEdit
			cmp		eax, [edi + MainDialog.EditResult]
			je		.someEdit
			jmp		.setFocus
		.someEdit:
			invoke	SendMessage, eax, EM_SETSEL, _0, -1	; select all text
			jmp		.setFocus
	.notIdmSelectAll:
		cmp		eax, IDM_EXIT
		jne		.notIdmExit
			stdcall	MainDialog_onExit
			jmp		.setFocus
	.notIdmExit:
		sub		eax, IDC_BUTTONADDHISTORY
		cmp		eax, IDC_BUTTONCOUNT
		jae		.done
		jmp		[.jmpTable + eax * dword.size]
		.buttonAddHistory:
			stdcall	MainDialog_onAddHistory
			jmp		.setFocus
		.buttonClear:
			invoke	SendMessage, [edi + MainDialog.EditInput], EM_SETSEL, _0, -1		; select all text
			invoke	SendMessage, [edi + MainDialog.EditInput], EM_REPLACESEL, 1, ""		; replacement can be undone
			jmp		.setFocus
		.buttonCalc:
			cmp		[edi + MainDialog.Evaluated], _0
			je		.setFocus
			lea		eax, [.resultStr]
			invoke	GetWindowText, [edi + MainDialog.EditResult], eax, MAX_RESULT_LENGTH
			invoke	SendMessage, [edi + MainDialog.EditInput], EM_SETSEL, _0, -1		; select all text
			lea		eax, [.resultStr]
			invoke	SendMessage, [edi + MainDialog.EditInput], EM_REPLACESEL, 1, eax	; replacement can be undone
			jmp		.setFocus
		.buttonCopy:	; should copy at least CF_TEXT format
			cmp		[edi + MainDialog.Evaluated], _0
			je		.setFocus
			invoke	GetWindowTextLengthA, [edi + MainDialog.EditInput]
			push	eax
			invoke	GetWindowTextLengthA, [edi + MainDialog.EditResult]
			pop		edx
			inc		eax
			mov		[.resultEditLength], eax
			lea		eax, [eax + edx + 3]
			mov		[.textLength], eax
			;shl		eax, 1	; eax *= word.size
			invoke	GlobalAlloc, GMEM_MOVEABLE, eax
			test	eax, eax
			je		.setFocus
				mov		[.memoryHandle], eax
				invoke	GlobalLock, eax
				test	eax, eax
				je		.freeMemory
					push	eax
					invoke	GetWindowTextA, [edi + MainDialog.EditInput], eax, [.textLength]
					pop		edx
					lea		edx, [edx + eax]; * word.size]
					mov		dword[edx], ' = '
					;~ mov		dword[edx], 3d0020h	; ' ='
					;~ mov		word[edx + dword.size], ' '
					lea		edx, [edx + 3]; * word.size]
					invoke	GetWindowTextA, [edi + MainDialog.EditInput], edx, [.resultEditLength]
					invoke	GlobalUnlock, [.memoryHandle]
					invoke	OpenClipboard, [edi + MainDialog.Window]
					test	eax, eax
					je		.clipboardFail
						invoke	EmptyClipboard
						invoke	SetClipboardData, CF_TEXT, [.memoryHandle]
						invoke	CloseClipboard
				.clipboardFail:
			.freeMemory:
				invoke	GlobalFree, [.memoryHandle]
			jmp		.setFocus
		.button0:
			push	_0
			push_unicode_string "0"
			jmp		.replaceSel
		.buttonMinus:
			push	_0
			push_unicode_string "-"
			jmp		.replaceSel
		.buttonPlus:
			push	_0
			push_unicode_string "+"
			jmp		.replaceSel
		.buttonMultiply:
			push	_0
			push_unicode_string "*"
			jmp		.replaceSel
		.buttonDivide:
			push	_0
			push_unicode_string "/"
			jmp		.replaceSel
		.buttonAsin:
			push	dword -1
			push_unicode_string "asin()"
			jmp		.replaceSel
		.button1:
			push	_0
			push_unicode_string "1"
			jmp		.replaceSel
		.button2:
			push	_0
			push_unicode_string "2"
			jmp		.replaceSel
		.button3:
			push	_0
			push_unicode_string "3"
			jmp		.replaceSel
		.button4:
			push	_0
			push_unicode_string "4"
			jmp		.replaceSel
		.button5:
			push	_0
			push_unicode_string "5"
			jmp		.replaceSel
		.button6:
			push	_0
			push_unicode_string "6"
			jmp		.replaceSel
		.button7:
			push	_0
			push_unicode_string "7"
			jmp		.replaceSel
		.button8:
			push	_0
			push_unicode_string "8"
			jmp		.replaceSel
		.button9:
			push	_0
			push_unicode_string "9"
			jmp		.replaceSel
		.buttonSin:
			push	dword -1
			push_unicode_string "sin()"
			jmp		.replaceSel
		.buttonCos:
			push	dword -1
			push_unicode_string "cos()"
			jmp		.replaceSel
		.buttonTan:
			push	dword -1
			push_unicode_string "tan()"
			jmp		.replaceSel
		.buttonCot:
			push	-1
			push_unicode_string "cot()"
			jmp		.replaceSel
		.buttonDiv:
			push	dword -2
			push_unicode_string "div(,)"
			jmp		.replaceSel
		.buttonAbs:
			push	dword -1
			push_unicode_string "abs()"
			jmp		.replaceSel
		.buttonXpowY:
			push	_0
			push_unicode_string "^"
			jmp		.replaceSel
		.buttonLg:
			push	dword -1
			push_unicode_string "lg()"
			jmp		.replaceSel
		.buttonLn:
			push	dword -1
			push_unicode_string "ln()"
			jmp		.replaceSel
		.buttonExp:
			push	dword -1
			push_unicode_string "exp()"
			jmp		.replaceSel
		.buttonSqrt:
			push	dword -1
			push_unicode_string "sqrt()"
			jmp		.replaceSel
		.buttonPerc:
			push	_0
			push_unicode_string "%"
			jmp		.replaceSel
		.buttonACos:
			push	dword -1
			push_unicode_string "acos()"
			jmp		.replaceSel
		.buttonATan:
			push	dword -1
			push_unicode_string "atan()"
			jmp		.replaceSel
		.buttonACot:
			push	-1
			push_unicode_string "acot()"
			jmp		.replaceSel
		.buttonSinh:
			push	dword -1
			push_unicode_string "sinh()"
			jmp		.replaceSel
		.buttonCosh:
			push	dword -1
			push_unicode_string "cosh()"
			jmp		.replaceSel
		.buttonTanh:
			push	dword -1
			push_unicode_string "tanh()"
			jmp		.replaceSel
		.buttonCoth:
			push	-1
			push_unicode_string "coth()"
			jmp		.replaceSel
		.buttonASinh:
			push	dword -1
			push_unicode_string "asinh()"
			jmp		.replaceSel
		.buttonACosh:
			push	dword -1
			push_unicode_string "acosh()"
			jmp		.replaceSel
		.buttonATanh:
			push	dword -1
			push_unicode_string "atanh()"
			jmp		.replaceSel
		.buttonACoth:
			push	-1
			push_unicode_string "acoth()"
			jmp		.replaceSel
		.buttonRoot:
			push	dword -2
			push_unicode_string "root(,)"
			jmp		.replaceSel
		.buttonXpow3:
			push	_0
			push_unicode_string "^3"
			jmp		.replaceSel
		.buttonXpow2:
			push	_0
			push_unicode_string "^2"
			jmp		.replaceSel
		.buttonLogN:
			push	dword -2
			push_unicode_string "logN(,)"
			jmp		.replaceSel
		.buttonLog2:
			push	dword -1
			push_unicode_string "log2()"
			jmp		.replaceSel
		.buttonLog10:
			push	dword -1
			push_unicode_string "lg()"
			jmp		.replaceSel
		.buttonPi:
			push	_0
			push_unicode_string "pi"
			jmp		.replaceSel
		.buttonEuler:
			push	_0
			push_unicode_string "euler"
			jmp		.replaceSel
		.buttonBracket1:
			push	_0
			push_unicode_string "("
			jmp		.replaceSel
		.buttonBracket2:
			push	_0
			push_unicode_string ")"
			jmp		.replaceSel
		.buttonBrackets:
			push	dword -1
			push_unicode_string "()"
			jmp		.replaceSel
		.buttonBin:
			push	dword -1
			push_unicode_string "bin()"
			jmp		.replaceSel
		.buttonOct:
			push	dword -1
			push_unicode_string "oct()"
			jmp		.replaceSel
		.buttonHex:
			push	dword -1
			push_unicode_string "hex()"
			jmp		.replaceSel
		.buttonMod:
			push	dword -2
			push_unicode_string "mod(,)"
			jmp		.replaceSel
		.buttonBitAnd:
			push	dword -2
			push_unicode_string "bitand(,)"
			jmp		.replaceSel
		.buttonBitOr:
			push	dword -2
			push_unicode_string "bitor(,)"
			jmp		.replaceSel
		.buttonBitXor:
			push	dword -2
			push_unicode_string "bitxor(,)"
			jmp		.replaceSel
		.buttonBitNot:
			push	dword -1
			push_unicode_string "bitnot()"
			jmp		.replaceSel
		.buttonBitLeft:
			push	dword -2
			push_unicode_string "bitleft(,)"
			jmp		.replaceSel
		.buttonBitRight:
			push	dword -2
			push_unicode_string "bitright(,)"
			jmp		.replaceSel
		.buttonPoint:
			push	_0
			push_unicode_string "."
			jmp		.replaceSel
		.buttonInt:
			push	-1
			push_unicode_string "int()"
			jmp		.replaceSel
		.buttonRound:
			push	-1
			push_unicode_string "round()"
			;~ jmp		.replaceSel
		.replaceSel:
			;~ invoke	SetWindowText, [edi + MainDialog.StaticHint]
			invoke	SendMessage, [edi + MainDialog.EditInput], EM_REPLACESEL, 1	; 1 = replacement can be undone
			lea		eax, [.selStart]
			invoke	SendMessage, [edi + MainDialog.EditInput], EM_GETSEL, eax, null
			pop		eax
			add		[.selStart], eax
			invoke	SendMessage, [edi + MainDialog.EditInput], EM_SETSEL, [.selStart], [.selStart]
		.setFocus:
			invoke	SetFocus, [edi + MainDialog.EditInput]
	.done:
	pop		ecx, eax
	leave
	return
align 4
; depends on controls identifiers
.jmpTable:	dd .buttonAddHistory, .button0, .buttonMinus, .buttonPlus, .buttonCalc, .buttonMultiply, .buttonDivide
			dd .button1, .button2, .button3, .button4, .button5, .button6, .button7, .button8, .button9
			dd .buttonSin, .buttonCos, .buttonTan, .buttonDiv, .buttonAbs, .buttonXpowY, .buttonLg, .buttonLn
			dd .buttonExp, .buttonSqrt, .buttonPerc, .buttonAsin, .buttonACos, .buttonATan
			dd .buttonSinh, .buttonCosh, .buttonTanh, .buttonASinh, .buttonACosh, .buttonATanh
			dd .buttonRoot, .buttonXpow3, .buttonXpow2, .buttonLogN, .buttonLog2, .buttonLog10
			dd .buttonPi, .buttonEuler, .buttonClear, .buttonBracket1, .buttonBracket2, .buttonBrackets
			dd .buttonBin, .buttonOct, .buttonHex, .buttonMod, .buttonCopy
			dd .buttonBitAnd, .buttonBitOr, .buttonBitXor, .buttonBitNot
			dd .buttonBitLeft, .buttonBitRight, .buttonPoint
			dd .buttonCot, .buttonACot, .buttonCoth, .buttonACoth, .buttonInt, .buttonRound
endp

; dialog proc
proc MainDialog_DialogProc, .Dialog, .Message, .WordParam, .LongParam
	enter
	push	ebx, esi, edi
		xor		ebx, ebx
		mov		esi, [.Dialog]
		
		invoke	GetWindowLongPtr, esi, GWLP_USERDATA
		mov		edi, eax

		mov		eax, [.Message]
		cmp		eax, WM_INITDIALOG
		jne		.notWmInitDialog
			mov		edi, [.LongParam]
			mov		[edi + MainDialog.Window], esi
			invoke	SetWindowLongPtr, esi, GWLP_USERDATA, edi
			stdcall	MainDialog_onInitDialog
			jmp		.return_true
	.notWmInitDialog:
		;~ cmp		eax, WM_GETDLGCODE
		;~ jne		.notWmGetDlgCode
			;~ mov		eax, DLGC_WANTCHARS;DLGC_WANTALLKEYS
			;~ jmp		.leave
	;~ .notWmGetDlgCode:
		test	edi, edi
		je		.return_false
			cmp		eax, WM_COMMAND
			jne		.notWmCommand
				mov		ecx, [.WordParam]
				mov		eax, ecx
				shr		eax, 16						; HIWORD(WordParam)
				cmp		eax, 1
				ja		.notCmd
					cmp		cx, IDCANCEL
					jne		.notIdmExit
						stdcall	MainDialog_onExit	; idcancel button handler
						jmp		.return_false
				.notIdmExit:
					movzx	edx, cx
					stdcall	MainDialog_onButtonPress, edx
			.notCmd:
				; test if notification from control
				cmp		ax, EN_CHANGE
				jne		.notEnChange
					movzx	edx, cx
					stdcall	MainDialog_onEnChange, edx
					jmp		.return_false
			.notEnChange:
				cmp		ax, CBN_SELCHANGE
				jne		.notCbnSelChange
					movzx	edx, cx
					stdcall	MainDialog_onCbnSelChange, edx
					jmp		.return_false
			.notCbnSelChange:
				cmp		ax, BN_CLICKED
				jne		.notBnClicked
					stdcall	MainDialog_Recalculate
					jmp		.return_false
			.notBnClicked:
					jmp		.return_false
		.notWmCommand:
			cmp		eax, WM_CLOSE
			jne		.notWmClose
				stdcall	MainDialog_onExit
				jmp		.return_true
		.notWmClose:
			cmp		eax, WM_DESTROY
			jne		.notWmDestroy
				invoke	PostQuitMessage, _0
		.notWmDestroy:
	.return_false:
		xor		eax, eax
.leave:
	pop		edi, esi, ebx
	leave
	return
.return_true:
	lea		eax, [_0 + 1]
	jmp		.leave
endp
