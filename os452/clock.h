/*
** SCCS ID:	@(#)clock.h	1.1	4/2/19
**
** File:	clock.h
**
** Author:	Warren R. Carithers and various CSCI-452 sections
**
** Contributor:
**
** Description:	Clock module declarations
*/

#ifndef _CLOCK_H_
#define _CLOCK_H_

#include "common.h"

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

extern uint32_t _system_time;   // the current system time

/*
** Prototypes
*/

/*
** _clk_init()
**
** initialize the clock module
*/
void _clk_init( void );

#endif

#endif
