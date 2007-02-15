#ifndef __FC_ENGINE_H__
#define __FC_ENGINE_H__

/*
 * KFontInst - KDE Font Installer
 *
 * (c) 2003-2007 Craig Drummond <craig@kde.org>
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
#include "Fc.h"

//Enable the following to use locale aware family name - if font supports this.
//#define KFI_USE_TRANSLATED_FAMILY_NAME

class QPixmap;
class KConfig;

typedef struct _XftFont  XftFont;

namespace KFI
{

class KDE_EXPORT CFcEngine
{
    public:

    struct TRange
    {
        TRange(quint32 f=0, quint32 t=0) : from(f), to(t) { }
        bool null() const { return 0==from && 0==to; }

        quint32 from,
                to;
    };

    static CFcEngine * instance();

    ~CFcEngine();

    void                  readConfig(KConfig &cfg);
    void                  writeConfig(KConfig &cfg);

    void                  setDirty() { itsFcDirty=true; }
    bool                  drawPreview(const QString &item, QPixmap &pix, int h,
                                      unsigned long style=KFI_NO_STYLE_INFO, int face=0);
    bool                  draw(const KUrl &url, int w, int h, QPixmap &pix, int faceNo, bool thumb,
                               const QList<TRange> &range=QList<TRange>(), const QString &name=QString(),
                               unsigned long style=KFI_NO_STYLE_INFO);
    int                   getNumIndexes() { return itsIndexCount; } // Only valid after draw has been called!
    const QString &       getName(const KUrl &url, int faceNo=0);
    bool                  getInfo(const KUrl &url, int faceNo, QString &full, QString &family, QString &foundry,
                                  QString &weight, QString &width, QString &spacing, QString &slant, QString &version);
    bool                  getInfo(const KUrl &url, int faceNo, Misc::TFont &info);
    static QFont          getQFont(const QString &family, unsigned long style, int size);

    const QVector<int> &  sizes() const { return itsSizes; }
    int                   alphaSize() const { return itsAlphaSize; }

    unsigned long         styleVal() { return FC::createStyleVal(itsWeight, itsWidth, itsSlant); }

    const QString &       getPreviewString()                  { return itsPreviewString; }
    static QString        getDefaultPreviewString();
    void                  setPreviewString(const QString &str)
                            { itsPreviewString=str.isEmpty() ? getDefaultPreviewString() : str; }
    static QString        getUppercaseLetters();
    static QString        getLowercaseLetters();
    static QString        getPunctuation();

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
                  itsVersion,
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
