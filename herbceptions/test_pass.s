	.att_syntax
	.file	"test_pass.cc"
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
	.globl	_Z1gv                           # -- Begin function _Z1gv
	.prefalign	4, .Lfunc_end1, nop
	.type	_Z1gv,@function
_Z1gv:                                  # @_Z1gv
	.cfi_startproc
# %bb.0:                                # %entry
	movb	$1, %al
	addb	$-1, %al
	retq
.Lfunc_end1:
	.size	_Z1gv, .Lfunc_end1-_Z1gv
	.cfi_endproc
                                        # -- End function
	.globl	main                            # -- Begin function main
	.prefalign	4, .Lfunc_end2, nop
	.type	main,@function
main:                                   # @main
	.cfi_startproc
# %bb.0:                                # %_ZSteqISt10win32_errcQoogssr3stdE10is_class_vIT_Egssr3stdE9is_enum_vIS1_EEbRKSt5errorS1_.exit
.Lfunc_end2:
	.size	main, .Lfunc_end2-main
	.cfi_endproc
                                        # -- End function
	.ident	"clang version 24.0.0git (git@github.com:trcrsired/llvm-project.git 59c14960c8219f19540c15774088058137c74b61)"
	.section	".note.GNU-stack","",@progbits
	.addrsig
