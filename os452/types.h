/*
** SCCS ID:	@(#)types.h	1.1	4/2/19
**
** File:	types.h
**
** Author:	CSCI-452 class of 20185
**
** Contributor:
**
** Description:	Basic type definitions for the baseline system.
*/

#ifndef _TYPES_H_
#define _TYPES_H_

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

// Boolean type (because _Bool is stupid)

typedef enum bool_e { false, true } bool;

// size-specific integer data types

typedef long long int		int64_t;
typedef unsigned long long int	uint64_t;

typedef int			int32_t;
typedef unsigned int		uint32_t;

typedef short			int16_t;
typedef unsigned short		uint16_t;

typedef char			int8_t;
typedef unsigned char		uint8_t;

// kernel function status return values

typedef enum status_e {
    SUCCESS = 0,  // the "no error" error code!
    ERROR,        // generic "something went wrong" code
    BAD_PARAM,    // invalid/erroneous parameter
    BAD_ALLOC,    // allocation failure
    NOT_FOUND,    // desired object not found
    EMPTY         // empty data structure (e.g., queue)
} status_t;

/*
** Globals
*/

/*
** Prototypes
*/

#endif

#endif
