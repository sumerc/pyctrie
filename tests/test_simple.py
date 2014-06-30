import sys
import triez
import _triez
import unittest

_SAVE_PROFILE_RESULTS = True

class TestBasic(unittest.TestCase):
    
    def test_temp(self):
        tr = triez.Trie()
        
        tr[u""] = 2
        tr[u"A"] = 1
        tr[u"to"] = 1
        tr[u"tea"] = 1
        tr[u"ted"] = 1
        tr[u"ten"] = 1
        tr[u"i"] = 1
        tr[u"in"] = 1
        tr[u"inn"] = 1
        
        keys = tr.keys(u"")
        print(list(keys))
        print(list(keys))
        for x in keys:
            if x == u"in":
                break
        print(list(keys))
        
        for x in tr:
            print(x)
        
    def test_basic(self):
    
        self.assertEqual(triez.Trie().node_count(), 1)

        tr = triez.Trie()
        del tr
        tr = triez.Trie()
        tr[u"key"] = 55
        
        self.assertTrue(u"key" in tr)
        
        self.assertFalse(u"ke" in tr)
        self.assertFalse(5 in tr)
        self.assertEqual(len(tr), 1)
        
        self.assertRaises(_triez.Error, tr.__getitem__, 5)
        
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
        except _triez.Error:
            pass
        
        del tr
        tr = triez.Trie()
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
                
        tr = triez.Trie()
        #tr = {}
        a = A()
        tr[u"mo"] = a
        self.assertEqual(sys.getrefcount(tr[u"mo"]), 3)
        del a
        self.assertEqual(sys.getrefcount(tr[u"mo"]), 2)
        self.assertTrue(isinstance(tr[u"mo"], A))
        ae = tr[u"mo"]
        del ae
        self.assertEqual(sys.getrefcount(tr[u"mo"]), 2)
        del tr[u"mo"]
        self.assertTrue(A._a_destructor_called)
        
    """
        import datrie; import string
        trie2 = datrie.Trie(string.ascii_lowercase)
        trie2[u"testing"] =  5
        
        @yappi.profile()
        def _p3():
            for i in range(OP_COUNT):
                val = trie2[u"testing"]
    """
    """
    def test_profile(self):
        import yappi
        import datetime
        OP_COUNT = 1000000
        
        def _get_stat(name):
            stats = yappi.get_func_stats()
            for stat in stats:
                if stat.name == name:
                    return stat

        def _diff_stats(p1, p2):
            ps1 = _get_stat(p1)
            ps2 = _get_stat(p2)
            
            if ps1.ttot > ps2.ttot:
                faster = p2; slower = p1; factor = ps1.ttot/ps2.ttot
            else:
                faster = p1; slower = p2; factor = ps2.ttot/ps1.ttot
                
            return "%s is %0.3f times faster than %s" % (faster, factor, slower)
                
        yappi.start()
        stats = []
        
        # profile search
        trie = _triez.Trie()
        trie[u"testing"] = 4 # a ucs1 string
        def _triez_search():
            for i in range(OP_COUNT):
                val = trie[u"testing"]
                
        d = {}
        d["test_key"] = "test_val"
        def _dict_search():
            for i in range(OP_COUNT):
                val = d["test_key"]
                
        _triez_search()
        _dict_search()
        stats.append(_diff_stats("_triez_search", "_dict_search"))
                
        # profile add
        trie = _triez.Trie()
        def _triez_add():
            for i in range(OP_COUNT):
                trie[u"testing"] = "test_val"
        d = {}
        def _dict_add():
            for i in range(OP_COUNT):
                d["testing"] = "test_val"
                
        _triez_add()
        _dict_add()
        stats.append(_diff_stats("_triez_add", "_dict_add"))
        
        # profile delete
        trie = _triez.Trie()
        def _triez_del():
            for i in range(OP_COUNT):
                trie[u"testing"] = "test_val"
                del trie[u"testing"]
        d = {}
        d["testing"] = "test_val"
        def _dict_del():
            for i in range(OP_COUNT):
                d[u"testing"] = "test_val"
                del d["testing"]
                
        _triez_del()
        _dict_del()
        stats.append(_diff_stats("_triez_del", "_dict_del"))

        yappi.stop()
        
        if _SAVE_PROFILE_RESULTS:
            from datetime import datetime
            d = datetime.now()
            fname = "profile_results/Triez_Python%s%s_%s.profile" % (sys.version_info[0], 
                sys.version_info[1], d.strftime("%y%d%m"))
            with open(fname, "w") as f:
                for stat in stats:
                    f.write(stat)
                    f.write('\n')
    """