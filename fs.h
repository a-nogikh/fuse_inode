#ifndef FS_H_INCLUDED
#define FS_H_INCLUDED


struct meta{
    int inodes_count;
    int blocks_count;
    int root_inode;

    int inode_first_block, inode_last_block;
    int inode_bitmap_fist_block, inode_bitmap_last_block;
    int disk_bitmap_first_block, disk_bitmap_last_block;
};

struct opened_file{
    inode *inode;
    int level_1_block_id;
    char level_1[BLOCK_SIZE];
    int level_2_block_id;
    char level_2[BLOCK_SIZE];
};
typedef struct opened_file opened_file;

#endif // FS_H_INCLUDED
