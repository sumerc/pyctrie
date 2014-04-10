// xtrees-tryouts.c : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "trie.h"
#include <stdio.h>
#include <stdlib.h>
#include "windows.h"

long long now() {
    LARGE_INTEGER s_frequency;
    BOOL s_use_qpc = QueryPerformanceFrequency(&s_frequency);
    if (s_use_qpc) {
        LARGE_INTEGER now;
        QueryPerformanceCounter(&now);
        return (1000LL * now.QuadPart) / s_frequency.QuadPart;
    } else {
        return GetTickCount();
    }
}

trie_t *g_root;

void print_suggestions(trie_key_t *kp)
{
    unsigned int i,ic;
    trie_key_t *kkp;

    ic = 0;
    kkp = GET(&kp);
    while(kkp != kp)
    {
        /*
        for(i=0;i<kkp->len;i++)
        {
            printf("%c", kkp->s[i]);
            i++;
        }
        printf("\r\n");
        */
        ic++;
        kkp = GET(&kp);        
    }
    printf("%d suggestions found.\r\n", ic);
}

int _tmain(int argc, _TCHAR* argv[])
{
    
    

    /*    
    trie_add(g_root, "A", 1, 1);
    trie_add(g_root, "to", 2, 1);
    trie_add(g_root, "tea", 3, 1);
    trie_add(g_root, "ted", 3,1);
    trie_add(g_root, "ten",3, 1);
    rc = trie_add(g_root, "ten", 3,1);
    trie_add(g_root, "i", 1,1);
    trie_add(g_root, "in",2, 1);
    trie_add(g_root, "inn",3, 1);
    trie_add(g_root, "te",2, 1);

    nd = trie_search(g_root, "A", 1);

    printf("node_count:%u\r\n", g_root->node_count);

    
    //rc = trie_remove(g_root, "t", 1);

    trie_remove(g_root, "A", 1);
    trie_remove(g_root, "to", 2);
    trie_remove(g_root, "tea", 3);
    trie_remove(g_root, "ted", 3);
    trie_remove(g_root, "ten", 3);
    trie_remove(g_root, "i", 1);
    trie_remove(g_root, "in", 2);
    trie_remove(g_root, "inn", 3);
    trie_remove2(g_root, "te", 2);
    
    printf("node_count:%u\r\n", g_root->node_count);
    */
    int dummy;
    int rc;
    trie_node_t *nd;
    FILE * fp;
    char line[256];
    long long t0;
    trie_key_t suggestions1,suggestions2,suggestions3;
    trie_key_t k;
    trie_key_t *kp;
    const size_t EDIT_DIST = 2;

    // change encoding to Turkish
    // 1254	windows-1254	ANSI Turkish; Turkish (Windows)
    BOOL rcb = SetConsoleOutputCP(CP_UTF8);
    rcb =SetConsoleCP(CP_UTF8);
    
    g_root = trie_create();
    fp = fopen("out_keys_8859_9", "r");
    if (fp) {
        while (fgets(line, sizeof(line), fp)) {            
            size_t gg = strlen(line);
            line[gg-1] = 0; // strip '/n'
            kp = malloc(sizeof(trie_key_t));
            kp->s = line;
            kp->len = gg-1;
            trie_add(g_root, kp, 1);
        }
    }
    fclose(fp);

    printf("node_count:%u, mem_used:%u\r\n", g_root->node_count, trie_mem_usage(g_root));

    //nd = trie_search(g_root->root, "abd-i mahsûs", 12);
    
    k.s = (char *)malloc(5);
    memcpy(k.s, "zebek", 5);
    k.len = 5;
    k.next = NULL;

    t0 = now();
    kp = &suggestions1; kp->s = NULL; kp->len=0;kp->next = kp; // make circular
    suggestR1(g_root, &k, EDIT_DIST, &kp); 
    printf("elapsed suggestR1:%lld msec.\r\n", now()-t0);

    print_suggestions(kp);
    
    kp = &suggestions2; kp->s = NULL; kp->len=0;kp->next = kp; // make circular
    t0 = now();
    suggestR2(g_root, &k, EDIT_DIST, &kp); 
    printf("elapsed suggestR2:%lld msec.\r\n", now()-t0);

    print_suggestions(kp);
    
    kp = &suggestions3; kp->s = NULL; kp->len=0;kp->next = kp; // make circular
    t0 = now();
    suggestI(g_root, &k, EDIT_DIST, &kp);
    printf("elapsed suggestI:%lld msec.\r\n", now()-t0);

    print_suggestions(kp);

    /*
    k.s = "s"; k.len = 1; k.next = NULL;
    nd = trie_prefix(g_root->root, &k);

    k.s = "e"; k.len = 1; k.next = NULL;
    nd = trie_prefix(nd, &k);

    k.s = "b"; k.len = 1; k.next = NULL;
    nd = trie_prefix(nd, &k);

    k.s = "k"; k.len = 1; k.next = NULL;
    nd = trie_prefix(nd, &k);*/

    fp = fopen("out_keys_8859_9", "r");
    if (fp) {
        t0 = now();
        while (fgets(line, sizeof(line), fp)) {            
            size_t gg = strlen(line);
            kp->s = line;
            kp->len = gg-1;
            trie_del_fast(g_root, kp);
        }
        printf("elapsed:%lld msec.\r\n", now()-t0);
    }
    fclose(fp);


    
    
    trie_destroy(g_root);
    
    scanf("%d", &dummy);
	return 0;
}

