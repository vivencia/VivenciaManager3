#ifndef VMPASSWORDMANAGER_GLOBAL_H
#define VMPASSWORDMANAGER_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(VMPASSWORDMANAGER_LIBRARY)
#  define VMPASSWORDMANAGERSHARED_EXPORT Q_DECL_EXPORT
#else
#  define VMPASSWORDMANAGERSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // VMPASSWORDMANAGER_GLOBAL_H