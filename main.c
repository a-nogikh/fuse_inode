#include <stdio.h>
#include <stdlib.h>
#include "bitmap.h"
#include "fs.h"

int main(int argc, char* argv[])
{
    device_init("device.txt");
//    fs_create(2000, 2000);
   fs_info *fss = fs_open();
    return fuse_init(fss, argc, argv);

    //return 0;

    device_init("device.txt");
    fs_create(1000, 1000);
    fs_info *fs = fs_open();
/*

    int inode = fs_find_inode(fs, "/aa.txt");

    int inodeid = fs_find_file(fs->root_inode, "aa.txt");
    opened_file *f2 = fs_open_inode(fs, inode_find(inodeid));
     FILE *tmp =fopen("test123.txt", "w");
    char *vv = (char *)malloc(f2->inode->size);
    fs_io(f2, 0, f2->inode->size, vv, FS_IO_READ);
    fwrite(vv, 1, f2->inode->size, tmp);
    fclose(tmp);
    fs_close_file(f2);

    return 0;
*/
    opened_file *file = fs_create_file(fs);
    fs_dir_add_file(fs->root_inode, "test.txt", file->inode->id);
    char data[1000];
    int i,j;
    for(i = 0; i < 512*1024; i++){
        for(j = 0; j < 100; j++){
            data[j] = 0;
        }
        sprintf(data,"%d\n", i);
        fs_io(file, file->inode->size, strlen(data), data, FS_IO_WRITE);
    }
   // fs_io(file, 0, strlen(data), data, FS_IO_WRITE);
    fs_close_file(file);
    fs_flush(fs);
    return 0;
}
