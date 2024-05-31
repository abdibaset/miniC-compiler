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
	movl	$0,	-12(%ebp)
	movl	8(%ebp),	%ebx
	movl	%ebx,	 ecx
 	cmpl	$0,	ecx
	jlt	.BB2
jmp	.BB3
	movl	-24(%ebp),	%ebx
	movl	%ebx,	%eax
	popl	%ebx
	leave
	ret
	movl	$10,	-12(%ebp)
jmp	.BB7
	movl	$2,	-20(%ebp)
	movl	$0,	-16(%ebp)
jmp	.BB4
	movl	-16(%ebp),	%ebx
	movl	8(%ebp),	%ecx
	movl	%ebx,	 edx
 	cmpl	%ecx,	edx
	jlt	.BB5
jmp	.BB6
	movl	-16(%ebp),	%ebx
	movl	-16(%ebp),	ebx
	addl	$2,	ebx
	movl	%eax,	-16(%ebp)
	movl	%ebx,	-16(%ebp)
jmp	.BB4
jmp	.BB7
	movl	-12(%ebp),	%ebx
	movl	-16(%ebp),	%ecx
	movl	-12(%ebp),	ebx
	addl	%ecx,	ebx
	movl	%eax,	-24(%ebp)
	movl	%ebx,	-24(%ebp)
jmp	.BB1
