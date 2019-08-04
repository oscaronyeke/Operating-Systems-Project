/*
** SCCS ID:	@(#)ulib.h	1.1	4/2/19
**
** File:	ulib.h
**
** Author:	CSCI-452 class of 20185
**
** Contributor: Oscar Onyeke Josh
**
** Description:	declarations for user-level library functions
*/

#ifndef _ULIB_H_
#define _ULIB_H_

#include "types.h"

/*
** General (C and/or assembly) definitions
*/

#ifndef __SP_ASM__

/*
** Start of C-only definitions
*/

/*
** Types
*/

/*
** Globals
*/

/*
** Prototypes
*/

/*
**********************************************
** SYSTEM CALLS
**********************************************
*/

/*
** read - read into a buffer from a stream
**
** usage:	n = read(stream,buf,size)
**
** @param stream I/O stream to read from
** @param buf    Buffer to read into
** @param size   Maximum capacity of the buffer
**
** @returns      The count of characters transferred, or an error code
*/
int32_t read( int stream, void *buffer, uint32_t length );

/*
** write - write from a buffer to a stream
**
** usage:	n = write(stream,buf,size)
**
** @param stream I/O stream to write to
** @param buf    Buffer to write from
** @param size   Number of bytes to write
**
** @returns      The count of characters transferred, or an error code
*/
int32_t write( int stream, const void *buf, uint32_t length );

/*
** fork - create a new process
**
** usage:	pid = fork();
**
** @returns 0 to the child, -1 on error or the child's PID to the parent
*/
int32_t fork( void );

/*
** exec - replace this program with a different one
**
** usage:	pid = exec(entry,prio,argc,arg1,arg2);
**
** @param entry The entry point of the new code
** @param prio  The desired priority for the process
** @param argc  Number of "command-line" arguments being passed
** @param arg1  First "command-line" argument
** @param arg2  Second "command-line" argument
**
** @returns Only on failure, returns an error code
*/
uint32_t exec( int (*entry)(int,void*,void*), uint8_t prio,
               int argc, void *arg1, void *arg2 );

/*
** wait - wait for a child process to terminate
**
** usage:	pid = wait(&status);
**
** @param status Pointer to int32_t into which the child's status is placed
**
** @returns The PID of the terminated child, or an error code
**
** If there are no children in the system, returns an error code (*status
** is unchanged).
**
** If there are one or more children in the system and at least one has
** terminated but hasn't yet been cleaned up, cleans up that process and
** returns its information; otherwise, blocks until a child terminates.
*/
int32_t wait( int32_t *status );

/*
** sleep - put the current process to sleep for some length of time
**
** usage:	sleep(n);
**
** @param n Desired sleep time, in MS
*/
void sleep( uint32_t msec );

/*
** exit - terminate the calling process
**
** usage:	exit(status);
**
** @param status Terminations tatus of this process
**
** @returns Does not return
*/
void exit( int32_t status );

/*
** time - retrieve the current system time
**
** usage:	n = time();
**
** @returns The current system time
*/
uint32_t time( void );

/*
** getpid - retrieve PID of this process
**
** usage:	n = getpid();
**
** @returns The PID of this process
*/
int32_t getpid( void );

/*
** getppid - retrieve PID of the parent of this process
**
** usage:	n = getppid();
**
** @returns The PID of the parent of this process
*/
int32_t getppid( void );

/*
** getprio - retrieve the priority of the specified process
**
** usage:	prio = getprio(pid);
**
** @param pid The PID of the process to check, or -1 for this process
**
** @returns The priority of the process, or an error code
*/
uint8_t getprio( int32_t pid );

/*
** setprio - set the priority of the specified process
**
** usage:	prio = setprio(pid,prio);
**
** @param pid  The PID of the process to modify, or -1 for this process
** @param prio The desired priority
**
** @returns The old priority of the process, or an error code
*/
uint8_t setprio( int32_t whom, uint8_t prio );

/*
** mkdir - creates a directory in the file system
**
** usage:       n = read(stream,buf,size)
**
** @param stream I/O stream to read from
** @param buf    Buffer to read into
** @param size   Maximum capacity of the buffer
**
** @returns      The count of characters transferred, or an error code
*/
int32_t mkdir(int stream, void *buf, uint32_t length);


/*
** create - creates a file in a directory system
**
** usage:       n = read(stream,buf,size)
**
** @param stream I/O stream to read from
** @param buf    Buffer to read into
** @param size   Maximum capacity of the buffer
**
** @returns      The count of characters transferred, or an error code
*/
int32_t create(int stream, void *buf, uint32_t length);

/*
** fread - creates a directory in the file system
**
** usage:       n = fread(stream,buf,size)
**
** @param stream I/O stream to read from
** @param buf    Buffer to read into
** @param size   Maximum capacity of the buffer
**
** @returns      The count of characters transferred, or an error code
*/
int32_t fread(int stream, void *buf, uint32_t length);


/*
** fwrite - creates a file in a directory system
**
** usage:       n = fwrite(stream,buf,size)
**
** @param stream I/O stream to read from
** @param buf    Buffer to read into
** @param size   Maximum capacity of the buffer
**
** @returns      The count of characters transferred, or an error code
*/
int32_t fwrite(int stream, void *buf, uint32_t length);

/*
** ls - list the files in a directory
**
** usage:       n = ls(stream,buf,size)
**
** @param stream I/O stream to read from
** @param buf    Buffer to read into
** @param size   Maximum capacity of the buffer
**
** @returns      The count of characters transferred, or an error code
*/
int32_t ls(int stream, void *buf, uint32_t length);


/*
** rm - removes a directory/file
**
** usage:       n = rm(stream,buf,size)
**
** @param stream I/O stream to read from
** @param buf    Buffer to read into
** @param size   Maximum capacity of the buffer
**
** @returns      The count of characters transferred, or an error code
*/
int32_t rm(int stream, void *buf, uint32_t length);



/*
** mq_open() - Open a message queue of some name. Will be created if it doesn't
** exist.
**
** @param name The name of the message queue to open or create
** @return > 0 on success, or < 0 on error.
*/
int16_t mq_open(char* name);

/*
** mq_read() - Read a message from an mqueue into a buffer.
**
** @param m        Message queue descriptor
** @param buf      Buffer to write the message into
** @param msg_len  Length of the message
** @param prio     Lowest priority of the message
** @param block    If the queue has no valid messages empty, should we wait until
**                 it does?
** @return         The number of bytes read
*/
int16_t mq_read(int16_t m, uint8_t* buf, uint32_t msg_len, uint8_t prio, bool block);

/*
** mq_write() - Write a message from a buffer into an mqueue.
**
** If the queue is full and block=true, wait until the queue has been read from.
**
** Return 0 on success, or less than 0 on error.
**
** @param m        Message queue descriptor
** @param buf      Buffer to write the message into
** @param msg_len  Length of the message
** @param prio     Priority of the message
** @param block    If the queue is full, should we wait until it's not?
** @return         0 on success, < 0 on error
**
*/
int16_t mq_write(int16_t m, uint8_t* buf, uint32_t msg_len, uint8_t prio, bool block);

/*
** Close an mqueue. All queued messages are destroyed.
**
** Return 0 on success, or less than 0 on error.
**
** @param m  The message queue descriptor to close
** @return   0 on success, < 0 on error
*/
int16_t mq_close(int16_t m);


/*
** bogus - a bogus system call, for testing our syscall ISR
**
** usage:       bogus();
*/
void bogus( void );

/*
**********************************************
** CONVENIENT "SHORTHAND" VERSIONS OF SYSCALLS
**********************************************
*/

/*
** spawn(entry,prio,argc,arg1,arg2 - create a new process running
**                                   a different program
**
** usage:       n = spawn(entry,prio,argc,arg1,arg2)
**
** Creates a new process, changes its priority, and then execs 'entry'
**
** @param entry The entry point of the new code
** @param prio  The desired priority for the process
** @param argc  Number of "command-line" arguments being passed
** @param arg1  First "command-line" argument
** @param arg2  Second "command-line" argument
**
** @returns     The PID of the child, or an error code
*/
int32_t spawn( int (*entry)(int,void*,void*), uint8_t prio,
               int argc, void *arg1, void *arg2 );

/*
** cwritech(ch) - write a single character to the console
**
** @param ch The character to write
**
** @returns The return value from calling write()
*/
int cwritech( char ch );

/*
** cwrites(str) - write a NUL-terminated string to the console
**
** @param str The string to write
**
** @returns The return value from calling write()
*/
int cwrites( const char *str );

/*
** cwrite(buf,size) - write a sized buffer to the console
**
** @param buf  The buffer to write
** @param size The number of bytes to write
**
** @returns The return value from calling write()
*/
int cwrite( const char *buf, uint32_t size );

/*
** swritech(ch) - write a single character to the SIO
**
** @param ch The character to write
**
** @returns The return value from calling write()
*/
int swritech( char ch );

/*
** swrites(str) - write a NUL-terminated string to the SIO
**
** @param str The string to write
**
** @returns The return value from calling write()
*/
int swrites( const char *str );

/*
** swrite(buf,size) - write a sized buffer to the SIO
**
** @param buf  The buffer to write
** @param size The number of bytes to write
**
** @returns The return value from calling write()
*/
int swrite( const char *buf, uint32_t size );

/*
**********************************************
** STRING MANIPULATION FUNCTIONS
**********************************************
*/

/*
** strlen(str) - return length of a NUL-terminated string
**
** @param str The string to examine
**
** @returns The length of the string, or 0
*/
uint32_t strlen( const char *str );

/*
** strcpy(dst,src) - copy a NUL-terminated string
**
** @param dst The destination buffer
** @param src The source buffer
**
** @returns The dst parameter
**
** NOTE:  assumes dst is large enough to hold the copied string
*/
char *strcpy( register char *dst, register const char *src );

/*
** strcat(dst,src) - append one string to another
**
** @param dst The destination buffer
** @param src The source buffer
**
** @returns The dst parameter
**
** NOTE:  assumes dst is large enough to hold the resulting string
*/
char *strcat( register char *dst, register const char *src );

/*
** strcmp(s1,s2) - compare two NUL-terminated strings
**
** @param s1 The first source string
** @param s1 The second source string
**
** @returns negative if s1 < s2, zero if equal, and positive if s1 > s2
*/
int strcmp( register const char *s1, register const char *s2 );

/*
** pad(dst,extra,padchar) - generate a padding string
**
** @param dst     Pointer to where the padding should begin
** @param extra   How many padding bytes to add
** @param padchar What character to pad with
**
** @returns Pointer to the first byte after the padding
**
** NOTE: does NOT NUL-terminate the buffer
*/
char *pad( char *dst, int extra, int padchar );

/*
** padstr(dst,str,len,width,leftadjust,padchar - add padding characters
**                                               to a string
**
** @param dst        The destination buffer
** @param str        The string to be padded
** @param len        The string length, or -1
** @param width      The desired final length of the string
** @param leftadjust Should the string be left-justified?
** @param padchar    What character to pad with
**
** @returns Pointer to the first byte after the padded string
**
** NOTE: does NOT NUL-terminate the buffer
*/
char *padstr( char *dst, char *str, int len, int width,
                   int leftadjust, int padchar );

/*
** sprint(dst,fmt,...) - formatted output into a string buffer
**
** @param dst The string buffer
** @param fmt Format string
**
** The format string parameter is followed by zero or more additional
** parameters which are interpreted according to the format string.
**
** NOTE:  assumes the buffer is large enough to hold the result string
**
** NOTE:  relies heavily on the x86 parameter passing convention
** (parameters are pushed onto the stack in reverse order as
** 32-bit values).
*/
void sprint( char *dst, char *fmt, ... );

/*
**********************************************
** MISCELLANEOUS USEFUL SUPPORT FUNCTIONS
**********************************************
*/

/*
** exit_helper()
**
** calls exit(%eax) - serves as the "return to" code for main()
** functions, in case they don't call exit() themselves
*/
void exit_helper( void );

/*
** cvt_dec(buf,value)
**
** convert a 32-bit signed value into a NUL-terminated character string
**
** @param buf    Destination buffer
** @param value  Value to convert
**
** @returns The number of characters placed into the buffer
**          (not including the NUL)
**
** NOTE:  assumes buf is large enough to hold the resulting string
*/
int cvt_dec( char *buf, int32_t value );

/*
** cvt_hex(buf,value)
**
** convert a 32-bit unsigned value into an (up to) 8-character
** NUL-terminated character string
**
** @param buf    Destination buffer
** @param value  Value to convert
**
** @returns The number of characters placed into the buffer
**          (not including the NUL)
**
** NOTE:  assumes buf is large enough to hold the resulting string
*/
int cvt_hex( char *buf, uint32_t value );

/*
** cvt_oct(buf,value)
**
** convert a 32-bit unsigned value into an (up to) 11-character
** NUL-terminated character string
**
** @param buf   Destination buffer
** @param value Value to convert
**
** @returns The number of characters placed into the buffer
**          (not including the NUL)
**
** NOTE:  assumes buf is large enough to hold the resulting string
*/
int cvt_oct( char *buf, uint32_t value );

/*
** report(ch,pid) - report to the console that user 'ch' is running as 'pid'
**
** @param ch   The one-character name of the user process
** @param whom The PID of the process
*/
void report( char ch, int32_t whom );

#endif

#endif
