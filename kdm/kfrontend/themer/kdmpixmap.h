/*
 *  Copyright (C) 2003 by Unai Garro <ugarro@users.sourceforge.net>
 *  Copyright (C) 2004 by Enrico Ros <rosenric@dei.unipd.it>
 *  Copyright (C) 2004 by Stephan Kulow <coolo@kde.org>
 *  Copyright (C) 2004 by Oswald Buddenhagen <ossi@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef KDMPIXMAP_H
#define KDMPIXMAP_H

#include "kdmitem.h"

#include <QImage>
#include <QPixmap>

class QSignalMapper;
class QSvgRenderer;

/*
 * KdmPixmap. A pixmap element
 */

class KdmPixmap : public KdmItem {
    Q_OBJECT

public:
    KdmPixmap(QObject *parent, const QDomNode &node);

protected:
    // reimplemented; returns the size of loaded pixmap
    virtual QSize sizeHint();

    // draw the pixmap
    virtual void drawContents(QPainter *p, const QRect &r);

    // handle switching between normal / active / prelight configurations
    virtual void statusChanged(bool descend);

    virtual void setGeometry(QStack<QSize> &parentSizes, const QRect &newGeometry, bool force);

    struct PixmapStruct {
        struct PixmapClass {
            PixmapClass()
                : svgRenderer(0), present(false), svgImage(false), package(false),
                  aspectMode(Qt::IgnoreAspectRatio) {}
            QString fullpath;
            QImage image;
            QSvgRenderer *svgRenderer;
            QPixmap readyPixmap;
            QRect targetArea;
            QColor tint;
            bool present;
            bool svgImage;
            bool package;
            QString svgElement;
            QSize svgSizeHint;
            Qt::AspectRatioMode aspectMode;
        } normal, active, prelight;
    } pixmap;

    QSignalMapper *qsm;

private:
    // Method to load the image given by the theme
    void definePixmap(const QDomElement &el, PixmapStruct::PixmapClass &pc);
    QString findBestPixmap(const QString &dir, const QString &pat,
                           const QRect &area, Qt::AspectRatioMode aspectMode);
    bool loadPixmap(PixmapStruct::PixmapClass &pc);
    bool loadSvg(PixmapStruct::PixmapClass &pc);
    bool calcTargetArea(PixmapStruct::PixmapClass &pClass, const QSize &sh);
    void applyTint(PixmapStruct::PixmapClass &pClass, QImage &img);
    PixmapStruct::PixmapClass &getClass(ItemState sts);
    PixmapStruct::PixmapClass &getCurClass() { return getClass(state); }

private Q_SLOTS:
    void slotAnimate(int sts);
};

#endif
