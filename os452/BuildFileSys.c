//
// Created by Oscar on 4/16/2019.
//

#include <x86arch.h>

#include "cio.h"
#include "filesys.h"
#include "types.h"
#include "lib.h"
#include "memory.h"
#include "support.h"


uint32_t _fs_mkdir(const uint8_t *path_name){
#if _DEBUG
    __cio_printf("fs_mkdir gets called\n");
#endif
    uint32_t res = mkdir_v2(path_name);
#if _DEBUG
    __cio_printf("exiting _fs_mkdir\n");
#endif
    return res;
}

uint32_t _fs_create(const uint8_t *path){
#if _DEBUG
    __cio_printf("_fs_create gets called\n");
#endif
    uint32_t res = create_v2(path);
    return res;
}

uint32_t _fs_readdir(const uint8_t *path){
#if _DEBUG
    __cio_printf("_fs_readdir gets called\n");
#endif
    struct in_core_inode* ci;
    ci = namei_v2(path);
    if (ci == NULL){
        return 0;
    }
    if (ci->file_type != DIRECTORY){
        return 0;
    }
    uint32_t off;
    struct directory_entry* dir_entry;
    for (off = 0; off < MAX_ENTRY_OFFSET - DIR_ENTRY_LENGTH; off += DIR_ENTRY_LENGTH){
        uint32_t blk_num;
        uint32_t offset_blk;
        uint8_t buf[BLOCK_SZ];
        if (bmap(ci, off, &blk_num, &offset_blk) == 0){
            return 0;  /* No such device or address */
        }
        if (block_read(blk_num, buf) == 0){
            return 0;  /* No such device or address */
        }
        dir_entry = (struct directory_entry*)(buf + offset_blk);
        uint32_t i_num = dir_entry->inode_num;
        if (i_num == BAD_I_NUM){  // the end of dir.
            break;
        }
        if (i_num == EMPTY_I_NUM){  // not a valid entry, may be deleted.
            continue;
        }
        if (i_num == 512){
            break;
        }
        // dir entry is valid, so show the entry.
        struct in_core_inode* i_entry = iget(i_num);

        if (i_entry == NULL){
            return 0;
        }
        if (iput(i_entry) == 0){
            return 0;  
        }
        else{
            __cio_printf("%s\n",dir_entry->file_name);
        }
    }
    ci->last_accessed = get_time();
    ci->modified = 1;
    uint32_t res = iput(ci);
    if (res != 1){
        __cio_printf( "iput error in read_v2\n");
        return 0;
    }
#if _DEBUG
    __cio_printf("readdir complete\n");
#endif
    return 1;
}

uint32_t _fs_rmdir(const uint8_t *path){
#if _DEBUG
    __cio_printf("\n_fs_rmdir gets called\n");
#endif
    uint32_t res = unlink(path);
    if (res == 0){
        __cio_printf("file could not be found\n");
        return 0;
    }
    return res;
}

uint32_t _fs_read(const uint8_t *path, uint8_t *buf, uint32_t size, uint32_t offset){
    uint32_t res;
    // from offset, copy size of bytes from the file indicated by path to buf
    struct in_core_inode* ci;
    ci = namei_v2(path);
#if _DEBUG
    __cio_printf("got the file %s\n",ci->owner_id);
#endif
    res = read_v2(ci, buf, ci->file_size, offset);
    return res;
}

uint32_t _fs_write(const uint8_t *path, const uint8_t *buf, uint32_t size, uint32_t offset){
    uint32_t res;
    uint32_t s;
    // copy buf to the file from the offset, update to size of bytes.
    struct in_core_inode* ci;
#if _DEBUG
    __cio_printf("looking for the file");
        __delay(400);
#endif
    ci = namei_v2(path);
    s = __strlen((char *)buf);
#if _DEBUG
    __cio_printf("got the file %s : with size %s\n",ci->owner_id,s);
        __delay(400);
#endif
    res = write_v2(ci, buf, s, offset);
    return res;
}

void _sys_buildfs(void){
    if (init_storage() == 0){
        __panic("error opening the storage\n");
        return ;
    }
    if (mkfs() == 0){
        __panic("error starting mkfs\n");
        return ;
    }
    if(cleanup_storage() == 0){
        __panic("error in cleanup_storage\n");
        return;
    }
    return;
}


