; ---------------------------------------------------------------------------
; Copyright 1995-1996 by Morten Welinder (terra@diku.dk)
; Distributed under the GPL, see the file COPYING for details.
; ---------------------------------------------------------------------------

; Determine cpu class and check >= 3.  If not, then exit.

check_cpu:
	pushf
	pop	bx
	and	bh,0x0f
	push	bx
	popf
	pushf
	pop	ax
	and	ax,0xf000
	cmp	ax,0xf000
	jz	@f1		; 8086/186
	or	bh,0xf0
	push	bx
	popf
	pushf
	pop	ax
	and	ax,0xf000
	jz	@f1		; 286

	mov	dx, sp		; Save old SP
	and	sp, 0xfffc	; Align it! (we will set AC flag)
	mov	al, FLAG_AC
	call	cpuflipflag
	mov	sp, dx
	jnc	@f3
	mov	al, FLAG_ID
	call	cpuflipflag
	jnc	@f4

	xor	eax, eax
	cpuid			; ebx:edx:ecx = vendor id; eax = max support
	or	eax, eax
	mov	ah, 5
	jz	@f5
	inc	ax		; call with eax = 1
	cpuid			; edx=features; ah&0f = family; al=model/step
	and	ah, 0x0f
@f5:
	mov	[_cpu], ah
	ret

@f1:
	mov	dx, @f2
	jmpl	bad_exit
@f2:
	.db	"This program needs a 386 or better.\r\n"
	.db	0

@f4:
	movb	[_cpu], 4
@f3:
	ret

cpuflipflag:
	push	si	; This plus the pushed ret address still stack aligned
	movzx	esi, al
	pushfd
	pop	eax
	mov	ebx, eax
	btc	eax, esi
	push	eax
	popfd
	pushfd
	pop	eax
	xor	eax, ebx
	push	ebx
	popfd
	bt	eax, esi
	pop	si
	ret
; ---------------------------------------------------------------------------
; Check that if we are in V86 mode then we have VCPI (to get out of it).

check_cpu_mode:
	mov	dx, @f2
	jmp	@f1
recheck_cpu_mode:
	mov	dx, @f3
@f1:
	smsw	ax
	test	al, 1
	je	@f1
	cmpb	[_vcpi_present], 0
	jne	@f1
	jmpl	bad_exit
@f1:
	ret
@f2:
	.db	"Protected mode not accessible.\r\n",0
@f3:
	.db	"Using XMS switched the CPU into V86 mode.\r\n",0
; ---------------------------------------------------------------------------
