	.text
	.file	"object.ll"
	.globl	Object_print            # -- Begin function Object_print
	.p2align	4, 0x90
	.type	Object_print,@function
Object_print:                           # @Object_print
	.cfi_startproc
# %bb.0:
	pushq	%rbx
	.cfi_def_cfa_offset 16
	.cfi_offset %rbx, -16
	movq	%rdi, %rbx
	movl	$str, %edi
	xorl	%eax, %eax
	callq	printf
	movq	%rbx, %rax
	popq	%rbx
	.cfi_def_cfa_offset 8
	retq
.Lfunc_end0:
	.size	Object_print, .Lfunc_end0-Object_print
	.cfi_endproc
                                        # -- End function
	.globl	Object_printBool        # -- Begin function Object_printBool
	.p2align	4, 0x90
	.type	Object_printBool,@function
Object_printBool:                       # @Object_printBool
	.cfi_startproc
# %bb.0:
	pushq	%rbx
	.cfi_def_cfa_offset 16
	.cfi_offset %rbx, -16
	movq	%rdi, %rbx
	movl	$str.1, %ecx
	movl	$str.2, %eax
	testl	%esi, %esi
	cmovneq	%rcx, %rax
	movl	$str, %edi
	movq	%rax, %rsi
	xorl	%eax, %eax
	callq	printf
	movq	%rbx, %rax
	popq	%rbx
	.cfi_def_cfa_offset 8
	retq
.Lfunc_end1:
	.size	Object_printBool, .Lfunc_end1-Object_printBool
	.cfi_endproc
                                        # -- End function
	.globl	Object_printInt32       # -- Begin function Object_printInt32
	.p2align	4, 0x90
	.type	Object_printInt32,@function
Object_printInt32:                      # @Object_printInt32
	.cfi_startproc
# %bb.0:
	pushq	%rbx
	.cfi_def_cfa_offset 16
	.cfi_offset %rbx, -16
	movq	%rdi, %rbx
	movl	$str.3, %edi
	xorl	%eax, %eax
	callq	printf
	movq	%rbx, %rax
	popq	%rbx
	.cfi_def_cfa_offset 8
	retq
.Lfunc_end2:
	.size	Object_printInt32, .Lfunc_end2-Object_printInt32
	.cfi_endproc
                                        # -- End function
	.globl	Object_inputLine        # -- Begin function Object_inputLine
	.p2align	4, 0x90
	.type	Object_inputLine,@function
Object_inputLine:                       # @Object_inputLine
	.cfi_startproc
# %bb.0:
	pushq	%rax
	.cfi_def_cfa_offset 16
	movl	$is_eol, %edi
	callq	read_until
	testq	%rax, %rax
	je	.LBB3_1
# %bb.2:
	popq	%rcx
	.cfi_def_cfa_offset 8
	retq
.LBB3_1:
	.cfi_def_cfa_offset 16
	movl	$str.4, %eax
	popq	%rcx
	.cfi_def_cfa_offset 8
	retq
.Lfunc_end3:
	.size	Object_inputLine, .Lfunc_end3-Object_inputLine
	.cfi_endproc
                                        # -- End function
	.globl	Object_inputBool        # -- Begin function Object_inputBool
	.p2align	4, 0x90
	.type	Object_inputBool,@function
Object_inputBool:                       # @Object_inputBool
	.cfi_startproc
# %bb.0:
	pushq	%r14
	.cfi_def_cfa_offset 16
	pushq	%rbx
	.cfi_def_cfa_offset 24
	pushq	%rax
	.cfi_def_cfa_offset 32
	.cfi_offset %rbx, -24
	.cfi_offset %r14, -16
	movl	$isspace, %edi
	callq	skip_while
	movl	$isspace, %edi
	callq	read_until
	testq	%rax, %rax
	je	.LBB4_8
# %bb.1:
	movq	%rax, %r14
	movq	%rax, %rdi
	callq	strlen
	movq	%rax, %rbx
	cmpq	$4, %rax
	jne	.LBB4_4
# %bb.2:
	movl	$str.1, %esi
	movl	$4, %edx
	movq	%r14, %rdi
	callq	strncmp
	testl	%eax, %eax
	je	.LBB4_3
.LBB4_4:
	cmpq	$5, %rbx
	jne	.LBB4_9
# %bb.5:
	movl	$str.2, %esi
	movl	$5, %edx
	movq	%r14, %rdi
	callq	strncmp
	testl	%eax, %eax
	jne	.LBB4_9
# %bb.6:
	movq	%r14, %rdi
	callq	free
	xorl	%eax, %eax
.LBB4_7:
                                        # kill: def $al killed $al killed $eax
	addq	$8, %rsp
	.cfi_def_cfa_offset 24
	popq	%rbx
	.cfi_def_cfa_offset 16
	popq	%r14
	.cfi_def_cfa_offset 8
	retq
.LBB4_3:
	.cfi_def_cfa_offset 32
	movq	%r14, %rdi
	callq	free
	movb	$1, %al
	jmp	.LBB4_7
.LBB4_9:
	movq	stderr(%rip), %rdi
	movl	$str.6, %esi
	movq	%r14, %rdx
	xorl	%eax, %eax
	callq	fprintf
	movq	%r14, %rdi
	callq	free
	movl	$1, %edi
	callq	exit
.LBB4_8:
	movq	stderr(%rip), %rdi
	movl	$str.5, %esi
	xorl	%eax, %eax
	callq	fprintf
	movl	$1, %edi
	callq	exit
.Lfunc_end4:
	.size	Object_inputBool, .Lfunc_end4-Object_inputBool
	.cfi_endproc
                                        # -- End function
	.globl	Object_inputInt32       # -- Begin function Object_inputInt32
	.p2align	4, 0x90
	.type	Object_inputInt32,@function
Object_inputInt32:                      # @Object_inputInt32
	.cfi_startproc
# %bb.0:
	pushq	%rbx
	.cfi_def_cfa_offset 16
	subq	$16, %rsp
	.cfi_def_cfa_offset 32
	.cfi_offset %rbx, -16
	movl	$isspace, %edi
	callq	skip_while
	movl	$isspace, %edi
	callq	read_until
	testq	%rax, %rax
	je	.LBB5_20
# %bb.1:
	movq	%rax, %rbx
	movq	%rax, %rdi
	callq	strlen
	cmpq	$3, %rax
	jb	.LBB5_4
# %bb.2:
	cmpb	$48, (%rbx)
	jne	.LBB5_4
# %bb.3:
	movb	$1, %cl
	cmpb	$120, 1(%rbx)
	je	.LBB5_12
.LBB5_4:
	cmpq	$4, %rax
	jb	.LBB5_11
# %bb.5:
	cmpb	$43, (%rbx)
	je	.LBB5_7
# %bb.6:
	cmpb	$45, (%rbx)
	jne	.LBB5_11
.LBB5_7:
	cmpb	$48, 1(%rbx)
	jne	.LBB5_11
# %bb.8:
	cmpb	$120, 2(%rbx)
	sete	%cl
	jmp	.LBB5_12
.LBB5_11:
	xorl	%ecx, %ecx
.LBB5_12:
	leaq	8(%rsp), %rsi
	movq	%rbx, %rdi
	testb	%cl, %cl
	je	.LBB5_14
# %bb.13:
	movl	$16, %edx
	jmp	.LBB5_15
.LBB5_14:
	movl	$10, %edx
.LBB5_15:
	callq	strtoll
	movq	8(%rsp), %rcx
	cmpb	$0, (%rcx)
	jne	.LBB5_21
# %bb.16:
	cmpq	$-2147483648, %rax      # imm = 0x80000000
	jl	.LBB5_19
# %bb.17:
	movl	$2147483648, %ecx       # imm = 0x80000000
	cmpq	%rcx, %rax
	jge	.LBB5_19
# %bb.18:
                                        # kill: def $eax killed $eax killed $rax
	addq	$16, %rsp
	.cfi_def_cfa_offset 16
	popq	%rbx
	.cfi_def_cfa_offset 8
	retq
.LBB5_19:
	.cfi_def_cfa_offset 32
	movq	stderr(%rip), %rdi
	movl	$str.9, %esi
	jmp	.LBB5_22
.LBB5_20:
	movq	stderr(%rip), %rdi
	movl	$str.7, %esi
	xorl	%eax, %eax
	callq	fprintf
	movl	$1, %edi
	callq	exit
.LBB5_21:
	movq	stderr(%rip), %rdi
	movl	$str.8, %esi
.LBB5_22:
	movq	%rbx, %rdx
	xorl	%eax, %eax
	callq	fprintf
	movl	$1, %edi
	callq	exit
.Lfunc_end5:
	.size	Object_inputInt32, .Lfunc_end5-Object_inputInt32
	.cfi_endproc
                                        # -- End function
	.globl	Object__new             # -- Begin function Object__new
	.p2align	4, 0x90
	.type	Object__new,@function
Object__new:                            # @Object__new
	.cfi_startproc
# %bb.0:
	pushq	%rax
	.cfi_def_cfa_offset 16
	movl	$8, %edi
	callq	malloc
	movq	%rax, %rdi
	callq	Object__init
	popq	%rcx
	.cfi_def_cfa_offset 8
	retq
.Lfunc_end6:
	.size	Object__new, .Lfunc_end6-Object__new
	.cfi_endproc
                                        # -- End function
	.globl	Object__init            # -- Begin function Object__init
	.p2align	4, 0x90
	.type	Object__init,@function
Object__init:                           # @Object__init
	.cfi_startproc
# %bb.0:
	movq	%rdi, %rax
	testq	%rdi, %rdi
	je	.LBB7_2
# %bb.1:
	movq	$vtable.Object, (%rax)
.LBB7_2:
	retq
.Lfunc_end7:
	.size	Object__init, .Lfunc_end7-Object__init
	.cfi_endproc
                                        # -- End function
	.p2align	4, 0x90         # -- Begin function read_until
	.type	read_until,@function
read_until:                             # @read_until
	.cfi_startproc
# %bb.0:
	pushq	%rbp
	.cfi_def_cfa_offset 16
	pushq	%r15
	.cfi_def_cfa_offset 24
	pushq	%r14
	.cfi_def_cfa_offset 32
	pushq	%r12
	.cfi_def_cfa_offset 40
	pushq	%rbx
	.cfi_def_cfa_offset 48
	.cfi_offset %rbx, -48
	.cfi_offset %r12, -40
	.cfi_offset %r14, -32
	.cfi_offset %r15, -24
	.cfi_offset %rbp, -16
	movq	%rdi, %r14
	movl	$1024, %r15d            # imm = 0x400
	movl	$1024, %edi             # imm = 0x400
	callq	malloc
	movq	%rax, %r12
	xorl	%ebx, %ebx
	testq	%r12, %r12
	jne	.LBB8_3
	jmp	.LBB8_2
	.p2align	4, 0x90
.LBB8_11:                               #   in Loop: Header=BB8_3 Depth=1
	incq	%rbx
	testq	%r12, %r12
	je	.LBB8_2
.LBB8_3:                                # =>This Inner Loop Header: Depth=1
	movq	stdin(%rip), %rdi
	callq	getc
	movl	%eax, %ebp
	cmpl	$-1, %eax
	je	.LBB8_5
# %bb.4:                                #   in Loop: Header=BB8_3 Depth=1
	movsbl	%bpl, %edi
	callq	*%r14
	testl	%eax, %eax
	jne	.LBB8_5
# %bb.9:                                #   in Loop: Header=BB8_3 Depth=1
	movb	%bpl, (%r12,%rbx)
	leaq	-1(%r15), %rax
	cmpq	%rax, %rbx
	jne	.LBB8_11
# %bb.10:                               #   in Loop: Header=BB8_3 Depth=1
	addq	%r15, %r15
	movq	%r12, %rdi
	movq	%r15, %rsi
	callq	realloc
	movq	%rax, %r12
	jmp	.LBB8_11
.LBB8_5:
	cmpl	$-1, %ebp
	je	.LBB8_7
# %bb.6:
	movq	stdin(%rip), %rsi
	movl	%ebp, %edi
	callq	ungetc
.LBB8_7:
	movb	$0, (%r12,%rbx)
	jmp	.LBB8_8
.LBB8_2:
	xorl	%r12d, %r12d
.LBB8_8:
	movq	%r12, %rax
	popq	%rbx
	.cfi_def_cfa_offset 40
	popq	%r12
	.cfi_def_cfa_offset 32
	popq	%r14
	.cfi_def_cfa_offset 24
	popq	%r15
	.cfi_def_cfa_offset 16
	popq	%rbp
	.cfi_def_cfa_offset 8
	retq
.Lfunc_end8:
	.size	read_until, .Lfunc_end8-read_until
	.cfi_endproc
                                        # -- End function
	.p2align	4, 0x90         # -- Begin function is_eol
	.type	is_eol,@function
is_eol:                                 # @is_eol
	.cfi_startproc
# %bb.0:
	xorl	%eax, %eax
	cmpl	$10, %edi
	sete	%al
	retq
.Lfunc_end9:
	.size	is_eol, .Lfunc_end9-is_eol
	.cfi_endproc
                                        # -- End function
	.p2align	4, 0x90         # -- Begin function skip_while
	.type	skip_while,@function
skip_while:                             # @skip_while
	.cfi_startproc
# %bb.0:
	pushq	%rbp
	.cfi_def_cfa_offset 16
	pushq	%rbx
	.cfi_def_cfa_offset 24
	pushq	%rax
	.cfi_def_cfa_offset 32
	.cfi_offset %rbx, -24
	.cfi_offset %rbp, -16
	movq	%rdi, %rbx
	jmp	.LBB10_1
	.p2align	4, 0x90
.LBB10_3:                               #   in Loop: Header=BB10_1 Depth=1
	movl	%ebp, %edi
	callq	*%rbx
	testl	%eax, %eax
	setne	%al
	testb	%al, %al
	je	.LBB10_5
.LBB10_1:                               # =>This Inner Loop Header: Depth=1
	movq	stdin(%rip), %rdi
	callq	getc
	movl	%eax, %ebp
	cmpl	$-1, %eax
	jne	.LBB10_3
# %bb.2:                                #   in Loop: Header=BB10_1 Depth=1
	xorl	%eax, %eax
	testb	%al, %al
	jne	.LBB10_1
.LBB10_5:
	cmpl	$-1, %ebp
	je	.LBB10_7
# %bb.6:
	movq	stdin(%rip), %rsi
	movl	%ebp, %edi
	callq	ungetc
.LBB10_7:
	addq	$8, %rsp
	.cfi_def_cfa_offset 24
	popq	%rbx
	.cfi_def_cfa_offset 16
	popq	%rbp
	.cfi_def_cfa_offset 8
	retq
.Lfunc_end10:
	.size	skip_while, .Lfunc_end10-skip_while
	.cfi_endproc
                                        # -- End function
	.type	str,@object             # @str
	.section	.rodata,"a",@progbits
	.globl	str
str:
	.asciz	"%s"
	.size	str, 3

	.type	str.1,@object           # @str.1
	.globl	str.1
str.1:
	.asciz	"true"
	.size	str.1, 5

	.type	str.2,@object           # @str.2
	.globl	str.2
str.2:
	.asciz	"false"
	.size	str.2, 6

	.type	str.3,@object           # @str.3
	.globl	str.3
str.3:
	.asciz	"%d"
	.size	str.3, 3

	.type	str.4,@object           # @str.4
	.globl	str.4
str.4:
	.zero	1
	.size	str.4, 1

	.type	str.5,@object           # @str.5
	.globl	str.5
	.p2align	4
str.5:
	.asciz	"Object::inputBool: cannot read word!\n"
	.size	str.5, 38

	.type	str.6,@object           # @str.6
	.globl	str.6
	.p2align	4
str.6:
	.asciz	"Object::inputBool: `%s` is not a valid boolean!\n"
	.size	str.6, 49

	.type	str.7,@object           # @str.7
	.globl	str.7
	.p2align	4
str.7:
	.asciz	"Object::inputInt32: cannot read word!\n"
	.size	str.7, 39

	.type	str.8,@object           # @str.8
	.globl	str.8
	.p2align	4
str.8:
	.asciz	"Object::inputInt32: `%s` is not a valid integer literal!\n"
	.size	str.8, 58

	.type	str.9,@object           # @str.9
	.globl	str.9
	.p2align	4
str.9:
	.asciz	"Object::inputInt32: `%s` does not fit a 32-bit integer!\n"
	.size	str.9, 57

	.type	vtable.Object,@object   # @vtable.Object
	.globl	vtable.Object
	.p2align	4
vtable.Object:
	.quad	Object_print
	.quad	Object_printBool
	.quad	Object_printInt32
	.quad	Object_inputLine
	.quad	Object_inputBool
	.quad	Object_inputInt32
	.size	vtable.Object, 48


	.section	".note.GNU-stack","",@progbits
