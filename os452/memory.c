/*
** SCCS ID:	@(#)memory.c	1.1	4/2/19
**
** File:	memory.c
**
** Author:	Kenneth Reek and the 4003-506 class of 20013
**
** Contributor:	Warren R. Carithers
**
** Description:	Functions to perform dynamic memory allocation in the OS.
**		NOTE: these should NOT be called by user processes!
*/

#define	__SP_KERNEL__

#include "common.h"
#include "klib.h"

#include <x86arch.h>
#include "bootstrap.h"
#include "memory.h"
#include "cio.h"

/*
** PRIVATE DEFINITIONS
*/

// parameters related to word size

#define	WORD_SIZE		sizeof(int)
#define	LOG2_OF_WORD_SIZE	2

/*
** Name:	adjacent
**
** Arguments:	addresses of two blocks
**
** Description:	Determines whether the second block immediately follows
**		the first one.
*/
#define	adjacent(first,second)	\
    ( &(first)->info.memory + (first)->length == (int *)(second) )

/*
** Name:	length_of
**
** Arguments:	address of a block
**
** Description:	Computes the total length of a block (including the
**		header).
*/
#define	length_of(block)	\
    ( (block)->length + sizeof( (block)->length ) / WORD_SIZE )

/*
** PRIVATE DATA TYPES
*/

/*
** This structure keeps track of a single block of memory.  It is also
** used to remember the length of a block that has been allocated.
*/

typedef struct st_blkinfo {
    uint32_t length;		// usable length of this block, in words
    union {
        struct st_blkinfo *next;	// pointer to the next free block
        int memory;		// first word of the block
    } info;
} Blockinfo;

/*
** PRIVATE GLOBAL VARIABLES
*/

static Blockinfo *_freelist;

/*
** PUBLIC GLOBAL VARIABLES
*/

extern int _end;	// end of the BSS section - provided by the linker

/*
** PRIVATE FUNCTIONS
*/

/*
** Name:	_add_block
**
** Description:	Add a block to the free list
** Arguments:	Base address of the block and its length in bytes
*/
static void _add_block( uint32_t base, uint32_t length ) {
    Blockinfo *block;

    /*
    ** create the "block"
    **
    ** the "usable length" of the block doesn't include the length
    ** field itself - that's hidden from the rest of the system
    */

    block = (Blockinfo *) base;
    block->length = (int *) (base + length) - &block->info.memory;
    block->info.next = NULL;

#ifdef DB_KMALLOC_FREELIST
    __cio_printf( "## _add_block(%08x,%d): addr %08x len %d\n",
                  base, length, (uint32_t) block, block->length );
#endif
    /*
    ** We maintain the free list in order by address, to simplify
    ** coalescing adjacent free blocks.
    **
    ** Handle the easiest case first.
    */

    if( _freelist == NULL ) {
        _freelist = block;
        return;
    }

    /*
    ** Unfortunately, it's not always that easy....
    **
    ** Find the correct insertion spot.
    */

    Blockinfo *prev, *curr;

    prev = NULL;
    curr = _freelist;

    while( curr && curr < block ) {
        prev = curr;
        curr = curr->info.next;
    }

    /*
    ** we've found the spot!
    **
    ** this code may look familiar; if it does, that's because
    ** it's based on the insertion code in _q_insert(). :-)
    */

    // the new block always points to its successor

    block->info.next = curr;

    if( prev == NULL ) {

        // add at the beginning

        if( curr == NULL ) {
            // oops!
	 _kpanic( "_add_block", "empty non-empty free list?" );
        }

        _freelist = block;

    } else {

        // inserting in the middle or at the end

        prev->info.next = block;

        // the new block already points to its successor

    }

}

/*
** PUBLIC FUNCTIONS
*/

/*
** Name:	_km_dump_freelist
**
** Description:	Dump the current contents of the free list to the console
*/
void _km_dump_freelist( void ){
    Blockinfo *block;

    __cio_printf( "&_freelist=%08x\n", &_freelist );

    for( block = _freelist; block != NULL; block = block->info.next ){

        __cio_printf( "block=%08x length=%08x (ends at %08x) next=%08x\n",
                       block, block->length,
		       block->length * 4 + 4 + (int)block,
                       block->info.next );
    }

}

/*
** Name:	_km_alloc
**
** Description:	Allocate a block of memory from the free list.  The length
**		argument is rounded up to the next integral number of words.
**		More memory may be returned than was requested, but the
**		caller must not depend on this in any way.  If no memory
**		is available, NULL is returned.
*/
void *_km_alloc( uint32_t desired_length ){
    Blockinfo *block;
    Blockinfo **pointer;

#ifdef DB_KMALLOC
    __cio_printf( ">> kmalloc(%d)", desired_length );
#endif
    /*
    ** Convert the length to words (must round the count up to the next
    ** integral multiple of the word size).
    */
    desired_length += WORD_SIZE - 1;
    desired_length >>= LOG2_OF_WORD_SIZE;
#ifdef DB_KMALLOC
    __cio_printf( ", len -> %d", desired_length );
#endif

    /*
    ** Look for the first entry that is large enough.
    */
    pointer = &_freelist;
    block = _freelist;
    while( block != NULL && block->length < desired_length ){
        pointer = &block->info.next;
        block = *pointer;
    }

    if( block == NULL ){
        /*
        ** No blocks were found that are large enough.
        */
        return NULL;
    }

    /*
    ** Found an entry.  See if it is bigger than we need.
    */
    if( block->length > desired_length
         + sizeof( Blockinfo ) / WORD_SIZE ){
        /*
        ** Yes, there is enough room to allocate only a piece
        ** of this block.  Construct a new block at the end of
        ** this one.
        */
#ifdef DB_KMALLOC
    __cio_printf( ", got %08x/%d", (uint32_t)block, block->length );
#endif
        Blockinfo *fragment;
        int fragment_size;

        fragment_size = sizeof( block->length ) / WORD_SIZE
             + desired_length;
        fragment = (Blockinfo *)( &block->info.memory + block->length
             - fragment_size );
        fragment->length = desired_length;
        block->length -= fragment_size;
        block = fragment;
    }
    else {
        /*
        ** No, it is just the right size.  Remove it from
        ** the list.
        */
        *pointer = block->info.next;
    }
#ifdef DB_KMALLOC
    __cio_printf( ", returns %08x/%d\n", (uint32_t) (&block->info.memory),
                  block->length );
#endif
    return &block->info.memory;
}

/*
** Name:	_km_free
**
** Description:	returns a memory block to the list of available blocks,
**		combining it with adjacent blocks if they're present.
*/
void _km_free( void *address ){
    Blockinfo *used;
    Blockinfo *block;
    Blockinfo *previous;

    /*
    ** Don't do anything if the address is NULL.
    */
    if( address == NULL ){
        return;
    }

    used = (Blockinfo *)( (char *)address - WORD_SIZE );
    block = _freelist;
    previous = NULL;

    /*
    ** Advance through the list until block and previous
    ** straddle the place where the new block should be inserted.
    */
    while( block != NULL && block < used ){
        previous = block;
        block = block->info.next;
    }

    /*
    ** Is the new block going to be the first one in the list?
    */
    if( previous != NULL ){
        /*
        ** No.  See if it is adjacent to the previous block.
        */
        if( adjacent( previous, used ) ){
            /*
            ** Yes, combine them.
            */
            previous->length += length_of( used );
            used = previous;
        }
        else {
            /*
            ** No: insert the new block.
            */
            used->info.next = previous->info.next;
            previous->info.next = used;
        }
    }
    else {
        /*
        ** Yes, it is first.  Must update the list pointer to insert it.
        */
        used->info.next = _freelist;
        _freelist = used;
    }

    /*
    ** Is the new block going to be the last block in the list?
    */
    if( block != NULL ){
        /*
        ** No.  See if it is adjacent to the next block.
        */
        if( adjacent( used, block ) ){
            /*
            ** Yes, combine them.
            */
            used->info.next = block->info.next;
            used->length += length_of( block );
        }
    }
}

/*
** Memory region information returned by the BIOS
**
** This data consists of a 32-bit integer followed
** by an array of region descriptor structures.
*/

typedef struct st_MemRegion {
    uint32_t base[2];	// base address
    uint32_t length[2];	// region length
    uint32_t type;	// type of region
    uint32_t acpi;	// ACPI 3.0 info
} __attribute__((packed)) region_t;

/*
** Region types
*/

#define	REGION_USABLE		1
#define	REGION_RESERVED		2
#define	REGION_ACPI_RECL	3
#define	REGION_ACPI_NVS		4
#define	REGION_BAD		5

/*
** ACPI 3.0 bit fields
*/

#define	REGION_IGNORE		0x01
#define	REGION_NONVOL		0x02

/*
** Name:	_km_init
**
** Description:	Find what memory is present on the system and construct
**		the list of free memory blocks.
*/
void _km_init( void ){
    int32_t entries;
    region_t *region;
    uint64_t cutoff;

    // unlike the other init routines, we'll announce at the end

    // initially, nothing in the free list

    _freelist = NULL;

    // set our cutoff point

    cutoff = (uint32_t) &_end;

    // round it up to the next multiple of 0x10000

    if( cutoff & 0xffff ) {
    	cutoff &= 0xffff0000;
	cutoff += 0x00010000;
    }

#ifdef DB_KMALLOC
    __cio_printf( "+++ _km_init, _end %08x cutoff %08x\n", 
                  (uint32_t) &_end, (uint32_t) cutoff );
#endif

    // if there are no entries, we have nothing to do!

    entries = *((int32_t *) MMAP_ADDRESS);
    // __cio_printf( "%d entries\n", entries );

    if( entries < 1 ) {  // note: entries == -1 could occur!
        return;
    }

    // iterate through the entries, adding things to the freelist

    region = ((region_t *) (MMAP_ADDRESS + 4));

    for( int i = 0; i < entries; ++i, ++region ) {

#ifdef DB_KMALLOC_INIT
        __cio_printf( "#%2d: ", i );
        __cio_printf( "B(%08x,%08x) L(%08x,%08x)",
                      region->base[1], region->base[0],
                      region->length[1], region->length[0] );
        __cio_printf( " T(%08x) A(%08x)\n", region->type, region->acpi );
#endif

        /*
        ** Determine whether or not we should ignore this region.
        **
        ** We ignore regions for several reasons:
        **
        **	ACPI indicates it should be ignored
        **	ACPI indicates it's non-volatile memory
        **	Region type isn't "usable"
        **
        ** Currently, only "normal" (type 1) regions are considered
        ** "usable" for our purposes.  We could potentially expand
        ** this to include ACPI "reclaimable" memory.
        */

        if( ((region->acpi) & REGION_IGNORE) == 0 ) {
            continue;
        }

        if( ((region->acpi) & REGION_NONVOL) == 1 ) {
            continue;  // we'll ignore this, too
        }

        if( (region->type) != REGION_USABLE ) {
            continue;  // we won't attempt to reclaim ACPI memory (yet)
        }

        // OK, we have a "normal" memory region - verify that it's usable

        // ignore it if it's above 4GB, or longer than that

        if( region->base[1] != 0 || region->length[1] != 0 ) {
            continue;
        }

        // see if it's below our arbitrary cutoff point
        	

        uint32_t base = region->base[0];
        uint32_t length = region->length[0];

        if( base < cutoff ) {

            // is the whole thing too low, or just part?

	 if( (base + length) < cutoff ) {
	     // too short!
	     continue;
	 }

	 // recalculate the length, starting at our cutoff point

	 uint32_t loss = cutoff - base;

	 // reset the length and the base address
	 length -= loss;
	 base = cutoff;

        }

        // add the new block

        _add_block( base, length );

    }

#ifdef DB_KMALLOC
    // finally, dump out the resulting free list
    _km_dump_freelist();
#endif

    // announce that we have completed initialization

    __cio_puts( " KMEM" );

}
