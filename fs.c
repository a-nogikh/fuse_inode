#include "fs.h"

// meta -> inode_bitmap -> inode -> disk_bitmap -> disk

void fs_create(int disk_blocks, int total_inodes){
    // adjusting total disk blocks to fit in the allocated bitmap
    int disk_bitmap_size = bitmap_get_blocks_count(disk_blocks);
    disk_blocks = bitmap_bits_from_blocks(disk_bitmap_size);

    int inode_blocks = inode_get_size(total_inodes);
    int inode_bitmap_size = bitmap_get_blocks_count(total_inodes);
    meta *meta_i = (meta *)malloc(sizeof(meta));
    meta_i->magic_number = FS_MAGIC;
    meta_i->blocks_count = 1 + inode_blocks + disk_bitmap_size + inode_bitmap_size + disk_blocks;
    meta_i->inode_bitmap_fist_block = 1;
    meta_i->inode_bitmap_last_block = meta_i->inode_bitmap_fist_block + inode_bitmap_size - 1;
    meta_i->inode_first_block = meta_i->inode_bitmap_last_block + 1;
    meta_i->inode_last_block = meta_i->inode_first_block + inode_blocks - 1;
    meta_i->disk_bitmap_first_block = meta_i->inode_last_block + 1;
    meta_i->disk_bitmap_last_block = meta_i->disk_bitmap_first_block + disk_bitmap_size - 1;
    meta_i->disk_first_block = meta_i->disk_bitmap_last_block + 1;
    meta_i->disk_bitmap_last_block = meta_i->disk_bitmap_last_block + disk_blocks - 1;
    meta_i->root_inode = 0;
    meta_i->used_inodes = 0;

    char tmp[BLOCK_SIZE];
    memset(tmp, 0, BLOCK_SIZE);

    int i = 0;
    for (; i < meta_i->blocks_count; i++){
        device_write_block(i, tmp);
    }

    // root inode
    inode_init(meta_i->inode_first_block, meta_i->inode_last_block);
    bitmap_instance *inode_bitmap = bitmap_init(meta_i->inode_bitmap_fist_block, meta_i->inode_bitmap_last_block);
    inode_t *node = inode_make(inode_bitmap);
    node->type = INODE_DIRECTORY;
    inode_save(node);

    meta_i->root_inode = node->id;
    meta_i->used_inodes++;
    device_write_block_ofs(0, (char *)meta_i, 0, sizeof(meta));

    inode_free(node);
    bitmap_flush(inode_bitmap);
    bitmap_free(inode_bitmap);
    free(meta_i);
}

fs_info *fs_open(){
    meta *meta_i = (meta *)malloc(sizeof(meta));
    device_read_block_ofs(0, (char *)meta_i, 0, sizeof(meta));
    if (meta_i->magic_number != FS_MAGIC){
        free(meta_i);
        return NULL;
    }

    bitmap_instance *inode_bitmap = bitmap_init(meta_i->inode_bitmap_fist_block, meta_i->inode_bitmap_last_block);
    bitmap_instance *disk_bitmap = bitmap_init(meta_i->disk_bitmap_first_block, meta_i->disk_bitmap_last_block);

    inode_init(meta_i->inode_first_block, meta_i->inode_last_block);

    fs_info *fs = (fs_info *)malloc(sizeof(fs_info));
    fs->disk_bitmap = disk_bitmap;
    fs->inode_bitmap = inode_bitmap;
    fs->meta = meta_i;

    inode_t *root_inode = inode_find(meta_i->root_inode);
    if (root_inode == NULL){
        free(meta_i);
        free(fs);
        bitmap_free(disk_bitmap);
        bitmap_free(inode_bitmap);
        return NULL;
    }
    fs->root_inode = NULL;
    fs->root_inode = fs_open_inode(fs, root_inode);
    return fs;
}

void fs_flush(fs_info *info){
    device_write_block_ofs(0, (char *)(info->meta), 0, sizeof(meta));
    bitmap_flush(info->disk_bitmap);
    bitmap_flush(info->inode_bitmap);
    inode_flush_data(info->root_inode);
}

opened_file *fs_open_inode(fs_info *info, inode_t *inode){
    if (inode == NULL){
        return NULL;
    }

    if (info->root_inode != NULL && info->root_inode->inode != NULL
            && inode->id == info->root_inode->inode->id){
        return info->root_inode;
    }

    opened_file *handle = (opened_file *)malloc(sizeof(opened_file));
    handle->flushed = 1;
    handle->inode = inode;
    handle->inode_bitmap = info->inode_bitmap;
    handle->disk_bitmap = info->disk_bitmap;
    handle->meta = info->meta;
    int i = 0;
    for (; i < CACHED_COUNT; i++){
        handle->cached_block_ids[i] = -1;
    }

    return handle;
}

opened_file *fs_create_file(fs_info *info){
    inode_t *inode = inode_make(info->inode_bitmap);
    if (inode == NULL){
        return NULL;
    }

    opened_file *handle = (opened_file *)malloc(sizeof(opened_file));
    handle->flushed = 0;
    handle->inode = inode;
    handle->inode_bitmap = info->inode_bitmap;
    handle->disk_bitmap = info->disk_bitmap;
    handle->meta = info->meta;
    int i = 0;
    for (; i < CACHED_COUNT; i++){
        handle->cached_block_ids[i] = -1;
    }

    inode_save(handle->inode);

    return handle;
}

void fs_close_file(opened_file *opened){
    inode_flush_data(opened);
    if (opened->inode->id != opened->meta->root_inode)
        free(opened);
}

int fs_dir_add_file(opened_file *opened, char *name, int inode_n){
    if (opened->inode->type != INODE_DIRECTORY){
        return 0;
    }

    int len = strlen(name), struct_len = len + sizeof(int) + sizeof(char);
    char *add = (char *)malloc(struct_len), *pos = add;
    *((int *)pos) = inode_n; pos += sizeof(int);
    *pos++ = (char)len;
    memcpy(pos, name, len);
    fs_io(opened, opened->inode->size, struct_len, add, FS_IO_WRITE);
    free(add);
    return 1;
}

void fs_truncate(opened_file *opened){
    opened->inode->size = 0;
}

void fs_save_dir(opened_file *opened, linked_file_list *list){
    fs_truncate(opened);
    linked_file_list *curr = list;
    while (curr != NULL){
        fs_dir_add_file(opened, curr->name, curr->inode_n);
        curr = curr->next;
    }
}

linked_file_list *fs_readdir(opened_file *opened){
    if (opened->inode->type != INODE_DIRECTORY){
        return NULL;
    }

    char *data = (char *)malloc(sizeof(char) * opened->inode->size + 256 + sizeof(block_n));
    fs_io(opened, 0, opened->inode->size, data, FS_IO_READ);

    int blid;
    char len;
    linked_file_list *list = NULL;

    char *pos = data, *last = data + opened->inode->size;
    while ( pos < last){
        blid = *pos; pos += sizeof(blid);
        len = *(pos++);


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
    return list;
}

int fs_find_file(opened_file *directory, char *file){
    linked_file_list *list = fs_readdir(directory), *curr = list;
    int result = -1;
    while (curr != NULL){
        if (strcmp(curr->name, file) == 0){
            result = curr->inode_n;
            break;
        }
        curr = curr->next;
    }
    fs_free_readdir(list);
    return result;
}

int fs_find_inode(fs_info *info, char *path){
    if (*path == '/'){
        path++;
    }
    if (!*path){
        return info->root_inode->inode->id;
    }

    int found_inode = -1, last_inode = -1, skip = 0;
    opened_file *curr = info->root_inode;
    char buf[FS_MAX_FILE_NAME + 1];

    while(1){
        int len = str_take_till(path, buf, '/', FS_MAX_FILE_NAME);
        if (len <= 0){
            found_inode = last_inode;
            break;
        }
        buf[len] = 0;
        while(len-- > 0) path++;

        if (curr == NULL){
            break;
        }

        int inode = fs_find_file(curr, buf);
        if (inode < 0){
            break;
        }

        if (curr != info->root_inode){
            fs_close_file(curr);
        }

        inode_t *inode_p = inode_find(inode);
        if (inode_p == NULL){
            break;
        }

        if(inode_p->type != INODE_DIRECTORY){
            curr = NULL;
            last_inode = inode;
            inode_free(inode_p);
            continue;
        }

        curr = fs_open_inode(info, inode_p);
        last_inode = inode;
    }

    return found_inode;
}

opened_file *fs_find_open_inode(fs_info *info, char *path){
    int inode_n = fs_find_inode(info, path);
    if (inode_n < 0){
        return NULL;
    }

    inode_t *inode = inode_find(inode_n);
    if (inode == NULL){
        return NULL;
    }

    return fs_open_inode(info, inode);
}

int fs_io(opened_file *opened, size_t offset, size_t count, char *buf, int dir){
    int tmp_ofs = offset % BLOCK_SIZE, bl_ofs = 0;

    int pos = offset, maxpos = offset + count, processed = 0;
    if (maxpos > opened->inode->size && dir == FS_IO_READ){
        maxpos = opened->inode->size;
    }

    for(; pos < maxpos; )
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
               real_block + opened->meta->disk_first_block, buf + (pos - offset),
               from, count
            );
        }
        else if (dir == FS_IO_READ){
            device_read_block_ofs(
               real_block + opened->meta->disk_first_block, buf + (pos - offset),
               from, count
            );
        }

        pos += count;
        processed += count;

        if(pos > opened->inode->size){
            opened->inode->size = pos;
            opened->flushed = 0;
        }

        if (count == 0){
            break;
        }
    }

    if (!opened->flushed){
        inode_flush_data(opened);
    }

    return processed;
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
