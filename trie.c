
#include "trie.h"
#include "string.h"

trie_node_t *_node_create(char key, uintptr_t value)
{
    trie_node_t *nd;

    nd = (trie_node_t *)malloc(sizeof(trie_node_t));
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
    free(nd);
}

trie_t *trie_create(void)
{
    trie_t *t;

    t = (trie_t *)malloc(sizeof(trie_t));
    if (t) {
        t->root = _node_create((char)0, (uintptr_t)0); // root is a dummy node
        t->node_count = 1;
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

    free(t);
}

size_t trie_mem_usage(trie_t *t)
{
    return sizeof(trie_t) + (t->node_count * sizeof(trie_node_t));
}

trie_node_t *trie_prefix(trie_node_t *t, trie_key_t *key)
{
    char ch;
    unsigned int i;    
    trie_node_t *curr, *parent;

    if (!t){
        return NULL;
    }
    
    i = 0;
    parent = t; 
    curr = t->children;
    while(i < key->len)
    {
        ch = key->s[i];
        while(curr && curr->key != key->s[i]) {
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

    r = trie_prefix(t->root, key);
    if (r && !r->value)
    {
        return NULL;
    }

    return r;
}

int trie_add(trie_t *t, trie_key_t *key, uintptr_t value)
{
    unsigned int i;
    char ch;
    trie_node_t *curr, *parent;
    
    i = 0;
    parent = t->root;
    curr = t->root->children;
    while(i < key->len)
    {
        ch = key->s[i];
        while(curr && curr->key != ch) {
            curr = curr->next;
        }
        if (!curr) {            
            curr = _node_create(ch, (uintptr_t)0);
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
    
    parent->value = value;
    return 1;
}

// Algorithm:  remove full-key by trying to remove one character at-a-time.
// Complexity: O(m^2)
int trie_del(trie_t *t, trie_key_t *key)
{
    char ch;
    unsigned int i, klen;
    trie_node_t *curr, *parent, *grand_parent, *next;
        
    klen = key->len;
    while(klen)
    {
        i = 0;
        next = grand_parent = NULL;
        parent = t->root;
        curr = t->root->children;
        while(i < klen)
        {
            ch = key->s[i];
            while(curr && curr->key != key->s[i]) {
                curr = curr->next;
            }
            if (!curr) {
                return 0;
            }

            grand_parent = parent;
            parent = curr;
            curr = curr->children;
            i++;
        }

        // if full key(not a suffix) is not added, then do not 
        // remove anything at all.
        if (!parent->value && i == key->len) {
            return 0;
        }

        // remove key
        parent->value = 0;

        // if no child and no val, then delete!
        if (grand_parent && !parent->children && !parent->value)
        {
            if (grand_parent->children == parent) {
                next = grand_parent->children->next;
                _node_delete(parent);
                grand_parent->children = next;
            } else { // node is in the next list of children
                curr = grand_parent->children;
                while(curr->next)
                {
                    if (curr->next == parent)
                    {
                        curr->next = parent->next;
                        _node_delete(parent);
                        break;
                    }
                    curr = curr->next;
                }
            }

            t->node_count--;
        }
        klen--;
    }

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

    // iterate forward and reverse the linked-list
    found = 1;
    i = 0;
    parent = t->root;
    curr = t->root->children;
    while(i < key->len)
    {
        prev = it = curr;
        while(it && it->key != key->s[i]) {
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

    return found;
}

/*
#define ADD_CANDIDATE(k) { \
    k->next = candidates.next; \
    candidates.next = k; \
}

#define DEL_CANDIDATE(k) { \
    free(k); \
}*/

trie_key_t *DUP_KEY(trie_key_t *src, size_t len, size_t index) 
{ 
    trie_key_t *pdk;

    pdk = (trie_key_t *)malloc(sizeof(trie_key_t));
    pdk->s = (char *)malloc(len);
    pdk->len = len;
    pdk->next = src->next;
    memcpy(pdk->s, src->s, index);

    return pdk;
}

void FREE_KEY(trie_key_t *src) 
{ 
    free(src->s);
    //free(src);
}

void PUT(trie_key_t **q, trie_key_t *e)
{
    e->next = (*q)->next;
    (*q)->next = e;
    *q = e;
}

int PUT_UNIQUE(trie_key_t **q, trie_key_t *e)
{
    trie_key_t *p;

    p = (*q);
    while(p) {
        if (e->len == p->len) {
            if (memcmp(p->s, e->s, p->len) == 0) {
                return 0;
            }
        }
        if (p->next == (*q))
        {
            break;
        }
        p = p->next;
    }   
    PUT(q, e);
    return 1;
}

trie_key_t *GET(trie_key_t **q)
{
    trie_key_t *head,*result;

    head = (*q)->next; result = head->next;
    if (head->next == *q) { // last elem
        head->next = (*q)->next;
        *q = (*q)->next;
    } else {
        head->next = head->next->next;
    }
    return result;
}
               
// Candidates added to a list for each step and called recursively at the end.
// Complexity: O(m^d), m = string length, d = edit distance
void _suggestR1(trie_t *t, trie_key_t *key, size_t ki, size_t cd, trie_key_t **suggestions)
{
    size_t klen;    
    trie_key_t pk,candidates; 
    trie_key_t *kp,*candidate,*pcandidates;
    trie_node_t *prefix,*p;

    klen = key->len;    
    candidates.s = NULL; candidates.len = 0; candidates.next = &candidates;

    pcandidates = &candidates;
    pk.s = key->s; pk.len = ki; pk.next = NULL;
    prefix = trie_prefix(t->root, &pk);
    if ((ki >= klen) || (cd == 0) || (!prefix)) {
        return;
    }
    
    // deletion (prefix + suffix[1:])
    if (klen > 1){
        kp = DUP_KEY(key, klen-1, ki); 

        memcpy(&kp->s[ki], &key->s[ki+1], klen-ki-1);

        PUT(&pcandidates, kp);
    }

    // transposition (prefix + suffix[1] + suffix[0] + suffix[2:])
    if (ki < klen-1) {
        kp = DUP_KEY(key, klen, ki);

        memcpy(&kp->s[ki], &key->s[ki+1], 1);
        memcpy(&kp->s[ki+1], &key->s[ki], 1);
        memcpy(&kp->s[ki+2], &key->s[ki+2], klen-ki-2);

        PUT(&pcandidates, kp);
    }
                             
    // insertion (prefix + x + suffix[:])
    p = prefix->children;
    while(p){
        kp = DUP_KEY(key, klen+1, ki);

        memcpy(&kp->s[ki], &p->key, 1);
        memcpy(&kp->s[ki+1], &key->s[ki], klen-ki);

        PUT(&pcandidates, kp);

        p = p->next;
    }
    
    // alteration (prefix + x + suffix[1:])
    p = prefix->children;
    while(p){
        kp = DUP_KEY(key, klen, ki);

        memcpy(&kp->s[ki], &p->key, 1);
        memcpy(&kp->s[ki+1], &key->s[ki+1], klen-ki-1);

        PUT(&pcandidates, kp);

        p = p->next;
    }

    // check for duplicates and update suggestions
    candidate = GET(&pcandidates);
    while(candidate != pcandidates)
    {
        _suggestR1(t, candidate, 0, cd-1, suggestions);

        // we already searched for the prefix so only search for the suffix[:]
        pk.s = &candidate->s[ki]; pk.len = candidate->len-ki; pk.next = NULL;
        p = trie_prefix(prefix, &pk);
        if (p && p->value) {
            PUT_UNIQUE(suggestions, candidate);
        } else {
            //FREE_KEY(candidate);
        }
        candidate = GET(&pcandidates);
    }

    _suggestR1(t, key, ki+1, cd, suggestions);

}

void suggestR1(trie_t *t, trie_key_t *key, size_t max_distance, trie_key_t **suggestions)
{
    _suggestR1(t, key, 0, max_distance, suggestions);           
}

// Each candidate is called recursively.
// Complexity: O(m^d), m = string length, d = edit distance
void _suggestR2(trie_t *t, trie_key_t *key, size_t ki, size_t cd, trie_key_t **suggestions)
{
    size_t klen;    
    trie_key_t pk; 
    trie_key_t *kp;
    trie_node_t *prefix,*p;

    klen = key->len;
    if (trie_search(t, key)) {
        PUT_UNIQUE(suggestions, key);
    } 

    pk.s = key->s; pk.len = ki; pk.next = NULL;
    prefix = trie_prefix(t->root, &pk);
    if ((ki >= klen) || (cd == 0) || (!prefix)) {
        return;
    }
    
    // deletion (prefix + suffix[1:])
    if (klen > 1){
        kp = DUP_KEY(key, klen-1, ki); 

        memcpy(&kp->s[ki], &key->s[ki+1], klen-ki-1);

        _suggestR2(t, kp, 0, cd-1, suggestions);
    }

    // transposition (prefix + suffix[1] + suffix[0] + suffix[2:])
    if (ki < klen-1) {
        kp = DUP_KEY(key, klen, ki);

        memcpy(&kp->s[ki], &key->s[ki+1], 1);
        memcpy(&kp->s[ki+1], &key->s[ki], 1);
        memcpy(&kp->s[ki+2], &key->s[ki+2], klen-ki-2);

        _suggestR2(t, kp, 0, cd-1, suggestions);
    }

                         
    // insertion (prefix + x + suffix[:])
    p = prefix->children;
    while(p){
        kp = DUP_KEY(key, klen+1, ki);

        memcpy(&kp->s[ki], &p->key, 1);
        memcpy(&kp->s[ki+1], &key->s[ki], klen-ki);

        _suggestR2(t, kp, 0, cd-1, suggestions);

        p = p->next;
    }
    
    // alteration (prefix + x + suffix[1:])
    p = prefix->children;
    while(p){
        kp = DUP_KEY(key, klen, ki);

        memcpy(&kp->s[ki], &p->key, 1);
        memcpy(&kp->s[ki+1], &key->s[ki+1], klen-ki-1);

        _suggestR2(t, kp, 0, cd-1, suggestions);

        p = p->next;
    }

    _suggestR2(t, key, ki+1, cd, suggestions);

}
       
void suggestR2(trie_t *t, trie_key_t *key, size_t max_distance, trie_key_t **suggestions)
{
    _suggestR2(t, key, 0, max_distance, suggestions);           
}

void suggestI(trie_t *t, trie_key_t *key, size_t max_distance, trie_key_t **suggestions)
{
    unsigned int i,ki,cs,klen;
    trie_key_t cds, pk;
    trie_key_t *q,*k,*kp;
    trie_node_t *p, *prefix;

    // init partial/result queue
    cds.s = NULL; cds.len = 0; cds.next = &cds; q = &cds; key->next = key;
    
    PUT(&q, key);
    cs = 1;
    max_distance += 1;
    while(max_distance--)
    {
        i = cs;
        cs = 0;
        while(i--)
        {        
            k = GET(&q); klen = k->len;
            if (trie_search(t, k))
            {
                PUT(suggestions, k);
            }

            for(ki=0;ki<klen;ki++)
            {
                // check prefix
                pk.s = k->s; pk.len = ki; pk.next = NULL;
                prefix = trie_prefix(t->root, &pk);
                if(!prefix) {
                    break; // no need to check remanining
                }

                // deletion (prefix + suffix[1:])
                if (klen > 1){
                    kp = DUP_KEY(k, klen-1, ki); 

                    memcpy(&kp->s[ki], &k->s[ki+1], klen-ki-1);
                                        
                    PUT(&q, kp);
                    cs++;
                }
                
                // transposition (prefix + suffix[1] + suffix[0] + suffix[2:])
                if (ki < klen-1) {
                    kp = DUP_KEY(key, klen, ki);

                    memcpy(&kp->s[ki], &k->s[ki+1], 1);
                    memcpy(&kp->s[ki+1], &k->s[ki], 1);
                    memcpy(&kp->s[ki+2], &k->s[ki+2], klen-ki-2);

                    PUT(&q, kp);
                    cs++;
                }
             
                // insertion (prefix + x + suffix[:])
                p = prefix->children;
                while(p){
                    kp = DUP_KEY(key, klen+1, ki);

                    memcpy(&kp->s[ki], &p->key, 1);
                    memcpy(&kp->s[ki+1], &k->s[ki], klen-ki);
                                        
                    PUT(&q, kp);
                    cs++;

                    p = p->next;
                }
                
                
                // alteration (prefix + x + suffix[1:])
                p = prefix->children;
                while(p){
                    kp = DUP_KEY(key, klen, ki);

                    memcpy(&kp->s[ki], &p->key, 1);
                    memcpy(&kp->s[ki+1], &k->s[ki+1], klen-ki-1);
                                        
                    PUT(&q, kp);
                    cs++;

                    p = p->next;
                }
            }            
        }
    }
}
