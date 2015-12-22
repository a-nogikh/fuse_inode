#include <stdio.h>
#include <stdlib.h>
#include "bitmap.h"

int main()
{
    device_init("C:\\fuse_inode\\device.txt");
    bitmap_instance *bitmap = bitmap_init(0, 10);
    bitmap_set(bitmap, 0, 1);
    int z = bitmap_find(bitmap, 0);
    printf("%d", z);
    return 0;


        int i = 0;
        for(; i < bitmap->bits_count; i ++){
            if (bitmap_get(bitmap, i)){
                printf("%d\n", i);
            }
        }
    /*
    bitmap_set(bitmap, 10, 1);
    bitmap_set(bitmap, 100, 1);
    bitmap_set(bitmap, 1000, 1);
    bitmap_set(bitmap, 10000, 1);
    bitmap_flush(bitmap);
*/
    printf("Hello world!\n");
    return 0;
}
