#ifndef YCONFIG_H
#define YCONFIG_H

#include "Python.h"

#define APP_NAME FFtreez

#if defined(MS_WINDOWS)
#define _WINDOWS
#elif (defined(__MACH__) && defined(__APPLE__))
#define _MACH
#else /* *nix */
#define _UNIX
#endif

#ifndef _MSC_VER /* non-windows compiler */
#include "stdint.h"
#endif

#if PY_MAJOR_VERSION >= 3
#define IS_PY3K
#endif

#endif
