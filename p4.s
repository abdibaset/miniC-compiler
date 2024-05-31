	.text
	.globl
	.type
	function main-func
func:
	pushl	%ebp
	movl	%esp,	%ebp
	subl	$20,	%esp
	pushl	%ebx
	movl	%,	8(%ebp)
	movl	$0,	-16(%ebp)
	movl	$0,	-12(%ebp)
	movl	8(%ebp),	%ebx
	movl	%ebx,	 ecx
 	cmpl	$0,	ecx
	jle	.BB2
jmp	.BB3
	movl	-24(%ebp),	%ebx
	movl	%ebx,	%eax
	popl	%ebx
	leave
	ret
	movl	$0,	-24(%ebp)
jmp	.BB1
jmp	.BB4
	movl	-16(%ebp),	%ebx
	movl	8(%ebp),	%ecx
	movl	%ebx,	 edx
 	cmpl	%ecx,	edx
	jlt	.BB5
jmp	.BB6
	pushl	%ecx
	pushl	%edx
	call	read
	movl	%eax,	%ebx
 	popl	%edx
	popl	%ecx
	movl	%ebx,	-20(%ebp)
	movl	-20(%ebp),	%ebx
	pushl	%ecx
	pushl	%edx
	pushl	%ebx
	call	print
	addl	$4,	%esp
	popl	%edx
	popl	%ecx
	movl	-12(%ebp),	%ecx
	movl	-12(%ebp),	ecx
	addl	%ebx,	ecx
	movl	%eax,	-12(%ebp)
	movl	%ecx,	-12(%ebp)
	movl	-16(%ebp),	%ecx
	movl	-16(%ebp),	ecx
	addl	$1,	ecx
	movl	%eax,	-16(%ebp)
	movl	%ecx,	-16(%ebp)
jmp	.BB4
jmp	.BB7
	movl	-12(%ebp),	%ebx
	movl	%ebx,	-24(%ebp)
jmp	.BB1
