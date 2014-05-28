import triez
import _triez
import unittest

class TestBasic(unittest.TestCase):
    
    def test_invalid_input(self):
        class A: pass
        trie = triez.Trie()
        try:
            trie.add(5, 4)
        except _triez.Error:
            pass        
        try:
            trie.search(5)
        except _triez.Error:
            pass
        try:
            trie.delete(A())
        except _triez.Error:
            pass
        res = trie.search(u"d")
        self.assertEqual(res, None)
        
    def test_basic(self):
        trie = triez.Trie()
        self.assertEqual(trie.node_count(), 1)
        trie.add(u"tst", 14)
        trie.add(u"tst", 15)
        self.assertEqual(trie.node_count(), 4)
        res = trie.search(u"tst")
        self.assertEqual(res, 15)
        res = trie.search(u"tst")
        self.assertEqual(res, 15)
        self.assertTrue(trie.delete(u"tst"))
        self.assertFalse(trie.delete(u"tst"))
        
        # TODO: use the example from Wikipedia
        
        
        
    """
    # it seems currently dict is 30x times faster than our trie.
    def test_profile(self):
        import yappi
        
        trie = triez.Trie()
        trie.add("test_key", "test_val")
        
        @yappi.profile()
        def _p1():
            ts = trie.search
            for i in range(1000000):
                val = ts("test_key")

        d = {}
        d["test_key"] = "test_val"
        
        @yappi.profile()
        def _p2():
            for i in range(1000000):
                val = d["test_key"]
                
        _p1()
        _p2()
    """