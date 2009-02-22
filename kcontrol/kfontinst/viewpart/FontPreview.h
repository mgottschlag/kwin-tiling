#ifndef __FONT_PREVIEW_H__
#define __FONT_PREVIEW_H__

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

#include <QtGui/QImage>
#include <QtCore/QSize>
#include <QtGui/QWidget>
#include <QtGui/QColor>
#include <QtGui/QPaintEvent>
#include <KDE/KUrl>
#include "KfiConstants.h"
#include "FcEngine.h"

class QWheelEvent;

namespace KFI
{

class CCharTip;
class CFcEngine;

class CFontPreview : public QWidget
{
    Q_OBJECT

    public:

    CFontPreview(QWidget *parent);
    virtual ~CFontPreview();

    void        paintEvent(QPaintEvent *);
    void        mouseMoveEvent(QMouseEvent *event);
    void        wheelEvent(QWheelEvent *e);
    QSize       sizeHint() const;
    QSize       minimumSizeHint() const;

    void        showFont(const KUrl &url, const QString &name=QString(),
                         unsigned long styleInfo=KFI_NO_STYLE_INFO, int face=0);
    void        showFont();
    void        showFace(int face);
    void        zoomIn();
    void        zoomOut();

    CFcEngine * engine() { return itsEngine; }

    public Q_SLOTS:

    void        setUnicodeRange(const QList<CFcEngine::TRange> &r);

    Q_SIGNALS:

    void        status(bool);
    void        doZoomIn();
    void        doZoomOut();

    private:

    QImage                   itsImage;
    KUrl                     itsCurrentUrl;
    int                      itsCurrentFace,
                             itsLastWidth,
                             itsLastHeight,
                             itsStyleInfo;
    QString                  itsFontName;
    QList<CFcEngine::TRange> itsRange;
    QList<CFcEngine::TChar>  itsChars;
    CFcEngine::TChar         itsLastChar;
    CCharTip                 *itsTip;
    CFcEngine                *itsEngine;

    friend class CCharTip;
};

}

#endif
