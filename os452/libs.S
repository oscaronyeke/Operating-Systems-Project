/*
** SCCS ID:	@(#)libs.S	1.1	4/2/19
**
** File:	libs.S
**
** Authors:	Jon Coles, Warren R. Carithers, K. Reek
**
** Contributor:
**
** Description:	Support library functions (assembly language)
**
** These functions are support routines used in various places
** throughout the framework.  They are written in assembly language
** for efficiency and/or because they require access to machine-level
** features that are hard to access from C.
*/

/*
** Parameter offsets
*/
ARG1	= 8			// Offset to 1st argument
ARG2	= 12			// Offset to 2nd argument

/*
** Name:	__inb, __inw, __inl
**
** Description: read a single byte, word, or longword from the specified
**		input port
** Arguments:	the port number
** Returns:	the byte, word, or longword value from the port (unused
**		high-order bytes are zeroed)
*/
	.globl	__inb, __inw, __inl

__inb:
	enter	$0,$0
	xorl	%eax,%eax	// Clear the high order bytes of %eax
	movl	ARG1(%ebp),%edx	// Move port number to %edx
	inb	(%dx)		// Get a byte from the port into %al (low
	leave			//   byte of %eax)
	ret
__inw:
	enter	$0,$0
	xorl	%eax,%eax	// Clear the high order bytes of %eax
	movl	ARG1(%ebp),%edx	// Move port number to %edx
	inw	(%dx)		// Get a word from the port into %ax (low
	leave			//   word of %eax)
	ret
__inl:
	enter	$0,$0
	xorl	%eax,%eax	// Clear the high order bytes of %eax
	movl	ARG1(%ebp),%edx	// Move port number to %edx
	inl	(%dx)		// Get a longword from the port into %eax
	leave
	ret
	
/*
** Name:	__outb, __outw, __outl
**
** Description: write a single byte, word, or longword to the specified
**		output port
** Arguments:	the port number, the value to write to it (unused high-order
**		bytes are ignored)
*/
	.globl	__outb, __outw, __outl
__outb:
	enter	$0,$0
	movl	ARG1(%ebp),%edx	// Get the port number into %edx,
	movl	ARG2(%ebp),%eax	//   and the value into %eax
	outb	(%dx)		// Output that byte to the port
	leave			//   (only %al is sent)
	ret
__outw:
	enter	$0,$0
	movl	ARG1(%ebp),%edx	// Get the port number into %edx,
	movl	ARG2(%ebp),%eax	//   and the value into %eax
	outw	(%dx)		// Output that word to the port.
	leave			//   (only %ax is sent)
	ret
__outl:
	enter	$0,$0
	movl	ARG1(%ebp),%edx	// Get the port number into %edx,
	movl	ARG2(%ebp),%eax	//   and the value into %eax
	outl	(%dx)		// Output that longword to the port.
	leave
	ret

/*
** __get_flags: return the current processor flags
**	unsigned int __get_flags( void );
*/
	.globl	__get_flags

__get_flags:
	pushfl			// Push flags on the stack,
	popl	%eax		//   and pop them into eax.
	ret

/*
** __pause: halt until something happens
**      void __pause( void );
*/
	.globl	__pause

__pause:
	enter	$0,$0
	sti
	hlt
	leave
	ret

/*
** __get_ra: get the return address for the calling function
**           (i.e., where whoever called us will go back to)
**	void *__get_ra( void );
*/
	.global	__get_ra

__get_ra:
	// DO NOT create a stack frame - use the caller's
	//
	// Caller's return address is between the saved EBP
	// and its first parameter
	movl	4(%ebp), %eax
	ret
