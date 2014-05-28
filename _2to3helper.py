import sys

_py3 = (sys.version_info >= (3, 0))

def is_str(s):
    if _py3:
        if not isinstance(s, str):
            return False
    else:            
        if not isinstance(s, basestring): # basestring = str + unicode
            return False
    return True
    
def uni(s):
    if not _py3:
        s = unicode(s)
    return s
    