import unittest

class TestBasic(unittest.TestCase):
    def test_basic(self):
        import _treez
        _trie_capsule = _treez.trie_create()
        _treez.trie_add(_trie_capsule)
