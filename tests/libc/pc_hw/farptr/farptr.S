	.file	"farptr.c"
gcc2_compiled.:
___gnu_compiled_c:
.text
	.align 2
.globl _main
_main:
	pushl %ebp
	movl %esp,%ebp
	subl $8,%esp
	pushl %ebx
	movl 8(%ebp),%ebx
	movw 12(%ebp),%dx
	movb 16(%ebp),%cl
	movl %edx,-4(%ebp)
	movl %ecx,-8(%ebp)
	call ___main
/APP
	
;------------ farpokeb
/NO_APP
	movl -4(%ebp),%edx
/APP
	movw %dx,%fs
	.byte 0x64
	movb %bl,(%ebx)
	movw %dx,%fs
	.byte 0x64
	movb %dl,(%ebx)
/NO_APP
	movl -8(%ebp),%ecx
/APP
	movw %dx,%fs
	.byte 0x64
	movb %cl,(%ebx)
	movw %dx,%fs
	.byte 0x64
	movb $18,(%ebx)
	
;------------ farpokew
	movw %dx,%fs
	.byte 0x64
	movw %bx,(%ebx)
	movw %dx,%fs
	.byte 0x64
	movw %dx,(%ebx)
/NO_APP
	movsbw %cl,%ax
/APP
	movw %dx,%fs
	.byte 0x64
	movw %ax,(%ebx)
	movw %dx,%fs
	.byte 0x64
	movw $4660,(%ebx)
	
;------------ farpokel
	movw %dx,%fs
	.byte 0x64
	movl %ebx,(%ebx)
/NO_APP
	movswl %dx,%eax
/APP
	movw %dx,%fs
	.byte 0x64
	movl %eax,(%ebx)
/NO_APP
	movsbl %cl,%eax
/APP
	movw %dx,%fs
	.byte 0x64
	movl %eax,(%ebx)
	movw %dx,%fs
	.byte 0x64
	movl $305419896,(%ebx)
	
;------------ farpeek*
	movw %dx,%fs
	.byte 0x64
	movb (%ebx),%al
/NO_APP
	movzbw %al,%dx
/APP
	movw %dx, %fs
	.byte 0x64
	movw (%ebx), %ax

/NO_APP
	movb %al,%cl
/APP
	movw %dx,%fs
	.byte 0x64
	movl (%ebx),%eax
/NO_APP
	movl %eax,%ebx
/APP
	
;------------ farsetsel
	movw %dx,%fs
	
;------------ farnspokeb
	.byte 0x64
	movb %bl,(%ebx)
	.byte 0x64
	movb %dl,(%ebx)
	.byte 0x64
	movb %cl,(%ebx)
	.byte 0x64
	movb $18,(%ebx)
	
;------------ farnspokew
	.byte 0x64
	movw %bx,(%ebx)
	.byte 0x64
	movw %dx,(%ebx)
/NO_APP
	movsbw %cl,%ax
/APP
	.byte 0x64
	movw %ax,(%ebx)
	.byte 0x64
	movw $4660,(%ebx)
	
;------------ farnspokel
	.byte 0x64
	movl %ebx,(%ebx)
/NO_APP
	movzbl %dl,%eax
/APP
	.byte 0x64
	movl %eax,(%ebx)
/NO_APP
	movsbl %cl,%eax
/APP
	.byte 0x64
	movl %eax,(%ebx)
	.byte 0x64
	movl $305419896,(%ebx)
	
;------------ farnspeek*
	.byte 0x64
	movb (%ebx),%al
	.byte 0x64
	movw (%ebx),%ax
/NO_APP
	movzwl %ax,%ebx
/APP
	.byte 0x64
	movl (%ebx),%eax
	
;------------
/NO_APP
	xorl %eax,%eax
	movl -12(%ebp),%ebx
	leave
	ret
