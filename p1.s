	.text
	.globl
	.type
	function main-func
func:
	pushl	%ebp
	movl	%esp,	%ebp
	subl	$16,	%esp
	pushl	%ebx
	movl	%,	8(%ebp)
	movl	$10,	-12(%ebp)
	movl	8(%ebp),	%ebx
	$10,	ecx
	addl	%ebx,	ecx
	movl	%eax,	-16(%ebp)
	movl	%ecx,	-16(%ebp)
	movl	-16(%ebp),	%ecx
	movl	%ecx,	-20(%ebp)
jmp	.BB1
	movl	-20(%ebp),	%ebx
	movl	%ebx,	%eax
	popl	%ebx
	leave
	ret
