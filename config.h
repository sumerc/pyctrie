#ifndef YCONFIG_H
#define YCONFIG_H

#include "Python.h"
#include "capsulethunk.h"

#define TRIE_CHAR Py_UNICODE

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

#if PY_MAJOR_VERSION >= 3
#define IS_PY3K
#endif

#endif
