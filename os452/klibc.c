/*
** SCCS ID:	@(#)klibc.c	1.1	4/2/19
**
** File:	klibc.c
**
** Author:	Warren R. Carithers and various CSCI-452 sections
**
** Contributor:
**
** Description:	C implementations of kernel library functions
*/

#define	__SP_KERNEL__

#include "common.h"

#include "klib.h"
#include "cio.h"
#include "queues.h"
#include "pcbs.h"

// needed for external queue declarations for _kpanic()
#include "scheduler.h"
#include "sio.h"
#include "syscalls.h"

/*
** PRIVATE DEFINITIONS
*/

/*
** PRIVATE DATA TYPES
*/

/*
** PRIVATE GLOBAL VARIABLES
*/

/*
** PUBLIC GLOBAL VARIABLES
*/

/*
** PRIVATE FUNCTIONS
*/

/*
** PUBLIC FUNCTIONS
*/

/*
** _put_char_or_code( ch )
**
** prints the character on the console, unless it is a non-printing
** character, in which case its hex code is printed
*/
void _put_char_or_code( int ch ) {

    if( ch >= ' ' && ch < 0x7f ) {
        __cio_putchar( ch );
    } else {
        __cio_printf( "\\x%02x", ch );
    }
}

/*
** _kmemset - set all bytes of a block of memory to a specific value
**
** usage:  _kmemset( buffer, length, value )
*/
void _kmemset( register uint8_t *buf, register uint32_t len,
                    register uint8_t value ) {

    while( len-- ) {
        *buf++ = value;
    }

}

/*
** _kmemclr - initialize all bytes of a block of memory to zero
**
** usage:  _kmemclr( buffer, length )
*/
void _kmemclr( register uint8_t *buf, register uint32_t len ) {

    while( len-- ) {
        *buf++ = 0;
    }

}

/*
** _kmemcpy - copy a block from one place to another
**
** usage:  _kmemcpy( dest, src, length )
**
** may not correctly deal with overlapping buffers
*/
void _kmemcpy( register uint8_t *dst, register uint8_t *src,
                    register uint32_t len ) {

    while( len-- ) {
        *dst++ = *src++;
    }

}

/*
** _kstrcpy - copy a NUL-terminated string
*/
char *_kstrcpy( register char *dst, register const char *src ) {
    char *tmp = dst;

    while( (*dst++ = *src++) )
        ;

    return( tmp );
}

/*
** _kstrlen() - return length of a NUL-terminated string
*/
uint32_t _kstrlen( register const char *str ) {
    register uint32_t len = 0;

    while( *str++ ) {
        ++len;
    }

    return( len );
}

/*
** _kstrcmp - compare two NUL-terminated strings
*/
int _kstrcmp( register const char *s1, register const char *s2 ) {

    while( *s1 != 0 && (*s1 == *s2) )
        ++s1, ++s2;

    return( *(const unsigned char *)s1 - *(const unsigned char *)s2 );
}

/*
** _kpanic - kernel-level panic routine
**
** usage:  _kpanic( mod, msg )
**
** Prefix routine for __panic() - can be expanded to do other things
** (e.g., printing a stack traceback)
**
** 'mod' argument is always printed; 'msg' argument is printed
** if it isn't NULL, followed by a newline
*/
void _kpanic( char *mod, char *msg ) {

    __cio_puts( "\n\n***** KERNEL PANIC *****\n\n" );
    __cio_printf( "Mod:  %s   Msg: %s\n", mod, msg ? msg : "(none)" );

    // dump a bunch of potentially useful information

    _pcb_dump( "Current", _current );

#if PANIC_DUMPS_QUEUES
    _queue_dump( "Sleep queue", _sleeping );
    _queue_dump( "Waiting queue", _waiting );
    _queue_dump( "Reading queue", _reading );
    _queue_dump( "Zombie queue", _zombie );
    _queue_dump( "Ready queue 0", _ready[PRIO_SYS] );
    _queue_dump( "Ready queue 1", _ready[PRIO_USER_H] );
    _queue_dump( "Ready queue 2", _ready[PRIO_USER_S] );
    _queue_dump( "Ready queue 3", _ready[PRIO_DEFERRED] );
#else
    __cio_printf( "Queue sizes:  sleep %d", _queue_length(_sleeping) );
    __cio_printf( " wait %d read %d zombie %d", _queue_length(_waiting),
                  _queue_length(_reading), _queue_length(_zombie) );
    __cio_printf( " ready [%d,%d,%d,%d]\n",
                  _queue_length(_ready[PRIO_SYS]),
                  _queue_length(_ready[PRIO_USER_H]),
                  _queue_length(_ready[PRIO_USER_S]),
	          _queue_length(_ready[PRIO_DEFERRED]) );
#endif

   _active_dump( "Processes", false );

   // could dump other stuff here, too

   __panic( "KERNEL PANIC" );
}
