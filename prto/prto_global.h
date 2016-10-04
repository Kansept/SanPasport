#ifndef PRTO_GLOBAL_H
#define PRTO_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(SANPASPORT_LIBRARY)
#  define PRTOSHARED_EXPORT Q_DECL_EXPORT
#else
#  define PRTOSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // PRTO_GLOBAL_H
