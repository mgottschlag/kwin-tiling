#ifndef SM_EXPORT_H
#define SM_EXPORT_H

/* needed for KDE_EXPORT and KDE_IMPORT macros */
#include <kdemacros.h>

#ifndef SM_EXPORT
# if defined(MAKE_PLASMA_APPLET_SYSTEM_MONITOR_LIB)
   /* We are building this library */ 
#  define SM_EXPORT KDE_EXPORT
# else
   /* We are using this library */ 
#  define SM_EXPORT KDE_IMPORT
# endif
#endif

# ifndef SM_EXPORT_DEPRECATED
#  define SM_EXPORT_DEPRECATED KDE_DEPRECATED SM_EXPORT
# endif

#endif
