#include "slist.h"
#include "pages.h"
#include "inode.h"
#include "directory.h"
#include "bitmap.h"
#include <string.h>
#include <stdio.h>
#include <errno.h>


void directory_init() {
    inode* rn = get_inode(alloc_inode());
    rn->mode = 040755;
}

int 
directory_lookup(inode* dd, const char* name) {
    if (strcmp("", name) == 0) {
        return 0; 
    }
    dirent* dirs = pages_get_page(dd->ptrs[0]);
    int n = 0;
    while(n < 64) {
        dirent current = dirs[n];
        if (current.bool && strcmp(name, current.name) == 0) {
            return current.inum;
        }
        n++;
    }
    return -1;
}

int 
tree_lookup(const char* path) {
    int current_node = NULL;
    
    slist* list = s_split(path, '/');
    slist* current_dir = list;
    while (current_dir != NULL) {
        inode* rn = get_inode(current_node);
        current_node = directory_lookup(rn, current_dir->data);
        if (current_node == -1) {
            s_free(list);
            return -1;
        }
        current_dir = current_dir->next;
    }
    s_free(list);
    return current_node;
}
    

int directory_put(inode* dd, const char* name, int inum) {
    int num = dd->size / sizeof(dirent);
    dirent new_dirs;
    dirent* dirs = pages_get_page(dd->ptrs[0]);

    strncpy(new_dirs.name, name, DIR_NAME); 
    new_dirs.bool = 1;
    new_dirs.inum = inum;

    int counted = 0; 
    int n = 1; 
    int numdirs = dd->size / sizeof(dirent);
    while(n < numdirs) {
        if (dirs[n].bool == 0) {
            dirs[n] = new_dirs;
            counted = 1;
        }
        n++;
    }
    if (counted == 0) {
        dirs[num] = new_dirs;
        dd->size += sizeof(dirent);
    }
    printf("run directory_put, put %s, inum %d, into page %d\n", name, inum, dd->ptrs[0]);
    return 0;
}


int directory_delete(inode* dd, const char* name) {
    printf("running directory_delete(%s)\n", name);
    dirent* dirs = pages_get_page(dd->ptrs[0]);
    for (int ii = 0; ii < dd->size / sizeof(dirent); ++ii) {
        if (strcmp(dirs[ii].name, name) == 0) {
            dirs[ii].bool = 0;
            reference_reduce(dirs[ii].inum);
            return 0;
        }
    }
    return -ENOENT;
}


slist* directory_list(const char* path) {
    printf("run directory_list\n");

    int count = tree_lookup(path);
    inode* node = get_inode(count);
    int numdirs = node->size / sizeof(dirent);
    dirent* dirs = pages_get_page(node->ptrs[0]);
    slist* result = NULL; 
    int n = 0;
    while(n < numdirs) {
        if (dirs[n].bool) {
            result = s_cons(dirs[n].name, result);
        }
        n++;
    }
    return result;
}


void print_directory(inode* dd) 
{
    printf("Contents:\n");
    int numdirs = dd->size / sizeof(dirent);
    dirent* dirs = pages_get_page(dd->ptrs[0]);
    for (int n = 0; n < numdirs; n++) {
        printf("%s\n", dirs[n].name);
    }
    printf("(end of contents)\n");
}

