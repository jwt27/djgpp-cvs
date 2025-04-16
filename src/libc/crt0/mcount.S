	.intel_syntax noprefix
	.text
	.globl	_mcount
_mcount:
	cmp	esp, offset _etext
	jb	Lstack_overflow
	push	eax
	push	ecx
	mov	eax, [esp+8]		# Our return address (callee)
	mov	ecx, [ebp+4]		# Callee's return address (caller)
	call	___mcount_internal
	pop	ecx
	pop	eax
	ret
Lstack_overflow:
	ud2
