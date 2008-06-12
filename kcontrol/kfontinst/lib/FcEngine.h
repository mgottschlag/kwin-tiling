#ifndef __FC_ENGINE_H__
#define __FC_ENGINE_H__

/*
 * KFontInst - KDE Font Installer
 *
 * Copyright 2003-2007 Craig Drummond <craig@kde.org>
 *
 * ----
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
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

#include <config-workspace.h>

#include <QtCore/QVector>
#include <QtCore/QRect>
#include <QtGui/QFont>
#include <QtGui/QColor>
#include <KDE/KUrl>
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

    struct TChar : public QRect
    {
        TChar(const QRect &r=QRect(), quint32 u=0)
            : QRect(r), ucs4(u) { }

        quint32 ucs4;
    };

    static CFcEngine * instance();

    CFcEngine();
    ~CFcEngine();

    void                  readConfig(KConfig &cfg);
    void                  writeConfig(KConfig &cfg);

    void                  setDirty() { itsFcDirty=true; }
    bool                  drawPreview(const QString &item, QPixmap &pix, int h,
                                      quint32 style=KFI_NO_STYLE_INFO, int face=0);
    bool                  draw(const KUrl &url, int w, int h, QPixmap &pix, int faceNo, bool thumb,
                               const QList<TRange> &range=QList<TRange>(), QList<TChar> *chars=NULL,
                               const QString &name=QString(), quint32 style=KFI_NO_STYLE_INFO);
    int                   getNumIndexes() { return itsIndexCount; } // Only valid after draw has been called!
    const QString &       getName(const KUrl &url, int faceNo=0);
    bool                  getInfo(const KUrl &url, int faceNo, Misc::TFont &info);
    static QFont          getQFont(const QString &family, quint32 style, int size);

    const QVector<int> &  sizes() const { return itsSizes; }
    int                   alphaSize() const { return itsAlphaSize; }

    quint32               styleVal() { return FC::createStyleVal(itsWeight, itsWidth, itsSlant); }

    const QString &       getPreviewString()                  { return itsPreviewString; }
    static QString        getDefaultPreviewString();
    void                  setPreviewString(const QString &str)
                            { itsPreviewString=str.isEmpty() ? getDefaultPreviewString() : str; }
    static QString        getUppercaseLetters();
    static QString        getLowercaseLetters();
    static QString        getPunctuation();

    static void           setTextCol(const QColor &col) { theirTextCol=col; }
    static const QColor & textCol()                     { return theirTextCol; }

    static const int      constScalableSizes[];
    static const int      constDefaultAlphaSize;

    private:

    bool                  parseUrl(const KUrl &url, int faceNo);
    bool                  parseName(const QString &name, quint32 style=KFI_NO_STYLE_INFO, const KUrl &url=KUrl());
    XftFont *             queryFont();
    XftFont *             getFont(int size);
    bool                  isCorrect(XftFont *f, bool checkFamily);
    void                  getSizes();
    void                  drawName(QPainter &painter, int x, int &y, int w);
    void                  addFontFile(const QString &file);
    void                  reinit();

    private:

    bool          itsInstalled,
                  itsFcDirty;
    QString       itsName,
                  itsFileName,
                  itsDescriptiveName;
    int           itsIndex,
                  itsIndexCount,
                  itsWeight,
                  itsWidth,
                  itsSlant,
                  itsAlphaSize;
    QVector<int>  itsSizes;
    KUrl          itsLastUrl;
    FcBool        itsScalable;
    QStringList   itsAddedFiles;
    QString       itsPreviewString;

    static QColor theirTextCol;
};

}

#endif
