#ifndef __FC_ENGINE_H__
#define __FC_ENGINE_H__

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <qstring.h>
#include <qvaluelist.h>
#include <kurl.h>
#include <fontconfig/fontconfig.h>

class QPixmap;

#ifdef HAVE_XFT
typedef struct _XftFont  XftFont;
#endif

namespace KFI
{

class CFcEngine
{
    public:

    CFcEngine();
    ~CFcEngine();

#ifdef HAVE_XFT
    bool    draw(const KURL &url, int w, int h, QPixmap &pix, int faceNo, bool thumb);
#endif
    int     getNumIndexes() { return itsIndexCount; } // Only valid after draw has been called!
    QString getName(const KURL &url, int faceNo=0);
    bool    getInfo(const KURL &url, int faceNo, QString &full, QString &family, QString &foundry,
                    QString &weight, QString &width, QString &spacing, QString &slant);

#ifdef HAVE_XFT
    static QString getPreviewString();
    static void    setPreviewString(const QString &str);
#endif
    static QString getFcString(FcPattern *pat, const char *val, int faceNo=0);
    static QString createName(FcPattern *pat, int faceNo=0);
    static QString weightStr(int weight, bool emptyNormal=true);
    static QString widthStr(int width, bool emptyNormal=true);
    static QString slantStr(int slant, bool emptyNormal=true);
    static QString spacingStr(int spacing);

    private:

    bool      parseUrl(const KURL &url, int faceNo, bool all=false);
#ifdef HAVE_XFT
    XftFont * getFont(int size, QPixmap *pix=NULL);
    void      getSizes(QPixmap *pix=NULL);
#endif

    private:

    bool            itsInstalled;
    QString         itsName,
                    itsDescriptiveName,
                    itsFoundry;
    int             itsIndex,
                    itsIndexCount,
                    itsWeight,
                    itsWidth,
                    itsSlant,
                    itsSpacing,
                    itsAlphaSize;
    QValueList<int> itsSizes;
    KURL            itsLastUrl;
    FcBool          itsScalable;
};

};

#endif
