
#ifndef TRIE_H
#define TRIE_H

#include "config.h"
#include "stdio.h"
#include "stdlib.h"

// trie_key_t->char_size <= sizeof(TRIE_CHAR). This is the only requirement. 
// That is obvious because for example we cannot hold a 6-byte char. in a 4-byte
// character trie.

typedef struct trie_key_s {
    char *s; // encoded string buffer
    unsigned long size; // how many characters (or code points) in encoded string
    unsigned char char_size; // character size of the encoding in bytes
    unsigned long alloc_size; // max allocated size of the string buffer. (in characters)
    struct trie_key_s *next;
} trie_key_t;

typedef struct trie_node_s {
    TRIE_CHAR key;
    TRIE_DATA value;
    struct trie_node_s *next;
    struct trie_node_s *children;
} trie_node_t;

typedef struct trie_s {
    unsigned long node_count;
    unsigned long item_count;
    unsigned long height; // max height of the trie (max(len(string)))
    struct trie_node_s *root;
} trie_t;

typedef int (*trie_enum_func_t)(trie_key_t *key, void *arg);

trie_t *trie_create(void);
void trie_destroy(trie_t *t);
unsigned long trie_mem_usage(trie_t *t);
trie_node_t *trie_search(trie_t *t, trie_key_t *key);
int trie_add(trie_t *t, trie_key_t *key, TRIE_DATA value);
int trie_del_fast(trie_t *t, trie_key_t *key);
void trie_keys(trie_t *t, trie_key_t *key, unsigned long max_depth,
    trie_enum_func_t efn, void *arg);
/*
void suggestR2WT(trie_t *t, trie_key_t *key, size_t max_distance, trie_t **suggestions);
void autocompleteR1WT(trie_t *t, trie_key_t *key, size_t max_depth, trie_t **suggestions);*/

#endif
