import sys
import triez
import _triez
import unittest
import multiprocessing # added to fix http://bugs.python.org/issue15881 for Py2.6

_SAVE_PROFILE_RESULTS = True

#import faulthandler; faulthandler.enable()

_is_py3k = (sys.version_info >= (3, 0))
if _is_py3k:
    _range = range
else:
    _range = xrange

def damerau_levenshtein(a, b):
    if not a: return len(b)
    if not b: return len(a)
    return min(damerau_levenshtein(a[1:], b[1:])+(a[0] != b[0]), 
        damerau_levenshtein(a[1:], b)+1, damerau_levenshtein(a, b[1:])+1)

def damerau_levenshtein_fast(a, b):
    """
    From: http://mwh.geek.nz/2009/04/26/python-damerau-levenshtein-distance/

    This implementation is O(N*M) time and O(M) space, for N and M the
    lengths of the two sequences.
    """
    oneago = None
    thisrow = list(range(1, len(b) + 1)) + [0]
    for x in _range(len(a)):
        twoago, oneago, thisrow = oneago, thisrow, [0] * len(b) + [x + 1]
        for y in _range(len(b)):
            delcost = oneago[y] + 1
            addcost = thisrow[y - 1] + 1
            subcost = oneago[y - 1] + (a[x] != b[y])
            thisrow[y] = min(delcost, addcost, subcost)
            # This block deals with transpositions
            if (x > 0 and y > 0 and a[x] == b[y - 1]
                and a[x-1] == b[y] and a[x] != b[y]):
                thisrow[y] = min(thisrow[y], twoago[y - 2] + 1)
    return thisrow[len(b) - 1]

def _print_keys_as_hex(keys):
    for k in keys:
        HEX_COLUMN_SIZE = 14
        for ch in k:
            fs = "0x%xh" % (ord(ch), )
            sys.stdout.write(fs)
            sys.stdout.write(" " * (HEX_COLUMN_SIZE-len(fs)))
        sys.stdout.write("\n")

class TestBasic(unittest.TestCase):

    # create a trie just like in http://en.wikipedia.org/wiki/Trie
    def _create_trie(self):
        tr = triez.Trie()
        tr[u"A"] = 1
        tr[u"to"] = 1
        tr[u"tea"] = 1
        tr[u"ted"] = 1
        tr[u"ten"] = 1
        tr[u"i"] = 1
        tr[u"in"] = 1
        tr[u"inn"] = 1

        return tr

    def _create_trie2(self):
        """
        A complex trie trie including different char sizes together.
        Note that Python2.x uses UTF16 internally which U+10001 starts mapping
        chars to 2 bytes.
        """
        tr = triez.Trie()
        # utf16,utf32: 0x0627
        tr[u"\N{ARABIC LETTER ALEF}"] = 1 
        tr[u"\N{ARABIC LETTER ALEF}\N{ARABIC LETTER ALEF}"] = 1

        # utf16: 0xD800 0xDF30, utf32: 0x00010330
        tr[u"\N{ARABIC LETTER ALEF}\N{GOTHIC LETTER AHSA}"] = 1 
        tr[u"\N{ARABIC LETTER ALEF}\N{GOTHIC LETTER AHSA}A"] = 1

        # utf16: 0xD800 0xDC01, utf32: 0x00010001
        tr[u"\N{ARABIC LETTER ALEF}\N{LINEAR B SYLLABLE B038 E}"] = 1
        tr[u"\N{ARABIC LETTER ALEF}ABC\N{GOTHIC LETTER AHSA}"] = 1

        return tr
    """
    def test_temp(self):
        tr = triez.Trie()
        
        #tr[u""] = 2
        tr[u"A"] = 1
        tr[u"to"] = 1
        tr[u"tea"] = 1
        tr[u"ted"] = 1
        tr[u"ten"] = 1
        tr[u"i"] = 1
        tr[u"in"] = 1
        tr[u"inn"] = 1
        #tr[u"\N{LINEAR B SYLLABLE B008 A}"] = 1
        #tr[u"\N{ARABIC LETTER ALEF}"] = 1
        #tr[u"\N{LINEAR B SYLLABLE B038 E}"] = 1 # UCS4 in PEP393
        #tr[u"\N{GOTHIC LETTER AHSA}"] = 1 # UCS4 in PEP393
        
        suffixes = tr.suffixes(u"")
        print(list(suffixes))
        print(list(suffixes))
        for x in suffixes:
            if x == u"in":
                break
        print(list(suffixes))
        for x in tr:
            print(x)
    """
    
    def test_corrections(self):
        MAX_EDIT_DISTANCE = 4

        tr = self._create_trie()
        corrections = tr.corrections()
        print(corrections)
        self.assertEqual(len(corrections), len(tr))

        #tr[u"\N{LINEAR B SYLLABLE B038 E}"] = 1
        corrections = tr.corrections(u"i", 2)
        self.assertEqual(corrections, set([u'i', u'to', u'inn', 
            u'A', u'in']))

        corrections = tr.corrections(u"i", 1)
        self.assertEqual(corrections, set([u'i', u'A', u'in']))
        #for x in corrections:
        #    print("%s:%d" % (x, damerau_levenshtein(u"i", x)))
        #print(corrections)

        # for all trie's elements check correction(x, depth) is generating correct
        # DL distance. depth should be 0 < x < 4.
        for x in tr.suffixes():
            for i in range(1, 4):
                crs = tr.corrections(x, i)
                for e in crs:
                    self.assertTrue(damerau_levenshtein(x, e) <= i)


    def test_corrections_unicode(self):
        tr = self._create_trie2()
        corrections = tr.corrections(u"\N{ARABIC LETTER ALEF}", 2)
        #_print_keys_as_hex(corrections)
        
    def test_prefixes(self):
        tr = self._create_trie()

        self.assertEqual(len(tr.prefixes(u"inn", 1)), 1)
        self.assertEqual(len(tr.prefixes(u"inn")), 3)
        self.assertTrue(u"i" in tr.prefixes(u"inn"))
        self.assertTrue(u"in" in tr.prefixes(u"inn"))
        self.assertTrue(u"inn" in tr.prefixes(u"inn"))
        prefixes = tr.iter_prefixes()
        self.assertEqual(len(list(prefixes)), 0)
        
        iprefixes = list(tr.iter_prefixes(u"inn"))
        prefixes = tr.prefixes(u"inn")
        self.assertTrue(prefixes, set(iprefixes))
  
        iprefixes = tr.iter_prefixes(u"inn")
        del tr[u"in"]
        self.assertRaises(RuntimeError, list, iprefixes)

        self.assertEqual(len(tr.prefixes(u"inn")), 
            len(list(tr.iter_prefixes(u"inn"))), 3)

    def test_suffixes(self):
        # del suffixes after referencing
        tr = self._create_trie()
        suffixes = tr.iter_suffixes(u"in")
        del tr[u"in"]
        del tr[u"inn"]
        self.assertRaises(RuntimeError, list, suffixes)
        self.assertRaises(RuntimeError, list, suffixes)

        tr = self._create_trie()
        suffixes = tr.iter_suffixes(u"i")
        del tr[u"in"]
        self.assertRaises(RuntimeError, list, suffixes)

        # trie self_iter and suffixes should be same
        suffixes = tr.iter_suffixes()
        self.assertEqual(len(list(tr)), len(list(suffixes)))

        # break iteration in the middle and test if it resets again
        for x in suffixes:
            if x == u"in":
                break
        self.assertEqual(len(list(tr)), len(list(suffixes)))
        
        # 0 len iteration
        tr = triez.Trie()
        for x in tr:
            pass
        
        # non-existent suffix iter
        tr = self._create_trie()
        self.assertEqual(len(list(tr.iter_suffixes(u"INVALID"))), 0)
        self.assertEqual(len(tr.suffixes()), len(list(tr.iter_suffixes())))
        self.assertEqual(len(tr.suffixes()), len(list(tr.iter_suffixes())), 
            len(tr))
      
    def test_suffixes_unicode(self):
        tr = self._create_trie2()
        suffixes = tr.suffixes(u"\N{ARABIC LETTER ALEF}")
        self.assertEqual(len(suffixes), 6)
        suffixes = tr.suffixes(u"\N{ARABIC LETTER ALEF}\N{GOTHIC LETTER AHSA}")
        self.assertTrue(set([u"\N{ARABIC LETTER ALEF}\N{GOTHIC LETTER AHSA}A", 
            u"\N{ARABIC LETTER ALEF}\N{GOTHIC LETTER AHSA}"]) == suffixes)

        suffixes = tr.suffixes(u"\N{ARABIC LETTER ALEF}", 3)
        #_print_keys_as_hex(suffixes)
        self.assertEqual(len(suffixes), 5)

    def test_prefixes_unicode(self):
        tr = self._create_trie2()
        prefixes = tr.prefixes(u"\N{ARABIC LETTER ALEF}\N{GOTHIC LETTER AHSA}A")
        self.assertEqual(len(prefixes), 3)
        self.assertTrue(set([u"\N{ARABIC LETTER ALEF}\N{GOTHIC LETTER AHSA}A", 
            u"\N{ARABIC LETTER ALEF}\N{GOTHIC LETTER AHSA}", 
            u"\N{ARABIC LETTER ALEF}"]) == prefixes)
        prefixes = tr.prefixes(u"\N{ARABIC LETTER ALEF}\N{ARABIC LETTER ALEF}")
        self.assertEqual(len(prefixes), 2)
        self.assertTrue(set([u"\N{ARABIC LETTER ALEF}\N{ARABIC LETTER ALEF}", 
            u"\N{ARABIC LETTER ALEF}"]) == prefixes)

    
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
        tr = self._create_trie()
        self.assertEqual(tr.node_count(), 11)
        
    def test_refcount(self):
        _GRC = sys.getrefcount
        class A:
            _a_destructor_called = False
            def __del__(self):
                A._a_destructor_called = True
                
        tr = triez.Trie()
        #tr = {}
        a = A()
        tr[u"mo"] = a
        self.assertEqual(_GRC(tr[u"mo"]), 3)
        del a
        self.assertEqual(_GRC(tr[u"mo"]), 2)
        self.assertTrue(isinstance(tr[u"mo"], A))
        ae = tr[u"mo"]
        del ae
        self.assertEqual(_GRC(tr[u"mo"]), 2)
        del tr[u"mo"]
        self.assertTrue(A._a_destructor_called)
        
        self.assertEqual(_GRC(tr), 2)
        suffixes = tr.iter_suffixes()
        self.assertEqual(_GRC(tr), 3)
        for x in suffixes: pass
        sfx2 = tr.iter_suffixes(u"")
        self.assertEqual(_GRC(suffixes), _GRC(sfx2))
    
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