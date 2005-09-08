#ifndef __PIXMAP_H__
#define __PIXMAP_H__


#include <qpixmap.h>
#include <q3dict.h>
#include <qstring.h>

class LayoutIcon {

    static Q3Dict<QPixmap> pixmaps;
    static const QString flagTemplate;
    static bool cacheWithFlags;

  public:
    static const QPixmap& findPixmap(const QString& code, bool showFlag);
};

#endif
