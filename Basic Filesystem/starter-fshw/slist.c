// based on cs3650 starter code

#include <string.h>
#include <stdlib.h>
#include <alloca.h>

#include "slist.h"

slist*
s_cons(const char* text, slist* rest)
{
    slist* xs = malloc(sizeof(slist));
    xs->data = strdup(text);
    xs->refs = 1;
    xs->next = rest;
    return xs;
}

void
s_free(slist* xs)
{
    if (xs == 0) {
        return;
    }

    xs->refs -= 1;

    if (xs->refs == 0) {
        s_free(xs->next);
        free(xs->data);
        free(xs);
    }
}

slist*
s_split(const char* text, char delim)
{
    if (*text == 0) {
        return 0;
    }

    int plen = 0;
    while (text[plen] != 0 && text[plen] != delim) {
        plen += 1;
    }

    int skip = 0;
    if (text[plen] == delim) {
        skip = 1;
    }

    slist* rest = s_split(text + plen + skip, delim);
    char*  part = alloca(plen + 2);
    memcpy(part, text, plen);
    part[plen] = 0;

    return s_cons(part, rest);
}

char*
s_reconstruct(slist* list, char delim)
{
    char* fullstr = malloc(2); // min length = null term + delim
    // Garbage in, Garbage out
    if (list == NULL) {
        fullstr[0] = delim;
        fullstr[1] = '\0';
        return fullstr;
    }
    
    slist* tmplist = list;
    int curlen = 0;
    
    while (tmplist != NULL) {
        curlen = strlen(list->data);

        // realloc to make room for the new string
        fullstr = realloc(fullstr, curlen + strlen(fullstr) + 1);

        strncat(fullstr, tmplist->data, curlen);
        strncat(fullstr, &delim, 1);

        tmplist = tmplist->next;
    }

    return fullstr;
}

slist*
s_droplast(slist* xs)
{
    if (xs == NULL || xs->next == NULL) {
        return NULL;
    }

    slist* tmplist = xs;
    slist* newlist = NULL;

    while (tmplist->next != NULL) {
        s_cons(tmplist->data, newlist);
        tmplist = tmplist->next;
    }

    return newlist;
}

char*
s_getlast(slist* xs)
{
    if(xs == NULL) {
        return NULL;
    }

    slist* tmplist = xs;

    while (tmplist->next != NULL) {
        tmplist = tmplist->next;
    }

    return tmplist->data;
}
