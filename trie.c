
#include "trie.h"
#include "string.h"

void KEY_CHAR_WRITE(trie_key_t *k, unsigned long index, TRIE_CHAR in)
{
    // assuming k->char_size >= sizeof(TRIE_CHAR)
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
    r->index = size-1;
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
    assert(k->index >= 0);
    
    k->_elems[k->index] = *e;
    k->index--;
}

iter_pos_t *POPI(iter_stack_t *k)
{
    if (k->index == k->size-1){
        return NULL;
    }

    k->index++;
    return &k->_elems[k->index];
}

iter_pos_t *PEEKI(iter_stack_t *k)
{
    if (k->index == k->size-1){
        return NULL;
    }

    return &k->_elems[k->index+1];
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
// however complexity is better.
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
    }

    return found;
}

iter_t * ITERATOR_CREATE(trie_t *t, trie_key_t *key, unsigned long max_depth)
{
    // TODO: wE DO NOT NEED THE reset LOGIC here, as we already do it in RESET.
    
    iter_t *r;
    iter_stack_t *k;
    trie_key_t *kp;
    iter_pos_t ipos;
    trie_node_t *prefix;

    // first: search key
    prefix = _trie_prefix(t->root, key);
    if (!prefix) {
        return NULL;
    }

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
    ipos.iptr = prefix; 
    ipos.pos = 0; 
    ipos.op.type = AUTOCOMPLETE; 
    ipos.op.index = kp->size-1;
    PUSHI(k, &ipos);

    // alloc iterator obj
    r = (iter_t *)malloc(sizeof(iter_t));
    if (!r) {
        KEYFREE(kp);
        STACKFREE(k);
        return NULL;
    }
    r->first = 1;
    r->last = 0;
    r->key = kp;
    r->first_key_size = kp->size;
    r->stack = k;
    r->max_depth = max_depth;
    r->trie = t;

    return r;
}

void ITERATOR_FREE(iter_t *iter)
{
    KEYFREE(iter->key);
    STACKFREE(iter->stack);
    TRIE_FREE(iter);
}

iter_t *iterkeys_reset(iter_t *iter)
{
    trie_node_t *prefix;
    iter_pos_t ipos;
        
    // pop all elems first
    while(POPI(iter->stack))
        ;
    
    // return key->size to original
    iter->key->size = iter->first_key_size;
    
    // get prefix in the trie
    prefix = _trie_prefix(iter->trie->root, iter->key);
    if (!prefix) {
        return NULL;
    }
    
    // push the first iter_pos
    ipos.iptr = prefix; 
    ipos.pos = 0; 
    ipos.op.type = AUTOCOMPLETE; 
    ipos.op.index = iter->key->size-1;
    PUSHI(iter->stack, &ipos);
    
    // set flags
    iter->first = 1;
    iter->last = 0;
}

iter_t *iterkeys_init(trie_t *t, trie_key_t *key, unsigned long max_depth)
{
    iter_t *iter;
        
    // finally create the iterator obj
    iter = ITERATOR_CREATE(t, key, max_depth);
    if (!iter) {
        return NULL;
    }
    
    return iter;
}

iter_t *iterkeys_next(iter_t *iter)
{
    iter_pos_t *ip;
    iter_pos_t ipos;
    int found;
    TRIE_DATA val;
    
    found = 0;
    while(1)
    {
        ip = PEEKI(iter->stack);
        if (!ip) { // no elem in stack
            iter->last = 1;
            return iter;
        }

        // check if the search string is already in the trie.
        if (iter->first) {
            iter->first = 0;
            val = ip->iptr->value;
            ip->iptr = ip->iptr->children; 
            ip->op.index++;
            if (val) {
                return iter;
            }
            
        }

        KEY_CHAR_WRITE(iter->key, ip->op.index, ip->iptr->key);
        iter->key->size = ip->op.index+1;

        if (ip->pos == 0 && ip->iptr->value) {
            found = 1;
        }

        if (ip->pos == 0) {
            ip->pos = 1;
            if (ip->iptr->children) {
                if (ip->op.index+1 < (iter->first_key_size + iter->max_depth)) {
                    ipos.iptr = ip->iptr->children;
                    ipos.op.type = AUTOCOMPLETE; 
                    ipos.op.index = ip->op.index+1;
                    ipos.pos = 0;
                    PUSHI(iter->stack, &ipos);
                }
            }
        } else if (ip->pos == 1) {
            POPI(iter->stack);
            if (ip->iptr->next) {
                ipos.iptr = ip->iptr->next;
                ipos.op.type = AUTOCOMPLETE; 
                ipos.op.index = ip->op.index;
                ipos.pos = 0;
                PUSHI(iter->stack, &ipos);
            }
        }

        if (found) {
            return iter;
        }
    }

    return iter;
}

void iterkeys_deinit(iter_t *iter)
{
    ITERATOR_FREE(iter);
}

// TODO: Do not call memcpy on one-char copy's for some of the edit distance 
// calculations.


/*
void _trie_keys(trie_node_t *v, trie_key_t *key, unsigned long depth, 
    trie_enum_func_t efn, void *arg, int *stop)
{
    int rc;
    trie_node_t *p;
    
    if(*stop)
        return;

    if (v->value) {
        rc = efn(key, arg);
        if(!rc) {
            *stop = 1;
            return;
        }
    }

    if (depth == 0) {
        return; // do not go further
    }
    
    p = v->children;
    while(p){
        key->size = key->alloc_size - depth + 1;
        KEY_CHAR_WRITE(key, key->size-1, p->key);
        //printf("%d\r\n", key->s[key->size-1]);
        
        _trie_keys(p, key, depth-1, efn, arg, stop);
        
        p = p->next;
    }
}

// if key is empty (key->size==0) then start enum from root.
void trie_keys(trie_t *t, trie_key_t *key, unsigned long max_depth,
    trie_enum_func_t efn, void *arg)
{
    trie_key_t *kp;
    trie_node_t *prefix;
    unsigned long max_length;
    int stop;
    
    prefix = t->root;
    if (key->size > 0) {
        prefix = _trie_prefix(t->root, key);
        if (!prefix) {
            return;
        }
    }
    
    // this key buffer should be able to hold any char in the trie, so char_size
    // should be TRIE_CHAR.
    max_length = key->size + max_depth;
    kp = KEYCREATE(max_length, sizeof(TRIE_CHAR)); 
    KEYCPY(kp, key, 0, 0, key->size);
    kp->size = key->size;
    stop = 0;
    
    _trie_keys(prefix, kp, max_depth, efn, arg, &stop);
    
    KEYFREE(kp);
}

// Complexity: O(m^d), m = string length, d = edit distance
void _suggestR2WT(trie_t *t, trie_key_t *key, unsigned long ki, unsigned long cd, trie_t *suggestions)
{
    unsigned long klen;
    trie_key_t pk;
    trie_node_t *prefix,*p;
    TRIE_CHAR ch;
    
    // TODO: Maybe we can avoid prefix match at everycall. We can use memoization.
    // search prefix 
    prefix = t->root;
    if (ki > 0) {
        pk.s = key->s; pk.len = ki; pk.next = NULL;
        prefix = _trie_prefix(t->root, &pk);
        if (!prefix) {
            return;
        }
    }
    
    // search suffix (which will complete the search for the full key)
    klen = key->len;
    pk.s = &key->s[ki]; pk.len = klen-ki; pk.next = NULL;
    p = trie_prefix(prefix, &pk);
    if (p && p->value) {
        trie_add(suggestions, key, 1);
    }
    
    // check bounds/depth
    if ((ki >= klen) || (cd == 0)) {
        return;
    }

    // deletion (prefix + suffix[1:])
    if (klen > 1){

        key->len -= 1;
        ch = key->s[ki];
        memcpy(&key->s[ki], &key->s[ki+1], key->len-ki);

        _suggestR2WT(t, key, 0, cd-1, suggestions);
        
        // rollback
        key->len += 1;
        memcpy(&key->s[ki+1], &key->s[ki], key->len-ki);
        key->s[ki] = ch;
    }

    // transposition (prefix + suffix[1] + suffix[0] + suffix[2:])
    if (ki < klen-1) {

        ch = key->s[ki];
        key->s[ki] = key->s[ki+1];
        key->s[ki+1] = ch;

        _suggestR2WT(t, key, 0, cd-1, suggestions);
        
        // rollback
        ch = key->s[ki+1];
        key->s[ki+1] = key->s[ki];
        key->s[ki] = ch;
    }

                         
    // insertion (prefix + x + suffix[:])
    p = prefix->children;
    while(p){
        key->len += 1;
        memcpy(&key->s[ki+1], &key->s[ki], key->len-ki);
        key->s[ki] = p->key;

        _suggestR2WT(t, key, 0, cd-1, suggestions);
        
        // rollback
        key->len -= 1;
        memcpy(&key->s[ki], &key->s[ki+1], key->len-ki);
        
        p = p->next;
    }
    
    // alteration (prefix + x + suffix[1:])
    p = prefix->children;
    while(p){
        ch = key->s[ki];
        key->s[ki] = p->key;
        
        _suggestR2WT(t, key, 0, cd-1, suggestions);
        
        // rollback
        key->s[ki] = ch;

        p = p->next;
    }

    _suggestR2WT(t, key, ki+1, cd, suggestions);
}
       
void suggestR2WT(trie_t *t, trie_key_t *key, unsigned long max_distance, trie_t **suggestions)
{
    trie_key_t *kp;

    *suggestions = trie_create();
    if (!*suggestions) {
        return;
    }

    kp = DUP_KEY(key, key->len + max_distance, key->len);
    kp->len = key->len;
    
    _suggestR2WT(t, kp, 0, max_distance, *suggestions);
    
    FREE_KEY(kp);
}
*/