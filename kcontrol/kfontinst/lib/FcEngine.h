#ifndef __FC_ENGINE_H__
#define __FC_ENGINE_H__

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <qstring.h>
#include <qvaluelist.h>
#include <kurl.h>
#include <kdeversion.h>
#include <fontconfig/fontconfig.h>

#define KFI_FC_HAS_WIDTHS    (FC_VERSION>=KDE_MAKE_VERSION(2, 2, 0))
#define KFI_FC_FULL_WEIGHTS  (FC_VERSION>=KDE_MAKE_VERSION(2, 2, 0))

#if !KFI_FC_FULL_WEIGHTS
#define FC_WEIGHT_THIN              0
#define FC_WEIGHT_EXTRALIGHT        40
#define FC_WEIGHT_ULTRALIGHT        FC_WEIGHT_EXTRALIGHT
#define FC_WEIGHT_BOOK              75
#define FC_WEIGHT_REGULAR           80
#define FC_WEIGHT_NORMAL            FC_WEIGHT_REGULAR
#define FC_WEIGHT_SEMIBOLD          FC_WEIGHT_DEMIBOLD
#define FC_WEIGHT_EXTRABOLD         205
#define FC_WEIGHT_ULTRABOLD         FC_WEIGHT_EXTRABOLD
#define FC_WEIGHT_HEAVY             FC_WEIGHT_BLACK
#endif

class QPixmap;

#ifdef HAVE_XFT
typedef struct _XftFont  XftFont;
#endif

namespace KFI
{

class KDE_EXPORT CFcEngine
{
    public:

    CFcEngine();
    ~CFcEngine();

#ifdef HAVE_XFT
    bool    draw(const KURL &url, int w, int h, QPixmap &pix, int faceNo, bool thumb);
#endif
    int     getNumIndexes() { return itsIndexCount; } // Only valid after draw has been called!
    QString getName(const KURL &url, int faceNo=0);
    bool    getInfo(const KURL &url, int faceNo, QString &full, QString &family, QString &foundry, QString &weight,
#ifdef KFI_FC_HAS_WIDTHS
                    QString &width,
#endif
                    QString &spacing, QString &slant);

#ifdef HAVE_XFT
    static QString getPreviewString();
    static void    setPreviewString(const QString &str);
#endif
    static QString getFcString(FcPattern *pat, const char *val, int faceNo=0);
    static QString createName(FcPattern *pat, int faceNo=0);
    static QString weightStr(int weight, bool emptyNormal=true);
#ifdef KFI_FC_HAS_WIDTHS
    static QString widthStr(int width, bool emptyNormal=true);
#endif
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
#ifdef KFI_FC_HAS_WIDTHS
                    itsWidth,
#endif
                    itsSlant,
                    itsSpacing,
                    itsAlphaSize;
    QValueList<int> itsSizes;
    KURL            itsLastUrl;
    FcBool          itsScalable;
};

}

#endif
