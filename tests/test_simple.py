import unittest

class TestBasic(unittest.TestCase):
    def test_basic(self):
        import _treez
        _trie_capsule = _treez.trie_create()
        
        res = _treez.trie_add(_trie_capsule, unicode("SUPO"), unicode(14))
        res = _treez.trie_search(_trie_capsule, unicode("SUPO"))
        print(res)
        print(_treez.trie_node_count(_trie_capsule))
        res = _treez.trie_delete(_trie_capsule, "SUPO")
        print(res)
        res = _treez.trie_search(_trie_capsule, "SUPO")
        print(res)
        print(_treez.trie_mem_usage(_trie_capsule))
        print(_treez.trie_node_count(_trie_capsule))
        _treez.trie_destroy(_trie_capsule)
        print(_treez.trie_node_count(_trie_capsule))