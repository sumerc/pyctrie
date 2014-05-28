"""

 triez - Fast, pure C, succinct trie

 Sumer Cip 2014

"""

import _triez
#from _2to3helper import is_str, uni

class TriezError(Exception): pass

class Trie(object): 
    def __init__(self):
        self._t = _triez.trie_create()
        
    def __del__(self):
        _triez.trie_destroy(self._t)
    
    def add(self, key, val):
        _triez.trie_add(self._t, key, val)
    
    def search(self, key):
        return _triez.trie_search(self._t, key)
            
    def delete(self, key):
        _triez.trie_delete(self._t, key)
    
    def node_count(self):
        return _triez.trie_node_count(self._t)
        
    def mem_usage(self):
        return _triez.trie_mem_usage(self._t)
        