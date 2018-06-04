; calculator written in assembly language
; This file is provided as is with no warranty of any kind. Use at your own risk.
%include "win32.inc"

;global _main

[section .text]

proc _main
		stdcall initApp

		invoke	GetModuleHandle, null
		mov		edi, gMainDlg
		stdcall	MainDialog_Create, eax

		;~ jmp		MainDialog_InputEditProc	; make ollydbg recognize instructions in this proc
		;~ stdcall	uninitApp
		invoke	ExitProcess, _0
    ;return
endp

%include "evaluator.inc"
%include "maindlg.inc"

[section .text]

%macro	testcase_evaluate 4
	[section .rdata]
	%%testval:	dt %4
	%%str:	unicode	%1
	%%str_length	equ ($ - %%str) / word.size
	dw	0
	[section .text]
	stdcall	Evaluate, %%str, %%str_length, %2, gTmpFloat
	%if %3 != 0
		jne		%%testcase_fail
		fld		tword[gTmpFloat]
		fld		st0
		stdcall	floatToString, TestCase_Text
		fld		tword[%%testval]		; testval val
		fsubp	st1						; val - testval
		fabs							; fabs(val - testval)
		fld		tword[ConstEpsilon]		; Const_Epsilon fabs(val - testval)
		fcompp							; compare and clear registers
		fstsw	ax						; copy fpu flags to cpu flags
		sahf
		ja		%%completed
	%else
		jne		%%completed
		fstp	tword[gTmpFloat]
	%endif
	%%testcase_fail:
		invoke	MessageBox, null, %%str, "fail", _0
	%%completed:
%endm

proc initApp
	;enter
		xor     ebx, ebx        ; ebx = null

		;~ invoke	InitCommonControlsEx, InitCtrls
		invoke	InitCommonControls

		finit
		%ifdef DEBUG
			; test cases
			testcase_evaluate "round(1.6)", ANGLE_MODE_DEGREES, 1, 2.0
			testcase_evaluate "int(1.6)", ANGLE_MODE_DEGREES, 1, 1.0
			testcase_evaluate "acoth(1.1)", ANGLE_MODE_DEGREES, 1, 1.5222612188617114980
			testcase_evaluate "acoth(0.1)", ANGLE_MODE_DEGREES, 0, 0.0
			testcase_evaluate "acot(0.1)", ANGLE_MODE_DEGREES, 1, 84.289406862500357490
			testcase_evaluate "cot(10)", ANGLE_MODE_DEGREES, 1, 5.6712818196177095210
			testcase_evaluate "acosh(0.1)", ANGLE_MODE_DEGREES, 0, 0.0
			testcase_evaluate "hex(abba)", ANGLE_MODE_DEGREES, 1, 43962.0
			testcase_evaluate "oct(777)", ANGLE_MODE_DEGREES, 1, 511.0
			testcase_evaluate "bin(1110)", ANGLE_MODE_DEGREES, 1, 14.0
			testcase_evaluate "div(111, 100)", ANGLE_MODE_DEGREES, 1, 1.0
			testcase_evaluate "mod(111, 100)", ANGLE_MODE_DEGREES, 1, 11.0
			testcase_evaluate "logn(10, 100)", ANGLE_MODE_DEGREES, 1, 2.0
			testcase_evaluate "(-1.000001)^777777", ANGLE_MODE_DEGREES, 1, -2.1766273923175944230
			testcase_evaluate "(-1.000001)^6666666", ANGLE_MODE_DEGREES, 1, 785.76885114786196250
			testcase_evaluate "root(8, 3)", ANGLE_MODE_DEGREES, 1, 2.0
			testcase_evaluate "33%", ANGLE_MODE_DEGREES, 1, 0.33
			testcase_evaluate "(4/0)", ANGLE_MODE_DEGREES, 0, 0.0
			testcase_evaluate "asin(1)", ANGLE_MODE_GRADS, 1, 99.999999999999999990	; 1.5707963267948966190
			testcase_evaluate "0.001", ANGLE_MODE_DEGREES, 1, 0.001
			testcase_evaluate "2^77777", ANGLE_MODE_DEGREES, 0, 0.0
			testcase_evaluate "2 + 2 * 2", ANGLE_MODE_DEGREES, 1, 6.0
			testcase_evaluate "sin(pi)", ANGLE_MODE_RADIANS, 1, 0.0
			testcase_evaluate "0.000001^0.000001", ANGLE_MODE_DEGREES, 1, 0.9999861845848757623
			testcase_evaluate "euler", ANGLE_MODE_DEGREES, 1, 2.7182818284590452354
			testcase_evaluate "pi", ANGLE_MODE_DEGREES, 1, 3.1415926535897932380
			testcase_evaluate "2 + 2 * 2", ANGLE_MODE_DEGREES, 1, 6.0
			testcase_evaluate "(2 + 2) * 2", ANGLE_MODE_DEGREES, 1, 8.0
			testcase_evaluate "3*2*-12345.5555555e-1", ANGLE_MODE_DEGREES, 1, -7407.3333333
			testcase_evaluate "-12345.5555555e-1*2*3", ANGLE_MODE_DEGREES, 1, -7407.3333333
			testcase_evaluate "0.0000000000000000000000000000000000001e-4900", ANGLE_MODE_DEGREES, 0, 0.
			testcase_evaluate "-1000e4931", ANGLE_MODE_DEGREES, 0, 0.0
			testcase_evaluate "2-", ANGLE_MODE_DEGREES, 0, 0.0
			testcase_evaluate "12345.", ANGLE_MODE_DEGREES, 1, 12345.0
			testcase_evaluate "12345.555", ANGLE_MODE_DEGREES, 1, 12345.555
			testcase_evaluate "-1.3434e-2", ANGLE_MODE_DEGREES, 1, -1.3434e-2
			testcase_evaluate " -2 -1 ", ANGLE_MODE_DEGREES, 1, -3.0
		%endif
	;leave
	return
endp

;~ proc uninitApp
		;~ ret
;~ endp

[section .data]
;~ InitCtrls:			istruc INITCOMMONCONTROLSEX
	;~ at INITCOMMONCONTROLSEX.dwSize
					;~ dd INITCOMMONCONTROLSEX.size
					;~ dd ICC_WIN95_CLASSES
					;~ iend

[section .bss]
gMainDlg:			resb MainDialog.size	; main dialog instance

%ifdef DEBUG
	[section .bss]
	gTmpFloat:			rest 1
	TestCase_Text:		resw 128
%endif
