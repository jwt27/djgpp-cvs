; ---------------------------------------------------------------------------
; Copyright 1995-1996 by Morten Welinder (terra@diku.dk)
; Distributed under the GPL, see the file COPYING for details.
; ---------------------------------------------------------------------------

_real_2f:
	cmp	ax, DPMI_INSTALL_CHECK
	je	@f1
	cmp	ax, DPMI_GET_CPU_MODE
	je	@f2

@b1:
	.db	OP_JMP_FAR
_old2f = .
	.dd	0
; ------------------------------
@f1:
;	call	check_borland
;	je	@b1			; Borland calling?  -- We're not here!
	mov	cl, 3
_cpu = . - 1
	xor	ax, ax			; Success
	mov	bx, DPMI_MASK_32	; Flags (32-bit)
	mov	dx, (DPMI_VERSION_MAJOR << 8) + DPMI_VERSION_MINOR
	mov	si, 0x1234
_client_low_data_needs = . - 2
	push	cs			; Entry
	pop	es
	mov	di, dpmistart
	iret
; ------------------------------
@f2:					; Return "real/v86 mode"
	mov	ax, 1
	iret
; ---------------------------------------------------------------------------
; Buggy, Over-Rated, Large, Arrogant, Narrow-minded, Drat

;check_borland:
;	push	ds
;	push	bx
;	mov	bx, sp
;	lds	bx, ss:[bx + 6]		; get CS:IP after "int 0x2f"
;	and	bl, 0xfc		; just in case we're at the end of
;					; a segment -- we don't want to fault
;@b1:
;	cmpd	[bx], 0x696d7064	; "dpmi"
;	jne	@f1
;	cmpd	[bx + 4], 0x69623631	; "16bi"
;	jne	@f1
;	cmpd	[bx + 8], 0x6c766f2e	; ".ovl"
;	je	@f2
;@f1:
;	dec	bx
;	jne	@b1
;	dec	bx			; ZR=0
;@f2:
;	pop	bx
;	pop	ds
;	ret
; ---------------------------------------------------------------------------
; CAUTION: the following three instructions must be 14 bytes long.  This is
; used in resident.inc.
	pushd	temp_high_stack
	pushd	_parse_extra_cmdline
	jmp	@f1
signature:
	.db	"-MW-"
dpmistart:
	pushd	temp_high_stack
	pushd	_client_create_entry
@f1:
	push	cs
	call	_go32
	; never returns
; ---------------------------------------------------------------------------
