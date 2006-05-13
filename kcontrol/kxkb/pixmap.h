#ifndef __PIXMAP_H__
#define __PIXMAP_H__


#include <QPixmap>
#include <q3dict.h>
#include <QString>

class LayoutIcon {

    static Q3Dict<QPixmap> pixmaps;
    static const QString flagTemplate;
    static bool cacheWithFlags;

  public:
    static const QPixmap& findPixmap(const QString& code, bool showFlag);
};

#endif
