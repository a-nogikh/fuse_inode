#include "fs.h"

// meta -> inode_bitmap -> inode -> disk_bitmap -> disk

void fs_create(int disk_blocks, int total_inodes){
    int inode_blocks = inode_get_size(total_inodes);
    int disk_bitmap_size = bitmap_get_blocks_count(disk_blocks);
    int inode_bitmap_size = bitmap_get_blocks_count(total_inodes);
    meta *meta = (meta *)malloc(sizeof(meta));
    meta->blocks_count = 1 + inode_blocks + disk_bitmap_size + inode_bitmap_size + disk_blocks;
    meta->inode_bitmap_fist_block = meta->disk_bitmap_last_block + 1;
    meta->inode_bitmap_last_block = meta->inode_bitmap_fist_block + inode_bitmap_size;
    meta->inode_first_block = meta->inode_bitmap_last_block + 1;
    meta->inode_last_block = meta->inode_first_block + inode_blocks;
    meta->disk_bitmap_first_block = meta;
    meta->disk_bitmap_last_block = meta->disk_bitmap_first_block + disk_bitmap_size;
    meta->disk_first_block = meta->disk_bitmap_last_block + 1;
    meta->disk_bitmap_last_block = meta->disk_bitmap_last_block + disk_blocks;
    meta->root_inode = 0;
    // meta->inode_current = 0; --> добавить

    char tmp[BLOCK_SIZE];
    int i = 0;
    for (; i < BLOCK_SIZE; i++){
        tmp[i] = 0;
    }

    for (int i = 0; i < meta->blocks_count; i++){
        device_write_block(i, tmp);
    }

    device_write_block_ofs(0, (char *)meta, 0, sizeof(meta));

    // add 1 inode, empty content

}

fs_info *fs_open(){

}

opened_file *fs_open_inode(fs_info *info, inode *node){
    opened_file *handle = (opened_file *)malloc(sizeof(opened_file));
    opened_file->flushed = 1;
    opened_file->inode = node;
    opened_file->inode_bitmap = info->inode_bitmap;
    opened_file->disk_bitmap = info->disk_bitmap;
    opened_file->meta = info->meta;
    int i = 0;
    for (; i < CACHED_COUNT; i++){
        opened_file->cached_block_ids[i] = -1;
    }

    return opened_file;
}

void fs_dir_add_file(opened_file *opened, char *name, int inode_n){
    int len = strlen(name), struct_len = len + sizeof(int) + sizeof(char);
    char *add = (char *)malloc(struct_len), *pos = add;
    *((int *)pos) = inode_n; pos += sizeof(int);
    *pos++ = (char)len;
    memcpy(pos, name, len);
    fs_io(opened, opened->inode->size, struct_len, FS_IO_WRITE);
    free(add);
}

linked_file_list *fs_readdir(opened_file *opened){
    char *data = (char *)malloc(sizeof(char) * opened->inode->size + 256 + sizeof(block_n));
    fs_io(opened, 0, opened->inode->size, data, FS_IO_READ);

    int blid;
    char len;
    linked_file_list *list = NULL;

    char *pos = data, *last = data + opened->inode->size;
    while ( pos < last){
        len = *(pos++);
        blid = *pos; pos += sizeof(blid);

        linked_file_list *item = (linked_file_list *)malloc(sizeof(linked_file_list));
        item->inode_n = blid;
        item->name = (char *)malloc(len + 1);
        memcpy(item->name, pos, len);
        *(item->name + len) = 0;

        item->next = list;
        list = item;
        pos += len;
    }

    free(data);
    return NULL;
}

int fs_find_file(opened_file *directory, char *file){
    linked_file_list *list = fs_readdir(directory), *curr = list;
    int result = -1;
    while (curr != NULL){
        if (stricmp(curr->name, file) == 0){
            result = curr->inode_n;
            break;
        }
        curr = curr->next;
    }
    fs_free_readdir(list);
    return result;
}

void fs_free_readdir(linked_file_list *list){
    linked_file_list *curr = list, *tmp;
    while(curr != NULL){
        tmp = curr;
        curr = curr->next;
        free(tmp->name);
        free(tmp);
    }
}

int fs_find_inode(char *path){
    // explode ...
    return -1;
}

int fs_io(opened_file *opened, size_t offset, size_t count, char *buf, int dir){
    int tmp_ofs = offset % BLOCK_SIZE, bl_ofs = 0;

    int pos = offset, maxpos = offset + count, processed = 0;
    if (maxpos > opened->inode->size){
        maxpos = opened->inode->size;
    }

    for(; pos < maxpos; pos++)
    {
        int seq_block = pos / BLOCK_SIZE;
        if (seq_block >= opened->inode->blocks_count && dir == FS_IO_READ){
            break; // no reason to continue
        }

        int real_block = fs_get_disk_block_id(opened, seq_block);
        if (real_block < 0){
            return -1; // error
        }

        int from = pos % BLOCK_SIZE,
            count = BLOCK_SIZE;
        if (count + from > BLOCK_SIZE) count = BLOCK_SIZE - from;
        if (count + pos > maxpos) count = maxpos - pos;

        if (dir == FS_IO_WRITE){
            device_write_block_ofs(
               real_block, buf + (pos - offset),
               from, count
            );
        }
        else if (dir == FS_IO_READ){
            device_read_block_ofs(
               real_block, buf + (pos - offset),
               from, count
            );
        }

        pos += count;
        processed += count;

        if(pos > opened->inode->size){
            opened->inode->size = pos;
            opened->flushed = 0;
        }
    }

    if (!opened->inode->flushed){
        inode_flush_data(opened);
    }

    return processed;
}
