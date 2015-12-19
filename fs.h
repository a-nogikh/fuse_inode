#ifndef FS_H_INCLUDED
#define FS_H_INCLUDED

#define FS_IO_WRITE 0
#define FS_IO_READ 1

struct fs_info{
    meta *meta;
    bitmap_instance *disk_bitmap, *inode_bitmap;
};
typedef struct fs_info fs_info;

struct meta{
    int inodes_count;
    int blocks_count;
    int root_inode;

    int inode_first_block, inode_last_block;
    int inode_bitmap_fist_block, inode_bitmap_last_block;
    int disk_bitmap_first_block, disk_bitmap_last_block;
    int disk_first_block, disk_last_block;
};
typedef struct meta meta;

struct linked_file_list{
    char *name;
    int inode_n;
    linked_file_list *next;
};
typedef struct linked_file_list linked_file_list;


#endif // FS_H_INCLUDED
