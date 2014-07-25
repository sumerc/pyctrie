
#include "trie.h"
#include "string.h"

void KEY_CHAR_WRITE(trie_key_t *k, unsigned long index, TRIE_CHAR in)
{
    assert(k->char_size <= sizeof(TRIE_CHAR));

    *(TRIE_CHAR *)&k->s[index*k->char_size] = in;
}

int KEY_CHAR_READ(trie_key_t *k, unsigned long index, TRIE_CHAR *out)
{
    if (index >= k->size) {
        return 0;
    }

    *out = (TRIE_CHAR)k->s[index*k->char_size];
    return 1;
}

void KEYCPY(trie_key_t *dst, trie_key_t *src, unsigned long dst_index,
        unsigned long src_index, unsigned long length)
{
    unsigned int i,j;
    char *srcb, *dstb;

    if (length == 0) {
        return;
    }

    assert(dst_index+length-1 < dst->size);
    assert(src_index+length-1 < src->size);

    for (i=0;i<length;i++) {
        srcb = &src->s[(src_index+i) * src->char_size];
        dstb = &dst->s[(dst_index+i) * dst->char_size];
        for (j=0;j<src->char_size;j++) {
            assert(dst->char_size >= src->char_size);
            dstb[j] = srcb[j];
        }
    }
}

trie_key_t *KEYCREATE(unsigned long length, unsigned char char_size)
{
    trie_key_t *k;

    k = (trie_key_t *)TRIE_MALLOC(sizeof(trie_key_t));
    if (!k) {
        return NULL;
    }
    k->s = (char *)TRIE_MALLOC(length*char_size);
    if (!k->s) {
        TRIE_FREE(k);
        return NULL;
    }

    k->size = length;
    k->char_size = char_size;
    k->alloc_size = length;

    return k;
}

void KEYFREE(trie_key_t *src)
{
    TRIE_FREE(src->s);
    TRIE_FREE(src);
}

iter_stack_t * STACKCREATE(unsigned long size)
{
    iter_stack_t *r;

    r = (iter_stack_t *)TRIE_MALLOC(sizeof(iter_stack_t));
    if (!r) {
        return NULL;
    }
    r->_elems = (iter_pos_t *)TRIE_MALLOC(size*sizeof(iter_pos_t));
    if (!r->_elems) {
        TRIE_FREE(r);
        return NULL;
    }
    r->index = 0;
    r->size = size;

    return r;
}

void STACKFREE(iter_stack_t *k)
{
    TRIE_FREE(k->_elems);
    TRIE_FREE(k);
}

void PUSHI(iter_stack_t *k, iter_pos_t *e)
{
    assert(k->index < k->size);

    k->_elems[k->index] = *e;
    k->index++;
}

iter_pos_t *POPI(iter_stack_t *k)
{
    if (k->index == 0) {
        return NULL;
    }

    k->index--;
    return &k->_elems[k->index];
}

iter_pos_t *PEEKI(iter_stack_t *k)
{
    if (k->index == 0) {
        return NULL;
    }

    return &k->_elems[k->index-1];
}

trie_node_t *_node_create(TRIE_CHAR key, TRIE_DATA value)
{
    trie_node_t *nd;

    nd = (trie_node_t *)TRIE_MALLOC(sizeof(trie_node_t));
    if (nd) {
        nd->key = key;
        nd->value = value;
        nd->next = NULL;
        nd->children = NULL;
    }
    return nd;
}

void _node_delete(trie_node_t *nd)
{
    TRIE_FREE(nd);
}

trie_t *trie_create(void)
{
    trie_t *t;

    t = (trie_t *)TRIE_MALLOC(sizeof(trie_t));
    if (t) {
        t->root = _node_create((TRIE_CHAR)0, (TRIE_DATA)0); // root is a dummy node
        t->node_count = 1;
        t->item_count = 0;
        t->height = 1;
        t->dirty = 0;
    }
    return t;
}

void trie_destroy(trie_t *t)
{
    trie_node_t *curr, *parent, *next;

    while(t->node_count > 0)
    {
        curr = t->root;
        parent = NULL;
        while(curr->children) {
            parent = curr;
            curr = curr->children;
        }

        if (parent) {
            next = parent->children->next;
            _node_delete(parent->children);
            parent->children = next;
        } else { // root remaining
            _node_delete(t->root);
        }
        t->node_count--;
    }
    TRIE_FREE(t);
}

unsigned long trie_mem_usage(trie_t *t)
{
    return sizeof(trie_t) + (t->node_count * sizeof(trie_node_t));
}

trie_node_t *_trie_prefix(trie_node_t *t, trie_key_t *key)
{
    TRIE_CHAR ch;
    unsigned int i;
    trie_node_t *curr, *parent;

    if (!t){
        return NULL;
    }

    i = 0;
    parent = t;
    curr = t->children;
    while(KEY_CHAR_READ(key, i, &ch))
    {
        while(curr && curr->key != ch) {
            curr = curr->next;
        }
        if (!curr) {
            return NULL;
        }

        parent = curr;
        curr = curr->children;
        i++;
    }

    return parent;
}

trie_node_t *trie_search(trie_t *t, trie_key_t *key)
{
    trie_node_t *r;

    r = _trie_prefix(t->root, key);
    if (r && !r->value)
    {
        return NULL;
    }

    return r;
}

int trie_add(trie_t *t, trie_key_t *key, TRIE_DATA value)
{
    TRIE_CHAR ch;
    unsigned int i;
    trie_node_t *curr, *parent;

    i = 0;
    parent = t->root;
    curr = t->root->children;
    while(KEY_CHAR_READ(key, i, &ch))
    {
        while(curr && curr->key != ch) {
            curr = curr->next;
        }
        if (!curr) {
            curr = _node_create(ch, (TRIE_DATA)0);
            if (!curr){
                return 0;
            }

            curr->next = parent->children;
            parent->children = curr;
            t->node_count++;
        }
        parent = curr;
        curr = curr->children;
        i++;
    }

    if (!parent->value) {
        t->item_count++;
        t->dirty = 1;
    }

    if (key->size > t->height) {
        t->height = key->size;
    }

    parent->value = value;
    return 1;
}

// Algorithm:  First traverse the key and reverse the linked-list of children,
// while reversing, move the branched nodes to the head then traverse back and
// try to delete the node if no children is associated with it. Harder to implement
// however performance is better as we iterate key only once.
// Complexity: O(m)
int trie_del_fast(trie_t *t, trie_key_t *key)
{
    unsigned int i, found;
    trie_node_t *curr, *prev, *it, *parent;
    TRIE_CHAR ch;

    // iterate forward and reverse the linked-list
    found = 1;
    i = 0;
    parent = t->root;
    curr = t->root->children;
    while(KEY_CHAR_READ(key, i, &ch))
    {
        prev = it = curr;
        while(it && it->key != ch) {
            prev = it;
            it=it->next;
        }
        if (!it) {
            found = 0;
            break; // should be rollbacked
        }
        if (it != curr) {
            // curr points to head of next-list here. move the branched item
            // to head(by pointing to curr) for latter back-traversing.
            prev->next = it->next;
            it->next = curr;
        }

        curr = it->children;
        it->children = parent;
        parent = it;
        i++;
    }

    // key is found, but do we really have added it?
    found = found && parent->value;

    // remove key
    if (found) {
        parent->value = 0;
    }

    // traverse backwards
    prev = curr;
    curr = parent;
    while(i--)
    {
        it = curr->children;
        curr->children = prev;

        // no-child and no-val and item is added/found?
        if (found && !curr->children && !curr->value) {
            // we know curr == it->children as we move it to head before.
            prev = curr->next;
            _node_delete(curr);
            t->node_count--;
        } else {
            prev = curr;
        }
        curr = it;
    }
    t->root->children = prev;
    if (found) {
        t->item_count--;
        t->dirty = 1;
    }

    return found;
}

void _suffixes(trie_node_t *p, trie_key_t *key, unsigned long index, 
    trie_enum_cbk_t cbk, void* cbk_arg)
{    
    if (p->value) {
        cbk(key, cbk_arg);
    }
    
    if (index == key->alloc_size) {
        return;
    }

    p = p->children;
    while(p){

        KEY_CHAR_WRITE(key, index, p->key);
        key->size = index+1;
        
        _suffixes(p, key, index+1, cbk, cbk_arg);
      
        p = p->next;
    }
}

void suffixes(trie_t *t, trie_key_t *key, unsigned long max_depth, 
    trie_enum_cbk_t cbk, void* cbk_arg)
{
    trie_key_t *kp;
    trie_node_t *prefix;
    unsigned long index;

    // first search key
    prefix = _trie_prefix(t->root, key);
    if (!prefix) {
        return;
    }

    // alloc a key that can hold size + max_depth chars.
    kp = KEYCREATE((key->size + max_depth), sizeof(TRIE_CHAR));
    if (!kp) {
        return;
    }
    KEYCPY(kp, key, 0, 0, key->size);
    kp->size = key->size;
    
    index = 0;
    if (kp->size > 0) {
        index = kp->size-1;
    }

    _suffixes(prefix, kp, index, cbk, cbk_arg);

    KEYFREE(kp);
    
    return;
}

iter_t * ITERATOR_CREATE(trie_t *t, trie_key_t *key, unsigned long max_depth)
{
    iter_t *r;
    iter_stack_t *k;
    trie_key_t *kp;

    // alloc a key that can hold size + max_depth chars.
    kp = KEYCREATE((key->size + max_depth), sizeof(TRIE_CHAR));
    if (!kp) {
        return NULL;
    }
    KEYCPY(kp, key, 0, 0, key->size);
    kp->size = key->size;

    // allocate stack and push the first iter_pos
    k = STACKCREATE(max_depth);
    if (!k) {
        KEYFREE(kp);
        return NULL;
    }

    // alloc iterator obj
    r = (iter_t *)malloc(sizeof(iter_t));
    if (!r) {
        KEYFREE(kp);
        STACKFREE(k);
        return NULL;
    }
    r->first = 1;
    r->last = 0;
    r->fail = 0;
    r->fail_reason = UNDEFINED;
    r->key = kp;
    r->stack0 = k;
    r->max_depth = max_depth;
    r->trie = t;
    t->dirty = 0; // reset dirty flag just before iteration

    return r;
}

void ITERATOR_FREE(iter_t *iter)
{
    KEYFREE(iter->key);
    STACKFREE(iter->stack0);
    TRIE_FREE(iter);
}

iter_t *itersuffixes_reset(iter_t *iter)
{
    trie_node_t *prefix;
    iter_pos_t ipos;

    // pop all elems first
    while(POPI(iter->stack0))
        ;

    // return key->size to original
    iter->key->size = iter->key->alloc_size-iter->max_depth;

    // get prefix in the trie
    prefix = _trie_prefix(iter->trie->root, iter->key);
    if (!prefix) {
        return NULL;
    }

    // push the first iter_pos
    ipos.iptr = prefix;
    ipos.pos = 0;
    ipos.op.index = iter->key->size-1;
    PUSHI(iter->stack0, &ipos);

    // set flags
    iter->first = 1;
    iter->last = 0;
    iter->fail = 0;
    iter->fail_reason = UNDEFINED;
    iter->trie->dirty = 0;

    return iter;
}

iter_t *itersuffixes_init(trie_t *t, trie_key_t *key, unsigned long max_depth)
{
    iter_t *iter;
    trie_node_t *prefix;

    // first search key
    prefix = _trie_prefix(t->root, key);
    if (!prefix) {
        return NULL;
    }

    // finally create the iterator obj
    iter = ITERATOR_CREATE(t, key, max_depth);
    if (!iter) {
        return NULL;
    }
    itersuffixes_reset(iter);

    return iter;
}

iter_t *itersuffixes_next(iter_t *iter)
{
    iter_pos_t *ip;
    iter_pos_t ipos;
    int found;
    TRIE_DATA val;

    found = 0;
    while(1)
    {
        // found a candidate?
        if (found) {
            break;
        }

        // trie changed during iteration?
        if (iter->trie->dirty) {
            iter->fail = 1;
            iter->fail_reason = CHG_WHILE_ITER;
            break;
        }

        // start processing the stack
        ip = PEEKI(iter->stack0);
        if (!ip) { // no elem in stack0
            iter->last = 1;
            break;
        }

        // check if the search string is already in the trie.
        if (iter->first) {
            iter->first = 0;
            val = ip->iptr->value;
            ip->iptr = ip->iptr->children;
            ip->op.index++;
            if (val) {
                break;
            }
        }

        if (!ip->iptr) {
            POPI(iter->stack0);
            continue;
        }

        KEY_CHAR_WRITE(iter->key, ip->op.index, ip->iptr->key);
        iter->key->size = ip->op.index+1;

        if (ip->pos == 0 && ip->iptr->value) {
            found = 1;
        }

        if (ip->pos == 0) {
            ip->pos = 1;
            if (ip->iptr->children) {
                if (ip->op.index+1 < (iter->key->alloc_size)) {
                    ipos.iptr = ip->iptr->children;
                    ipos.op.index = ip->op.index+1;
                    ipos.pos = 0;
                    PUSHI(iter->stack0, &ipos);
                }
            }
        } else if (ip->pos == 1) {
            POPI(iter->stack0);
            if (ip->iptr->next) {
                ipos.iptr = ip->iptr->next;
                ipos.op.index = ip->op.index;
                ipos.pos = 0;
                PUSHI(iter->stack0, &ipos);
            }
        }
    }

    return iter;
}

void itersuffixes_deinit(iter_t *iter)
{
    assert(iter != NULL);

    ITERATOR_FREE(iter);
}
