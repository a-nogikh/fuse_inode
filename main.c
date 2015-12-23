#include <stdio.h>
#include <stdlib.h>
#include "bitmap.h"
#include "fs.h"

int main()
{
    device_init("C:\\fuse_inode\\device.txt");
    fs_create(1000, 1000);
    fs_info *fs = fs_open();

    opened_file *file = fs_create_file(fs);
    fs_dir_add_file(fs->root_inode, "test.txt", file->inode->id);
    const char *data = "Test text";
    fs_io(file, 0, strlen(data), data, FS_IO_WRITE);
    fs_close_file(file);

    file = fs_create_file(fs);
    fs_dir_add_file(fs->root_inode, "abcd.txt", file->inode->id);
    fs_close_file(file);


    linked_file_list *list = fs_readdir(fs->root_inode), *curr = list;
    while (curr != NULL){
        printf("item: %s | ", curr->name);
        curr = curr->next;
    }

    int o = fs_find_file(fs->root_inode, "aaa.txt");


/*    char data2[1024];
    memset(data2, 0, sizeof(data));
    fs_io(file, 0, file->inode->size, data2, FS_IO_READ);
    printf("%s", data2);

    fs_close_file(file);*/
    fs_flush(fs);

    return 0;
}
