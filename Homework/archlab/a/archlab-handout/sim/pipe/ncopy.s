	.file	"ncopy.c"
	.text
	.p2align 4,,15
	.globl	ncopy
	.type	ncopy, @function
ncopy:
.LFB24:
	.cfi_startproc
	pushl	%edi
	.cfi_def_cfa_offset 8
	.cfi_offset 7, -8
	xorl	%eax, %eax
	pushl	%esi
	.cfi_def_cfa_offset 12
	.cfi_offset 6, -12
	pushl	%ebx
	.cfi_def_cfa_offset 16
	.cfi_offset 3, -16
	movl	24(%esp), %edx
	movl	16(%esp), %ecx
	movl	20(%esp), %ebx
	testl	%edx, %edx
	jle	.L2
	.p2align 4,,7
	.p2align 3
.L3:
	addl	$4, %ecx
	movl	-4(%ecx), %esi
	addl	$4, %ebx
	leal	1(%eax), %edi
	testl	%esi, %esi
	cmovg	%edi, %eax
	subl	$1, %edx
	movl	%esi, -4(%ebx)
	jne	.L3
.L2:
	popl	%ebx
	.cfi_restore 3
	.cfi_def_cfa_offset 12
	popl	%esi
	.cfi_restore 6
	.cfi_def_cfa_offset 8
	popl	%edi
	.cfi_restore 7
	.cfi_def_cfa_offset 4
	ret
	.cfi_endproc
.LFE24:
	.size	ncopy, .-ncopy
	.section	.rodata.str1.1,"aMS",@progbits,1
.LC0:
	.string	"count=%d\n"
	.section	.text.startup,"ax",@progbits
	.p2align 4,,15
	.globl	main
	.type	main, @function
main:
.LFB25:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register 5
	andl	$-16, %esp
	subl	$16, %esp
	movl	$8, 8(%esp)
	movl	$.LC0, 4(%esp)
	movl	$1, (%esp)
	movl	$1, src
	movl	$2, src+4
	movl	$3, src+8
	movl	$4, src+12
	movl	$5, src+16
	movl	$6, src+20
	movl	$7, src+24
	movl	$8, src+28
	movl	$1, dst
	movl	$2, dst+4
	movl	$3, dst+8
	movl	$4, dst+12
	movl	$5, dst+16
	movl	$6, dst+20
	movl	$7, dst+24
	movl	$8, dst+28
	call	__printf_chk
	movl	$0, (%esp)
	call	exit
	.cfi_endproc
.LFE25:
	.size	main, .-main
	.comm	dst,32,32
	.comm	src,32,32
	.ident	"GCC: (Ubuntu 4.8.4-2ubuntu1~14.04.1) 4.8.4"
	.section	.note.GNU-stack,"",@progbits
