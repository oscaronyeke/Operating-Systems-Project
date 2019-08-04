/*
** SCCS ID:     @(#)filesys.c   1.1     4/2/19
**
** File:        filesys.c
**
** Author:      Oscar Onyeke
**
** Contributor:
**
**/
#include <x86arch.h>

#include "common.h"

#include "filesys.h"
#include "types.h"
#include "cio.h"
#include "klib.h"
#include "memory.h"
#include "clock.h"
#include "support.h"

static uint8_t *storage;                 // pointer to in-memory storage
static uint32_t store_block_off;    // storage block offset
static uint32_t storage_fd;                // on-disk storage file desc

static uint32_t max_single = DIRECT_BLOCKS_PER_INODE + RANGE_SINGLE; // max single indirect block num
static uint32_t max_double = DIRECT_BLOCKS_PER_INODE + RANGE_SINGLE + RANGE_DOUBLE; // max double indirect block num
struct namei_cache_element namei_cache[NAMEI_CACHE_SZ];


uint8_t* strtok(uint8_t* s, uint8_t* delm)
{
    static uint32_t currIndex = 0;
    if(!s || !delm || s[currIndex] == '\0')
        return NULL;
    uint8_t *W = (uint8_t *)_km_alloc(sizeof(uint8_t)*100);
    uint32_t i = currIndex, k = 0, j = 0;
    while (s[i] != '\0'){
        j = 0;
        while (delm[j] != '\0'){
            if (s[i] != delm[j])
                W[k] = s[i];
            else goto It;
            j++;
        }
        i++;
        k++;
    }
    It:
    W[i] = 0;
    currIndex = i+1;
    return W;
}

uint32_t  init_storage(){

    uint64_t dev_size = BLOCK_SZ * NUM_BLOCKS;

    uint8_t *initPtr;
    uint32_t i;

    storage = _km_alloc(dev_size);
    initPtr = &storage[0];
    if(storage != NULL)
    {
        for(i = 0; i < dev_size; ++i)
        {
            initPtr = 0;
            initPtr++;
        }
    }

    storage_fd = IN_MEM_FD;

    init_namei_cache();

    return storage_fd;
}

uint32_t cleanup_storage(){
    _km_free(storage);
    return 1;
}

static uint32_t reset_storage(void)
{
    uint32_t i;
    uint8_t buf[BLOCK_SZ];
    _kmemset(buf, (uint32_t) sizeof(buf),0);
    for (i = 0; i < NUM_BLOCKS; i++)
    {
        if (block_write(i, buf) == 0)
        {
            __panic( "block_write error ");
            __cio_printf("block# %d in reset_storage()\n", i);
            return 0;
        }
    }
    return 1;
}

uint32_t block_read(uint32_t block, uint8_t *buffer){
    _kmemcpy(buffer,&storage[block*BLOCK_SZ],BLOCK_SZ);
    return 1;
}

uint32_t block_write(uint32_t block, const uint8_t *buffer) {
    uint32_t ret_status;
    if (block >= NUM_BLOCKS) {
        __panic( "block_write error: block num exceeds max block num\n");
        return 0;
    }

    __memcpy(&storage[block * BLOCK_SZ], buffer, BLOCK_SZ);
    ret_status = 1;
    return ret_status;
}

static struct super_block* super;

static void dump_buffer(uint8_t * buf)
{
    uint32_t *p = (uint32_t*)buf;
    uint32_t j;

    for (j = 0; j < BLOCK_SZ/4; j++)
        __cio_printf("%d ", *p++);

    __cio_printf("\n");
}

void dump(void)
{
    dump_super();
    //dump_datablocks();
}

void dump_super(void)
{
    uint32_t i;

    __cio_printf("\n\ndumping super block...\n\n");
    __cio_printf("sizeof superblock = %u\n", sizeof(struct super_block));
    __cio_printf("sizeof disk_inode = %u, in_core_inode = %u\n", sizeof(struct disk_inode), sizeof(struct in_core_inode));
    __cio_printf("block size = %d, total blocks = %d, filesystem size = %lld\n", super->block_size, super->num_blocks, super->fs_size);
    __cio_printf("max free blocks = %d, num of free blocks = %d\n", super->max_free_blocks, super->num_free_blocks);
    __cio_printf("data block offset = %d\n", super->data_block_offset);
    __cio_printf("free_block_list_head = %d\n", super->free_block_list_head);
    __cio_printf("next_free_block_idx = %d\n", super->next_free_block_index);
    __cio_printf("max free inodes = %d, num of free inodes = %d\n", super->max_free_inodes, super->num_free_inodes);
    __cio_printf("remembered inode = %d\n", super->remembered_inode);
    __cio_printf("free inode list: ");

    for (i = 0; i < MAX_FREE_ILIST_SIZE; i++)
        __cio_printf("%d ", super->free_ilist[i]);

    __cio_printf("\nnext_free_inode_idx = %d\n", super->next_free_inode_idx);
}

void dump_datablocks(void)
{
    uint8_t buf[BLOCK_SZ];
    uint32_t i;
    for (i = 0; i < NUM_BLOCKS; i++)
    {
        __cio_printf("block #%d: ", i);
        //uint32_t *p = (uint32_t*)(disk[i].data);
        if(block_read(i, buf) == 0)
        {
            __cio_printf( "error: block_read failure.\n");
            return;
        }
        dump_buffer(buf);
    }
}

static uint32_t update_super(void)
{
    // write superblock to disk
    uint8_t buf[BLOCK_SZ];
    _kmemset(buf, sizeof(buf), 0);
    __memcpy(buf, super, sizeof(struct super_block));
    if (block_write(0, buf) == 0)
    {
        __panic( "error: block_write superblock#0 when update superblock\n");
        return 0;
    }
    return 1;
}

static uint32_t init_free_block_list(void)
{
    uint32_t offset = super->data_block_offset;
    uint32_t* p;
    uint32_t i;
    uint8_t buf[BLOCK_SZ];

    for (;offset < super->num_blocks; offset += FREE_BLOCKS_PER_LINK)
    {
        _kmemset(buf, sizeof(buf), 0);

        p = (uint32_t*)buf;

        // set next data index block pointer
        if (offset + FREE_BLOCKS_PER_LINK >= super->num_blocks)
            *p++ = 0;
        else
            *p++ = offset + FREE_BLOCKS_PER_LINK;

        // fill in the data index blocks with free block indices
        for(i = 1; i < FREE_BLOCKS_PER_LINK && offset + i < super->num_blocks; i++)
            *p++ = offset + i;

        // write data index block back to storage
        if (block_write(offset, buf) == 0)
        {
            __panic( "error: block_write wrong when init free block list\n");
            return 0;
        }
    }

    return 1;
}

uint32_t block_alloc(void)
{
    uint32_t block_num;
/*
        if (super->locked == 1)
        {
                __panic( "error: super block locked\n");
                return 0;
        }
*/
    if (super->num_free_blocks == 0)
    {
        __panic( "no free block: num_free_blocks==0\n");
        return 0;
    }
    uint8_t buf[BLOCK_SZ];
    uint32_t old_list_head = super->free_block_list_head;

    if (block_read(old_list_head, buf) == 0)
    {
        __panic( "error: block_read wrong when block_alloc\n");
        return 0;
    }

    if (super->next_free_block_index == 0)
    {
        uint32_t* freelist_head = (uint32_t*)buf;

        block_num = super->free_block_list_head;
        super->free_block_list_head = freelist_head[0];
        super->next_free_block_index = 1;
    }
    else if (super->next_free_block_index < FREE_BLOCKS_PER_LINK)
    {
        uint32_t* freelist_head = (uint32_t*)buf;
        block_num = freelist_head[super->next_free_block_index];
        /* This is the case when there are no more free blocks in the final
        link. The final block is the link itself. So, need to change the
        block_num to the link itself, and update free_block_list_head. */
        if (block_num == 0)
        {
            block_num = super->free_block_list_head;
            super->free_block_list_head = 0;
            super->next_free_block_index = 1;
        }
        else
        {
            freelist_head[super->next_free_block_index] = 0;
            super->next_free_block_index += 1;
            super->next_free_block_index %= FREE_BLOCKS_PER_LINK;
        }
    }
    else
    {
        __panic( "wrong next_free_block_idx\n");
        return 0;
    }

    if (block_write(old_list_head, buf) == 0)
    {
        __panic( "error: block_write when block_alloc\n");
        return 0;
    }

    super->num_free_blocks -= 1;
    if (update_super() == 0)
    {
        __panic( "update superblock error\n");
        return 0;
    }
/*
        super->locked = 0;
        super->modified = 1;
*/
    return block_num;
}

uint32_t block_free(uint32_t block_num)
{

    if (block_num < super->data_block_offset)
    {
        __panic( "error: trying to free a non-data block ");
        __cio_printf("%d, or disk is busted: super->data_block_offset = %d\n", block_num, super->data_block_offset);
        return 0;
    }
    if (block_num >= NUM_BLOCKS)
    {
        __panic( "error: try to free a ");
        __cio_printf("block# %d that exceeds num_blocks\n", block_num);
        return 0;
    }
    else if (block_num == 0)
    {
        __panic( "error: trying to free block #0\n");
        return 0;
    }
    uint8_t buf[BLOCK_SZ];
    uint8_t zero_buf[BLOCK_SZ];
    _kmemset(zero_buf, sizeof(zero_buf), 0);
    /* indicates current free list link head full, so need to put the freed
       block_num as the new free list link head. */
    if (super->next_free_block_index == 1)
    {
/*
                if (block_read(block_num, buf) == 0)
                {
                        __panic( "error: block_read wrong when block_free\n");
                        return 0;
                }
*/
        _kmemset(buf, sizeof(buf), 0);
        uint32_t* freelist_head = (uint32_t*)buf;
        freelist_head[0] = super->free_block_list_head;
        super->free_block_list_head = block_num;
        super->next_free_block_index = 0;
        if (block_write(block_num, buf) == 0)
        {
            __panic( "error: block_write wrong when block_free\n");
            return 0;
        }
    }
    else if (super->next_free_block_index == 0)
    {
        if (block_read(super->free_block_list_head, buf) == 0)
        {
            __panic( "error: block_read wrong when block_free\n");
            return 0;
        }
        super->next_free_block_index = FREE_BLOCKS_PER_LINK - 1;
        uint32_t* freelist_head = (uint32_t*)buf;
        freelist_head[super->next_free_block_index] = block_num;
        if (block_write(block_num, zero_buf) == 0)
        {
            __panic( "block_write error when zeroing the ");
            __cio_printf("block # %d\n", block_num);
            return 0;
        }
        if (block_write(super->free_block_list_head, buf) == 0)
        {
            __panic( "error: block_write wrong when block_free\n");
            return 0;
        }
    }
    else if (super->next_free_block_index > 1
             && super->next_free_block_index< FREE_BLOCKS_PER_LINK)
    {
        if (block_read(super->free_block_list_head, buf) == 0)
        {
            __panic( "error: block_read wrong when block_free\n");
            return 0;
        }
        super->next_free_block_index -= 1;
        uint32_t* freelist_head = (uint32_t*)buf;
        freelist_head[super->next_free_block_index] = block_num;
        if (block_write(block_num, zero_buf) == 0)
        {
            __panic( "block_write error when zeroing the ");
            __cio_printf("block # %d\n", block_num);
            return 0;
        }
        if (block_write(super->free_block_list_head, buf) == 0)
        {
            __panic( "error: block_write wrong when block_free\n");
            return 0;
        }
    }
    else
    {
        __panic( "wrong next_free_block_index\n");
        return 0;
    }
    super->num_free_blocks += 1;
    if (update_super() == 0)
    {
        __panic( "update superblock error in block_free\n");
        return 0;
    }
/*
        super->locked = 0;
        super->modified = 1;
*/
    return 1;
}


/************************* Layer 1: inode algorithms ********************************/

// search on disk free inodes and add them on the free ilist
// when free ilist is empty.
static uint32_t fill_free_ilist(void)
{
    uint32_t block_num;
    uint32_t i, offset;
    uint8_t buf[BLOCK_SZ]; // bigger than block size
    uint32_t k = super->remembered_inode;
    uint32_t remembered_i = k;
    if (k >= super->max_free_inodes)
    {
        __panic( "error: inode ");
        __cio_printf("%d exceeds maximum inode num\n", k);
        return 0;
    }
    for (i = 0; i < MAX_FREE_ILIST_SIZE; i++)
    {
        //__cio_printf("i = %d\n", i);
        while(k < super->max_free_inodes)
        {
            block_num = 1 + k / INODES_PER_BLOCK;
            offset = k % INODES_PER_BLOCK;
            //__cio_printf("block_num = %d, offset = %d\n", block_num, offset);
            if (block_read(block_num, buf) == 0)
            {
                __panic( "error: block_read when fill free ilist\n");
                return 0;
            }
            //__cio_printf("after block_read\n");
            struct disk_inode* pi = (struct disk_inode*)buf;
            struct disk_inode* q = pi + offset;
            //__cio_printf("after q\n");
            if (q->file_type == UNUSED)
            {
                //__cio_printf("free_ilist[%d] = %d\n", i, k);
                super->free_ilist[i] = k;
                if (k > remembered_i)
                    remembered_i = k;
                k++;
                break;
            }
            k++;
        }
    }
    super->remembered_inode = remembered_i;
    //__cio_printf("complete: i = %d\n", i);
    while (i < MAX_FREE_ILIST_SIZE)
    {
        __cio_printf("empty free ilist i=%d\n", i);
        super->free_ilist[i] = 0;
        i++;
    }
    return 1;
}

static uint32_t init_free_ilist(void)
{
    fill_free_ilist();
    return 1;
}

uint32_t get_time(void){
    return _system_time;
}

static uint32_t init_disk_inode(struct disk_inode* di)
{
    di->file_type = REGULAR;
    __strcpy((char *)di->owner_id, "unknown");
    di->last_accessed = get_time();
    di->last_modified = get_time();
    di->inode_last_mod = get_time();
    di->link_count = 0;  //
    di->file_size = 0;
    di->blocks_in_use = 0;
    uint32_t i;
    for(i = 0; i < DIRECT_BLOCKS_PER_INODE; i++)
    {
        di->block_addr[i] = 0;
    }
    di->single_ind_block = 0;
    di->double_ind_block = 0;
    return 1;
}

static uint32_t init_in_core_inode(struct in_core_inode* ci, uint32_t i_num)
{
    ci->file_type = REGULAR;
    __strcpy((char *)ci->owner_id,"unknown");
    ci->last_accessed = get_time();
    ci->last_modified = get_time();
    ci->inode_last_mod = get_time();
    ci->link_count = 1;
    ci->file_size = 0;
    ci->blocks_in_use = 0;
    uint32_t i;
    for(i = 0; i < DIRECT_BLOCKS_PER_INODE; i++)
    {
        ci->block_addr[i] = 0;
    }
    ci->single_ind_block = 0;
    ci->double_ind_block = 0;
    ci->locked = 0;
    ci->modified = 1;
    ci->i_num = i_num;
    return 1;
}

static uint32_t init_inode_from_disk(struct in_core_inode* ci, const struct disk_inode* di, uint32_t i_num)
{
    ci->file_type = di->file_type;
    __strcpy((char *) ci->owner_id, (char *)di->owner_id);
    ci->last_accessed = di->last_accessed;
    ci->last_modified = di->last_modified;
    ci->inode_last_mod = di->inode_last_mod;
    ci->link_count = di->link_count;
    ci->file_size = di->file_size;
    ci->blocks_in_use = di->blocks_in_use;
    uint32_t i;
    for(i = 0; i < DIRECT_BLOCKS_PER_INODE; i++)
    {
        ci->block_addr[i] = di->block_addr[i];
    }
    ci->single_ind_block = di->single_ind_block;
    ci->double_ind_block = di->double_ind_block;
    ci->locked = 0;
    ci->modified = 0;
    ci->i_num = i_num;
    return 1;
}

static uint32_t init_inode_from_kernel(struct disk_inode* dst, const struct in_core_inode* src)
{
    dst->file_type = src->file_type;
    __strcpy((char *)dst->owner_id, (char *)src->owner_id);
    dst->last_accessed = src->last_accessed;
    dst->last_modified = src->last_modified;
    dst->inode_last_mod = src->inode_last_mod;
    dst->link_count = src->link_count;
    dst->file_size = src->file_size;
    dst->blocks_in_use = src->blocks_in_use;
    uint32_t i;
    for(i = 0; i < DIRECT_BLOCKS_PER_INODE; i++)
    {
        dst->block_addr[i] = src->block_addr[i];
    }
    dst->single_ind_block = src->single_ind_block;
    dst->double_ind_block = src->double_ind_block;
    return 1;
}

/* allocate in-core inodes */
struct in_core_inode* ialloc(void)
{
    uint32_t ret;
    struct in_core_inode* ci = NULL;
    uint8_t buf[BLOCK_SZ];
    if (super->num_free_inodes <= 0)
    {
        __panic( "error: no more free inodes on disk\n");
        return NULL;
    }
    while (1)
    {

        /* ilist empty in superblock */
        if (super->next_free_inode_idx == MAX_FREE_ILIST_SIZE)
        {
            ret = fill_free_ilist();
            if (ret == 0)
            {
                __panic( "error: free ilist not filled\n");
                return NULL;
            }
        }
        uint32_t i_num = super->free_ilist[super->next_free_inode_idx];
        super->free_ilist[super->next_free_inode_idx] = 0;
        super->next_free_inode_idx += 1;
        ci = (struct in_core_inode*)_km_alloc(sizeof(struct in_core_inode));
        init_in_core_inode(ci, i_num);
        uint32_t block_num = 1 + i_num / INODES_PER_BLOCK;
        uint32_t offset = i_num % INODES_PER_BLOCK;
        if (block_read(block_num, buf) == 0)
        {
            __panic( "error: block_read ");
            __cio_printf("block# %d when ialloc\n", block_num);
            return NULL;
        }
        struct disk_inode* di = (struct disk_inode*)buf;
        di = di + offset;
        if (di->file_type != 0) /* inode is not free */
        {
            __panic( "error: inode not free after all\n");
            continue;
        }
        init_disk_inode(di);
        /////
        /////
        if (block_write(block_num, buf) == 0)
        {
            __panic( "error: block_write ");
            __cio_printf("block# %d when ialloc\n", block_num);
            return NULL;
        }
        super->num_free_inodes -= 1;
        if (update_super() == 0)
        {
            __panic( "update superblock error\n");
            return NULL;
        }
        return ci;
    }
}

uint32_t ifree(struct in_core_inode* ci)
{
    uint32_t i_num = ci->i_num;
/*
        if (super->locked == 1)
        {
                __panic( "error: superblock locked\n");
                return 0;
        }
*/


    if (i_num >= super->max_free_inodes)
    {
        __panic( "error: i_num exceeds max inode num\n");
        return 0;
    }
    if (super->next_free_inode_idx == 0) /* free ilist full*/
    {
        if (i_num < super->remembered_inode)
        {
            super->remembered_inode = i_num;
        }
    }
    else if (super->next_free_inode_idx <= MAX_FREE_ILIST_SIZE)
    {
        super->next_free_inode_idx --;
        super->free_ilist[super->next_free_inode_idx] = i_num;
    }
    else
    {
        __panic( "error: wrong next_free_inode_idx\n");
        return 0;
    }
    uint8_t buf[BLOCK_SZ];
    uint32_t block_num = 1 + i_num / INODES_PER_BLOCK;
    uint32_t offset = i_num % INODES_PER_BLOCK;
    if (block_read(block_num, buf) == 0)
    {
        __panic( "error: block_read ");
        __cio_printf("block# %d\n", block_num);
        return 0;
    }
    struct disk_inode* di = (struct disk_inode*)buf;
    di = di + offset;
    di->file_type = UNUSED;
    if (block_write(block_num, buf) == 0)
    {
        __panic( "error: block_write ");
        __cio_printf("block# %d\n", block_num);
        return 0;
    }
    _km_free(ci); // ultimately free the in-core inode structure
    ci = NULL;
    super->num_free_inodes += 1;
    if (update_super() == 0)
    {
        __panic( "update superblock error\n");
        return 0;
    }
    return 1;
}

// map a logical file byte offset to file system block
// given an inode and byte offset, return a block_num and byte offset in the block
uint32_t bmap(const struct in_core_inode* ci, const uint32_t off, uint32_t* ret_block_num,
              uint32_t* ret_off_block)
{
    uint32_t logical_block;
    uint32_t block_num;
    uint32_t off_block;
    logical_block = off / BLOCK_SZ;
    off_block = off % BLOCK_SZ;
    uint8_t buf[BLOCK_SZ];
    if (logical_block < DIRECT_BLOCKS_PER_INODE)
    {
        block_num = ci->block_addr[logical_block];
    }
    else if (logical_block < max_single)
    {
        block_num = ci->single_ind_block;
        logical_block -= DIRECT_BLOCKS_PER_INODE;
        if (block_read(block_num, buf) == 0)
        {
            __panic( "block_read error when ");
            __cio_printf("bmap block# %d\n", block_num);
            return 0;
        }
        uint32_t *p = (uint32_t*)buf;
        block_num = p[logical_block];
    }
    else if (logical_block < max_double)
    {
        block_num = ci->double_ind_block;
        if (block_read(block_num, buf) == 0)
        {
            __panic( "block_read error when ");
            __cio_printf("bmap block# %d\n", block_num);
            return 0;
        }
        uint32_t *p = (uint32_t*)buf;
        logical_block -= max_single;
        uint32_t indirect_block = logical_block / RANGE_SINGLE;
        uint32_t indirect_off = logical_block % RANGE_SINGLE;
        block_num = p[indirect_block];
        if (block_read(block_num, buf) == 0)
        {
            __panic( "block_read error when ");
            __cio_printf("bmap block# %d\n", block_num);
            return 0;
        }
        p = (uint32_t*)buf;
        block_num = p[indirect_off];
    }
    else
    {
        __panic( "logical block num ");
        __cio_printf("%d out of max range\n", logical_block);
        return 0;
    }
    *ret_block_num = block_num;
    *ret_off_block = off_block;
    return 1;
}

struct in_core_inode* iget(int32_t i_num)
{
    uint32_t block_num = 1 + i_num / INODES_PER_BLOCK;
    uint32_t offset = i_num % INODES_PER_BLOCK;
    uint8_t buf[BLOCK_SZ];
    if (block_read(block_num, buf) == 0)
    {
        __panic( "error: block_read ");
        __cio_printf("block#%d when iget\n", block_num);
        return NULL;
    }
    struct disk_inode *di = (struct disk_inode*)buf;
    di = di + offset;
    struct in_core_inode *ci = (struct in_core_inode*)_km_alloc(sizeof(struct in_core_inode));
    init_inode_from_disk(ci, di, i_num);
    ci->locked = 0;
    return ci;
}

static uint32_t free_disk_blocks(struct in_core_inode* ci)
{
    uint32_t i;
    uint32_t block_num;
    // free direct blocks
#if _DEBUG
    __cio_printf("free direct blocks\n");
#endif
    for (i = 0; i < DIRECT_BLOCKS_PER_INODE; i++)
    {
        block_num = ci->block_addr[i];
        if (block_num == 0)
            continue;
        if (block_free(block_num) == 0)
        {
            __panic( "block_free error ");
            __cio_printf("when free disk block#%d\n", block_num);
            return 0;
        }
        ci->block_addr[i] = 0;
    }

    // free single indirect blocks
    if (ci->blocks_in_use <= DIRECT_BLOCKS_PER_INODE)
        goto finish_free_disk_blocks;
    block_num = ci->single_ind_block;
    if (block_num == 0) // doesn't in use.
        goto free_double_indirect;
    uint8_t buf[BLOCK_SZ];
    if (block_read(block_num, buf) == 0)
    {
        __panic( "block_read error ");
        __cio_printf("block#%d when free single indirect\n", block_num);
        return 0;
    }
    uint32_t *p = (uint32_t*)buf;
    for (i = 0; i < RANGE_SINGLE; i++)
    {
        block_num = p[i];
        if (block_num == 0)
            continue;
        if (block_free(block_num) == 0)
        {
            __panic( "block_free error ");
            __cio_printf("when free disk block#%d\n", block_num);
            return 0;
        }
    }
    block_num = ci->single_ind_block;
    if (block_free(block_num) == 0)
    {
        __panic( "block_free error ");
        __cio_printf("block#%d when free single indirect\n", block_num);
        return 0;
    }
    ci->single_ind_block = 0;

    // free double indirect blocks
    if (ci->blocks_in_use <= max_single)
        goto finish_free_disk_blocks;
    free_double_indirect:
#if _DEBUG
    __cio_printf("free double indirect blocks\n");
#endif
    block_num = ci->double_ind_block;
    if (block_num == 0) // doesn't in use.
        return 0;
    if (block_read(block_num, buf) == 0)
    {
        __panic( "block_read error");
        __cio_printf(" block#%d when free single indirect\n", block_num);
        return 0;
    }
    p = (uint32_t*)buf;
    for (i = 0; i < RANGE_SINGLE; i++)
    {
        block_num = p[i];
        if (block_num == 0)
            continue;
        uint8_t buf2[BLOCK_SZ];
        if (block_read(block_num, buf2) == 0)
        {
            __panic( "block_read error");
            __cio_printf(" block#%d when free double indirect\n", block_num);
            return 0;
        }
        uint32_t *q = (uint32_t*)buf2;
        uint32_t j;
        for (j = 0; j < RANGE_SINGLE; j++)
        {
            if (q[j] == 0)
                continue;
            if (block_free(q[j]) == 0)
            {
                __panic( "block_free error ");
                __cio_printf("block# %d when free single indirect in double indirect\n", q[j]);
                return 0;
            }
        }
        if (block_free(p[i]) == 0)
        {
            __panic( "block_free error ");
            __cio_printf("block# %d for single indirect block table in double indirect\n", p[i]);
            return 0;
        }
    }
    block_num = ci->double_ind_block;
    if (block_free(block_num) == 0)
    {
        __panic( "block_free error ");
        __cio_printf("block#%d when free single indirect\n", block_num);
        return 0;
    }
    ci->double_ind_block = 0;
    finish_free_disk_blocks:
    return 1;
}

static uint32_t free_blocks_for_truncate(struct in_core_inode *ci, uint32_t new_blocks_in_use);

// release an inode
uint32_t iput(struct in_core_inode* ci)
{
    /*if (ci->locked == 1)
    {
        __panic( "warn: inode locked\n");
        return 0;
    }*/
    if (ci == NULL)
    {
        __panic( "ci is null pointer\n");
        return 0;
    }
    ci->locked = 1;
    if (1)
    {
        if (ci->link_count == 0)
        {
            // free all disk blocks
            //if (truncate_v2(ci, 0) != 0)
            if (free_disk_blocks(ci) == 0)
                //if (free_blocks_for_truncate(ci, 0) != 0)
            {
                __panic( "error truncate all disk blocks in iput\n");
                return 0;
            }
            //ci->file_type = 0; // will be set in ifree().
            // free inode
            if (ifree(ci) == 0)
            {
                __panic( "error: ifree when iput\n");
                return 0;
            }
        }
        if (ci && ci->modified == 1) // update disk inode from in-core inode
        {
            uint32_t i_num = ci->i_num;
            uint32_t block_num = 1 + i_num / INODES_PER_BLOCK;
            uint32_t offset = i_num % INODES_PER_BLOCK;
            // the reason why we need to call block_read() is because we just want to
            // update the inode in question, and leave the other inodes unmodified.
            uint8_t buf[BLOCK_SZ];
            if (block_read(block_num, buf) == 0)
            {
                __panic( "block_read error ");
                __cio_printf("block# %d when iput\n", block_num);
                return 0;
            }
            struct disk_inode* di = (struct disk_inode*)buf;
            di = di + offset;
            init_inode_from_kernel(di, ci);
            if (block_write(block_num, buf) == 0)
            {
                __panic( "block_write error ");
                __cio_printf("block#%d when iput\n", block_num);
                return 0;
            }
        }
        // no free list for inode cache.
    }
    ci->locked = 0;
    return 1;
}

void dump_in_core_inode(struct in_core_inode* ci)
{
    __cio_printf("\n\ndumping in-core inode\n\n");
    __cio_printf("file_type = %d\n", ci->file_type);
    __cio_printf("owner = %s\n", ci->owner_id);
    __cio_printf("last_accessed = %d\n", ci->last_accessed);
    __cio_printf("last_modified = %d\n", ci->last_modified);
    __cio_printf("inode_last_mod = %d\n", ci->inode_last_mod);
    __cio_printf("link_count = %d\n", ci->link_count);
    __cio_printf("file_size = %d\n", ci->file_size);
    __cio_printf("blocks_in_use = %d\n", ci->blocks_in_use);
    __cio_printf("direct block addr = \n");
    uint32_t i;
    for (i = 0; i < DIRECT_BLOCKS_PER_INODE; i++)
    {
        __cio_printf("%d ", ci->block_addr[i]);
    }
    __cio_printf("\nindirect single block# = %d, indirect double block# = %d\n",
                 ci->single_ind_block, ci->double_ind_block);
    __cio_printf("locked = %d\n", ci->locked);
    __cio_printf("i_num = %d\n", ci->i_num);
}

/************************* Layer 1: make fs ***********************************/
static uint32_t create_superblock(void)
{
    super = (struct super_block*)_km_alloc(sizeof(struct super_block));
    if (super == NULL)
    {
        __panic( "create superblock error: no memory\n");
        return 0;
    }
    super->block_size = BLOCK_SZ;
    super->num_blocks = NUM_BLOCKS;
    super->fs_size = (long)(super->block_size) * (long)(super->num_blocks);
    /* disk blocks */
    super->max_free_blocks = super->num_free_blocks = NUM_BLOCKS - 1 - ILIST_SPACE;
    super->data_block_offset = ILIST_SPACE + 1;
    super->free_block_list_head = super->data_block_offset;
    super->next_free_block_idx = 1;
    /* inodes */
    super->max_free_inodes = super->num_free_inodes = INODES_PER_BLOCK * ILIST_SPACE;
    __memset(super->free_ilist, sizeof(super->free_ilist), 0);
    super->next_free_inode_idx = 0;
/*
        super->modified = 0;
        super->locked = 0;
*/

    return 1;
}

static uint32_t read_superblock(void)
{
    uint32_t block_num = 0;
    uint8_t buf[BLOCK_SZ];
    _kmemset(buf, sizeof(buf), 0);
    if (block_read(block_num, buf) == 0)
    {
        __panic( "block_read error in read_superblock\n");
        return 0;
    }
    super = (struct super_block*)_km_alloc(sizeof(struct super_block));
    if (super == NULL)
    {
        __panic( "create superblock error: no memory\n");
        return 0;
    }
    __memcpy(super, buf, sizeof(struct super_block));
    return 1;
}

static uint32_t root_i_num;
//static struct in_core_inode* curr_dir_inode;
static uint32_t curr_dir_i_num;

uint32_t mkrootdir(void)
{

    struct in_core_inode *r = ialloc();
    if (r == NULL)
    {
        __panic( "ialloc error in mkrootdir\n");
        return 0;
    }

    root_i_num = r->i_num;
    r->file_type = DIRECTORY;
    __strcpy((char *)r->owner_id, "root2");
    r->file_size = BLOCK_SZ;  // root dir has one block that is equal to BLOCK_SZ.
    r->blocks_in_use = 1;  // root dir has one block that is equal to BLOCK_SZ.
    uint32_t block_num = block_alloc();


    if (block_num == 0)
    {
        __panic( "block_alloc error in mkrootdir\n");
        return 0;
    }
    r->block_addr[0] = block_num;
    uint8_t buf[BLOCK_SZ];
    _kmemset(buf, sizeof(buf), 0);
    struct directory_entry *entry;
    entry = (struct directory_entry*)buf;
    _kstrcpy((char *)entry->file_name, ".");
    entry->inode_num = root_i_num;
    entry++;
    _kstrcpy((char *)entry->file_name, "..");
    entry->inode_num = root_i_num;
    // indicate the end of a directory
    entry++;
    entry->inode_num = BAD_I_NUM;
    if (block_write(block_num, buf) == 0)
    {
        __panic( "block_write error ");
        __cio_printf("block#%d in mkrootdir\n", block_num);
        return 0;
    }
    if (iput(r) == 0)
    {
        __panic( "iput error in mkrootdir\n");
        return 0;
    }
    return 1;
}

/* the mkfs system call creates superblock, free block list and free ilist on disk
 * return 1: successful; return 0: failure.
 */
uint32_t mkfs(void)
{
    if (reset_storage() == 0)
    {
        __panic( "error: reset storage\n");
        return 0;
    }
    create_superblock();


    init_free_block_list();
    init_free_ilist();
    if (mkrootdir() == 0)
    {
        __panic( "error when mkrootdir\n");
        return 0;
    }
    if (update_super() == 0)
    {
        __panic( "update superblock error\n");
        return 0;
    }
    curr_dir_i_num = root_i_num; // init current directory
    return 1;
}

uint32_t init_super(void)
{
    if (read_superblock() != 1)
    {
        __panic( "read_superblock error in init_super\n");
        return 0;
    }
    //init_free_ilist();
    curr_dir_i_num = root_i_num; // init current directory
    return 1;
}

void init_namei_cache()
{
    uint32_t j;


    for(j = 0; j < NAMEI_CACHE_SZ; ++j)
    {
        namei_cache[j].path[0] = '\0';
        namei_cache[j].iNode = NULL;
        namei_cache[j].timestamp = get_time();  // stamp everything NOW
    }

    return;
}

struct namei_cache_element *find_namei_cache_by_path(const uint8_t *path)
{
    uint32_t i;

    for(i = 0; i < NAMEI_CACHE_SZ; ++i)
    {
        if(__strcmp((char *)path, (char *)namei_cache[i].path) == 0)
            return &namei_cache[i];
    }

    return NULL;
}

struct namei_cache_element *find_namei_cache_by_oldest()
{
    uint32_t tstamp, oldest, i;


    tstamp = get_time();

    for(i = 0; i < NAMEI_CACHE_SZ; ++i)
    {
        if(tstamp > namei_cache[i].timestamp)
        {
            oldest = i;
            tstamp = namei_cache[i].timestamp;
        }
    }

    return &namei_cache[oldest];
}

struct in_core_inode* namei_v2(const uint8_t * path_name)
{
    struct in_core_inode* working_inode;
    uint8_t *path_tok;
    struct directory_entry* dir_entry;
    uint8_t path[MAX_PATH_LEN];
#if USE_NAMEI_CACHE
    struct namei_cache_element *cached_path = NULL;

    // look in cache for this path
    if((cached_path = find_namei_cache_by_path(path_name)) !=
       NULL)
    {
        //gettimeofday(&tv, NULL);
        cached_path->timestamp = get_time();
//#if _DEBUG
        __cio_printf("namei: found cached path for %s\n",
               path_name);
//#endif

        return cached_path->iNode;
    }

    // path not cached, search fs for path
#endif
    __strcpy((char *)path, (char *)path_name);
    if (path == NULL)
    {
        __panic( "error: path = NULL\n");
        return NULL;
    }

    if (path[0] == '/')
    {

        working_inode = iget(root_i_num);
        if (working_inode == NULL)
        {
            __panic( "error: get root inode fails in namei_v2\n");
            return NULL;
        }
    }
    else
    {
        working_inode = iget(curr_dir_i_num);
        if (working_inode == NULL)
        {
            __panic( "error: get current directory inode fails in namei_v2\n");
            return NULL;
        }
    }

    path_tok = strtok(path,(uint8_t *)"/");
#if _DEBUG
    __cio_printf("The first segment of the path %s\n",path_tok);
#endif
    if(_kstrcmp((char *)path_tok,"/\0")){
#if _DEBUG
        __cio_printf("exited namei_v2 using _kstrcmp((char *)path_tok,...\n",path_tok);
#endif
        working_inode = iget(curr_dir_i_num);
        return working_inode;
    }
    while (path_tok)
    {
#if _DEBUG
        __cio_printf("The first segment of the path %s\n",path_tok);
        __delay(400);
#endif


        if (working_inode->file_type != DIRECTORY)
        {
            __panic( "error: curr working dir is not a directory\n");
            return NULL;
        }
        if (working_inode->i_num == root_i_num && __strcmp((char *)path_tok,(char *) "..") == 0)
        {
            path_tok = strtok(NULL,(uint8_t *) "/");
            continue;
        }
        // read directory entry by entry

        uint32_t off;  // the offset of dir entry in a directory
        for (off = 0; off < MAX_ENTRY_OFFSET; off += DIR_ENTRY_LENGTH)
        {

            uint32_t block_num;
            uint32_t offset_block; // offset in a disk block
            if (bmap(working_inode, off, &block_num, &offset_block) == 0)
            {
                __panic( "bmap error in namei_v2\n");
                return NULL;
            }
            uint8_t buf[BLOCK_SZ];
            if (block_read(block_num, buf) == 0)
            {
                __panic( "block_read error ");
                __cio_printf("block# %d in namei_v2\n", block_num);
                return NULL;
            }
            dir_entry = (struct directory_entry*)(buf + offset_block);

            if (dir_entry->inode_num == BAD_I_NUM)
            {
                __cio_printf("end of the directory \n");
                return NULL;
            }
            if (dir_entry->inode_num == EMPTY_I_NUM){
                continue;
            }

//#if _DEBUG
            __cio_printf("A file has been located %s\n",path_tok);
            __delay(400);
//#endif

            if (__strcmp((char *)dir_entry->file_name, (char *)path_tok) == 0)
            {
                uint32_t i_num = (uint32_t)(dir_entry->inode_num);
                if (iput(working_inode) == 0)
                {
                    __panic( "iput error in namei_v2\n");
                    return NULL;
                }
                working_inode = iget(i_num);
                if (working_inode == NULL)
                {
                    __panic( "iget error in namei_v2\n");
                    return NULL;
                }
                break;
            }
        }
        if (off == MAX_ENTRY_OFFSET)
        {// already reaches the max entries, but not found.
            __cio_printf("cannot find the inode until the max entries in curr_dir for token %s\n", path_tok);
            return NULL;
        }

        path_tok = strtok(NULL, (uint8_t *)"/");
    }

#if USE_NAMEI_CACHE
    // replace oldest cached path with this one
    //gettimeofday(&tv, NULL);
    if(cached_path == NULL)
    {
        cached_path = find_namei_cache_by_oldest();
        __strcpy((char *)cached_path->path, (char *)path_name);
        cached_path->iNode = working_inode;
    }
    cached_path->timestamp = get_time();
//#if _DEBUG
    __cio_printf("namei: cached mapping to path %s\n",
           path_name);
//#endif
#endif

    working_inode->locked = 0;
    return working_inode;
}

uint32_t separate_node_name(const uint8_t *path, uint8_t *node_name)
{
    return separate_node(path, node_name, 1);
}

uint32_t separate_node_path(const uint8_t *path, uint8_t *node_path)
{
    return separate_node(path, node_path, 0);
}

uint32_t separate_node(const uint8_t *path, uint8_t *node_part, uint32_t part)
{
    uint8_t node_path[__strlen((char *)path)];
    uint8_t node_name[__strlen((char *)path)];
    uint32_t l = 0;
    uint32_t substr_len = 0;

    l = _kstrlen((char *)path);
    do
    {
        ++substr_len;

        if(path[l] == '/')
        {
            if(l == __strlen((char *)path)-1)
            {
                //__cio_printf("no node name\n");
                return 0;
            }

            __memcpy(node_name, &path[l+1], substr_len);
            break;
        }

        --l;

    } while(l >= 0);

    //__cio_printf("l = %d\n", l);
    if(l == 0)
    {
        __memcpy(node_path, "/\0", 2);
    }
    else
    {
        __memcpy(node_path, path, l);
        node_path[l] = '\0';  // fix a bug: need to add an end char.
    }

    if(part == 0)
    {
        _kmemcpy(node_part, node_path, __strlen((char *)node_path) + 1);
    }
    else
    {
        _kmemcpy(node_part, node_name, __strlen((char *)node_name) + 1);
    }

    return 1;
}

/* use separate utilities to split the end of the path and the rest of the path.
   the end of the path should exist in the current filesystem, and should be a directory
*/
uint32_t mkdir_v2(const uint8_t * path_name){

    struct in_core_inode *ci;
    uint8_t path[MAX_PATH_LEN];
    __strcpy((char *)path, (char *)path_name);
    uint8_t node_name[__strlen((char *)path_name)];
    uint8_t node_path[__strlen((char *)path_name)];
    if (path == NULL)
    {
        __panic( "error: path = NULL during mkdir\n");
        return 0;
    }
    // seperate node and path to node in 2 strings (mkdir needs this)
    separate_node_name(path, node_name);
    separate_node_path(path, node_path);
#if _DEBUG


    __cio_printf("Node name: %s\n",node_name);
    __cio_printf("Node path: %s\n",node_path);
#endif
    ci = namei_v2(node_path);
    if (ci == NULL)
    {
        __cio_printf("%s: No such file or directory, therefore directory %s cannot be created\n",node_path,node_name);
        __panic( "mkdir: cannot create directory\n");
        __cio_printf(" %s: No such file or directory\n", node_path);
        return 0;
    }

    // create a directory node_path under inode ci

    uint32_t off;
    struct directory_entry* dir_entry;

    for (off = 0; off < MAX_ENTRY_OFFSET - DIR_ENTRY_LENGTH; off += DIR_ENTRY_LENGTH)
    {
        uint32_t block_num;
        uint32_t offset_block;
        if (bmap(ci, off, &block_num, &offset_block) == 0)
        {
            __panic( "bmap error in mkdir_v2 bad address\n");
            return 0; // bad address
        }

#if _DEBUG
        __cio_printf("created the b map\n");
        __cio_printf("block _num for directory: %d\n",block_num);

#endif
        uint8_t buf[BLOCK_SZ];
        if (block_read(block_num, buf) == 0)
        {
            __panic( "block_read error in unlink cannot read the disk\n");
            return 0; // cannot read disk
        }
#if _DEBUG
        __cio_printf("read the block\n");
#endif
        dir_entry = (struct directory_entry*)(buf + offset_block);
        if (dir_entry->inode_num != BAD_I_NUM
            && dir_entry->inode_num != EMPTY_I_NUM)
        { // meaning current dir entry is occupied.
            continue;
        }
        // there is dir entry space left starting from this
        struct in_core_inode* new_inode = ialloc();
        if (new_inode == NULL)
        {
            __panic( "ialloc error in mkdir_v2\n");
            return 0;
        }
        new_inode->file_type = DIRECTORY;
        new_inode->file_size = BLOCK_SZ;
        new_inode->blocks_in_use = 1;
        // need to add ".", ".." and -1 (end) to a new directory inode.
        uint32_t new_dir_block = block_alloc();
        if (new_dir_block == 0)
        {
            __panic( "block_alloc error in create_v2\n");
            return 0;
        }
#if _DEBUG
        __cio_printf("added the file to the directory\n");
#endif
        new_inode->block_addr[0] = new_dir_block;
        uint8_t new_buf[BLOCK_SZ];
        _kmemset(new_buf, sizeof(buf), 0);
        struct directory_entry *new_dir_entry;
        new_dir_entry = (struct directory_entry*)new_buf;
        _kstrcpy((char *)new_dir_entry->file_name, ".");
        new_dir_entry->inode_num = new_inode->i_num;  // himself
        new_dir_entry++;
        _kstrcpy((char *)new_dir_entry->file_name, "..");
        new_dir_entry->inode_num = ci->i_num;   // his parent inode
        // indicate the end of a directory
        new_dir_entry++;
        new_dir_entry->inode_num = BAD_I_NUM;
        if (block_write(new_dir_block, new_buf) == 0)
        {
            __panic( "block_write error in create_v2\n");
            return 0;
        }
#if _DEBUG
        __cio_printf("wrote to the blocks and is begining the iput function\n");
#endif
        // the last step is to update to the disk
#if _DEBUG
        __cio_printf("we are updating the disk \n");
#endif
        if (iput(new_inode) == 0)
        {
            __panic( "iput error in create_v2\n");
            return 0;
        }
        if (dir_entry->inode_num == EMPTY_I_NUM)
        {
            dir_entry->inode_num = (new_inode->i_num);
            _kstrcpy((char *)dir_entry->file_name, (char *)node_name);
        }
        else if (dir_entry->inode_num == BAD_I_NUM)
        {
            dir_entry->inode_num = (new_inode->i_num);
            _kstrcpy((char *)dir_entry->file_name, (char *)node_name);
            // if the dir has the last space for a bad inode num, then
            // write a BAD_I_NUM indicating the end of the directory.
            dir_entry ++;
            dir_entry->inode_num = BAD_I_NUM;
        }
        else
        {
            __panic( "fatal error: dir_entry->inode_num");
            __cio_printf(" %d wrong in mkdir_v2\n", dir_entry->inode_num);
            return 0;
        }

        if (block_write(block_num, buf) == 0)
        {
            __panic( "block_write error in create_v2\n");
            return 0;
        }
#if _DEBUG
        __cio_printf("finsished everything\n");
#endif
        break;
    }
    if (off == MAX_ENTRY_OFFSET - DIR_ENTRY_LENGTH)
    {
        __panic( "error: directory is full, no entries can be added\n");
        return 0;
    }
#if _DEBUG
    __cio_printf("Had a proper exit\n");
        __delay(200);
#endif

    return 1;
}

// removes a directory and a file
uint32_t unlink(const uint8_t *path_name)
{
    struct in_core_inode *ci;
    uint8_t path[MAX_PATH_LEN];
    __strcpy((char *)path, (char *)path_name);
    uint8_t node_name[__strlen((char *)path_name)];
    uint8_t node_path[__strlen((char *)path_name)];
    if (path == NULL)
    {
        __panic( "error: path = NULL in unlink\n");
        return 0;
    }
    // seperate node and path to node in 2 strings (mkdir needs this)
    separate_node_name(path, node_name);
    separate_node_path(path, node_path);

#if _DEBUG
    __cio_printf("Node name: %s\n",node_name);
    __cio_printf("Node path: %s\n",node_path);
#endif

    ci = namei_v2(node_path);
    if (ci == NULL)
    {
        __panic( "rmdir: cannot find path to directory");
        __cio_printf( " %s\n", path);
        return 0;
    }
    // create a directory node_path under inode ci

    uint32_t off = 0; // start from the 3rd entry.
    struct directory_entry* dir_entry;
    for (; off < MAX_ENTRY_OFFSET - DIR_ENTRY_LENGTH; off += DIR_ENTRY_LENGTH)
    {

        uint32_t block_num;
        uint32_t offset_block;
        if (bmap(ci, off, &block_num, &offset_block) == 0)
        {
            __panic( "bmap error in unlink bad address\n");
            return 0; // bad address
        }
#if _DEBUG
        __cio_printf("bmap for rm is done \n");
#endif

        uint8_t buf[BLOCK_SZ];
        if (block_read(block_num, buf) == 0)
        {
            __panic( "block_read error in unlink cannot read the disk\n");
            return 0; // cannot read disk
        }
#if _DEBUG
        __cio_printf("block_read for rm is done \n");
#endif
        dir_entry = (struct directory_entry*)(buf + offset_block);
        if (dir_entry->inode_num == BAD_I_NUM)
        {
#if _DEBUG
            __cio_printf( "reaches the end, \n");
            __cio_printf(" cannot find dir %s\n", path);
#endif
            return 0;
        }
        if (dir_entry->inode_num == EMPTY_I_NUM){
            continue;
        }
#if _DEBUG
        __cio_printf("current entry we are observing is %s \n",dir_entry->file_name);
#endif
        if (__strcmp((char *)dir_entry->file_name, (char *)node_name) == 0){


            // find the directory. read it and delete it.
            int32_t i_num = dir_entry->inode_num;
            struct in_core_inode *target_inode = iget(i_num);
#if _DEBUG
            __cio_printf("we found the target and are getting its inode \n");
#endif
            if (target_inode == NULL)
            {
                __panic( "no disk inode corresponding to this i_num");
                __cio_printf(" %d\n", i_num);
                return 0;
            }

            // remove the directory
            if (target_inode->link_count > 0){
                target_inode->link_count --;
            }
#if _DEBUG
            __cio_printf("we are putting the inode back \n");
#endif
            if (iput(target_inode) == 0){
                __panic( "iput error i_num = ");
                __cio_printf("%d in unlink\n", i_num);
                return 0;
            }
            dir_entry->inode_num = EMPTY_I_NUM; // reset inode num to indicate it is free.
#if _DEBUG
            __cio_printf("we are writing to a block \n");
#endif
            if (block_write(block_num, buf) == 0){
                __panic( "block_write error in unlink\n");
                return 0;
            }
            break;
        }

    }
    if (off == MAX_ENTRY_OFFSET - DIR_ENTRY_LENGTH)
    {
        __panic( "cannot find the directory to delete\n");
        return 0;
    }
#if _DEBUG
    __cio_printf("we are now doing a normal exit \n");
            __delay(200);
#endif
    return 1;
}

/* use separate utilities to split the end of the path and the rest of the path.
   the end of the path should exist in the current filesystem, and should be a directory
*/
uint32_t create_v2(const uint8_t * path_name)
{
    struct in_core_inode *ci;
    uint8_t path[MAX_PATH_LEN];
    __strcpy((char *)path, (char *)path_name);
    uint8_t node_name[__strlen((char *)path_name)];
    uint8_t node_path[__strlen((char *)path_name)];
    if (path == NULL)
    {
        __panic( "error: path = NULL during create_v2\n");
        return 0;
    }
    separate_node_name(path, node_name);
    separate_node_path(path, node_path);

#if _DEBUG
    __cio_printf("Node name: %s\n",node_name);
    __cio_printf("Node path: %s\n",node_path);
#endif

    ci = namei_v2(node_path);
    if (ci == NULL)
    {
        __panic( "create_v2: cannot create open the directory\n");
        __cio_printf(" %s: No such file or directory\n", path);
        return 0;
    }
    // create a directory node_path under inode ci

    uint32_t off;
    struct directory_entry* dir_entry;
    for (off = 0; off < MAX_ENTRY_OFFSET - DIR_ENTRY_LENGTH; off += DIR_ENTRY_LENGTH)
    {

        uint32_t block_num;
        uint32_t offset_block;
        if (bmap(ci, off, &block_num, &offset_block) == 0)
        {
            __panic( "bmap error in create_v2\n");
            return 0;
        }
        uint8_t buf[BLOCK_SZ];
        if (block_read(block_num, buf) == 0)
        {
            __panic( "block_read error in create_v2\n");
            return 0;
        }
        dir_entry = (struct directory_entry*)(buf + offset_block);
        if (dir_entry->inode_num != BAD_I_NUM
            && dir_entry->inode_num != EMPTY_I_NUM)
        { // meaning current dir entry is occupied.
            continue;
        }
        // there is dir entry space left starting from this
        struct in_core_inode* new_inode = ialloc();
        if (new_inode == NULL)
        {
            __panic( "ialloc error in create_v2\n");
            return 0;
        }
        new_inode->file_type = REGULAR;
        new_inode->file_size = 0;
        new_inode->blocks_in_use = 0;
        // the last step is to update to the disk
        if (iput(new_inode) == 0)
        {
            __panic( "iput error in create_v2\n");
            return 0;
        }
        if (dir_entry->inode_num == EMPTY_I_NUM)
        {
            dir_entry->inode_num = (new_inode->i_num);
            __strcpy((char *)dir_entry->file_name, (char *)node_name);
        }
        else if (dir_entry->inode_num == BAD_I_NUM)
        {
            dir_entry->inode_num = (new_inode->i_num);
            __strcpy((char *)dir_entry->file_name, (char *)node_name);
            // if the dir has the last space for a bad inode num, then
            // write a BAD_I_NUM indicating the end of the directory.
            dir_entry ++;
            dir_entry->inode_num = BAD_I_NUM;
        }
        else
        {
            __panic( "fatal error: \n");
            __cio_printf("dir_entry->inode_num %d wrong in mkdir_v2\n", dir_entry->inode_num);
            return 0;
        }

        if (block_write(block_num, buf) == 0)
        {
            __panic( "block_write error in create_v2\n");
            return 0;
        }
        break;
    }
    if (off == MAX_ENTRY_OFFSET - DIR_ENTRY_LENGTH)
    {
        __panic( "error: directory is full, no entries can be added\n");
        return 0;
    }
    return 1;
}

uint32_t rmdir(const uint8_t * path)
{
    return unlink(path);
}

/******************* file operations *************************************/

uint32_t read_v2(struct in_core_inode* ci, uint8_t * buf, uint32_t size, uint32_t offset)
{
    // from offset, copy size of bytes from the file indicated by path to buf
    if (ci == NULL){
        return 0;
    }
    uint32_t res;
    uint32_t count = 0; // the bytes that are copied to buf
    if (offset > ci->file_size)
    {
        __panic( "read error: \n");
        __cio_printf("offset %d exceeds file size %d\n", offset, ci->file_size);
        return 0;
    }
    if (offset + size > ci->file_size)
    {
        size = ci->file_size - offset;  // update the max bytes to read.

    }
    while (count < size)
    {
        uint32_t block_num;
        uint32_t offset_block;  // offset in disk block
        res = bmap(ci, offset + count, &block_num, &offset_block);
        if (res != 1)
        {
            __panic( "bmap error in read\n");
            _kmemset(buf + count, size - count, 0);
            return 0;
        }

        uint8_t block_buf[BLOCK_SZ];
        res = block_read(block_num, block_buf);
        if (res != 1)
        {
            __panic( "block_read error\n");
            __cio_printf("block# %d in read\n", block_num);
            _kmemset(buf + count, size - count, 0);
            return 0;
        }
        if (offset_block + (size - count) <= BLOCK_SZ)
        { // the left bytes to copy doesn't exceeds block_size
            _kmemcpy(buf + count, block_buf + offset_block, size - count);
            count = size;
        }
        else
        {
            uint32_t to_copy = BLOCK_SZ - offset_block;
            _kmemcpy(buf + count, block_buf + offset_block, to_copy);
            count += to_copy;
        }
    }
    ci->last_accessed = get_time();
    ci->modified = 1;
    res = iput(ci);
    if (res != 1)
    {
        __panic( "iput error in read_v2\n");
        return 0;
    }
    else
    {

    }
    return count;
}

static uint32_t alloc_blocks_for_write(struct in_core_inode *ci, uint32_t blocks_to_alloc)
{
    //uint32_t bytes_to_alloc = offset + size - ;
    //uint32_t blocks_to_alloc = (bytes_to_alloc + BLOCK_SZ) / BLOCK_SZ;
    uint32_t i;
    uint32_t count = ci->blocks_in_use; // blocks allocated.
    uint32_t limit = ci->blocks_in_use + blocks_to_alloc;

    // first alloc disk blocks for direct blocks.
    while (count < limit && count < DIRECT_BLOCKS_PER_INODE)
    {
        uint32_t block_num = block_alloc();
        if (block_num == 0)
        {
            __panic( "block_alloc error\n");
            __cio_printf("block# %d in alloc_blocks\n", block_num);
            return 0;
        }
        ci->block_addr[count] = block_num;
        ci->blocks_in_use += 1;
        count += 1;
    }
    // if count < limit, alloc blocks for single indirect.
    while (count < limit && count < max_single)
    {
        uint32_t block_num;
        if (ci->single_ind_block == 0) // single hasn't been alloc
        {
            block_num = block_alloc();
            if (block_num == 0)
            {
                __panic( "block_alloc error\n");
                __cio_printf("block# %d in alloc_blocks\n", block_num);
                return 0;
            }
            // init block
            uint8_t sub_buf[BLOCK_SZ];
            _kmemset(sub_buf, sizeof(sub_buf), 0);
            if (block_write(block_num, sub_buf) == 0)
            {
                __panic( "block_write error \n");
                __cio_printf("block# %d in alloc_blocks\n", block_num);
                return 0;
            }
            ci->single_ind_block = block_num;
            //ci->blocks_in_use += 1; // count should not be incremented.
        }
        else
            block_num = ci->single_ind_block;
        uint8_t buf[BLOCK_SZ];
        _kmemset(buf, sizeof(buf), 0);
        if (block_read(block_num, buf) == 0)
        {
            __panic( "block_read error \n");
            __cio_printf("block# %d in alloc_blocks\n", block_num);
            return 0;
        }
        uint32_t *p = (uint32_t*)buf;
        i = count - DIRECT_BLOCKS_PER_INODE;
        if (i < 0 || i >= RANGE_SINGLE)
        {
            __panic( "index error \n");
            __cio_printf("%d for single indirect in alloc_blocks\n", i);
            return 0;
        }
        p[i] = block_alloc();
        if (p[i] == 0)
        {
            __panic( "block_alloc error \n");
            __cio_printf("block# %d in alloc_blocks\n", p[i]);
            return 0;
        }
        // init block
        uint8_t sub_buf[BLOCK_SZ];
        _kmemset(sub_buf, sizeof(sub_buf), 0);
        if (block_write(p[i], sub_buf) == 0)
        {
            __panic( "block_write error \n");
            __cio_printf("block# %d in alloc_blocks\n", block_num);
            return 0;
        }
        //
        if (block_write(block_num, buf) == 0)
        {
            __panic( "block_write error\n");
            __cio_printf("block# %d in alloc_blocks\n", block_num);
            return 0;
        }
        count ++;
        ci->blocks_in_use += 1;
    }

    return 1;
}

static uint32_t alloc_blocks_for_truncate(struct in_core_inode *ci, uint32_t new_blocks_in_use);
// on success: returns the number of bytes written is returned. 0: nothing is written.
// on failure: returns -1.
uint32_t write_v2(struct in_core_inode* ci, const uint8_t * buf, uint32_t size, uint32_t offset)
{
    uint32_t res;
    // copy buf to the file from the offset, update to size of bytes.
    if (ci == NULL){
        return 0;
    }
    if (offset + size > MAX_FILE_SIZE)
    {
        __panic( "new file size exceeds maximum file size\n");
        return 0;
    }
    if (offset + size > ci->file_size)
    {

        uint32_t new_blocks_in_use = (offset + size + BLOCK_SZ - 1 )/BLOCK_SZ;
        if (new_blocks_in_use > ci->blocks_in_use)
        {
            // allocate more blocks.
            if (alloc_blocks_for_truncate(ci, new_blocks_in_use) != 1)
            {
                __panic( "alloc blocks error in write\n");
                dump_super();
                dump_datablocks();
                exit(0);
                return 0;
            }
            ci->blocks_in_use = new_blocks_in_use;
            ci->modified = 1;
        }
    }
    uint32_t count = 0; // the bytes that are copied to buf
    while (count < size)
    {
        uint32_t block_num;
        uint32_t offset_block;
        res = bmap(ci, offset + count, &block_num, &offset_block);
        if (res != 1)
        {
            __panic( "bmap error in write\n");
            return 0;
        }

        uint8_t block_buf[BLOCK_SZ];
        res = block_read(block_num, block_buf);
        if (res != 1)
        {
            __panic( "block_read error\n");
            __cio_printf("block# %d in write\n", block_num);
            return 0;
        }
        if (offset_block + (size - count) <= BLOCK_SZ)
        {
            __memcpy(block_buf + offset_block, buf + count, size - count);
#if _DEBUG
            __cio_printf("%d copied to disk buf\n", size - count);
#endif
            count = size;
        }
        else
        {
            uint32_t to_copy = BLOCK_SZ - offset_block;
            __memcpy(block_buf + offset_block, buf + count, to_copy);

            count += to_copy;
        }
        res = block_write(block_num, block_buf);

        if (res != 1)
        {
            __panic( "block_write error\n");
            __cio_printf("block# %d in write\n", block_num);
            return 0;
        }
    }
    ci->file_size = offset + size;
    ci->last_modified = get_time();
    ci->inode_last_mod = get_time();
    ci->modified = 1;
    res = iput(ci);
    if (res != 1)
    {
        __panic( "iput error in write\n");
        return 0;
    }
    else
    {
#if _DEBUG
        __cio_printf("iput is successful\n");
#endif
    }
    return count;
}

// start is included, end is not. Free [start, end).
static uint32_t free_direct_blocks(struct in_core_inode *ci, uint32_t abs_start, uint32_t abs_end)
{
    if (abs_start < 0)
        return 0;
    else if (abs_end > DIRECT_BLOCKS_PER_INODE)
        return 0;
    else if (abs_start > abs_end)
        return 0;
    uint32_t i;
    uint32_t block_num;
    uint32_t start = abs_start;
    uint32_t end = abs_end;
#if _DEBUG
    __cio_printf("free_direct_blocks, start = %d, end = %d\n", start, end);
#endif
    for (i = start; i < end; i++)
    {
        block_num = ci->block_addr[i];
        if (block_num == 0)
            continue;
        if (block_free(block_num) == 0)
        {
            __panic( "block_free error \n");
            __cio_printf("block# %d when free direct blocks\n", block_num);
            return 0;
        }
        ci->block_addr[i] = 0;
    }
    return 1;
}

static uint32_t alloc_direct_blocks(struct in_core_inode *ci, uint32_t abs_start, uint32_t abs_end)
{
    if (abs_start < 0)
        return 0;
    else if (abs_end > DIRECT_BLOCKS_PER_INODE)
        return 0;
    else if (abs_start > abs_end)
        return 0;
    uint32_t i;
    uint32_t block_num;
    uint32_t start = abs_start;
    uint32_t end = abs_end;
    for (i = start; i < end; i++)
    {
        block_num = block_alloc();
        if (block_num == 0)
        {
            __panic( "balooc error when alloc direct blocks\n");
            return 0;
        }
        ci->block_addr[i] = block_num;
    }
    return 1;
}

static uint32_t multi_block_free(uint32_t s_block_num, uint32_t start, uint32_t end)
{
    uint8_t buf[BLOCK_SZ];
    if (s_block_num == 0)
        return 1;
    if (block_read(s_block_num, buf) == 0)
    {
        __panic( "block_read error ");
        __cio_printf("block# %d in multi_block_free\n", s_block_num);
        return 0;
    }
    uint32_t *p = (uint32_t*)buf;
    uint32_t i;
    uint32_t block_num;
    for (i = start; i < end; i++)
    {
        block_num = p[i];
        if (block_num == 0)
            continue;
        if (block_free(block_num) == 0)
        {
            __panic( "block_free error ");
            __cio_printf("block# %d when multi_block_free\n", block_num);
            return 0;
        }
        p[i] = 0;
    }
    if (block_write(s_block_num, buf) == 0)
    {
        __panic( "block_write error ");
        __cio_printf("block# %d when multi_block_free\n", s_block_num);
        return 0;
    }
    return 1;
}

static uint32_t multi_block_alloc(uint32_t s_block_num, uint32_t start, uint32_t end)
{
    uint8_t buf[BLOCK_SZ];
    if (block_read(s_block_num, buf) == 0)
    {
        __panic( "block_read error ");
        __cio_printf("block# %d in multi_block_alloc\n", s_block_num);
        return 0;
    }
    uint32_t *p = (uint32_t*)buf;
    uint32_t i;
    uint32_t block_num;
    for (i = start; i < end; i++)
    {
        block_num = block_alloc();
        if (block_num == 0)
        {
            __panic( "block_alloc error when multi_block_alloc\n");
            return 0;
        }
        p[i] = block_num;
    }
    if (block_write(s_block_num, buf) == 0)
    {
        __panic( "block_write error ");
        __cio_printf("block# %d when multi_block_alloc\n", s_block_num);
        return 0;
    }
    return 1;
}

// start is included, end is not. Free [start, end).
static uint32_t free_single_ind_blocks(struct in_core_inode *ci, uint32_t abs_start, uint32_t abs_end)
{
    if ((abs_start < DIRECT_BLOCKS_PER_INODE) || (abs_end > max_single) || (abs_start > abs_end))
    {
        __panic( "range error when free single indirect blocks: ");
        __cio_printf("start %d, end %d\n", abs_start, abs_end);
        return 0;
    }
    // start, end is relative to single indirect blocks
    uint32_t start = abs_start - DIRECT_BLOCKS_PER_INODE;
    uint32_t end = abs_end - DIRECT_BLOCKS_PER_INODE;
    uint32_t s_block_num;
    s_block_num = ci->single_ind_block;
    if (multi_block_free(s_block_num, start, end) == 0)
    {
        __panic( "multi_block_free error ");
        __cio_printf("block# %d when free single indirect blocks\n", s_block_num);
        return 0;
    }
    if (start == 0 && end == RANGE_SINGLE)
    {// need to release the single indirect table block.
        if (block_free(s_block_num) == 0)
        {
            __panic( "block_free error ");
            __cio_printf("block# %d when free single indirect blocks\n", s_block_num);
            return 0;
        }
        ci->single_ind_block = 0;
        ci->modified = 1;
    }
    if (iput(ci) == 0)
    {
        __panic( "iput error in free_single_ind_blocks\n");
        return 0;
    }
    return 1;
}

// start is included, end is not. Free [start, end).
static uint32_t alloc_single_ind_blocks(struct in_core_inode *ci, uint32_t abs_start, uint32_t abs_end)
{
    if ((abs_start < DIRECT_BLOCKS_PER_INODE) || (abs_end > max_single) || (abs_start > abs_end))
    {
        __panic( "range error when alloc single indirect blocks: ");
        __cio_printf("start %d, end %d\n", abs_start, abs_end);
        return 0;
    }
    // start, end is relative to single indirect blocks
    uint32_t start = abs_start - DIRECT_BLOCKS_PER_INODE;
    uint32_t end = abs_end - DIRECT_BLOCKS_PER_INODE;
    uint32_t s_block_num;
    if (ci->single_ind_block == 0)
    { // need to alloc a block for single ind.
        s_block_num = block_alloc();
        if (s_block_num == 0)
        {
            __panic( "block_alloc error when alloc single indirect blocks\n");
            return 0;
        }
        ci->single_ind_block = s_block_num;
        ci->modified = 1;
    }
    s_block_num = ci->single_ind_block;

    if (multi_block_alloc(s_block_num, start, end) == 0)
    {
        __panic( "multi_block_alloc error ");
        __cio_printf("block# %d when alloc single indirect blocks\n", s_block_num);
        return 0;
    }
    if (iput(ci) == 0)
    {
        __panic( "iput error in alloc_single_ind_blocks\n");
        return 0;
    }
    return 1;
}

static uint32_t free_double_ind_blocks(struct in_core_inode *ci, uint32_t abs_start, uint32_t abs_end)
{
    if ((abs_start < max_single) || (abs_end > max_double) || (abs_start > abs_end))
    {
        __panic( "range error when free double indirect blocks \n");
        return 0;
    }
    // start, end is relative to double indirect blocks
    uint32_t start = abs_start - max_single;
    uint32_t end = abs_end - max_single;
    uint32_t first_ind_block_idx = start / RANGE_SINGLE;
    uint32_t first_ind_block_off = start % RANGE_SINGLE;
    uint32_t last_ind_block_idx = end / RANGE_SINGLE;
    uint32_t last_ind_block_off = end % RANGE_SINGLE;
    uint32_t i;
    uint32_t d_block_num; // double indirect block num.
    d_block_num = ci->double_ind_block;
    if (d_block_num == 0)
    {
        __panic( "error: d_block_num = 0, not possible in free_double_ind_blocks\n");
        return 0;
    }
    uint8_t d_ind_buf[BLOCK_SZ];
    if (block_read(d_block_num, d_ind_buf) == 0)
    {
        __panic( "block_read error ");
        __cio_printf("block# %d when free double ind blocks\n", d_block_num);
        return 0;
    }
    uint32_t *d_p = (uint32_t*)d_ind_buf;
    uint32_t s_block_num;
    // get the first ind block and free from the off until the end;
    if (first_ind_block_idx == last_ind_block_idx)
    {  // just free blocks in one indirect block
        s_block_num = d_p[first_ind_block_idx];
        if (multi_block_free(s_block_num, first_ind_block_off, last_ind_block_off) == -1)
        {
            __panic( "multi_block_free error ");
            __cio_printf("block# %d when free double ind blocks\n", s_block_num);
            return 0;
        }
        if (first_ind_block_off == 0 && last_ind_block_off == RANGE_SINGLE-1)
        {
            if (block_free(s_block_num) == 0)
            {
                __panic( "block_free error ");
                __cio_printf("block# %d when free double indirect blocks\n", s_block_num);
                return 0;
            }
            d_p[first_ind_block_idx] = 0;
        }
    }
    else if (first_ind_block_idx < last_ind_block_idx)
    { // free blocks in the first, the last and full middle indirect blocks
        // free blocks in the first_ind_block
        s_block_num = d_p[first_ind_block_idx];
        if (multi_block_free(s_block_num, first_ind_block_off, RANGE_SINGLE) == 0)
        {
            __panic( "multi_block_free error ");
            __cio_printf("block# %d when free double ind blocks\n", s_block_num);
            return 0;
        }
        if (first_ind_block_off == 0)
        {
            if (block_free(s_block_num) == 0)
            {
                __panic( "block_free error block# ");
                __cio_printf( "%d when free double indirect blocks\n", s_block_num);
                return 0;
            }
            d_p[first_ind_block_idx] = 0;
        }
        // free blocks in the middle ind blocks
        for (i = first_ind_block_idx + 1; i <= last_ind_block_idx - 1; i++)
        {
            s_block_num = d_p[i];
            if (multi_block_free(s_block_num, 0, RANGE_SINGLE) == 0)
            {
                __panic( "multi_block_free error ");
                __cio_printf("block# %d when free double ind blocks\n", s_block_num);
                return 0;
            }
            if (block_free(s_block_num) == 0)
            {
                __panic( "block_free error ");
                __cio_printf("block# %d when free double indirect blocks\n", s_block_num);
                return 0;
            }
            d_p[i] = 0;

        }
        // free blocks in the last_ind_block
        s_block_num = d_p[last_ind_block_idx];
        if (multi_block_free(s_block_num, 0, last_ind_block_off) == 0)
        {
            __panic( "multi_block_free error ");
            __cio_printf("block# %d when free double ind blocks\n", s_block_num);
            return 0;
        }
        if (last_ind_block_off == RANGE_SINGLE - 1)
        {
            if (block_free(s_block_num) == 0)
            {
                __panic( "block_free error ");
                __cio_printf("block# %d when free double indirect blocks\n", s_block_num);
                return 0;
            }
            d_p[last_ind_block_idx] = 0;
        }
    }
    else
    {
        __panic( "error: first_ind_block_idx > last_ind_block_idx\n");
        return 0;
    }

    // write d_ind_buf back to disk.
    if (block_write(d_block_num, d_ind_buf) == 0)
    {
        __panic( "block_write error block# ");
        __cio_printf("%d when free double ind blocks\n", d_block_num);
        return 0;
    }

    if (start == 0 && end == RANGE_DOUBLE)
    { // free the double indirect table
        if (block_free(d_block_num) == 0)
        {
            __panic( "block_free error block# ");
            __cio_printf("%d when free double ind blocks\n", d_block_num);
            return 0;
        }
        ci->double_ind_block = 0;
        ci->modified = 1;
    }
    if (iput(ci) == 0)
    {
        __panic( "iput error in free_double_ind_blocks\n");
        return 0;
    }

    return 1;
} // free_double_ind_blocks()

static uint32_t alloc_double_ind_blocks(struct in_core_inode *ci, uint32_t abs_start, uint32_t abs_end)
{
    if ((abs_start < max_single) || (abs_end > max_double) || (abs_start > abs_end))
    {
        __panic( "range error when alloc double indirect blocks: ");
        __cio_printf("start %d, end %d\n", abs_start, abs_end);
        return 0;
    }
    // start, end is relative to double indirect blocks
    uint32_t start = abs_start - max_single;
    uint32_t end = abs_end - max_single;
    uint32_t first_ind_block_idx = start / RANGE_SINGLE;
    uint32_t first_ind_block_off = start % RANGE_SINGLE;
    uint32_t last_ind_block_idx = end / RANGE_SINGLE;
    uint32_t last_ind_block_off = end % RANGE_SINGLE;
    uint32_t i;
    uint32_t d_block_num; // double indirect block num.
    if (ci->double_ind_block == 0)
    {
        d_block_num = block_alloc();
        if (d_block_num == 0)
        {
            __panic( "block_alloc error in alloc double ind blocks\n");
            return 0;
        }
        ci->double_ind_block = d_block_num;
        ci->modified = 1;
    }
    d_block_num = ci->double_ind_block;
    uint8_t d_ind_buf[BLOCK_SZ];
    if (block_read(d_block_num, d_ind_buf) == 0)
    {
        __panic( "block_read error ");
        __cio_printf("block# %d when alloc double ind blocks\n", d_block_num);
        return 0;
    }
    uint32_t *d_p = (uint32_t*)d_ind_buf;
    uint32_t s_block_num;
    // get the first ind block and alloc from the off until one block less than the end;
    if (first_ind_block_idx == last_ind_block_idx)
    {  // just alloc blocks in one indirect block
        if (d_p[first_ind_block_idx] == 0)
        {
            s_block_num = block_alloc();
            if (s_block_num == 0)
            {
                __panic( "block_alloc error in alloc double ind blocks\n");
                return 0;
            }
            d_p[first_ind_block_idx] = s_block_num;
        }
        s_block_num = d_p[first_ind_block_idx];
        if (multi_block_alloc(s_block_num, first_ind_block_off, last_ind_block_off) == 0)
        {
            __panic( "multi_block_alloc error ");
            __cio_printf("block# %d when alloc double ind blocks - place 1, d_block_num = %d, first_ind_block_idx = %d\n",
                         s_block_num, d_block_num, first_ind_block_idx);
            return 0;
        }
    }
    else if (first_ind_block_idx < last_ind_block_idx)
    { // alloc blocks in the first, the last and full middle indirect blocks
        // alloc blocks in the first_ind_block
        if (d_p[first_ind_block_idx] == 0)
        {
            s_block_num = block_alloc();
            if (s_block_num == 0)
            {
                __panic( "block_alloc error in alloc double ind blocks\n");
                return 0;
            }
            d_p[first_ind_block_idx] = s_block_num;
        }
        s_block_num = d_p[first_ind_block_idx];
        if (multi_block_alloc(s_block_num, first_ind_block_off, RANGE_SINGLE) == 0)
        {
            __panic( "multi_block_alloc error ");
            __cio_printf("block# %d when alloc double ind blocks - place 2\n", s_block_num);
            return 0;
        }
        // free blocks in the middle ind blocks
        for (i = first_ind_block_idx + 1; i <= last_ind_block_idx - 1; i++)
        {
            s_block_num = block_alloc();
            if (s_block_num == 0)
            {
                __panic( "block_alloc error in alloc double ind blocks\n");
                return 0;
            }
            d_p[i] = s_block_num;
            if (multi_block_alloc(s_block_num, 0, RANGE_SINGLE) == 0)
            {
                __panic( "multi_block_alloc error");
                __cio_printf("block# %d when alloc double ind blocks - place 3\n", s_block_num);
                return 0;
            }
        }
        // free blocks in the last_ind_block
        s_block_num = block_alloc();
        if (s_block_num == 0)
        {
            __panic( "block_alloc error in alloc double ind blocks\n");
            return 0;
        }
        d_p[last_ind_block_idx] = s_block_num;
        if (multi_block_alloc(s_block_num, 0, last_ind_block_off) == -1)
        {
            __panic( "multi_block_alloc error ");
            __cio_printf("block# %d when alloc double ind blocks - place 4\n", s_block_num);
            return 0;
        }
    }
    else
    {
        __panic( "error: first_ind_block_idx > last_ind_block_idx\n");
        return 0;
    }

    // write d_ind_buf back to disk.
    if (block_write(d_block_num, d_ind_buf) == 0)
    {
        __panic( "block_write error ");
        __cio_printf("block# %d when alloc double ind blocks\n", d_block_num);
        return 0;
    }

    if (iput(ci) == 0)
    {
        __panic( "iput error in alloc_double_ind_blocks\n");
        return 0;
    }

    return 1;
} // alloc_double_ind_blocks()


// new_blocks_in_use < ci->blocks_in_use
// (x, y): x and y belong to {A, B, C}
// A: x < DIRECT_BLOCKS_PER_INODE
// B: DIRECT_BLOCKS_PER_INODE < x < max_single
// C: max_single < x < max_double
static uint32_t free_blocks_for_truncate(struct in_core_inode *ci, uint32_t new_blocks_in_use)
{
    uint32_t abs_start = new_blocks_in_use;
    uint32_t abs_end = ci->blocks_in_use;
    if (abs_start >= max_double)
    {
        __panic( "abs_start ");
        __cio_printf("%d exceeds max_double %d in free_blocks_for_truncate\n", abs_start, max_double);
        return 0;
    }
    if (abs_end > max_double)
    {
        __panic( "abs_end ");
        __cio_printf("%d exceeds max_double %d in free_blocks_for_truncate\n", abs_end, max_double);
        return 0;
    }

    if (abs_start >= 0 && abs_start < DIRECT_BLOCKS_PER_INODE)
    {
        if (abs_end <= DIRECT_BLOCKS_PER_INODE)  // (A, A)
        {
            if (free_direct_blocks(ci, abs_start, abs_end) != 1)
            {
                __panic( "free_direct_blocks error\n");
                return 0;
            }
        }
        else if (abs_end > DIRECT_BLOCKS_PER_INODE && abs_end <= max_single) // (A, B)
        {
            if (free_direct_blocks(ci, abs_start, DIRECT_BLOCKS_PER_INODE) != 1)
            {
                __panic( "free_direct_blocks error\n");
                return 0;
            }
            if (free_single_ind_blocks(ci, DIRECT_BLOCKS_PER_INODE, abs_end) != 1)
            {
                __panic( "free_single_ind_blocks error\n");
                return 0;
            }
        }
        else if (abs_end > max_single && abs_end <= max_double) // (A, C)
        {
            if (free_direct_blocks(ci, abs_start, DIRECT_BLOCKS_PER_INODE) != 1)
            {
                __panic( "free_direct_blocks error\n");
                return 0;
            }
            if (free_single_ind_blocks(ci, DIRECT_BLOCKS_PER_INODE, max_single) != 1)
            {
                __panic( "free_single_ind_blocks error\n");
                return 0;
            }
            if (free_double_ind_blocks(ci, max_single, abs_end) != 1)
            {
                __panic( "free_double_ind_blocks error\n");
                return 0;
            }
        }
    }
    else if (abs_start >= DIRECT_BLOCKS_PER_INODE && abs_start < max_single)
    {
        if (abs_end > DIRECT_BLOCKS_PER_INODE && abs_end <= max_single) // (B, B)
        {
            if (free_single_ind_blocks(ci, abs_start, abs_end) != 1)
            {
                __panic( "free_single_ind_blocks error\n");
                return 0;
            }
        }
        else if (abs_end > max_single && abs_end <= max_double) // (B, C)
        {
            if (free_single_ind_blocks(ci, abs_start, max_single) != 1)
            {
                __panic( "free_single_ind_blocks error\n");
                return 0;
            }
            if (free_double_ind_blocks(ci, max_single, abs_end) != 1)
            {
                __panic( "free_double_ind_blocks error\n");
                return 0;
            }
        }
    }
    else if (abs_start >= max_single && abs_start < max_double)
    {
        if (abs_end > max_single && abs_end <= max_double) // (C, C)
        {
            if (free_double_ind_blocks(ci, abs_start, abs_end) != 1)
            {
                __panic( "free_double_ind_blocks error\n");
                return 0;
            }
        }
    }

    return 1;
} // free_blocks_for_truncate()

// new_blocks_in_use > ci->blocks_in_use
// (x, y): x and y belong to {A, B, C}
// A: x < DIRECT_BLOCKS_PER_INODE
// B: DIRECT_BLOCKS_PER_INODE < x < max_single
// C: max_single < x < max_double
static uint32_t alloc_blocks_for_truncate(struct in_core_inode *ci, uint32_t new_blocks_in_use)
{
    uint32_t abs_start = ci->blocks_in_use;
    uint32_t abs_end = new_blocks_in_use;
    if (abs_start >= max_double)
    {
        __panic( "abs_start ");
        __cio_printf(" %d exceeds max_double %d in alloc_blocks_for_truncate\n", abs_start, max_double);
        return 0;
    }
    if (abs_end > max_double)
    {
        __panic( "abs_end ");
        __cio_printf("%d exceeds max_double %d in alloc_blocks_for_truncate\n", abs_end, max_double);
        return 0;
    }

    if (abs_start >= 0 && abs_start < DIRECT_BLOCKS_PER_INODE)
    {
        if (abs_end <= DIRECT_BLOCKS_PER_INODE)  // (A, A)
        {
            if (alloc_direct_blocks(ci, abs_start, abs_end) != 1)
            {
                __panic( "alloc_direct_blocks error\n");
                return 0;
            }
        }
        else if (abs_end > DIRECT_BLOCKS_PER_INODE && abs_end <= max_single) // (A, B)
        {
            if (alloc_direct_blocks(ci, abs_start, DIRECT_BLOCKS_PER_INODE) != 1)
            {
                __panic( "alloc_direct_blocks error\n");
                return 0;
            }
            if (alloc_single_ind_blocks(ci, DIRECT_BLOCKS_PER_INODE, abs_end) != 1)
            {
                __panic( "alloc_single_ind_blocks error\n");
                return 0;
            }
        }
        else if (abs_end > max_single && abs_end <= max_double) // (A, C)
        {
            if (alloc_direct_blocks(ci, abs_start, DIRECT_BLOCKS_PER_INODE) != 1)
            {
                __panic( "alloc_direct_blocks error\n");
                return 0;
            }
            if (alloc_single_ind_blocks(ci, DIRECT_BLOCKS_PER_INODE, max_single) != 1)
            {
                __panic( "alloc_single_ind_blocks error\n");
                return 0;
            }
            if (alloc_double_ind_blocks(ci, max_single, abs_end) != 1)
            {
                __panic( "alloc_double_ind_blocks error\n");
                return 0;
            }
        }
    }
    else if (abs_start >= DIRECT_BLOCKS_PER_INODE && abs_start < max_single)
    {
        if (abs_end > DIRECT_BLOCKS_PER_INODE && abs_end <= max_single) // (B, B)
        {
            if (alloc_single_ind_blocks(ci, abs_start, abs_end) != 1)
            {
                __panic( "alloc_single_ind_blocks error\n");
                return 0;
            }
        }
        else if (abs_end > max_single && abs_end <= max_double) // (B, C)
        {
            if (alloc_single_ind_blocks(ci, abs_start, max_single) != 1)
            {
                __panic( "alloc_single_ind_blocks error\n");
                return 0;
            }
            if (alloc_double_ind_blocks(ci, max_single, abs_end) != 1)
            {
                __panic( "alloc_double_ind_blocks error\n");
                return 0;
            }
        }
    }
    else if (abs_start >= max_single && abs_start < max_double)
    {
        if (abs_end > max_single && abs_end <= max_double) // (C, C)
        {
            if (alloc_double_ind_blocks(ci, abs_start, abs_end) != 1)
            {
                __panic( "alloc_double_ind_blocks error\n");
                return 0;
            }
        }
    }

    return 1;
} // alloc_blocks_for_truncate()


/*If the file previously was larger than this size, the extra data is lost. If the file previously was shorter, it is extended, and the extended part reads as null bytes ('\0').*/
uint32_t truncate_v2(struct in_core_inode* ci, uint32_t length)
{
    if (ci == NULL)
        return 0;
    if (length < ci->file_size)
    {
        uint32_t new_blocks_in_use = (length-1 + BLOCK_SZ)/BLOCK_SZ;
        if (new_blocks_in_use < ci->blocks_in_use)
        {
            // truncate blocks.
            if (free_blocks_for_truncate(ci, new_blocks_in_use) != 1)
            {
                __panic( "free blocks error in truncate\n");
                return 0;
            }
        }
        else if (new_blocks_in_use == ci->blocks_in_use)
        {
            // truncate file.
            ci->file_size = length;
        }
        else
        {
            __panic( "error: new_blocks_in_use > block_in_use, while length < file_size\n");
            return 0;
        }
        ci->blocks_in_use = new_blocks_in_use;
        ci->modified = 1;
    }
    else if (length == ci->file_size)
    { // nothing to do.
    }
    else  // length > file_size
    {
        uint32_t new_blocks_in_use = (length-1 + BLOCK_SZ)/BLOCK_SZ;
        if (new_blocks_in_use > ci->blocks_in_use)
        {
            // allocate more blocks.
            if (alloc_blocks_for_truncate(ci, new_blocks_in_use) != 1)
            {
                __panic( "alloc blocks error in truncate\n");
                return 0;
            }
        }
        else if (new_blocks_in_use == ci->blocks_in_use)
        {
            // just update the file size;
            ci->file_size = length;
        }
        else
        {
            __panic( "error: new_blocks_in_use < blocks_in_use, while length > file_fize\n");
            return 0;
        }
        ci->blocks_in_use = new_blocks_in_use;
        ci->modified = 1;
    }
    if (iput(ci) == 0)
    {
        __panic( "iput error when truncate\n");
        return 0;
    }
    return 1;
}
