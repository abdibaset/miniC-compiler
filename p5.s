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
jmp	.BB2
	movl	-24(%ebp),	%ebx
	movl	%ebx,	%eax
	popl	%ebx
	leave
	ret
	movl	-16(%ebp),	%ebx
	movl	8(%ebp),	%ecx
	movl	%ebx,	 edx
 	cmpl	%ecx,	edx
	jlt	.BB3
jmp	.BB4
	pushl	%ecx
	pushl	%edx
	call	read
	movl	%eax,	%ebx
 	popl	%edx
	popl	%ecx
	movl	%ebx,	-20(%ebp)
	movl	-20(%ebp),	%ebx
	movl	-12(%ebp),	%ecx
	movl	%ebx,	 edx
 	cmpl	%ecx,	edx
	jgt	.BB5
jmp	.BB6
	movl	-12(%ebp),	%ebx
	movl	%ebx,	-24(%ebp)
jmp	.BB1
	movl	-20(%ebp),	%ebx
	movl	%ebx,	-12(%ebp)
jmp	.BB6
	movl	-16(%ebp),	%ebx
	movl	-16(%ebp),	ebx
	addl	$1,	ebx
	movl	%eax,	-16(%ebp)
	movl	%ebx,	-16(%ebp)
jmp	.BB2
