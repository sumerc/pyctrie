import triez
import unittest

class TestBasic(unittest.TestCase):
    def test_basic(self):
        
        trie = triez.Trie()
        print(trie.node_count())
        trie.add("SUPO", 14)
        print(trie.node_count())
        
        res = trie.search("SUPO")
        print(res)
        """
        print(trie_node_count(_trie_capsule))
        res = trie_delete(_trie_capsule, unicode("SUPO"))
        print(res)
        res = trie_search(_trie_capsule, unicode("SUPO"))
        print(res)
        print(trie_mem_usage(_trie_capsule))
        print(trie_node_count(_trie_capsule))
        trie_destroy(_trie_capsule)
        print(trie_node_count(_trie_capsule))
        """