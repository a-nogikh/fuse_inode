#define FUSE_USE_VERSION 26
#define OPENED_FILES_LIMIT 100

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include "fs.h"
#include "fuse.h"
#include "utils.h"

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
        inode_t *inode_s = inode_find(inode_n);
        opened = fs_open_inode(fs, inode_s);
        cache[cache_id].file = opened;
        cache[cache_id].inode_n = inode_n;
    }
    else
        opened = cache[cache_id].file;

    if (opened == NULL)
        return -ENOENT;

/*    if ((fi->flags & 3) != O_RDONLY)
        return -EACCES; */

    fi->fh = cache_id;
    return 0;
}

static int fuse_read(const char *path, char *buf, size_t size, off_t offset,
      struct fuse_file_info *fi)
{
    opened_file *opened = NULL;
    printf("TRY READ[%d/%d/%d]", fi->fh, cache[fi->fh].inode_n, cache[fi->fh].file == NULL ? 0 : 1);
    if (fi->fh < 0 || fi->fh >= CACHED_COUNT
        || (opened = cache[fi->fh].file) == NULL)
                return -ENOENT;

   return fs_io(opened, offset, size, buf, FS_IO_READ);
}

static int fuse_write(const char * path, const char * buf, size_t size, off_t offset,
        struct fuse_file_info *fi){

    opened_file *opened = NULL;
    printf("TRY WRITE[%d/%d/%d]", fi->fh, cache[fi->fh].inode_n, cache[fi->fh].file == NULL ? 0 : 1);

    if (fi->fh < 0 || fi->fh >= CACHED_COUNT
        || (opened = cache[fi->fh].file) == NULL)
                return -ENOENT;

   return fs_io(opened, offset, size, buf, FS_IO_WRITE);
}

static int fuse_mknod(const char *path, mode_t mode, dev_t rdev)
{
    if (!S_ISREG(mode)){
        return -EINVAL;
    }

    int len = strlen(path);
    char *dir = (char *)malloc(len + 1),
        *name = (char *)malloc(len + 1);

    int i = len - 1, j = 0;
    for (;i >= 0 && path[i] != '/'; i--);

    while (j <= i){
        dir[j] = path[j];
        j++;
    }
    dir[j] = 0;

    j = 0;
    while (i + j + 1 < len){
        name[j] = path[i+j+1];
        j++;
    }
    name[j] = 0;

    printf("dir : %s file : %s\n", dir, name);
    int inode_n = fs_find_inode(fs, dir);
    printf("inode: %d\n", inode_n);

    if (inode_n < 0){
        free(dir); free(name);
        return -ENOENT;
    }

    opened_file *dirh = fs_open_inode(fs, inode_find(inode_n));
    if (dirh == NULL){
        free(dir); free(name);
        return -EIO;
    }

    if (fs_find_file(dirh, name) >= 0){
        free(dir); free(name); fs_close_file(dirh);
        return -EEXIST;
    }

    opened_file *file = fs_create_file(fs);
    file->inode->type = INODE_FILE;
    fs_dir_add_file(dirh, name, file->inode->id);
    fs_close_file(dirh);
    fs_close_file(file);
    free(dir);free(name);
    fs_flush(fs);
    return 0;

}

// not tested
int fuse_rename(const char * old, const char *neww){
    char old_path[255], old_name[255];
    char new_path[255], new_name[255];

    str_before_last(old, old_path, '/');
    str_after_last(old, old_name, '/');
    str_before_last(neww, new_path, '/');
    str_after_last(neww, new_name, '/');

    opened_file *orig_dir = fs_find_open_inode(fs,old_path),
            *new_dir = fs_find_open_inode(fs,new_path);

    int old_id = fs_find_file(orig_dir, old_name),
        new_id = fs_find_file(new_dir, new_name);
    if (old_id < 0){
        fs_close_file(orig_dir);
        fs_close_file(new_dir);
        return -ENOENT;
    }

    if (new_id >= 0){
        fs_close_file(orig_dir);
        fs_close_file(new_dir);
        return -EALREADY;
    }

    linked_file_list *list = fs_readdir(orig_dir), *curr = list, *now = NULL;
    int found = 0;
    while (curr != NULL){
        if (curr->inode_n != old_id){
            linked_file_list *item = (linked_file_list *)malloc(sizeof(linked_file_list));
            item->inode_n = curr->inode_n;
            item->name = strdup(curr->name);
            if (now != NULL) now->next = item;
            now = item;
        }
        curr = curr->next;
    }

    fs_save_dir(orig_dir, now);
    fs_free_readdir(now);
    fs_free_readdir(list);
    fs_dir_add_file(new_dir, new_name, old_id);

    fs_close_file(orig_dir);
    fs_close_file(new_dir);
    fs_flush(fs);

    printf("orig: %s / %s\n", old, neww);
    printf("new: %s / %s\n", new_name, new_path);
    printf("old: %s / %s\n", old_name, old_path);
    return 0;
}

static struct fuse_operations hello_oper = {
   .getattr   = fuse_getattr,
   .readdir   = fuse_readdir,
   .open     = fuse_open,
   .read     = fuse_read,
   .write    = fuse_write,
   .mknod    = fuse_mknod,
   .rename  = fuse_rename
};

int fuse_init(fs_info *fs_info, int argc, char *argv[])
{
    fs = fs_info;
    cache_clear();
    return fuse_main(argc, argv, &hello_oper, NULL);
}
