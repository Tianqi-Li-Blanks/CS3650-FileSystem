// inode implementation

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "inode.h"
#include "pages.h"
#include "bitmap.h"

/* Definition of the inode structure:

typedef struct inode {
    int refs; // reference count
    int mode; // permission & type
    int size; // bytes
    int ptrs[2]; // direct pointers
    int iptr; // single indirect pointer
    time_t atim;
    time_t ctim;
    time_t mtim;
} inode; */

void 
print_inode(inode* node) {
    printf("node position: %p:\n", node);
    printf("the refs num: %d\n", node->refs);
    printf("node mode: %d\n", node->mode);
    printf("node size: %d\n", node->size);
    printf("node pointers: %d, %d\n", node->ptrs[0], node->ptrs[1]);
    printf("node indirect pointer: %d\n", node->iptr);
}

inode* 
get_inode(int inum) {
    inode* nodes = get_inode_bitmap() + 32;
    return &(nodes[inum]);
}

int 
alloc_inode() {
    int count;
    int n = 0;
    while (n < 256) {
        void* bm = get_inode_bitmap();
        if (!bitmap_get(bm, n)) {
            bitmap_put(bm, n, 1);
            count = n;
            n++;
            break;
        }
        n++;
    }
    inode* node = get_inode(count);
    node->ptrs[0] = alloc_page();
    node->size = 0;
    node->mode = 0;
    node->refs = 1;
    
    time_t current_time = time(0);
    node->time_access = current_time;
    node->time_modify = current_time;
    node->time_create = current_time;
    return count;
}

void 
free_inode(int inum) {
    printf("+ free_inode(%d)\n", inum);

    inode* node = get_inode(inum);
    void* bm = get_inode_bitmap(); 
    shrink_inode(node, NULL);
    free_page(node->ptrs[0]);
    bitmap_put(bm, inum, 0);
}

int 
shrink_inode(inode* node, int size) {
    int count = node->size / 4096; 
    while(count > size / 4096){
        if (count >= 2) { 
            int* indir = pages_get_page(node->iptr);
            free_page(indir[count - 2]); 
            indir[count - 2] = 0;
            if (count == 2) { 
                free_page(node->iptr); 
                node->iptr = 0;
            }
            count--;
        } else { 
            free_page(node->ptrs[count]); 
            node->ptrs[count] = 0; 
            count--;
        }
    } 
    node->size = size;
    return 0;  
}

int 
grow_inode(inode* node, int size) {
    int count = node->size / 4096 + 1;
    int max = size / 4096;
    while(count <= max) {
        if (count >= 2) { 
            if (node->iptr == 0) { 
                node->iptr = alloc_page();
            }
            int* indirect = pages_get_page(node->iptr); 
            indirect[count - 2] = alloc_page();
            count++;
        } else {
            node->ptrs[count] = alloc_page();
            count++;
        }
    }
    node->size = size;
    return 0;
}


int 
inode_get_pnum(inode* node, int size) {
    int count = size / 4096;
    if (count >= 2) {
       int* indir = pages_get_page(node->iptr);
       return indir[count - 2];
    } else {   
       return node->ptrs[count];
    }
}

void 
reference_reduce(int inum) {
    inode* node = get_inode(inum);
    node->refs = node->refs - 1;
    if (node->refs < 1) {
        free_inode(inum);
    }
}
