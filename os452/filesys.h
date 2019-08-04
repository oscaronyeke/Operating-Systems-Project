//
// Created by Oscar on 4/4/2019.
//

#ifndef OS452_FILESYS_H
#define OS452_FILESYS_H


#define BLOCK_SZ        512

#define NUM_BLOCKS      512
#define ADDR_SZ         4     // unsigned uint32_t is 4 bytes
#define IN_MEM_FD       1    // in-memory fake file descriptor
#define FREE_BLOCKS_PER_LINK (2)     // # of block idx #s in a block

#define ILIST_SPACE (128)                  // # of blocks that contains inodes

#define MAX_FREE_ILIST_SIZE (64)          // # of free inodes in superblock ilist
#define FILE_OWNER_ID_LEN 16              // length of file owner's ID
#define DIRECT_BLOCKS_PER_INODE 10             // # of direct block addr in an inode
#define RANGE_SINGLE   (BLOCK_SZ>>2)            // block range of single indirect
#define RANGE_DOUBLE   (RANGE_SINGLE*RANGE_SINGLE)   // bLK range of double indirect
#define INODES_PER_BLOCK  (BLOCK_SZ/sizeof(struct disk_inode))    // # of inodes per block
#define DIR_ENTRY_LENGTH  64  // in bytes
#define DIR_ENTRIES_PER_BLOCK   (BLOCK_SZ/DIR_ENTRY_LENGTH)
#define FILE_NAME_LEN     (DIR_ENTRY_LENGTH-4)    // file name size in a dir entry
#define MAX_ENTRY_OFFSET  (10*DIR_ENTRY_LENGTH) // max entries in a directory
#define BAD_I_NUM         (0)    // inode num indicates the end of a dir entry
#define EMPTY_I_NUM        (-1)    // inode num indicates an unused dir entry
#define MAX_PATH_LEN      (50)   // max characters in a path
#define MAX_FILE_SIZE     (1000) //

#define NAMEI_CACHE_SZ          32      // number of path->inode mappings

#define _DEBUG       0 // 1: show debug info
#define USE_NAMEI_CACHE         0

#include "types.h"

struct super_block {
    uint32_t block_size;           // the block size
    uint32_t num_blocks;           // total number of blocks on the disk.
    uint64_t fs_size;            // file system size
    uint32_t max_free_blocks;      // max number of free blocks
    uint32_t num_free_blocks;      // current number of free blocks
    uint32_t data_block_offset;    // the block number of the first data block on the file system
    /* free block list */
    uint32_t free_block_list_head; // the block # that contains free block list numbers.
    uint32_t next_free_block_index;  // the index which points to the first available free block
    /* list of free inodes*/
    uint32_t max_free_inodes;    // max number of free inodes on disk
    uint32_t num_free_inodes;    // current number of free inodes on disk
    uint32_t remembered_inode;   // starting from this inode, a search routine can find as many
    // free inodes to fill the free_ilist. Before this number, there
    // should be no free inode.
    uint32_t free_ilist[MAX_FREE_ILIST_SIZE];    // a cache for the list of free inodes
    uint32_t next_free_inode_idx;                // the index which points to the next available inode.
    uint32_t next_free_block_idx;
};

enum FILE_TYPE {
    UNUSED=0,
    REGULAR=1,
    DIRECTORY
};


struct disk_inode {
    enum FILE_TYPE file_type;
    const uint8_t owner_id[FILE_OWNER_ID_LEN];     // the id of inode owner
    uint32_t accesspermission;
    uint32_t last_accessed;          // last time the file was accessed
    uint32_t last_modified;          // last modification time of the file
    uint32_t inode_last_mod;         // last modification time to the inode
    uint32_t link_count;             // hard link for the inode.
    uint32_t file_size;
    uint32_t blocks_in_use;  // currently allocated blocks. indexing blocks not counted.
    uint32_t block_addr[DIRECT_BLOCKS_PER_INODE];    // direct blocks on the inode
    uint32_t single_ind_block;  // single indirect block addr
    uint32_t double_ind_block;  // double indirect block addr
};

struct in_core_inode {
    enum FILE_TYPE file_type;
    const uint8_t owner_id[FILE_OWNER_ID_LEN];
    uint32_t accesspermission;
    uint32_t last_accessed;
    uint32_t last_modified;
    uint32_t inode_last_mod;
    uint32_t link_count;
    uint32_t file_size;
    uint32_t blocks_in_use;  // allocated blocks, doesn't count indexing blocks.
    // for directory, file_size is always a multiple of blocks_in_use.
    // but file doesn't.
    uint32_t block_addr[DIRECT_BLOCKS_PER_INODE];
    uint32_t single_ind_block;  // single indirect block addr
    uint32_t double_ind_block;  // double indirect block addr
    // below are in-core fields
    uint32_t locked;         //
    uint32_t modified;       // if modified == 1, then write disk when iput() is called.
    int32_t i_num;          // the inode number
    //uint32_t ref_count;      // reference count is used by open file table.
};

struct directory_entry {
    int32_t inode_num;
    const uint8_t file_name[FILE_NAME_LEN];
};

struct namei_cache_element {
    uint8_t path[DIR_ENTRY_LENGTH];
    struct in_core_inode *iNode;
    uint32_t timestamp;
};

// Procedures required to prepare a storage device for mounting. Returns
// 1 on success and 0 on failure.
uint32_t init_storage(void);

// Procedures required to prepare for demounting. Returns 1 on success and
//  on failure.
uint32_t cleanup_storage(void);

// Reads block at specified storage block number. Fills provided buffer
// with a block of data and returns 1 on success, 0 on failure.
uint32_t block_read(uint32_t block, uint8_t *buffer);

// Writes contents of buffer to specified storage block offset. Returns
// 1 on success and 0 on failure.
uint32_t block_write(uint32_t block, const uint8_t *buffer);

// dump info about superblock, free lists, and disk data
void dump(void);
void dump_super(void); // only dump super
void dump_datablocks(void); // only dump datablocks

// get current time
uint32_t get_time(void);

// the mkfs system call creates superblock, free block list and free ilist on disk
// return 1: successful; return 0: failure.
uint32_t mkfs(void);

// initialize superblock in memory. This function doesn't write to disk.
uint32_t init_super(void);

// allocate a block
uint32_t block_alloc(void);

// free a block
uint32_t block_free(uint32_t);

// read inode from disk, and make an in-core copy
struct in_core_inode* iget(int32_t i_num);

// called when the kernel releases an inode
uint32_t iput(struct in_core_inode* pi);

void dump_in_core_inode(struct in_core_inode* ci);

// alloc an in-core inode from free ilist
struct in_core_inode* ialloc(void);

// free an in-core inode, put it back on free ilist
uint32_t ifree(struct in_core_inode* ci);

// map a logical file byte offset to file system block

// input : ci - in_core inode
//         off - byte offset
// output: block_num - disk block#
//         offset_block - byte offset in the block
uint32_t bmap(const struct in_core_inode* ci, const uint32_t off, uint32_t* block_num,uint32_t* offset_block);

// setup namei cache
void init_namei_cache(void);

struct namei_cache_element *find_namei_cache_by_path(const uint8_t *path);

struct namei_cache_element *find_namei_cache_by_oldest(void);

// a slight modified version of namei
struct in_core_inode* namei_v2(const uint8_t *path);

// a slight modified version of mkdir
uint32_t mkdir_v2(const uint8_t *path);

// remove a directory
uint32_t rmdir(const uint8_t *path);

// create root directory, 1 success, 0 failure
uint32_t mkrootdir(void);

// modified version of create
uint32_t create_v2(const uint8_t *path);

// delete a name and possibly the file it refers to
uint32_t unlink(const uint8_t *pathname);

// Utility functions for splitting paths into node name and path to node
uint32_t separate_node_name(const uint8_t *path, uint8_t *node_name);
uint32_t separate_node_path(const uint8_t *path, uint8_t *node_path);
uint32_t separate_node(const uint8_t *path, uint8_t *node_part, uint32_t part); //0 path, 1 node name


/* below are system calls for open/close/read/write a file*/


// on success, returns a 1
// on failure, returns 0
uint32_t open_v2(const uint8_t* path, uint32_t mode);

// close a file descriptor.
// returns 1 on success, 0 on error
uint32_t close_v2(const uint8_t* path);


// on success: returns the number of bytes read is returned. 1: end of file
// on failure: returns 0
uint32_t read_v2(struct in_core_inode* ci, uint8_t* buf, uint32_t size, uint32_t offset1);

// on success: returns the number of bytes written is returned. 1: nothing is written.
// on failure: returns 0.
uint32_t write_v2(struct in_core_inode* ci, const uint8_t* buf, uint32_t size, uint32_t offset1);

/*If the file previously was larger than this size, the extra data is lost. If the file previously
 * was shorter, it is extended, and the extended part reads as null bytes ('\0').*/
uint32_t truncate_v2(struct in_core_inode* ci, uint32_t length);
#endif //OS452_FILESYS_H

