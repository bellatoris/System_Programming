	.file	"test.c"
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
	xorl	%edi, %edi
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
	jg	.L6
	jmp	.L2
	.p2align 4,,7
	.p2align 3
.L16:
	movl	4(%ecx), %esi
	xorl	%eax, %eax
	testl	%esi, %esi
	setg	%al
	movl	%esi, 4(%ebx)
	movl	%eax, %esi
	addl	%esi, %edi
	cmpl	$2, %edx
	je	.L2
	addl	$12, %ecx
	movl	-4(%ecx), %esi
	addl	$12, %ebx
	xorl	%eax, %eax
	testl	%esi, %esi
	setg	%al
	subl	$3, %edx
	movl	%esi, -4(%ebx)
	movl	%eax, %esi
	addl	%esi, %edi
	testl	%edx, %edx
	jle	.L2
.L6:
	movl	(%ecx), %esi
	xorl	%eax, %eax
	testl	%esi, %esi
	setg	%al
	movl	%esi, (%ebx)
	movl	%eax, %esi
	addl	%esi, %edi
	cmpl	$1, %edx
	jne	.L16
.L2:
	movl	%edi, %eax
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
	.section	.text.startup,"ax",@progbits
	.p2align 4,,15
	.globl	main
	.type	main, @function
main:
.LFB25:
	.cfi_startproc
	rep ret
	.cfi_endproc
.LFE25:
	.size	main, .-main
	.comm	dst,32,32
	.comm	src,32,32
	.ident	"GCC: (Ubuntu 4.8.4-2ubuntu1~14.04.1) 4.8.4"
	.section	.note.GNU-stack,"",@progbits
