import Triez
import unittest

class TestBasic(unittest.TestCase):

    def test_basic(self):
        print(Triez.Trie().node_count())
        tr = Triez.Trie()
        print(tr.mem_usage())
        tr[u"key"] = 55        
        res = u"key" in tr        
        print(res)
        
        res = u"key2" in tr
        print(res)
        
        res = 5 in tr
        print(res)
        
        
        print(len(tr))
        print(tr.mem_usage())
        
        v = tr[5]
        print(v)
        
    """
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
    # it seems currently dict is %5 faster than our trie.
    def test_profile(self):
        import yappi
        
        trie = Triez.Trie()
        trie[u"testing汉"] = 5
        trie[u"testingß"] = 6
        
        @yappi.profile()
        def _p1():
            for i in range(10000000):
                val = trie["testing"]
        
        d = {}
        d["test_key"] = "test_val"
        
        @yappi.profile()
        def _p2():
            for i in range(10000000):
                val = d["test_key"]
                
        import datrie; import string
        trie2 = datrie.Trie(string.ascii_lowercase)
        trie2[u"testing"] =  5
        
        @yappi.profile()
        def _p3():
            for i in range(10000000):
                val = trie2[u"testing"]
                
        _p1()
        _p2()
        _p3()
    