#ifndef DEVICE_H_INCLUDED
#define DEVICE_H_INCLUDED

#define BLOCK_SIZE 1024

int device_init(const char *file_path);
void device_write_block(int n, char* src);
void device_write_block_ofs(int n, char *src, int ofs, int len);
void device_read_block(int n, char *dst);
void device_read_block_ofs(int n, char *dst, int ofs, int len);
void device_close();

#endif // DEVICE_H_INCLUDED
