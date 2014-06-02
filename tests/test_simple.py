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
        
        self.assertRaises(Triez.Error, tr.__getitem__, 5)
        
        ucs1_string = u"testing"
        ucs2_string = u"testing\N{ARABIC LETTER ALEF}"
        ucs4_string = u"testing\N{GOTHIC LETTER AHSA}"
        
        tr[ucs1_string] = 4 
        tr[ucs2_string] = 5 
        tr[ucs4_string] = 6
        
        self.assertEqual(tr[ucs1_string], 4)
        self.assertEqual(tr[ucs2_string], 5)
        self.assertEqual(tr[ucs4_string], 6)
        del tr[ucs2_string]
        self.assertRaises(KeyError, tr.__getitem__, ucs2_string)
        
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
        
        del tr
        tr = Triez.Trie()
        tr[u"A"] = 1
        tr[u"to"] = 1
        tr[u"tea"] = 1
        tr[u"ted"] = 1
        tr[u"ten"] = 1
        tr[u"i"] = 1
        tr[u"in"] = 1
        tr[u"inn"] = 1        
        self.assertEqual(tr.node_count(), 11)
    
    def test_refcount(self):
        class A:
            _a_destructor_called = False
            def __del__(self):
                A._a_destructor_called = True
                
        tr = Triez.Trie()
        a = A()
        tr[u"mo"] = a
        del a
        self.assertTrue(isinstance(tr[u"mo"], A))
        ae = tr[u"mo"]
        del ae # destructor should be called on this borrowed ref. 
        self.assertTrue(A._a_destructor_called)
    
    """
    # it seems currently dict is %15 faster than our trie. but we are 3x times faster
    # than datrie.
    def test_profile(self):
        import yappi
        
        trie = Triez.Trie()
        trie[u"testing"] = 4 # a ucs1 string
        
        @yappi.profile()
        def _p1():
            for i in range(10000000):
                val = trie[u"testing"]
                
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