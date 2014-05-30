#ifndef YCONFIG_H
#define YCONFIG_H

#include "Python.h"
#include "capsulethunk.h"

#if PY_MAJOR_VERSION >= 3
#define IS_PY3K
#endif

#ifdef IS_PY3K
#if PY_MINOR_VERSION  >= 3
#define IS_PEP393_AVAILABLE
#endif
#endif

// if PEP393 is available, use UCS1 internally as it will probably give the best
// overall trie CHAR_SIZE value for ISO languages. 
#ifdef IS_PEP393_AVAILABLE
#define TRIE_CHAR Py_UCS1
#else
#define TRIE_CHAR Py_UNICODE
#endif

#define TRIE_MALLOC PyMem_Malloc
#define TRIE_FREE PyMem_Free

#if defined(MS_WINDOWS)
#define _WINDOWS
#elif (defined(__MACH__) && defined(__APPLE__))
#define _MACH
#else /* *nix */
#define _UNIX
#endif

#ifdef _MSC_VER 
#include "stddef.h" 
#else
#include "stdint.h"
#endif



#endif
