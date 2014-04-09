
#ifndef TRIE_H
#define TRIE_H

#include "stdio.h"
#include "stdlib.h"

#ifdef _MSC_VER 
#include "stddef.h" // uintptr_t
#endif

typedef struct trie_key_s {
    char *s;
    size_t len;
    struct trie_key_s *next;
} trie_key_t;

typedef struct trie_node_s {
    char key;
    uintptr_t value;
    struct trie_node_s *next;
    struct trie_node_s *children;
} trie_node_t;

typedef struct trie_s {
    unsigned int node_count;
    struct trie_node_s *root;
} trie_t;

trie_key_t *GET(trie_key_t **q);

trie_t *trie_create(void);
void trie_destroy(trie_t *t);
size_t trie_mem_usage(trie_t *t);

trie_node_t *trie_prefix(trie_node_t *t, trie_key_t *key);
trie_node_t *trie_search(trie_t *t, trie_key_t *key);
int trie_add(trie_t *t, trie_key_t *key, uintptr_t value);
int trie_del(trie_t *t, trie_key_t *key);
int trie_del_fast(trie_t *t, trie_key_t *key);
void suggestR1(trie_t *t, trie_key_t *key, size_t max_distance, trie_key_t **suggestions);
void suggestR2(trie_t *t, trie_key_t *key, size_t max_distance, trie_key_t **suggestions);
void suggestI(trie_t *t, trie_key_t *key, size_t max_distance, trie_key_t **suggestions);

#endif
