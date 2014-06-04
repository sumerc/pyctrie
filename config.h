#ifndef YCONFIG_H
#define YCONFIG_H

#include "Python.h"

#if PY_MAJOR_VERSION >= 3
#define IS_PY3K
#endif

#ifdef IS_PY3K
#if PY_MINOR_VERSION  >= 3
#define IS_PEP393_AVAILABLE
#endif
#endif

#ifdef IS_PEP393_AVAILABLE
#define TRIE_CHAR Py_UCS4
#else
#define TRIE_CHAR Py_UNICODE
#endif

#define TRIE_MALLOC PyMem_Malloc
#define TRIE_FREE PyMem_Free
#define TRIE_DATA uintptr_t

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
