	.att_syntax
	.file	"test.cc"
	.section	.text.unlikely.,"ax",@progbits
	.globl	main                            # -- Begin function main
	.prefalign	4, .Lfunc_end0, nop
	.type	main,@function
main:                                   # @main
	.cfi_startproc
# %bb.0:                                # %_ZSteqISt10win32_errcQoogssr3stdE10is_class_vIT_Egssr3stdE9is_enum_vIS1_EEbRKSt5errorS1_.exit
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
.Lfunc_end0:
	.size	main, .Lfunc_end0-main
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
