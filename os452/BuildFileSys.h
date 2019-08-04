//
// Created by Oscar on 4/23/2019.
//

#ifndef OS452_BUILDFILESYS_H
#define OS452_BUILDFILESYS_H

#include "types.h"



/*
**  _fs_mkdir - gets the path to a directory as an argument, _fs_mkdir then
**  creates a directory in the specified location
**
**  argument: - path_name - uint8_t string that leads to the directory where the file will be created
**
**  returns: 1 on success, 0 on failure
**/
uint32_t _fs_mkdir(const uint8_t *path_name);

/*
**  _fs_create - gets the path to a file as an argument, _fs_create then
**  creates a file in the specified location
**
**  argument: - path_name - uint8_t string that leads to the directory where the file will be created
**
**  returns: 1 on success, 0 on failure
**/
uint32_t _fs_create(const uint8_t *path);

/*
**  _fs_readdir - gets the path to a directory as an argument, _fs_readdir then
**  then reads every file in the directory
**
**  argument: - path_name - uint8_t string that leads to the directory that will be read
**
**  returns: 1 on success, 0 on failure
**/
uint32_t _fs_readdir(const uint8_t *path);

/*
**  _fs_rmdir - gets the path to a directory/file as an argument, _fs_rmdir then
**  then removes the file/directory at that location
**
**  argument: - path_name - uint8_t string that leads to the directory that will be read
**
**  returns: 1 on success, 0 on failure
**
**/
uint32_t _fs_rmdir(const uint8_t *path);

/*
**  _fs_read - gets the path to a directory as an argument, _fs_read then
**  then reads a specificed file of length size and puts it in the char buffer
**
**  argument: - path_name - uint8_t string that leads to the file that will be read
**              buf - where the characters that will be read ae placed into
**              size - how far the program wll read
**              offset - where the read will start from
**  returns: 1 on success, 0 on failure
**
**/
uint32_t _fs_read(const uint8_t *path, uint8_t *buf, uint32_t size, uint32_t offset);

/*
**  _fs_write - gets the path to a directory as an argument, _fs_write then
**  then writes to a specificed file of length size by reading the char buffer
**
**  argument: - path_name - uint8_t string that leads to the file that will be written to
**              buf - where the characters that will be read from
**              size - how far the program wll read
**              offset - where the read will start from
**  returns: 1 on success, 0 on failure
**
**/
uint32_t _fs_write(const int8_t *path, const uint8_t *buf, uint32_t size, uint32_t offset);

/*
**   _sys_buildfs - prepares the file system by calling startup routines in filesys.c
**
**
**/
void _sys_buildfs(void);
#endif //OS452_BUILDFILESYS_H

