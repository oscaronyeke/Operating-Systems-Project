/*
** SCCS ID:	@(#)lib.h	1.1	4/2/19
**
** File:	lib.h
**
** Author:	Warren R. Carithers
**
** Description:	Library of support functions
**
** This module implements a simple collection of support functions
** similar to the standard C library.  Several of these functions
** were originally found in the CIO module; others were in the
** startup module.
*/

#ifndef _CLIB_H_
#define _CLIB_H_

/*
** Name:	__bound
**
** Description:	This function confines an argument within specified bounds.
**
** Arguments:	The bounds and the value to be constrained.
** Returns:	The constrained value.
*/
unsigned int __bound( unsigned int min, unsigned int value,
                        unsigned int max );

/*
** Name:	__memset
**
** Description:	initialize all bytes of a block of memory to a specific value
**
** Arguments:	The buffer, its length (in bytes), and the init value
*/
void __memset( void *buf, register unsigned int len,
               register unsigned int value );

/*
** Name:	__memclr
**
** Description:	Initialize all bytes of a block of memory to zero
**
** Arguments:	The buffer and its length (in bytes)
*/
void __memclr( void *buf, register unsigned int len );

/*
** Name:	__memcpy
**
** Description:	Copy a block from one place to another
**
** May not correctly deal with overlapping buffers
**
** Arguments:	The destination and source buffers, and the length (in bytes)
*/
void __memcpy( void *dst, register const void *src,
               register unsigned int len );

/*
** Name:        __strlen
**
** Description: Calculate the length of a C-style string.
**
** Arguments:   The string.
** Returns:     The string's length.
*/
unsigned int __strlen( register const char *str );

/*
** Name:        __strcmp
**
** Description: Compare two strings
**
** Arguments:   The two source buffers.
** Returns:	< 0 if s1 < s2; 0 if equal; > 0 if s1 > s2
*/
int __strcmp( register const char *s1, register const char *s2 );

/*
** Name:        __strcpy
**
** Description: Copy a string into a destination buffer
**
** May not correctly deal with overlapping buffers
**
** Arguments:   The destination and source buffers.
*/
char *__strcpy( register char *dst, register const char *src );

/*
** Name:	__cvtdec0
**
** Description:	Convert an integer value into a decimal character string.
**
** Arguments:	The integer to convert and a buffer to hold the characters.
** Returns:	A pointer to the buffer.
*/
char *__cvtdec0( char *buf, int value );

/*
** Name:	__cvtdec
**
** Description:	Convert an integer value into a decimal character string.
**
** Arguments:	The integer to convert and a buffer to hold the characters.
** Returns:	The length of the string.
*/
int __cvtdec( char *buf, int value );

extern char __hexdigits[];

/*
** Name:	__cvthex
**
** Description:	Convert an integer value into a hex character string.
**
** Arguments:	The integer to convert and a buffer to hold the characters.
** Returns:	The length of the string.
*/
int __cvthex( char *buf, int value );

/*
** Name:	__cvtoct
**
** Description:	Convert an integer value into an octal character string.
**
** Arguments:	The integer to convert and a buffer to hold the characters.
** Returns:	The length of the string.
*/
int __cvtoct( char *buf, int value );

/*
** Name:	__inb, __inw, __inl
**
** Description:	Read one byte, word, or longword from an input port
**
** Argument:	The port from which to read
**
** Returns:	The data we read (extended to an integer)
*/
int __inb( int port );
int __inw( int port );
int __inl( int port );

/*
** Name:	__outb, __outw, __outl
**
** Description:	Write one byte, word, or longword to an output port
**
** Arguments:	The port to write to, and the value to write (only the
**		low-order bytes are written)
*/
void __outb( int port, int value );
void __outw( int port, int value );
void __outl( int port, int value );

/*
** Name:	__get_flags
**
** Description:	Get the current processor flags
**
** Returns:	The EFLAGS register after entry to this function
*/
unsigned int __get_flags( void );

/*
** Name:	__pause
**
** Description:	Pause until something happens
*/
void __pause( void );

/*
** __get_ra:
**
** Description: Get the return address for the calling function
**              (i.e., where whoever called us will go back to)
**
** Returns:	The address the calling routine will return to.
*/
void *__get_ra( void );

#endif
