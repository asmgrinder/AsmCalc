; Expression evaluator in assembly language
; This file is provided as is with no warranty of any kind. Use at your own risk.

MAX_POWER			equ 7
MAX_DIGITS			equ 18

ANGLE_MODE_DEGREES	equ 0
ANGLE_MODE_RADIANS	equ 1
ANGLE_MODE_GRADS	equ 2

ROUND_NEAREST		equ (0 << 10)
ROUND_MINUSINFINITY	equ	(1 << 10)
ROUND_PLUSINFINITY	equ (2 << 10)
ROUND_ZERO			equ	(3 << 10)
ROUND_MASK			equ (3 << 10)

FPU_SR_ES			equ (1 << 7)	; general error flag
FPU_SR_SF			equ (1 << 6)	; stack error (overflow (c1 = 1) or antioverflow (c1 = 0))
FPU_SR_PE			equ (1 << 5)	; inexact result
FPU_SR_UE			equ (1 << 4)	; antioverflow (result too small)
FPU_SR_OE			equ (1 << 3)	; overflow
FPU_SR_ZE			equ (1 << 2)	; zero divide
FPU_SR_DE			equ (1 << 1)	; denormalized operand
FPU_SR_IE			equ (1 << 0)	; invalid operation

[section .text]

proc Evaluate, .ExpressionStr, .Length, .AngleMode, .ResultPtr
	enter	.dummy resd 1,\
			.percDivider resd 1,\
			.bitArg1 resd 1,\
			.bitArg2 resd 1,\
			.result1 rest 1,\
			.result2 rest 1;.power resd 1, 
		push	ecx, edx, esi, edi					; macro to push multiple ops
			cld
			fclex
			mov		esi, [.ExpressionStr]
			mov		ecx, [.Length]
			;xor		eax, eax
			; exclude starting spaces
		.skip_spaces_start:
			mov		eax, .noExpressionStr
			test	ecx, ecx
			je		.fail
			lodsw
			dec		ecx
			test	ax, ax
			jne		.notZero
				mov		eax, .wrongLineLengthStr
				jmp		.fail
		.notZero:
			cmp		ax, ' '
			jbe		.skip_spaces_start
			dec		esi								; step back one char
			dec		esi
			inc		ecx
			; exclude ending spaces
		.skip_spaces_end:
			mov		eax, .noExpressionStr
			test	ecx, ecx
			je		.fail
			dec		ecx
			mov		ax, [esi + ecx * word.size]
			test	ax, ax
			jne		.notZero2
				mov		eax, .wrongLineLengthStr
				jmp		.fail
		.notZero2:
			cmp		ax, ' '
			jbe		.skip_spaces_end
			inc		ecx
			mov		[.ExpressionStr], esi			; remember string without starting & ending spaces
			mov		[.Length], ecx					; local parameters copy won't affect caller's variables
			xor		edx, edx						; pass
			inc		edx
			inc		edx
		.pass_cycle:
				mov		esi, [.ExpressionStr]
				mov		ecx, [.Length]
				; recursively process operations
				xor		edi, edi					; brackets count
			.search_operation:						; go backward along string
				dec		ecx
				mov		ax, [esi + ecx * word.size]				; get char
				; switch (ax)
				cmp		ax, ')'						; case ')'
				jne		.not_closing_bracket
					inc		edi
					jmp		.continue_cycle
			.not_closing_bracket:
				cmp		ax, '('						; case '('
				jne		.not_opening_bracket
					test	edi, edi					; if outside of brackets (0 == edi) - error
					jne		.bracketBalanceOk
						mov		eax, .bracketsBalanceStr
						jmp		.fail
				.bracketBalanceOk:
					dec		edi
					jmp		.continue_cycle
			.not_opening_bracket:
				test	edi, edi						; if inside brackets
				jne		.continue_cycle					; don't test operations
					cmp		dl, 2
					jne		.notPass2
						cmp		ax, '-'					; case '-' (can be inside e-n)
						jne		.notMinus
						test	ecx, ecx
						jne		.notUnaryMinus
						push	edi
							lea		eax, [.result1]
							push	eax					; fourth parameter to Evaluate call
							lea		edi, [esi + word.size]
							mov		eax, [.Length]
							dec		eax
							stdcall	Evaluate, edi, eax, [.AngleMode]
						pop		edi
						jne		.fail
						fld		tword[.result1]
						fchs
						jmp		.done
					.notUnaryMinus:
						call	.testIfOpOk
						je		.notMinus
						call	.calculateSubexpressions
						jne		.fail
							fsubp	st1, st0
							call	.checkFpuState
							je		.done
							jmp		.failFcomp
					.notMinus:
						cmp		ax, '+'					; case '+' (can be inside e+n)
						jne		.notPlus
						call	.testIfOpOk
						je		.notPlus
						call	.calculateSubexpressions
						jne		.fail
							faddp	st1, st0
							call	.checkFpuState
							je		.done
							jmp		.failFcomp
					.notPlus:
						jmp		.continue_cycle
				.notPass2:
						test	edx, edx
						je		.notPass1
						cmp		ax, '/'					; case '/'
						jne		.notDivide
							call	.calculateSubexpressions
							jne		.fail
							ftst
							fstsw	ax
							sahf
							jne		.doDivide
								;fcompp
								mov		eax, .divisionByZeroStr
								jmp		.failFcompp
						.doDivide:
							fdivp	st1
							call	.checkFpuState
							je		.done
							jmp		.failFcomp
					.notDivide:
						cmp		ax, '*'					; case '*'
						jne		.notMultiply
						call	.calculateSubexpressions
						jne		.fail
							fmulp	st1
							call	.checkFpuState
							je		.done
							jmp		.failFcomp
					.notMultiply:
						jmp		.continue_cycle
				.notPass1:
						cmp		ax, '^'
						jne		.notPower
							call	.calculateSubexpressions
							jne		.fail	;_fcompp
							stdcall	power
							jne		.fail
							call	.checkFpuState
							je		.done
							;~ fxch							; if base == 0
							;~ ftst
							;~ fstsw	ax
							;~ sahf
							;~ jne		.notZeroBase
						;~ .resultEq1:
								;~ fcompp
								;~ fld1						; then result is 1
								;~ jmp		.done
						;~ .notZeroBase:
							;~ fabs
							;~ mov		eax, _0
							;~ ja		.basePositive
							;~ fxch
							;~ ftst							; base is negative
							;~ fstsw	ax
							;~ sahf
							;~ je		.resultEq1				; if power == 0
							;~ ; test if power is integer
							;~ fld		st0
							;~ stdcall	setRoundMode, ROUND_NEAREST
							;~ frndint
							;~ fsub	st0, st1
							;~ fabs
							;~ fld		tword[ConstEpsilon]
							;~ fcompp
							;~ fstsw	ax
							;~ sahf
							;~ mov		eax, .negativeBaseStr
							;~ jb		.failFcompp			; power is not integer and not very close to
							;~ frndint
							;~ fist	dword[.power]
							;~ mov		eax, [.power]
							;~ cmp		eax, _0
							;~ jg		.powerPositive
							;~ neg		eax
						;~ .powerPositive:
							;~ cmp		eax, 1000000000
							;~ mov		eax, .exponentTooBig
							;~ jae		.failFcompp
							;~ xor		eax, eax
							;~ fxch
							;~ test	byte[.power], 1
							;~ je		.basePositive
							;~ inc		eax
					;~ .basePositive:
							;~ fyl2x
							;~ call	pow2
							;~ test	eax, eax
							;~ je		.keepSign
							;~ fchs
						;~ .keepSign:
							;~ call	.checkFpuState
							;~ je		.done
							jmp		.failFcomp
					.notPower:						
			.continue_cycle:							; inner cycle
				test	ecx, ecx
				jne		.search_operation
			mov		eax, .bracketsBalanceStr
			test	edi, edi
			jne		.fail
			; pass cycle (outer cycle)
			dec		edx
			jns		.pass_cycle

			; if no operations outside brackets found try to strip brackets
			mov		esi, [.ExpressionStr]
			mov		ecx, [.Length]
			cmp		word[esi], '('
			jne		.read_func_or_number_or_const
			cmp		word[esi + ecx * word.size - word.size], ')'
			jne		.read_func_or_number_or_const
			inc		esi		; add esi, word.size
			inc		esi
			dec		ecx
			dec		ecx
			stdcall	Evaluate, esi, ecx, [.AngleMode], [.ResultPtr]
			je		.leave	; jump if success
			;~ dec		esi
			;~ dec		esi			
			;~ inc		ecx
			;~ inc		ecx
			jmp		.fail
			; if brackets strip fail the string can still be a number or constant
		.read_func_or_number_or_const:

			xor		edx, edx
			;mov		edx, .funcsList.length
		.testAllFuncs:
			stdcall	checkFuncName, dword[.funcsList + edx * dword.size]
			jne		.notEqualFunc
			push	edi
				lea		edi, [esi + eax * word.size]
				sub		eax, ecx
				neg		eax
				stdcall	Evaluate, edi, eax, [.AngleMode], [.ResultPtr]
			pop		edi
			jne		.fail
			mov		eax, [.ResultPtr]
			fld		tword[eax]
			jmp		[.jmpTable + edx * dword.size]; - dword.size]
		.bitnot:
			fistp	dword[.bitArg1]
			not		dword[.bitArg1]
			fild	dword[.bitArg1]
			jmp		.done
		.sin:
			stdcall	convertAngleToRadians, [.AngleMode]
			fsin
			call	.checkFpuState
			jne		.failFcomp
			jmp		.done
		.cos:
			stdcall	convertAngleToRadians, [.AngleMode]
			fcos
			call	.checkFpuState
			jne		.failFcomp
			jmp		.done
		.tan:
			stdcall	convertAngleToRadians, [.AngleMode]
			fptan
			fcomp	st1
			call	.checkFpuState
			jne		.failFcomp
			jmp		.done
		.asin:
			fld1
			fld		st1
			fmul	st0, st0
			fsubp	st1, st0
			fsqrt
			fpatan
			stdcall	convertRadiansToAngle, [.AngleMode]
			call	.checkFpuState
			jne		.failFcomp
			jmp		.done
		.acos
			fld1
			fld		st1
			fmul	st0, st0
			fsubp	st1, st0
			fsqrt
			fxch
			fpatan
			stdcall	convertRadiansToAngle, [.AngleMode]
			call	.checkFpuState
			jne		.failFcomp
			jmp		.done
		.atan:
			fld1
			fpatan
			stdcall	convertRadiansToAngle, [.AngleMode]
			call	.checkFpuState
			jne		.failFcomp
			jmp		.done
		.sqrt:
			ftst
			fstsw	ax
			sahf
			mov		eax, .negativeSqrtArgStr
			jb		.failFcomp
			fsqrt
			call	.checkFpuState
			jne		.failFcomp
			jmp		.done
		.abs:
			fabs
			call	.checkFpuState
			jne		.failFcomp
			jmp		.done
		.exp:
			stdcall	exp
			;~ fldl2e
			;~ fmulp	st1, st0
			;~ call	pow2
			call	.checkFpuState
			jne		.failFcomp
			jmp		.done
		.log10:
		.lg:
			ftst
			fstsw	ax
			sahf
			mov		eax, .negativeLogArgStr
			jb		.failFcomp
			fld1
			fldl2t
			fdivp	st1, st0
			fxch
			fyl2x
			call	.checkFpuState
			jne		.failFcomp
			jmp		.done
		.ln:
			ftst
			fstsw	ax
			sahf
			mov		eax, .negativeLogArgStr
			jb		.failFcomp
			fld1
			fldl2e
			fdivp	st1, st0
			fxch
			fyl2x
			call	.checkFpuState
			jne		.failFcomp
			jmp		.done
		.log2:
			ftst
			fstsw	ax
			sahf
			mov		eax, .negativeLogArgStr
			jb		.failFcomp
			fld1
			fxch
			fyl2x
			call	.checkFpuState
			jne		.failFcomp
			jmp		.done
		.sinh:
			fld		st0
			stdcall	exp
			fxch
			fchs
			stdcall	exp
			fsubp	st1, st0
			fld1
			fadd	st0, st0
			fdivp	st1, st0
			call	.checkFpuState
			jne		.failFcomp
			jmp		.done
		.cosh:
			fld		st0
			stdcall	exp
			fxch
			fchs
			stdcall	exp
			faddp	st1, st0
			fld1
			fadd	st0, st0
			fdivp	st1, st0
			call	.checkFpuState
			jne		.failFcomp
			jmp		.done
		.tanh:
			fld		st0
			stdcall	exp
			fxch	st1
			fchs
			stdcall	exp
			fxch
			fld		st1
			fsub	st0, st1
			fchs
			fxch	st2
			faddp	st1, st0
			fdivp	st1, st0
			call	.checkFpuState
			jne		.failFcomp
			jmp		.done
		.asinh:
			fld		st0
			fmul	st0, st0
			fld1
			faddp
			fsqrt
			faddp
			call	.checkFpuState
			jne		.failFcomp
			jmp		.ln
		.acosh:
			fld		st0
			fmul	st0, st0
			fld1
			fsubp
			ftst
			fstsw	ax
			sahf
			mov		eax, .negativeSqrtArgStr
			jb		.failFcompp
			fsqrt
			faddp
			call	.checkFpuState
			jne		.failFcomp
			jmp		.ln
		.atanh:
			fld1
			fld		st1
			fsubp
			fxch
			fld1
			faddp
			fxch
			fdivp	st1, st0
			; ln
			ftst
			fstsw	ax
			sahf
			mov		eax, .negativeLogArgStr
			jb		.failFcomp
			fld1
			fldl2e
			fdivp	st1, st0
			fxch
			fyl2x
			fld1
			fadd	st0, st0
			fdivp	st1, st0
			call	.checkFpuState
			jne		.failFcomp
			jmp		.done
		.notEqualFunc:
			inc		edx
			cmp		edx, .funcsList.length
			jb		.testAllFuncs

			; functions with 2 arguments
			xor		edx, edx
			;mov		edx, .funcsList.length
		.testAllFuncs2:
			stdcall	checkFuncName, dword[.funcsList2 + edx * dword.size]
			jne		.notEqualFunc2
			add		esi, eax					; word.size
			add		esi, eax
			sub		ecx, eax
			call	.calculateTwoArgs
			jne		.fail
			jmp		[.jmpTable2 + edx * dword.size]; - dword.size]
		.root:
			fld1
			fxch
			fdivp	st1, st0
			stdcall	power
			jne		.fail
			call	.checkFpuState
			jne		.failFcomp
			jmp		.done
		.logn:
			fld1
			fxch
			fyl2x
			fxch
			fld1
			fxch
			fyl2x
			fdivp	st1
			call	.checkFpuState
			jne		.failFcomp
			jmp		.done
		.div:
			fdivp
			stdcall	setRoundMode, ROUND_ZERO
			frndint
			stdcall	setRoundMode, ROUND_NEAREST
			call	.checkFpuState
			jne		.failFcomp
			jmp		.done
		.mod:
			fld		st1
			fld		st1
			fdivp
			stdcall	setRoundMode, ROUND_ZERO
			frndint
			stdcall	setRoundMode, ROUND_NEAREST
			fmulp	st1, st0
			fsubp	st1, st0
			call	.checkFpuState
			jne		.failFcomp
			jmp		.done
		.bitand:
			fistp	dword[.bitArg2]
			fistp	dword[.bitArg1]
			mov		eax, [.bitArg2]
			and		[.bitArg1], eax
			fild	dword[.bitArg1]
			jmp		.done
		.bitor:
			fistp	dword[.bitArg2]
			fistp	dword[.bitArg1]
			mov		eax, [.bitArg2]
			or		[.bitArg1], eax
			fild	dword[.bitArg1]
			jmp		.done
		.bitxor:
			fistp	dword[.bitArg2]
			fistp	dword[.bitArg1]
			mov		eax, [.bitArg2]
			xor		[.bitArg1], eax
			fild	dword[.bitArg1]
			jmp		.done
		.bitleft:
			fistp	dword[.bitArg2]
			fistp	dword[.bitArg1]
			mov		ecx, [.bitArg2]
			mov		eax, .shiftCounterTooBigStr
			test	ecx, ~ 31				; not 31
			jne		.fail					; bits with No >= 5 should be 0
			mov		eax, [.bitArg1]
			shl		eax, cl
			mov		[.bitArg1], eax
			fild	dword[.bitArg1]
			jmp		.done
		.bitright:
			fistp	dword[.bitArg2]
			fistp	dword[.bitArg1]
			mov		ecx, [.bitArg2]
			mov		eax, .shiftCounterTooBigStr
			test	ecx, ~ 31				; not 31
			jne		.fail					; bits with No >= 5 should be 0
			mov		eax, [.bitArg1]
			shr		eax, cl
			mov		[.bitArg1], eax
			fild	dword[.bitArg1]
			jmp		.done
		.notEqualFunc2:
			inc		edx
			cmp		edx, .funcsList2.length
			jb		.testAllFuncs2
			; test for bin(), oct(), hex()
			stdcall	checkFuncName, "bin"
			jne		.notBin
			add		esi, eax
			add		esi, eax
			sub		ecx, eax
			call	.stripBrackets
			jne		.fail
			mov		eax, .tooManyDigits
			cmp		ecx, 32
			jae		.fail
			xor		edx, edx
			xor		eax, eax
		.read_bin:
			jecxz	.bin_read
			lodsw
			dec		ecx
			sub		ax, '0'
			cmp		ax, 1
			jbe		.binOk
				mov		eax, .notBinNumberStr
				jmp		.fail
		.binOk:
			shl		edx, 1
			add		edx, eax
			jmp		.read_bin
		.bin_read:
			mov		dword[.dummy], edx
			fild	dword[.dummy]
			jmp		.done
		.notBin:
			stdcall	checkFuncName, "oct"
			jne		.notOct
			add		esi, eax
			add		esi, eax
			sub		ecx, eax
			call	.stripBrackets
			jne		.fail
			mov		eax, .tooManyDigits
			cmp		ecx, 11
			jae		.fail
			xor		edx, edx
			xor		eax, eax
		.read_oct:
			jecxz	.oct_read
			lodsw
			dec		ecx
			sub		ax, '0'
			cmp		ax, 7
			jbe		.octOk
				mov		eax, .notOctNumberStr
				jmp		.fail
		.octOk:
			shl		edx, 3
			add		edx, eax
			jmp		.read_oct
		.oct_read:
			mov		dword[.dummy], edx
			fild	dword[.dummy]
			jmp		.done
		.notOct:
			stdcall	checkFuncName, "hex"
			jne		.notHex
			add		esi, eax
			add		esi, eax
			sub		ecx, eax
			call	.stripBrackets
			jne		.fail
			mov		eax, .tooManyDigits
			cmp		ecx, 8
			jae		.fail
			xor		edx, edx
			xor		eax, eax
		.read_hex:
			jecxz	.hex_read
			lodsw
			dec		ecx
			or		al, 20h
			sub		ax, '0'
			cmp		ax, 10
			jb		.hexDigitOk
			
			cmp		ax, 'a' - '0'
			jb		.notHexNum
			sub		ax, 'a' - '9' - 1
		.hexDigitOk:
			cmp		al, 15
			jbe		.hexOk
		.notHexNum:
				mov		eax, .notHexNumberStr
				jmp		.fail
		.hexOk:
			shl		edx, 4
			add		edx, eax
			jmp		.read_hex
		.hex_read:
			mov		dword[.dummy], edx
			fild	dword[.dummy]
			jmp		.done
		.notHex:
			; test for consts
			cmp		ecx, 2					; test for pi
			jne		.notPi
			mov		eax, [esi]
			or		eax, 200020h			; make lowercase
			cmp		eax, 690070h			; L"pi"
			jne		.notPi
				fldpi
				jmp		.done
		.notPi:
			stdcall	checkFuncName, "euler"	; test for euler
			jne		.notEuler
				fld		tword[.euler]
				jmp		.done
		.notEuler:
			; test if number ends with %
			push	1
			pop		dword[.percDivider]
			cmp		word[esi + ecx * word.size - word.size], '%'
			jne		.noPerc
			dec		ecx
			push	100
			pop		dword[.percDivider]
		.noPerc:
			stdcall	stringToFloat, esi, ecx
			jne		.fail
			; divide by 1 if no %, and by 100 if there was
			fild	dword[.percDivider]
			fdivp	st1, st0
		.done:
			mov		edx, [.ResultPtr]
			test	edx, edx
			mov		eax, .nullResultPointerStr
			je		.leave
			fstp	tword[edx]
			xor		eax, eax
	.leave:
		pop		edi, esi, edx, ecx			; macro to pop multiple ops
		test	eax, eax
		fclex
	leave
	return
.failFcompp:
	fcomp	st1
.failFcomp:
	fistp	dword[.dummy]
.fail:
	jmp		.leave
align 4
.100:		dd	100
.euler:		dt	2.7182818284590452354

.testIfOpOk:
	; test if there isn't operation to the left of '-' or '+'
		push	eax, ecx, esi
	.skip_spaces_left:
		test	ecx, ecx
		je		.test_done
		dec		ecx
		movzx	eax, word[esi + ecx * word.size]
		cmp		ax, ' '
		jbe		.skip_spaces_left
	.spaces_skipped:
		cmp		ax, '-'
		je		.test_done
		cmp		ax, '+'
		je		.test_done
		cmp		ax, '*'
		je		.test_done
		cmp		ax, '/'
		je		.test_done
		cmp		ax, '^'
		je		.test_done
		cmp		ax, 'e'
		je		.test_done
		cmp		ax, 'E'
	.test_done:
		pop		esi, ecx, eax
		ret

.calculateSubexpressions:
	lea		eax, [.result1]
	stdcall	Evaluate, esi, ecx, [.AngleMode], eax
	jne		.ret
	push	edi
		lea		eax, [.result2]
		push	eax											; fourth parameter to call Evaluate
		lea		edi, [esi + ecx * word.size + word.size]
		mov		eax, [.Length]
		dec		eax
		sub		eax, ecx
		stdcall	Evaluate, edi, eax, [.AngleMode]
	pop		edi
	jne		.ret
		fld		tword[.result1]
		fld		tword[.result2]
.ret:
	ret

.stripBrackets:
	.remove_first_bracket:
		mov		eax, .noExpressionStr
		jecxz	.stripped
		lodsw
		dec		ecx
		cmp		ax, '('
		jne		.remove_first_bracket
	.remove_last_bracket:
		mov		eax, .noExpressionStr
		jecxz	.stripped
		dec		ecx
		mov		ax, [esi + ecx * word.size]
		cmp		ax, ')'
		jne		.remove_last_bracket
	.remove_start_spaces:
		mov		eax, .noExpressionStr
		jecxz	.stripped
		lodsw
		dec		ecx
		dec		eax
		cmp		ax, ' '
		jb		.remove_start_spaces
		dec		esi
		dec		esi
		inc		ecx
	.remove_end_spaces:
		mov		eax, .noExpressionStr
		jecxz	.stripped
		dec		ecx
		mov		ax, [esi + ecx * word.size]
		dec		eax
		cmp		ax, ' '
		jb		.remove_end_spaces
		inc		ecx
		xor		eax, eax
.stripped:
		test	eax, eax
	ret

.calculateTwoArgs:
	push	ecx, edx, esi, edi
		call	.stripBrackets
		jne		.noExpr
	;~ .remove_first_bracket:
		;~ mov		eax, .noExpressionStr
		;~ test	ecx, ecx
		;~ je		.noExpr
		;~ lodsw
		;~ dec		ecx
		;~ cmp		ax, '('
		;~ jne		.remove_first_bracket
	;~ .remove_last_bracket:
		;~ mov		eax, .noExpressionStr
		;~ jecxz	.noExpr
		;~ mov		ax, [esi + ecx * word.size - word.size]
		;~ dec		ecx
		;~ cmp		ax, ')'
		;~ jne		.remove_last_bracket
		
		xor		edi, edi	; brackets balance
		push	ecx
			xor		edx, edx
			dec		edx
		.findSemicolon:
			jecxz	.searchEnd
			dec		ecx
			mov		ax, [esi + ecx * word.size]
			cmp		ax, ')'
			jne		.notCBracket
			inc		edi
		.notCBracket:
			cmp		ax, '('
			jne		.notOBracket
			mov		eax, .bracketsBalanceStr
			test	edi, edi
			je		.searchEnd
			dec		edi
		.notOBracket:
			test	edi, edi
			jne		.nextChar
				mov		edx, ecx
				cmp		ax, ','
				je		.searchEnd
				cmp		ax, ';'
				je		.searchEnd
			.notSemicolon:
		.nextChar:
			test	ecx, ecx
			jne		.findSemicolon
		.searchEnd:
		pop		ecx
		mov		eax, .expectedSemicolonStr
		inc		edx
		je		.noExpr
		dec		edx
		lea		eax, [.result1]
		stdcall	Evaluate, esi, edx, [.AngleMode], eax
		jne		.noExpr
		inc		edx
		lea		edi, [esi + edx * word.size]
		sub		ecx, edx
		lea		eax, [.result2]
		stdcall	Evaluate, edi, ecx, [.AngleMode], eax
		jne		.noExpr
		fld		tword[.result1]
		fld		tword[.result2]
		xor		eax, eax
	.noExpr:
	pop		edi, esi, edx, ecx
	test	eax, eax
	ret

;~ FPU_SR_ES	; general error flag
;~ FPU_SR_SF	; stack error (overflow (c1 = 1) or antioverflow (c1 = 0))
;~ FPU_SR_PE	; inexact result
;~ FPU_SR_UE	; antioverflow (result too small)
.checkFpuState:
	push	edx
		fstsw	ax
		mov		edx, .overflowStr
		test	ax, FPU_SR_OE	; overflow
		jne		.error
		mov		edx, .divisionByZeroStr
		test	ax, FPU_SR_ZE	; zero divide
		jne		.error
		mov		edx, .denormalizedStr
		test	ax, FPU_SR_DE	; denormalized operand
		jne		.error
		mov		edx, .internalErrorStr
		test	ax, FPU_SR_IE	; invalid operation
		jne		.error
		xor		edx, edx
	.error:
		mov		eax, edx
	pop		edx
	ret

	[section .rdata]
	.noExpressionStr		unicode "Incomplete expression", 0
	.wrongLineLengthStr		unicode "Invalid line length", 0
	.bracketsBalanceStr		unicode "Brakets balance error", 0
	.divisionByZeroStr		unicode "Division by zero", 0
	.negativeSqrtArgStr		unicode "Sqrt argument is negative", 0
	.negativeLogArgStr		unicode "Logarithm arg is negative", 0
	.shiftCounterTooBigStr	unicode "Shift counter (2nd parameter) should be < 32", 0
	.expectedSemicolonStr	unicode "No delimiter(,;) found in parameters of two parameters function", 0
	.tooManyDigits			unicode "Too many digits", 0
	.notBinNumberStr		unicode "Not a binary number", 0
	.notOctNumberStr		unicode "Not an octal number", 0
	.notHexNumberStr		unicode "Not a hexadecimal number", 0
	.nullResultPointerStr	unicode "Null result pointer passed", 0
	.overflowStr			unicode "Overflow", 0
	.denormalizedStr		unicode "Denormalized", 0
	.internalErrorStr		unicode "Internal error", 0

	wcstbl .funcsList, "bitnot", "sinh", "cosh", "tanh", "asinh", "acosh", "atanh",\
						"sin", "cos", "tan", "asin", "acos", "atan", "sqrt", "abs",\
						"exp", "lg", "ln", "log2", "log10"
	wcstbl .funcsList2, "root", "logn", "div", "mod", "bitand", "bitor", "bitxor", "bitleft", "bitright"

	[section .text]
	align 4
	.jmpTable	dd .bitnot, .sinh, .cosh, .tanh, .asinh, .acosh, .atanh
				dd .sin, .cos, .tan, .asin, .acos, .atan, .sqrt, .abs
				dd .exp, .lg, .ln, .log2, .log10
	.jmpTable2	dd .root, .logn, .div, .mod, .bitand, .bitor, .bitxor, .bitleft, .bitright
endp

; ebx == null, esi - source unicode string, ecx - this string length, .FuncStr - string to compare
proc checkFuncName, .FuncStr
	enter
	push	ecx, edx, esi, edi
		xor		edx, edx
		mov		edi, [.FuncStr]
	.compare:
		inc		edx
		jecxz	.done
		lodsw
		or		ax, 20h
		scasw
		je		.compare
		dec		edx
		cmp		word[edi - word.size], bx
		je		.done
		test	esi, esi
	.done:
		mov		eax, edx
	pop		edi, esi, edx, ecx
	leave
	return
endp

; convert angle
proc convertAngleToRadians, .AngleMode
	enter
		mov		eax, [.AngleMode]
		cmp		eax, ANGLE_MODE_GRADS
		ja		.return
		jmp		[.jmpTable + eax * dword.size]
	.degrees:
		fld		tword[.pi_180]
		fmulp
		jmp		.return
	.grads:
		fld		tword[.pi_200]
		fmulp
.return:
	leave
	return
align 4
.jmpTable	dd .degrees, .return, .grads
.pi_180		dt 0.0174532925199432958
.pi_200		dt 0.0157079632679489662
endp

; convert angle
proc convertRadiansToAngle, .AngleMode
	enter
		mov		eax, [.AngleMode]
		cmp		eax, ANGLE_MODE_GRADS
		ja		.return
		jmp		[.jmpTable + eax * dword.size]
	.degrees:
		fld		tword[.180_pi]
		fmulp
		jmp		.return
	.grads:
		fld		tword[.200_pi]
		fmulp
.return:
	leave
	return
align 4
.jmpTable	dd .degrees, .return, .grads
.180_pi		dt 57.295779513082320880
.200_pi		dt 63.661977236758134300
endp

; converts text to float in st0 (expects ebx == null)
proc stringToFloat, .TextPtr, .Length
	enter	.digit resd 1
	push	ecx, edx, esi, edi
		cld
		stdcall	setRoundMode, ROUND_NEAREST
		xor		edx, edx					; signs flags
		mov		ecx, [.Length]
		mov		esi, [.TextPtr]
		fldz								; start accumulating number from zero
	.skip_spaces_start:						; skip spaces maintaining length counter
		mov		eax, .emptyStringStr
		test	ecx, ecx
		je		.fail
		;xor		eax, eax
		lodsw
		dec		ecx
		test	ax, ax
		je		.not_space
		cmp		ax, ' '
		jbe		.skip_spaces_start
	.not_space:
		; test if we have minus
		inc		dl							; mark mantissa minus sign
		cmp		ax, '-'
		je		.not_sign					; if minus skip unmarking and stepping back
		dec		dl							; unmark mantissa minus sign
		dec		esi							; step back
		dec		esi
		inc		ecx
	.not_sign:
		xor		eax, eax
	.first_number_part:
		test	ecx, ecx
		je		.done						; break if length counter is 0
		lodsw								; load another char and move pointer
		dec		ecx							; decrement length counter
		cmp		ax, '.'
		je		.second_number_part			; if '.' found - move to second number part
		cmp		ax, 'e'						; if 'e' or 'E' found - move to exponent part
		je		.exponent_part
		cmp		ax, 'E'
		je		.exponent_part
		sub		ax, '0'						; translate digit char to digit value
		cmp		ax, 10
		jae		.step_back_skip_spaces		; if not digit and not '.' and not 'e' and not 'E' - try to skip spaces
		fld		dword[ConstFloat10]			; multiply partial sum by 10
		fmulp
		mov		[.digit], eax				; store digit to variable to load into fpu register
		fild	dword[.digit]
		faddp	st1							; and add another digit value to it
		jmp		.first_number_part			; next iteration
	.second_number_part:
		fld1								; will become weight of digit position
		xor		eax, eax
	.loop_second_part:
		test	ecx, ecx
		je		.done_with_drop				; break if length counter is 0
		lodsw								; load another char and move pointer
		dec		ecx							; decrement length counter
		cmp		ax, 'e'
		je		.drop_weight_exponent		; if 'e' or 'E' found - move to exponent part
		cmp		ax, 'E'
		je		.drop_weight_exponent
		sub		ax, '0'						; translate digit char to digit value
		cmp		ax, 10
		jb		.digit_ok
			fcomp	st1						; drop weight
		.step_back_skip_spaces:
			dec		esi
			dec		esi
			inc		ecx
			jmp		.skip_spaces_end		; if not digit and not 'e' and not 'E' - try to skip spaces
	.digit_ok:
		mov		[.digit], eax				; store digit to variable to load into fpu register
		fld		dword[ConstFloat10]			; load 10
		fdivp	st1							; divide weight of digit position by 10
		fild	dword[.digit]
		fmul	st0, st1					; multiply digit by weight of it's position
		fadd	st2, st0					; add weighted digit to sum
		fcomp	st1							; drop weighted digit
		jmp		.loop_second_part
	.drop_weight_exponent:
		fcomp	st1							; drop weight
	.exponent_part:
		fldz								; partial sum for exponent
		lodsw
		dec		ecx
		cmp		ax, '+'
		je		.sign_found
		inc		dh							; mark exponent sign
		cmp		ax, '-'
		je		.sign_found
		dec		dh							; unmark exponent sign
		dec		esi							; step back if no sign char found
		dec		esi
		inc		ecx
	.sign_found:
		xor		eax, eax
	.loop_exponent_part:
		jecxz	.exponent_read				; break if length counter is 0
		lodsw
		dec		ecx
		sub		ax, '0'
		cmp		ax, 10
		jae		.get_back_exponent_read
		mov		[.digit], eax
		fld		dword[ConstFloat10]
		fmulp	st1
		fild	dword[.digit]
		faddp	st1
		jmp		.loop_exponent_part
	.get_back_exponent_read:
		dec		esi
		dec		esi
		inc		ecx
	.exponent_read:
		; apply exponent sign
		test	dh, dh
		je		.no_exp_sign
		fchs
	.no_exp_sign:
		; test if exponent has too big magnitude
		fld1
		fld		st2
		fyl2x						; log2(mantissa)
		fldl2t						; log2(10) log2(mantissa)
		fdivp	st1, st0			; log10(val)
		fadd	st0, st1
		fabs
		ficomp	dword[.4930]
		fstsw	ax
		sahf
		jbe		.expOk
			fcomp	st1
			mov		eax, .exponentIsTooBig
			jmp		.fail
	.expOk:
		; convert power of 10 into power of 2
		fldl2t						; log2(10) log10(exp) mantissa
		fmulp	st1					; log2(exp) mantissa
		fld		st0					; log2(exp) log2(exp) mantissa
		frndint						; int(log2(exp)) log2(exp) mantissa
		fsub	st1, st0			; int(log2(exp)) frac(log2(exp)) mantissa
		fxch						; frac(log2(exp)) int(log2(exp)) mantissa
		f2xm1						; 2^frac(log2(exp))-1  int(log2(exp)) mantissa
		fld1						; 1 2^frac(log2(exp))-1 int(log2(exp)) mantissa
		faddp	st1					; 2^frac(log2(exp)) int(log2(exp)) mantissa
		fscale						; 2^frac(log2(exp)) * 2^int(log2(exp)) int(log2(exp)) mantissa
		fxch						; drop st1
		fcomp	st1					; exp mantissa
		fmulp	st1					; mantissa * exp
	.skip_spaces_end:
		;xor		eax, eax
		jecxz	.done				; if end of string - done
		lodsw						; load char and move to next one
		dec		ecx
		test	ax, ax			; if zero char - fail
		je		.fail
		cmp		ax, ' '				; if space (or tab)
		jbe		.skip_spaces_end	; continue cycle
	.fail:							; otherwise as there should only be spaces - fail
		mov		eax, .extraCharacters
		ficomp	dword[.digit]		; clear fpu registers
;		xor		eax, eax			; place zero into eax and set ZF
		jmp		.leave
	.done_with_drop:
		ficomp	dword[.digit]		; clear fpu registers
	.done:							; apply mantissa sign
		test	dl, dl
		je		.no_mantissa_sign
		fchs
	.no_mantissa_sign:
		xor		eax, eax			; no error message
;		xor		eax, eax			; write 1 into eax and reset ZF
;		inc		eax
.leave:
		test	eax, eax			; set flags
		fclex
	pop		edi, esi, edx, ecx
	leave
	return

  align 4
  .4930:				dd	4930
  .emptyStringStr	unicode "Empty input", 0
  .exponentIsTooBig	unicode "Exponent is too big", 0
  .extraCharacters	unicode "Extra characters after number", 0
endp

; prints value from st0 (ebx == null)
proc floatToString, .TextPtr
	enter 	.intPow10 resd 1, .digit resd 1, .tmp resd 1
	push	esi, edi
		xor		edx, edx			; minus sign flag and exp print flag
		mov		[.tmp], edx
		stdcall	setRoundMode, ROUND_NEAREST
		;mov		eax, [.FloatPtr]	; load pointer to float number
		; get power of 10 and take int and frac parts
		fld1						; 1
		fxch
		;~ fldz						; 0 val 1
		;~ fcomp	st1					; val 1 ; cmp st0, st1 and drop st0
		ftst						; cmp st0, 0
		fstsw	ax					; copy fpu flags to cpu flags
		sahf
		jne		.notZero
			mov		edi, [.TextPtr]
			mov		dword[edi], '0'
			;fistp	dword[.tmp]
			fcompp
			jmp		.leave
	.notZero:
		ja		.noAbs				; if (val > 0) skip sign change
		fchs						; change val sign
		inc		dl					; mark it to output sign later
	.noAbs:
		fyl2x						; log2(val)
		fldl2t						; log2(10) log2(val)
		fdivp	st1, st0			; log10(val)
		fld		st0					; log10(val) log10(val)
		stdcall	setRoundMode, ROUND_ZERO	; for frndint
		frndint						; int(log10(val)) log10(val)
		stdcall	setRoundMode, ROUND_NEAREST
		fsub	st1, st0			; int(log10(val)) frac(log10(val))
		fistp	dword[.intPow10]	; frac(log10(val))
		; get mantissa from frac part of power 10
		fldl2t						; log2(10) frac(log10(val))
		fmulp	st1					; log2(mantissa)
		call	pow2				; 2^log2(mantissa) == mantissa
		fild	dword[.minus57]		; add bias to avoid .(9)
		fld1
		fscale
		fxch
		fcomp	st1
		faddp	st1
		; get all mantissa digits
		cld
		mov		edi, [.TextPtr]
		test	dl, dl				; test if we have minus
        je		.dont_print_minus
			mov		ax, '-'			; yes, we have
			stosw
	.dont_print_minus:
		; find out if exponential format will be better
		mov		ecx, [.intPow10]	; use power for placing delimiter
		cmp		ecx, MAX_POWER		; test if exponent is small enough
		jg		.exponential
		cmp		ecx, -MAX_POWER
		jg		.no_power
	.exponential:					; if not, print exponent
		xor		ecx, ecx			; reset delimiter position
		inc		dh					; mark print exponent
	.no_power:
		inc		ecx
		jg		.pow_pos
			mov		eax, 02e0030h	; L"0."
			stosd
		.zero_loop:
			jecxz	.end_zero_loop
			stosw
			inc		ecx
			jmp		.zero_loop
		.end_zero_loop:
	.pow_pos:
		mov		esi, ecx
		mov		ecx, MAX_DIGITS
	.digits:
		fld		st0
		; frndint should round down, the number is positive
		stdcall	setRoundMode, ROUND_ZERO
		frndint
		; for the rest of calculations
		stdcall	setRoundMode, ROUND_NEAREST
		fsub	st1, st0
		fistp	dword[edi]
		add		dword[edi], '0'
		inc		edi
		inc		edi
		fld		dword[ConstFloat10]
		fmulp	st1
		dec		esi
		jne		.no_point
		mov		ax, '.'
		stosw
	.no_point:
		loop	.digits
		ficomp	dword[.tmp]			; drop no more needed value
	.removeZeroes:
		dec		edi
		dec		edi
		cmp		word[edi], '0'
		je		.removeZeroes
		cmp		word[edi], '.'
		je		.notPoint
		inc		edi
		inc		edi
	.notPoint:
		mov		[edi], _0
		; test if we need to print exponent
		test	dh, dh
		je		.no_exp
			; print exponent
			mov		ax, 'e'
			stosw
			mov		eax, [.intPow10]
			call	printInteger
	.no_exp:
.leave:
	pop		edi, esi
	fclex
	leave
	return
	align 4
.minus57:	dd	-57
endp

proc exp
		fldl2e
		fmulp	st1, st0
		call	pow2
	return
endp

; st1 ^ st0
proc power
	enter	.intPower resd 1
		fxch							; if base == 0
		ftst
		fstsw	ax
		sahf
		jne		.notZeroBase
	.resultEq1:
			fcompp
			fld1						; then result is 1
			jmp		.done
	.notZeroBase:
		fabs
		mov		eax, _0
		ja		.basePositive
		fxch
		ftst							; base is negative
		fstsw	ax
		sahf
		je		.resultEq1				; if power == 0
		; test if power is integer
		fld		st0
		stdcall	setRoundMode, ROUND_NEAREST
		frndint
		fsub	st0, st1
		fabs
		fld		tword[ConstEpsilon]
		fcompp
		fstsw	ax
		sahf
		mov		eax, .negativeBaseStr
		jb		.failFcompp			; power is not integer and not very close to
		frndint
		fist	dword[.intPower]
		mov		eax, [.intPower]
		cmp		eax, _0
		jg		.powerPositive
		neg		eax
	.powerPositive:
		cmp		eax, 1000000000
		mov		eax, .exponentTooBig
		jae		.failFcompp
		xor		eax, eax
		fxch
		test	byte[.intPower], 1
		je		.basePositive
		inc		eax
.basePositive:
		fyl2x
		call	pow2
		test	eax, eax
		je		.keepSign
		fchs
	.keepSign:
	.done:
		xor		eax, eax
.return:
		test	eax, eax
	leave
	return
.failFcompp:
	fcompp
	jmp		.return
; error messages
align 2
.negativeBaseStr		unicode "Base of power is negative", 0
.exponentTooBig			unicode "Exponent value is too big", 0
endp

; 2 ^ st0
proc pow2
	fld		st0
	stdcall	setRoundMode, ROUND_MINUSINFINITY	; for frndint
	frndint
	stdcall	setRoundMode, ROUND_NEAREST
	fsub	st1, st0
	fxch
	f2xm1
	fld1
	faddp
	fscale
	fxch
	fcomp	st1
	return
endp

; eax should be signed integer number, edi should be pointer to unicode string buffer
proc printInteger
		test	eax, eax
		jns		.positive_number
		neg		eax
		mov		word[edi], '-'
		inc		edi
		inc		edi
	.positive_number:
		call	recursivePrintInteger
		return
endp

; eax should be positive integer number, edi should be pointer to unicode string buffer
proc recursivePrintInteger
		push	edx
		xor		edx, edx
		div		dword[.10]
		test	eax, eax
		je		.skip_self_call
		call	recursivePrintInteger
	.skip_self_call:
		add		dl, '0'
		mov		[edi], edx
		inc		edi
		inc		edi
		pop		edx
	return
align 4
.10		dd		10
endp

; sets rounding mode
proc setRoundMode, .Mode
	enter	.fpuCR resd 1
		push	eax
		fstcw	[.fpuCR]
		mov		eax, [.Mode]
		and		word[.fpuCR], ~ROUND_MASK
		or		[.fpuCR], eax
		or		word[.fpuCR], (3 << 8)		; also set max precision
		fldcw	[.fpuCR]
		pop		eax
	leave
	return
endp

[section .rdata]
ConstFloat10		dd 10.0			; used by stringToFloat and floatToString
ConstEpsilon:		dt 1.0e-17		; some epsilon good for comparing values

[section .text]
