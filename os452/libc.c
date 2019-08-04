/*
** SCCS ID:	@(#)libc.c	1.1	4/2/19
**
** File:	libc.c
**
** Author:	Warren R. Carithers
**
** Description:	Library of support functions (C language)
**
** This module implements a simple collection of support functions
** similar to the standard C library.  Several of these functions
** were originally found in the CIO module.
**
*/

#include "lib.h"

/*
** Name:	__bound
**
** Description:	This function confines an argument within specified bounds.
**
** Arguments:	The bounds and the value to be constrained.
** Returns:	The constrained value.
*/
unsigned int __bound( unsigned int min, unsigned int value,
                        unsigned int max ){
	if( value < min ){
		value = min;
	}
	if( value > max ){
		value = max;
	}
	return value;
}

/*
** Name:        __memset
**
** Description: initialize all bytes of a block of memory to a specific value
**
** Arguments:   The buffer, its length (in bytes), and the init value
*/
void __memset( void *buf, register unsigned int len,
               register unsigned int value ) {
	register unsigned char *bp = buf;

	/*
	** We could speed this up by unrolling it and copying
	** words at a time (instead of bytes).
	*/

	while( len-- ) {
		*bp++ = value;
	}
}

/*
** Name:        __memclr
**
** Description: Initialize all bytes of a block of memory to zero
**
** Arguments:   The buffer and its length (in bytes)
*/
void __memclr( void *buf, register unsigned int len ) {
	register unsigned char *dest = buf;

	/*
	** We could speed this up by unrolling it and clearing
	** words at a time (instead of bytes).
	*/

	while( len-- ) {
     		*dest++ = 0;
	}

}

/*
** Name:        __memcpy
**
** Description: Copy a block from one place to another
**
** May not correctly deal with overlapping buffers
**
** Arguments:   The destination and source buffers, and the length (in bytes)
*/
void __memcpy( void *dst, register const void *src,
               register unsigned int len ) {
	register unsigned char *dest = dst;
	register const unsigned char *source = src;

	/*
	** We could speed this up by unrolling it and copying
	** words at a time (instead of bytes).
	*/

	while( len-- ) {
		*dest++ = *source++;
	}

}

/*
** Name:        __strlen
**
** Description: Calculate the length of a C-style string.
**
** Arguments:   The string.
** Returns:     The string's length.
*/

unsigned int __strlen( register const char *str ){
	register unsigned int len = 0;

	while( *str++ ){
		++len;
	}
	return len;
}

/*
** Name:        __strcpy
**
** Description: Copy a string into a destination buffer
**
** May not correctly deal with overlapping buffers
**
** Arguments:   The destination and source buffers.
*/
char *__strcpy( register char *dst, register const char *src ) {
	char *tmp = dst;

	while( (*dst++ = *src++) )
		;

	return( tmp );
}

/*
** Name:        __strcmp
**
** Description: Compare two strings
**
** Arguments:   The two source buffers.
** Returns:     < 0 if s1 < s2; 0 if equal; > 0 if s1 > s2
*/
int __strcmp( register const char *s1, register const char *s2 ) {

	while( *s1 && (*s1 == *s2) )
		++s1, ++s2;

	return( *s1 - *s2 );
}

/*
** Name:	__cvtdec0
**
** Description:	Convert an integer value into a decimal character string.
**
** Arguments:	The integer to convert and a buffer to hold the characters.
** Returns:	A pointer to the buffer.
*/
char *__cvtdec0( char *buf, int value ){
	int	quotient;

	quotient = value / 10;
	if( quotient < 0 ){
		quotient = 214748364;
		value = 8;
	}
	if( quotient != 0 ){
		buf = __cvtdec0( buf, quotient );
	}
	*buf++ = value % 10 + '0';
	return buf;
}

/*
** Name:	__cvtdec
**
** Description:	Convert an integer value into a decimal character string.
**
** Arguments:	The integer to convert and a buffer to hold the characters.
** Returns:	A pointer to the buffer.
*/
int __cvtdec( char *buf, int value ){
	char	*bp = buf;

	if( value < 0 ){
		*bp++ = '-';
		value = -value;
	}
	bp = __cvtdec0( bp, value );
	*bp = '\0';

	return bp - buf;
}

char __hexdigits[] = "0123456789ABCDEF";

/*
** Name:	__cvthex
**
** Description:	Convert an integer value into a hex character string.
**
** Arguments:	The integer to convert and a buffer to hold the characters.
** Returns:	A pointer to the buffer.
*/
int __cvthex( char *buf, int value ){
	int	i;
	int	chars_stored = 0;
	char	*bp = buf;

	for( i = 0; i < 8; i += 1 ){
		int	val;

		val = ( value & 0xf0000000 );
		if( i == 7 || val != 0 || chars_stored ){
			chars_stored = 1;
			val >>= 28;
			val &= 0xf;
			*bp++ = __hexdigits[ val ];
		}
		value <<= 4;
	}
	*bp = '\0';

	return bp - buf;
}

/*
** Name:	__cvtoct
**
** Description:	Convert an integer value into an octal character string.
**
** Arguments:	The integer to convert and a buffer to hold the characters.
** Returns:	A pointer to the buffer.
*/
int __cvtoct( char *buf, int value ){
	int	i;
	int	chars_stored = 0;
	char	*bp = buf;
	int	val;

	val = ( value & 0xc0000000 );
	val >>= 30;
	for( i = 0; i < 11; i += 1 ){

		if( i == 10 || val != 0 || chars_stored ){
			chars_stored = 1;
			val &= 0x7;
			*bp++ = __hexdigits[ val ];
		}
		value <<= 3;
		val = ( value & 0xe0000000 );
		val >>= 29;
	}
	*bp = '\0';

	return bp - buf;
}
