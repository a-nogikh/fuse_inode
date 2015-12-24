#ifndef FUSE_H_INCLUDED
#define FUSE_H_INCLUDED

struct fuse_opened_cache{
    int inode_n;
    opened_file *file;
};

typedef struct fuse_opened_cache fuse_opened_cache;

#endif // FUSE_H_INCLUDED
