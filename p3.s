	.text
	.globl
	.type
	function main-func
func:
	pushl	%ebp
	movl	%esp,	%ebp
	subl	$24,	%esp
	pushl	%ebx
	movl	%,	8(%ebp)
	movl	$1,	-12(%ebp)
	movl	$1,	-16(%ebp)
	movl	$0,	-20(%ebp)
jmp	.BB2
	movl	-28(%ebp),	%ebx
	movl	%ebx,	%eax
	popl	%ebx
	leave
	ret
	movl	-20(%ebp),	%ebx
	movl	8(%ebp),	%ecx
	movl	%ebx,	 edx
 	cmpl	%ecx,	edx
	jlt	.BB3
jmp	.BB4
	movl	-12(%ebp),	%ebx
	pushl	%ecx
	pushl	%edx
	pushl	%ebx
	call	print
	addl	$4,	%esp
	popl	%edx
	popl	%ecx
	movl	-20(%ebp),	%ecx
	movl	-20(%ebp),	ecx
	addl	$1,	ecx
	movl	%eax,	-20(%ebp)
	movl	%ecx,	-20(%ebp)
	movl	-16(%ebp),	%ecx
	movl	%ebx,	 edx
 	addl	%ecx,	edx
	movl	%eax,	-24(%ebp)
	movl	%edx,	-24(%ebp)
	movl	%ecx,	-12(%ebp)
	movl	-24(%ebp),	%ecx
	movl	%ecx,	-16(%ebp)
jmp	.BB2
	movl	-20(%ebp),	%ebx
	movl	%ebx,	-28(%ebp)
jmp	.BB1
