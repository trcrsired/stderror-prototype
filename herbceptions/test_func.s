	.att_syntax
	.file	"test_func.cc"
	.text
	.globl	_Z1fv                           # -- Begin function _Z1fv
	.prefalign	4, .Lfunc_end0, nop
	.type	_Z1fv,@function
_Z1fv:                                  # @_Z1fv
	.cfi_startproc
# %bb.0:                                # %entry
	pushq	%rax
	.cfi_def_cfa_offset 16
	callq	__cxa_error_domain_win32@PLT
	movb	$1, %cl
	addb	$-1, %cl
	movl	$2, %edx
	popq	%rcx
	.cfi_def_cfa_offset 8
	retq
.Lfunc_end0:
	.size	_Z1fv, .Lfunc_end0-_Z1fv
	.cfi_endproc
                                        # -- End function
	.section	.text.unlikely.,"ax",@progbits
	.globl	main                            # -- Begin function main
	.prefalign	4, .Lfunc_end1, nop
	.type	main,@function
main:                                   # @main
	.cfi_startproc
# %bb.0:                                # %entry
	pushq	%rbx
	.cfi_def_cfa_offset 16
	.cfi_offset %rbx, -16
	callq	__cxa_error_domain_win32@PLT
	movq	stderr@GOTPCREL(%rip), %rcx
	movq	(%rcx), %rbx
	movl	$2, %edi
	callq	*32(%rax)
	movl	%eax, %edi
	callq	strerror@PLT
	leaq	.L.str(%rip), %rsi
	movq	%rbx, %rdi
	movl	$1, %edx
	movq	%rax, %rcx
	xorl	%eax, %eax
	callq	fprintf@PLT
	xorl	%eax, %eax
	popq	%rbx
	.cfi_def_cfa_offset 8
	retq
.Lfunc_end1:
	.size	main, .Lfunc_end1-main
	.cfi_endproc
                                        # -- End function
	.type	.L.str,@object                  # @.str
	.section	.rodata.str1.1,"aMS",@progbits,1
.L.str:
	.asciz	"%d\n%s\n"
	.size	.L.str, 7

	.ident	"clang version 24.0.0git (git@github.com:trcrsired/llvm-project.git 59c14960c8219f19540c15774088058137c74b61)"
	.section	".note.GNU-stack","",@progbits
	.addrsig
