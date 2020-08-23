#include "bitmap.h"
#include <stdio.h>

int bitmap_get(void* bm, int ii) {
    int n = ii % 8;
    return ((char*)bm)[ii / 8] >> n & 1;
}

void bitmap_print(void* bm, int size) {
    int total = size * 4;
    for (int n = 0; n < total; ++n) {
        printf("%d", bitmap_get(bm, n));
    }
}

void bitmap_put(void* bm, int ii, int vv) {
    bitmap_print(bm, ii);
    int bit = (vv) ? 1 : 0;
    char* bitmap = (char*)bm;
    if (bitmap_get(bm, ii) != bit) {
        bitmap[(ii / 8)] ^= 1 << ii % 8; 
    }
    else {
        return;
    }
    bitmap_print(bm, ii);
}

