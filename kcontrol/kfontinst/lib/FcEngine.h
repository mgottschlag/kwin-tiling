#ifndef __FC_ENGINE_H__
#define __FC_ENGINE_H__

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <qstring.h>
#include <q3valuevector.h>
#include <qfont.h>
//Added by qt3to4:
#include <QPixmap>
#include <kurl.h>
#include <kdeversion.h>
#include <fontconfig/fontconfig.h>

#if (FC_VERSION<20200)

#define KFI_FC_NO_WIDTHS
#define KFI_FC_LIMITED_WEIGHTS

#endif

#ifdef KFI_FC_LIMITED_WEIGHTS

#undef FC_WEIGHT_LIGHT
#define FC_WEIGHT_THIN              0
#define FC_WEIGHT_EXTRALIGHT        40
#define FC_WEIGHT_ULTRALIGHT        FC_WEIGHT_EXTRALIGHT
#define FC_WEIGHT_LIGHT             50
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
#ifndef KFI_FC_NO_WIDTHS
                    QString &width,
#endif
                    QString &spacing, QString &slant);
    QFont   getQFont(const QString &name, int size);

    const QVector<int> & sizes() const { return itsSizes; }
    int                       alphaSize() const { return itsAlphaSize; }

    static QString getPreviewString();
    static void    setPreviewString(const QString &str);
    static QString getUppercaseLetters();
    static QString getLowercaseLetters();
    static QString getPunctuation();
    static QString getFcString(FcPattern *pat, const char *val, int faceNo=0);
    static QString createName(FcPattern *pat, int faceNo=0);
    static QString weightStr(int weight, bool emptyNormal=true);
#ifndef KFI_FC_NO_WIDTHS
    static QString widthStr(int width, bool emptyNormal=true);
#endif
    static QString slantStr(int slant, bool emptyNormal=true);
    static QString spacingStr(int spacing);

    static const int constScalableSizes[];
    static const int constDefaultAlphaSize;

    private:

    bool      parseUrl(const KURL &url, int faceNo, bool all=false);
    void      parseName(const QString &name, int faceNo, bool all=false);
#ifdef HAVE_XFT
    XftFont * getFont(int size, QPixmap *pix=NULL);
    void      getSizes(QPixmap *pix=NULL);
#endif

    private:

    bool              itsInstalled;
    QString           itsName,
                      itsDescriptiveName,
                      itsFoundry;
    int               itsIndex,
                      itsIndexCount,
                      itsWeight,
#ifndef KFI_FC_NO_WIDTHS
                      itsWidth,
#endif
                      itsSlant,
                      itsSpacing,
                      itsAlphaSize;
    QVector<int> itsSizes;
    KURL              itsLastUrl;
    FcBool            itsScalable;
};

}

#endif
