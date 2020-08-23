// based on cs3650 starter code

#ifndef INODE_H
#define INODE_H

#include "pages.h"
#include <time.h>

typedef struct inode {
    int refs; 
    int mode; 
    int size; 
    int ptrs[2]; 
    int iptr; 
    time_t time_access; 
    time_t time_create; 
    time_t time_modify; 
} inode;



void reference_reduce(int inum);

void print_inode(inode* node);
inode* get_inode(int inum);
int alloc_inode();
void free_inode(int inum);
int grow_inode(inode* node, int size);
int shrink_inode(inode* node, int size);
int inode_get_pnum(inode* node, int fpn);

#endif
