	.file	"opt.c"
gcc2_compiled.:
___gnu_compiled_c:
.text
	.align 2
.globl _main
_main:
	pushl %ebp
	movl %esp,%ebp
	call ___main
	.align 2,0x90
L15:
/APP
	.byte 0x64
	movb ($550),%al
/NO_APP
	testb %al,%al
	jl L15
	xorl %eax,%eax
	leave
	ret
