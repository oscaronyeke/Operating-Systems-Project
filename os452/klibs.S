/*
** SCCS ID:	@(#)klibs.S	1.1	4/2/19
**
** File:	klibs.S
**
** Author:	Warren R. Carithers and various CSCI-452 sections
**
** Contributor:
**
** Description:	Kernel library assembly-language implementations
*/

#define	__SP_KERNEL__
#define	__SP_ASM__

/*
** _get_ebp - return current contents of EBP at the time of the call
**
** Could be used, e.g., by _kpanic to print a traceback
*/

	.globl	_get_ebp
_get_ebp:
	movl	%ebp, %eax
	ret
