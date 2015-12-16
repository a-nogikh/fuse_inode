#include "device.h"
#include <stdlib.h>
#include <string>

FILE *file = NULL;

static void seek_to_block(int n, int ofs);
static void fill_buffer(size_t sz, char *buf, int len);


int device_init(const char *file_path){
    file = fopen(file_path, "rb+");
    if (!file){
        file = fopen(file_path, "wb+");
    }

    if (!file){
        return 0;
    }
    return 1;
}

void device_write_block(int n, char* src){
    seek_to_block(n, 0);
    fwrite(src, sizeof(char), BLOCK_SIZE, file);
}

void device_write_block_ofs(int n, char *src, int ofs, int len){
    seek_to_block(n, ofs);
    fwrite(src, sizeof(char), len, file);
}

void device_clear_block(int n, char fill){
    char tmp[BLOCK_SIZE];
    memset(tmp, fill, BLOCK_SIZE);
    device_write_block(n, tmp);
}

void device_read_block(int n, char *dst){
    seek_to_block(n, 0);
    size_t read_result = fread(dst, sizeof(char), BLOCK_SIZE, file);
    fill_buffer(read_result, dst, BLOCK_SIZE);
}

void device_read_block_ofs(int n, char *dst, int ofs, int len){
    seek_to_block(n, ofs);
    size_t read_result = fread(dst, sizeof(char), len, file);
    fill_buffer(read_result, dst, len);
}

void device_close(){
    if (file == NULL){
        return;
    }
    fclose(file);
    file = NULL;
}

// private functions

static void seek_to_block(int n, int ofs){
    fseek(file, n * BLOCK_SIZE + ofs, SEEK_SET);
}

static void fill_buffer(size_t sz, char *buf, int len){
    if (sz == len){
        return;
    }
    memset(buf + sz, 0, sizeof(char) * (len - sz));
}
