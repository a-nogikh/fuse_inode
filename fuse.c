#define FUSE_USE_VERSION 26
#define OPENED_FILES_LIMIT 100

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include "fs.h"
#include "fuse.h"

static const char *hello_str = "Hello World!\n";
static const char *hello_path = "/hello";
static fs_info *fs;

fuse_opened_cache cache[OPENED_FILES_LIMIT];

static void cache_clear(){
    int i = 0;
    for (; i < OPENED_FILES_LIMIT; i++){
        cache[i].inode_n = -1;
    }
}

// O(n), might be rewriten to O(1)
static int cache_find_or_create(int inode_id){
    int free_id = -1, i = 0;
    for (; i < OPENED_FILES_LIMIT; i++){
        if (cache[i].inode_n == -1 && free_id < 0){
            free_id = i;
        }

        if (cache[i].inode_n == inode_id){
            return i;
        }
    }

    return free_id;
}

static int fuse_getattr(const char *path, struct stat *stbuf)
{
   memset(stbuf, 0, sizeof(struct stat));
   int inode_n = fs_find_inode(fs, path);
   if (inode_n < 0){
        return -ENOENT;
   }
    inode_t *inode = inode_find(inode_n);
    if (inode == NULL){
        return -ENOENT;
    }

    if (inode->type == INODE_FILE){
        stbuf->st_mode = S_IFREG | 0777;
        stbuf->st_nlink = 1;
        stbuf->st_size = inode->size;
    }
    else
    {
        stbuf->st_mode = S_IFDIR | 0777;
        stbuf->st_nlink = 1;
    }

   return 0;
}

static int fuse_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
        off_t offset, struct fuse_file_info *fi)
{
    int inode = fs_find_inode(fs, path);
    if (inode < 0){
        return -ENOENT;
    }

    opened_file *opened = fs_open_inode(fs, inode_find(inode));
    if (opened == NULL || opened->inode->type != INODE_DIRECTORY){
        return -ENOENT;
    }

    linked_file_list *list = fs_readdir(opened), *curr = list;

    filler(buf, ".", NULL, 0);
    filler(buf, "..", NULL, 0);

    while (curr != NULL){
        filler(buf, curr->name, NULL, 0);
        curr = curr->next;
    }

    fs_free_readdir(list);
    fs_close_file(opened);
    return 0;
}

static int fuse_open(const char *path, struct fuse_file_info *fi)
{
    int inode_n = fs_find_inode(fs, path);
    printf("%s : inode n %d\n", path, inode_n);
    if (inode_n < 0)
        return -ENOENT;

    int cache_id = cache_find_or_create(inode_n);
    printf("cache_id: %d\n", cache_id);
    if (cache_id < 0)
        return -ENOENT;

    opened_file *opened = NULL;
    if (cache[cache_id].inode_n != inode_n){
        opened = fs_open_inode(fs, inode_find(inode_n));
        cache[cache_id].file = opened;
    }
    else
        opened = cache[cache_id].file;

    if (opened == NULL)
        return -ENOENT;

    if ((fi->flags & 3) != O_RDONLY)
        return -EACCES;

    fi->fh = cache_id;
    return 0;
}

static int fuse_read(const char *path, char *buf, size_t size, off_t offset,
      struct fuse_file_info *fi)
{
    opened_file *opened = NULL;
    if (fi->fh < 0 || fi->fh >= CACHED_COUNT
        || (opened = cache[fi->fh].file) == NULL)
                return -ENOENT;

   return fs_io(opened, offset, size, buf, FS_IO_READ);
}

static struct fuse_operations hello_oper = {
   .getattr   = fuse_getattr,
   .readdir   = fuse_readdir,
   .open     = fuse_open,
   .read     = fuse_read,
};

int fuse_init(fs_info *fs_info, int argc, char *argv[])
{
    fs = fs_info;
    cache_clear();
    return fuse_main(argc, argv, &hello_oper, NULL);
}
