#ifndef FS_H_INCLUDED
#define FS_H_INCLUDED

#define FS_IO_WRITE 0
#define FS_IO_READ 1

#define FS_MAGIC 0x11223344

#define FS_MAX_FILE_NAME 255

#include "bitmap.h"
#include "inode.h"
#include "device.h"
#include <stdio.h>

struct meta{
    int magic_number;

    int inodes_count, used_inodes;
    int blocks_count;
    int root_inode;

    int inode_first_block, inode_last_block;
    int inode_bitmap_fist_block, inode_bitmap_last_block;
    int disk_bitmap_first_block, disk_bitmap_last_block;
    int disk_first_block, disk_last_block;
};
typedef struct meta meta;

#define CACHED_DIRECT_POINTERS 0
#define CACHED_INDIRECT_1 1
#define CACHED_INDIRECT_2 2
#define CACHED_COUNT 3

struct opened_file {
    inode_t *inode;
    meta *meta;
    bitmap_instance *disk_bitmap;
    bitmap_instance *inode_bitmap;

    int flushed;

    char cached_blocks[CACHED_COUNT][BLOCK_SIZE];
    block_n cached_block_ids[CACHED_COUNT];
    int cached_block_flushed[CACHED_COUNT];
};
typedef struct opened_file opened_file;

struct fs_info{
    meta *meta;
    bitmap_instance *disk_bitmap, *inode_bitmap;
    opened_file *root_inode;
};
typedef struct fs_info fs_info;

struct linked_file_list{
    char *name;
    int inode_n;
    struct linked_file_list *next;
};
typedef struct linked_file_list linked_file_list;

// functions

void fs_create(int disk_blocks, int total_inodes);
fs_info *fs_open();
void fs_flush(fs_info *info);
opened_file *fs_open_inode(fs_info *info, inode_t *node);
void fs_close_file(opened_file *opened);
int fs_dir_add_file(opened_file *opened, char *name, int inode_n);
linked_file_list *fs_readdir(opened_file *opened);
int fs_find_file(opened_file *directory, char *file);
int fs_find_inode(fs_info *info, char *path);
int fs_io(opened_file *opened, size_t offset, size_t count, char *buf, int dir);
void fs_free_readdir(linked_file_list *list);
opened_file *fs_create_file(fs_info *info);

#endif // FS_H_INCLUDED
