import Triez
import unittest

class TestBasic(unittest.TestCase):

    def test_basic(self):
    
        self.assertEqual(Triez.Trie().node_count(), 1)

        tr = Triez.Trie()
        del tr
        tr = Triez.Trie()
        tr[u"key"] = 55
        
        self.assertTrue(u"key" in tr)
        self.assertFalse(u"ke" in tr)
        self.assertFalse(5 in tr)
        self.assertEqual(len(tr), 1)
        self.assertRaises(KeyError, tr.__getitem__, 5)
        
        tr[u"testing"] = 4 # a ucs1 string
        tr[u"testing汉"] = 5 # a ucs2 string
        tr[u"testing\N{GOTHIC LETTER AHSA}"] = 6 # a UCS4 string
        
        self.assertEqual(tr[u"testing"], 4)
        self.assertEqual(tr[u"testing汉"], 5)
        self.assertEqual(tr[u"testing\N{GOTHIC LETTER AHSA}"], 6)
        del tr[u"testing汉"]
        self.assertRaises(KeyError, tr.__getitem__, u"testing汉")
        try:
            del tr[u"tes"]
            raise Exception("KeyError should be raised here.")
        except KeyError:
            pass
        try:
            tr[5] = 54
            raise Exception("Triez.Error should be raised here.")
        except Triez.Error:
            pass
            
        
        # TODO: use the example from Wikipedia
        
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
        
        
    
    # it seems currently dict is %5 faster than our trie.
    def test_profile(self):
        import yappi
        
        trie = Triez.Trie()
        trie[u"testing"] = 4 # a ucs1 string
        
        @yappi.profile()
        def _p1():
            for i in range(10000000):
                val = trie[u"testing汉"]
                
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
    """