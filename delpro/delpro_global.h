#ifndef DELPRO_GLOBAL_H
#define DELPRO_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(DELPRO_LIBRARY)
#  define DELPROSHARED_EXPORT Q_DECL_EXPORT
#else
#  define DELPROSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // DELPRO_GLOBAL_H
