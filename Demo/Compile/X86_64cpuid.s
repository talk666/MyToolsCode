.globl	OPENSSL_ia32_rdrand_bytes
.type	OPENSSL_ia32_rdrand_bytes,@function
.align	16
OPENSSL_ia32_rdrand_bytes:
.cfi_startproc	
	xorq	%rax,%rax
	cmpq	$0,%rsi
	je	.Ldone_rdrand_bytes

	movq	$8,%r11
.Loop_rdrand_bytes:
.byte	73,15,199,242
	jc	.Lbreak_rdrand_bytes
	decq	%r11
	jnz	.Loop_rdrand_bytes
	jmp	.Ldone_rdrand_bytes

.align	16
.Lbreak_rdrand_bytes:
	cmpq	$8,%rsi
	jb	.Ltail_rdrand_bytes
	movq	%r10,(%rdi)
	leaq	8(%rdi),%rdi
	addq	$8,%rax
	subq	$8,%rsi
	jz	.Ldone_rdrand_bytes
	movq	$8,%r11
	jmp	.Loop_rdrand_bytes

.align	16
.Ltail_rdrand_bytes:
	movb	%r10b,(%rdi)
	leaq	1(%rdi),%rdi
	incq	%rax
	shrq	$8,%r10
	decq	%rsi
	jnz	.Ltail_rdrand_bytes

.Ldone_rdrand_bytes:
	xorq	%r10,%r10
	.byte	0xf3,0xc3
.cfi_endproc	
.size	OPENSSL_ia32_rdrand_bytes,.-OPENSSL_ia32_rdrand_bytes




.globl	OPENSSL_ia32_rdseed_bytes
.type	OPENSSL_ia32_rdseed_bytes,@function
.align	16
OPENSSL_ia32_rdseed_bytes:
.cfi_startproc	
	xorq	%rax,%rax
	cmpq	$0,%rsi
	je	.Ldone_rdseed_bytes

	movq	$8,%r11
.Loop_rdseed_bytes:
.byte	73,15,199,242
	jc	.Lbreak_rdseed_bytes
	decq	%r11
	jnz	.Loop_rdseed_bytes
	jmp	.Ldone_rdseed_bytes

.align	16
.Lbreak_rdseed_bytes:
	cmpq	$8,%rsi
	jb	.Ltail_rdseed_bytes
	movq	%r10,(%rdi)
	leaq	8(%rdi),%rdi
	addq	$8,%rax
	subq	$8,%rsi
	jz	.Ldone_rdseed_bytes
	movq	$8,%r11
	jmp	.Loop_rdseed_bytes

.align	16
.Ltail_rdseed_bytes:
	movb	%r10b,(%rdi)
	leaq	1(%rdi),%rdi
	incq	%rax
	shrq	$8,%r10
	decq	%rsi
	jnz	.Ltail_rdseed_bytes

.Ldone_rdseed_bytes:
	xorq	%r10,%r10
	.byte	0xf3,0xc3
.cfi_endproc	
.size	OPENSSL_ia32_rdseed_bytes,.-OPENSSL_ia32_rdseed_bytes
