	.file	"stack-probe.c"
	.text
	.globl	low
	.bss
	.align 8
	.type	low, @object
	.size	low, 8
low:
	.zero	8
	.globl	high
	.align 8
	.type	high, @object
	.size	high, 8
high:
	.zero	8
	.text
	.globl	update_range
	.type	update_range, @function
update_range:
.LFB0:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	movq	%rdi, -8(%rbp)
	movq	low(%rip), %rax
	cmpq	%rax, -8(%rbp)
	jnb	.L2
	movq	-8(%rbp), %rax
	movq	%rax, low(%rip)
.L2:
	movq	high(%rip), %rax
	cmpq	-8(%rbp), %rax
	jnb	.L4
	movq	-8(%rbp), %rax
	movq	%rax, high(%rip)
.L4:
	nop
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE0:
	.size	update_range, .-update_range
	.section	.rodata
.LC0:
	.string	"Stack >= %ld KB\n"
	.text
	.globl	probe
	.type	probe, @function
probe:
.LFB1:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$32, %rsp
	movl	%edi, -20(%rbp)
	leaq	-20(%rbp), %rax
	movq	%rax, %rdi
	call	update_range
	movq	high(%rip), %rax
	movq	%rax, %rcx
	movq	low(%rip), %rax
	movq	%rax, %rdx
	movq	%rcx, %rax
	subq	%rdx, %rax
	movq	%rax, -8(%rbp)
	movq	-8(%rbp), %rdx
	movq	%rdx, %rax
	sarq	$63, %rax
	shrq	$54, %rax
	addq	%rax, %rdx
	andl	$1023, %edx
	subq	%rax, %rdx
	movq	%rdx, %rax
	cmpq	$31, %rax
	jg	.L6
	movq	-8(%rbp), %rax
	leaq	1023(%rax), %rdx
	testq	%rax, %rax
	cmovs	%rdx, %rax
	sarq	$10, %rax
	movq	%rax, %rdx
	leaq	.LC0(%rip), %rax
	movq	%rdx, %rsi
	movq	%rax, %rdi
	movl	$0, %eax
	call	printf@PLT
.L6:
	movl	$0, %edi
	call	probe
	nop
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE1:
	.size	probe, .-probe
	.globl	main
	.type	main, @function
main:
.LFB2:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	movq	stdout(%rip), %rax
	movl	$0, %esi
	movq	%rax, %rdi
	call	setbuf@PLT
	movq	$-1, low(%rip)
	movq	$0, high(%rip)
	movl	$0, %edi
	call	probe
	movl	$0, %eax
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE2:
	.size	main, .-main
	.ident	"GCC: (GNU) 16.2.1 20260810"
	.section	.note.GNU-stack,"",@progbits
