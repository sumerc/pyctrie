
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

typedef enum iter_op_type_e {
    UNDEFINED = 0,
    DELETE,
    TRANSPOSE,
    INSERT,
    CHANGE,
    INDEXCHG,
    AUTOCOMPLETE,
} iter_op_type_t;

// iterator related structs
typedef struct iter_op_s {
    iter_op_type_t type;
    char dch; // the deleted char in delete/transpose ops
    unsigned long index;
    unsigned long depth;
} iter_op_t;

typedef struct iter_pos_s {
    iter_op_t op;
    unsigned long pos; // used for simulating multiple recursive calls in the same body.
    trie_node_t *iptr; // used for holding the current processing node.
} iter_pos_t;

// fast, pre-allocated iter_pos_t stack
typedef struct iter_stack_s {
    iter_pos_t *_elems;
    unsigned long index;
    unsigned long size;
}iter_stack_t;

typedef struct iter_s {
    int first;
    int last;
    trie_key_t *key;
    unsigned long first_key_size;
    iter_stack_t *stack;
    unsigned long max_depth;
    trie_t *trie;
} iter_t;

trie_t *trie_create(void);
void trie_destroy(trie_t *t);
unsigned long trie_mem_usage(trie_t *t);
trie_node_t *trie_search(trie_t *t, trie_key_t *key);
int trie_add(trie_t *t, trie_key_t *key, TRIE_DATA value);
int trie_del_fast(trie_t *t, trie_key_t *key);

// iterkeys
iter_t *iterkeys_init(trie_t *t, trie_key_t *key, unsigned long max_depth);
iter_t *iterkeys_next(iter_t *iter);
void iterkeys_deinit(iter_t *iter);
iter_t *iterkeys_reset(iter_t *iter);

#endif
