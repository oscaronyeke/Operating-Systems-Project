/*
** SCCS ID:	@(#)users.h	1.1	4/2/19
**
** File:	users.h
**
** Author:	Warren R. Carithers and various CSCI-452 classes
**
** Contributor: Josh Bicking Oscar Onyeke
**
** Description:	control of user-level routines
*/

#ifndef _USERS_H_
#define _USERS_H_

/*
** General (C and/or assembly) definitions
*/

// delay loop counts

#define	DELAY_LONG	100000000
#define	DELAY_STD	  2500000
#define	DELAY_ALT	  4500000

// a delay loop

#define	DELAY(n)	for(int _dlc = 0; _dlc < (DELAY_##n); ++_dlc) continue;

#ifndef __SP_ASM__

/*
** Start of C-only definitions
*/

/*
** User process controls.
**
** The comment field of these definitions contains a list of the
** system calls this user process uses.
**
** To spawn a specific user process from the initial process,
** uncomment its entry in this list.
*/

// syscalls in this system:  read, write, fork, exec, wait, sleep, exit,
//                           time, getpid, getppid, getprio, setprio
//
// fork and exec may be called directly by these processes, or may
// be called by the spawn() function in the library

//	user	          baseline system calls in use
//		    rd  wrt frk exe wat sle xit tim pid ppd gtp stp
#define SPAWN_A	//   .   X   .   .   .   .   X   .   .   .   .   .
#define SPAWN_B	//   .   X   .   .   .   .   X   .   .   .   .   .
#define SPAWN_C	//   .   X   .   .   .   .   X   .   .   .   .   .
#define SPAWN_D	//   .   X   X   X   .   .   X   .   .   .   X   X
#define SPAWN_E	//   .   X   .   .   .   X   X   .   X   .   .   .
#define SPAWN_F	//   .   X   .   .   .   X   X   .   X   .   .   .
#define SPAWN_G	//   .   X   .   .   .   X   X   .   X   .   .   .
	// user H doesn't call exit()
#define SPAWN_H	//   .   X   .   .   .   .   .   .   .   .   .   .
// no user I
	// user J tries to spawn() 2*MAX_PROCS children
//#define SPAWN_J	//   .   X   X   X   .   .   X   .   .   .   .   .
//#define SPAWN_K	//   .   X   X   X   .   X   X   .   .   .   .   .
//#define SPAWN_L	//   .   X   X   X   .   X   X   X   .   .   .   .
//#define SPAWN_M	//   .   X   X   X   .   .   X   .   .   .   .   .
	// user N spawns W and Z at low priority
#define SPAWN_N	//   .   X   X   X   .   .   X   .   .   .   .   .
// no user O
#define SPAWN_P	//   .   X   .   .   .   X   X   X   .   .   .   .
 	// user Q makes a bogus system call
#define SPAWN_Q	//   .   X   .   .   .   .   X   .   .   .   .   .
	// user R should loop forever
#define SPAWN_R	//   X   X   .   .   .   X   X   .   .   .   .   .
	// user S should loop forever
#define SPAWN_S	//   .   X   .   .   .   X   X   .   .   .   .   .
#define SPAWN_T	//   .   X   X   X   X   X   X   .   .   .   .   .
#define SPAWN_U	//   .   X   X   X   .   X   X   .   .   .   .   .

// Message queue processes
//#define SPAWN_MQ_A
//#define SPAWN_MQ_B_C
//#define SPAWN_MQ_D
//#define SPAWN_MQ_E_F
//#define SPAWN_MQ_G_H
//#define SPAWN_MQ_I_J
//#define SPAWN_MQ_K
#if 0
#endif
// no user V
// users W-Z are spawned by other processes

/*
** Users W-Z are spawned from other processes; they
** should never be spawned directly by init().
*/

// user W:	//   .   X   .   .   .   X   X   .   X   .   .   .
	// user X exits with a non-zero status
// user X:	//   .   X   .   .   .   .   X   .   X   .   .   .
// user Y:	//   .   X   .   .   .   X   X   .   .   .   .   .
// user Z:	//   .   X   .   .   .   .   X   .   X   X   .   .


/*
** Prototypes for externally-visible routines
*/

/*
** init - initial user process
**
** after spawning the other user processes, loops forever calling wait()
*/
int init( int argc, void *arg1, void *arg2 );

/*
** idle - what to run when there's nothing else to run
*/
int idle( int argc, void *arg1, void *arg2 );

#endif

#endif
