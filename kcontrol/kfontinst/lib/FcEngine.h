#ifndef __FC_ENGINE_H__
#define __FC_ENGINE_H__

/*
 * KFontInst - KDE Font Installer
 *
 * (c) 2003-2006 Craig Drummond <craig@kde.org>
 *
 * ----
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <QString>
#include <QStringList>
#include <QVector>
#include <QFont>
#include <QColor>
#include <kurl.h>
#include <kdeversion.h>
#include <fontconfig/fontconfig.h>
#include "KfiConstants.h"
#include "Misc.h"

//Enable the following to use locale aware family name - if font supports this.
//#define KFI_USE_TRANSLATED_FAMILY_NAME

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
class KConfig;

typedef struct _XftFont  XftFont;

namespace KFI
{

//
// Ideally only want this class to contain KFI_FC_NO_WIDTHS
#ifdef KFI_FC_NO_WIDTHS
#define KFI_FC_WIDTH_ULTRACONDENSED     50
#define KFI_FC_WIDTH_EXTRACONDENSED     63
#define KFI_FC_WIDTH_CONDENSED          75
#define KFI_FC_WIDTH_SEMICONDENSED      87
#define KFI_FC_WIDTH_NORMAL             100
#define KFI_FC_WIDTH_SEMIEXPANDED       113
#define KFI_FC_WIDTH_EXPANDED           125
#define KFI_FC_WIDTH_EXTRAEXPANDED      150
#define KFI_FC_WIDTH_ULTRAEXPANDED      200
#else
#define KFI_FC_WIDTH_ULTRACONDENSED     FC_WIDTH_ULTRACONDENSED
#define KFI_FC_WIDTH_EXTRACONDENSED     FC_WIDTH_EXTRACONDENSED
#define KFI_FC_WIDTH_CONDENSED          FC_WIDTH_CONDENSED
#define KFI_FC_WIDTH_SEMICONDENSED      FC_WIDTH_SEMICONDENSED
#define KFI_FC_WIDTH_NORMAL             FC_WIDTH_NORMAL
#define KFI_FC_WIDTH_SEMIEXPANDED       FC_WIDTH_SEMIEXPANDED
#define KFI_FC_WIDTH_EXPANDED           FC_WIDTH_EXPANDED
#define KFI_FC_WIDTH_EXTRAEXPANDED      FC_WIDTH_EXTRAEXPANDED
#define KFI_FC_WIDTH_ULTRAEXPANDED      FC_WIDTH_ULTRAEXPANDED
#endif

class KDE_EXPORT CFcEngine
{
    public:

    static const int STD_PREVIEW = -1;
    static const int ALL_CHARS   = -2;

    static CFcEngine * instance();

    ~CFcEngine();

    void                  readConfig(KConfig &cfg);
    void                  writeConfig(KConfig &cfg);

    void                  setDirty() { itsFcDirty=true; }
    bool                  drawPreview(const QString &item, QPixmap &pix, int h,
                                      unsigned long style=KFI_NO_STYLE_INFO, int face=0);
    bool                  draw(const KUrl &url, int w, int h, QPixmap &pix, int faceNo, bool thumb,
                               int unicodeStart=STD_PREVIEW, const QString &name=QString(),
                               unsigned long style=KFI_NO_STYLE_INFO);
    int                   getNumIndexes() { return itsIndexCount; } // Only valid after draw has been called!
    const QString &       getName(const KUrl &url, int faceNo=0);
    bool                  getInfo(const KUrl &url, int faceNo, QString &full, QString &family, QString &foundry,
                                  QString &weight, QString &width, QString &spacing, QString &slant);
    bool                  getInfo(const KUrl &url, int faceNo, Misc::TFont &info);
    static QFont          getQFont(const QString &family, unsigned long style, int size);

    const QVector<int> &  sizes() const { return itsSizes; }
    int                   alphaSize() const { return itsAlphaSize; }

    unsigned long         styleVal() { return createStyleVal(itsWeight, itsWidth, itsSlant); }

    static unsigned long  createStyleVal(const QString &name);
    static unsigned long  createStyleVal(int weight, int width, int slant)
                            { return ((weight&0xFF)<<16)+((width&0xFF)<<8)+(slant&0xFF); }
    static QString        styleValToStr(unsigned long style);
    static void           decomposeStyleVal(int styleInfo, int &weight, int &width, int &slant);
    static unsigned long  styleValFromStr(const QString &style);
    const QString &       getPreviewString()                  { return itsPreviewString; }
    static QString        getDefaultPreviewString();
    void                  setPreviewString(const QString &str)
                            { itsPreviewString=str.isEmpty() ? getDefaultPreviewString() : str; }
    static QString        getUppercaseLetters();
    static QString        getLowercaseLetters();
    static QString        getPunctuation();
    static QString        getFcString(FcPattern *pat, const char *val, int index=0);
#ifdef KFI_USE_TRANSLATED_FAMILY_NAME
    static QString        getFcLangString(FcPattern *pat, const char *val, const char *valLang);
#endif
    static int            getFcInt(FcPattern *pat, const char *val, int index=0, int def=-1);
    static void           getDetails(FcPattern *pat, QString &name, int &styleVal, int &index);
    static QString        createName(FcPattern *pat);
    static QString        createName(FcPattern *pat, int weight, int width, int slant);
    static QString        createName(const QString &family, int styleInfo);
    static QString        createName(const QString &family, int weight, int width, int slant);
    static QString        weightStr(int weight, bool emptyNormal=true);
    static QString        widthStr(int width, bool emptyNormal=true);
    static QString        slantStr(int slant, bool emptyNormal=true);
    static QString        spacingStr(int spacing);

    static void           setBgndCol(const QColor &col) { theirBgndCol=col; }
    static const QColor & bgndCol()                     { return theirBgndCol; }
    static void           setTextCol(const QColor &col) { theirTextCol=col; }
    static const QColor & textCol()                     { return theirTextCol; }

    static const int      constScalableSizes[];
    static const int      constDefaultAlphaSize;

    private:

    CFcEngine();

    bool                  parseUrl(const KUrl &url, int faceNo, bool all=false);
    bool                  parseName(const QString &name, int faceNo,
                                    unsigned long style=KFI_NO_STYLE_INFO, bool all=false,
                                    const KUrl &url=KUrl());
    XftFont *             queryFont();
    XftFont *             getFont(int size);
    bool                  isCorrect(XftFont *f, bool checkFamily);
    void                  getSizes();
    void                  drawName(QPainter &painter, int x, int &y, int w, int offset);
    void                  addFontFile(const QString &file);
    void                  reinit();

    private:

    bool          itsInstalled,
                  itsFcDirty;
    QString       itsName,
                  itsFileName,
                  itsDescriptiveName,
                  itsFoundry;
    int           itsIndex,
                  itsIndexCount,
                  itsWeight,
                  itsWidth,
                  itsSlant,
                  itsSpacing,
                  itsAlphaSize;
    QVector<int>  itsSizes;
    KUrl          itsLastUrl;
    FcBool        itsScalable;
    QStringList   itsAddedFiles;
    QString       itsPreviewString;

    static QColor theirBgndCol;
    static QColor theirTextCol;
};

}

#endif
