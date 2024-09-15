#pragma once
#include <QtGlobal>

#ifdef UTILS_LIBRARY
# define UTILS_EXPORT Q_DECL_EXPORT
#else
# define UTILS_EXPORT Q_DECL_IMPORT
#endif


