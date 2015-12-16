#ifndef FS_H_INCLUDED
#define FS_H_INCLUDED


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

#endif // FS_H_INCLUDED
