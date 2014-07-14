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
//Python dynamically choose encoding of the string if PEP393 is available.
#define TRIE_CHAR Py_UCS4
#else
//Py_UNICODE type represents the storage type which is used by Python internally  
//as basis for holding Unicode ordinals. Python’s default builds use a 16-bit  
//type for Py_UNICODE and store Unicode values internally as UCS2.
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
