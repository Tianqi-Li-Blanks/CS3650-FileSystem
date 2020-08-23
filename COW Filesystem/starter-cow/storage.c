#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <assert.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <alloca.h>
#include <string.h>
#include <libgen.h>
#include <bsd/string.h>
#include <stdint.h>

#include "storage.h"
#include "slist.h"
#include "util.h"
#include "pages.h"
#include "inode.h"
#include "directory.h"
#include "bitmap.h"

static void 
link_char(const char* path, char* tmp1, char* tmp2) {
    slist* copy_dirs = s_split(path, '/');
    slist* dirs = copy_dirs;

    tmp2[0] = 0;
    while (dirs->next) {
        strncat(tmp2, dirs->data, 48);
        strncat(tmp2, "/", 1);
        dirs = dirs->next;
    }
    memcpy(tmp1, dirs->data, strlen(dirs->data));
    tmp1[strlen(dirs->data)] = 0;
    s_free(copy_dirs);
}

void
storage_init(const char* path) 
{
    pages_init(path);
    void* pg = get_pages_bitmap();
    if (!bitmap_get(pg, 4)) {
        directory_init();
    }
}


int 
storage_stat(const char* path, struct stat* st) {

    printf("+ storage_stat(%s)\n", path);
    int count = tree_lookup(path);
    if (count > 0) {
        inode* node = get_inode(count);
        printf("+ storage_stat(%s); inode %d\n", path, count);
        st->st_nlink = node->refs;
        st->st_size = node->size;
        st->st_mode = node->mode;
        st->st_atime = node->time_access;
        st->st_ctime = node->time_create;
        st->st_mtime = node->time_modify;
        return 0;
    }
    return -1;
}

int
storage_read(const char* path, char* buf, size_t size, off_t offset)
{
    int count = 0;
    int r_size = size;
    int copy_off = offset;
    int inum = tree_lookup(path);
    inode* node = get_inode(inum);


    while (r_size > 0) {
        int n = inode_get_pnum(node, copy_off);
        char* pg = pages_get_page(n);
        pg = pg + copy_off % 4096;
        int temp = 4096 - (copy_off % 4096);
        int mins = min(r_size, temp);

        memcpy(buf + count, pg, mins);
        copy_off = mins + copy_off;
        count = mins + count;
        r_size = r_size - mins;
    }
    return size;
}

int 
storage_write(const char* path, const char* buf, size_t size, off_t offset)
{
    int inum = tree_lookup(path);
    inode* node = get_inode(inum);

    int tsize = size + offset;
    if (node->size < tsize) {
        storage_truncate(path, size + offset);
    }
    int count = 0;
    int r_size = size;
    int copy_off = offset;

    while (r_size > 0) {
        int n = inode_get_pnum(node, copy_off);
        char* pg = pages_get_page(n);
        pg = pg + copy_off % 4096;
        int temp = 4096 - (copy_off % 4096);
        int mins = min(r_size, temp);

        memcpy(pg, buf + count, mins);
        count = mins + count;
        copy_off = mins + copy_off;
        r_size = r_size - mins;
    }
    return size;    
}

int 
storage_truncate(const char* path, off_t size) 
{
    int inum = tree_lookup(path);
    inode* node = get_inode(inum);

    if (node->size >= size) {
        shrink_inode(node, size);
    } 
    else {
        grow_inode(node, size);
    }
    return 0;
}

int
storage_mknod(const char* path, int mode) {
    if (tree_lookup(path) != -1) {
        return -EEXIST;
    }
 
    char* tmp1 = malloc(50);
    char* tmp2 = malloc(strlen(path));
    link_char(path, tmp1, tmp2);

    int node_num = tree_lookup(tmp2);
    if (node_num < 0) {
        free(tmp1);
        free(tmp2);   
        return -ENOENT;
    }
    
    int inum = alloc_inode();
    inode* node = get_inode(inum);
    node->mode = mode;
    node->size = 0;

    node->refs = 1;
    inode* dirs= get_inode(node_num);

    directory_put(dirs, tmp1, inum);
    free(tmp1);
    free(tmp2);
    return 0;
}

slist* 
storage_list(const char* path) 
{
    return directory_list(path);
}

int
storage_unlink(const char* path) 
{
    char* tmp1 = malloc(50);
    char* tmp2 = malloc(strlen(path));
    link_char(path, tmp1, tmp2);

    int n = tree_lookup(tmp2);
    inode* node = get_inode(n);
    int rv = directory_delete(node, tmp1);

    free(tmp1);
    free(tmp2);
    return rv;
}

int    
storage_link(const char* from, const char* to) 
{
    int num = tree_lookup(to);
    if (num < 0) {
        return num;
    }

    char* tmp1 = malloc(50);
    char* tmp2 = malloc(strlen(from));
    link_char(from, tmp1, tmp2);

    int n = tree_lookup(tmp2);
    inode* node = get_inode(n);
    directory_put(node, tmp1, num);
    get_inode(num)->refs ++;
    
    free(tmp1);
    free(tmp2);
    return 0;
}

int    
storage_rename(const char* from, const char* to) 
{
    storage_link(to, from);
    storage_unlink(from);
    return 0;
}

int    
storage_set_time(const char* path, const struct timespec ts[2])
{
    int num = tree_lookup(path);
    if (num < 0) {
        return -ENOENT;
    }
    inode* node = get_inode(num);
    node->time_access = ts[0].tv_sec;
    node->time_modify = ts[1].tv_sec;
    return 0;
}

int
storage_access(const char* path) {

    int rv = tree_lookup(path);
    if (rv >= 0) {
        inode* node = get_inode(rv);
        time_t curtime = time(NULL);
        node->time_access= curtime;
        return 0;
    }
    else
        return -ENOENT;
}

int
storage_sym_link(const char* from, const char* to) {
    int rv = storage_mknod(from, 0120000);
    if (rv < 0 ) {
        return rv;
    }
    storage_write(from, to, strlen(to), 0);
    return 0;
}


void   
storage_update_time(const char* path) {
    int inum = tree_lookup(path);
    if (inum < 0) {
        return;
    }
    inode* node = get_inode(inum);
    node->time_create = time(NULL);
}

int    
storage_mode(const char* path, mode_t mode) {
    int inum = tree_lookup(path);
    if (inum < 0) {
        return -ENOENT;
    }
    inode* node = get_inode(inum);
    node->mode = node->mode & mode & ~07777;
}




