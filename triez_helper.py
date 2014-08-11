import sys

_is_py3k = (sys.version_info >= (3, 0))

if _is_py3k:
    xrange = range

def is_str(s):
    if _is_py3k:
        if not isinstance(s, str):
            return False
    else:            
        if not isinstance(s, basestring): # basestring = str + unicode
            return False
    return True
    
def uni(s):
    if _is_py3k:
        s = str(s)
    else:
        s = unicode(s)
    return s
    
def is_py3k():
    return _is_py3k

def u(s):
    if not _is_py3k:
        return unicode(s)
    return s
