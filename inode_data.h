#ifndef INODE_DATA_H_INCLUDED
#define INODE_DATA_H_INCLUDED

#include "fs.h"
#include "inode.h"




int fs_get_disk_block_id(opened_file *handle, int seq_block);
void inode_flush_data(opened_file *opened);

#endif // INODE_DATA_H_INCLUDED
