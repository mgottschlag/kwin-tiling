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

#include "kdmpixmap.h"
#include "kdmthemer.h"

#include <kstandarddirs.h>

#include <QDirIterator>
#include <QPainter>
#include <QSignalMapper>
#include <QSvgRenderer>

#include <math.h>

KdmPixmap::KdmPixmap(QObject *parent, const QDomNode &node)
    : KdmItem(parent, node)
    , qsm(0)
{
    itemType = "pixmap";
    if (!isVisible())
        return;

    // Set default values for pixmap (note: strings are already Null)
    pixmap.normal.tint.setRgb(0xFFFFFF);
    pixmap.normal.present = true;

    // Read PIXMAP TAGS
    QDomNodeList childList = node.childNodes();
    for (int nod = 0; nod < childList.count(); nod++) {
        QDomNode child = childList.item(nod);
        QDomElement el = child.toElement();
        QString tagName = el.tagName();

        if (tagName == "normal") {
            definePixmap(el, pixmap.normal);
            parseColor(el.attribute("tint", "#ffffff"), el.attribute("alpha", "1.0"), pixmap.normal.tint);
        } else if (tagName == "active") {
            pixmap.active.present = true;
            definePixmap(el, pixmap.active);
            parseColor(el.attribute("tint", "#ffffff"), el.attribute("alpha", "1.0"), pixmap.active.tint);
        } else if (tagName == "prelight") {
            pixmap.prelight.present = true;
            definePixmap(el, pixmap.prelight);
            parseColor(el.attribute("tint", "#ffffff"), el.attribute("alpha", "1.0"), pixmap.prelight.tint);
        }
    }
}

void
KdmPixmap::definePixmap(const QDomElement &el, PixmapStruct::PixmapClass &pClass)
{
    QString fileName = el.attribute("file");
    if (!fileName.isEmpty()) {
        if (fileName.at(0) != '/')
            fileName.prepend(themer()->baseDir() + '/');
        pClass.fullpath = fileName;
    } else {
        fileName = el.attribute("wallpaper");
        if (fileName.isEmpty())
            return;
        QString xf = KStandardDirs::locate("wallpaper", fileName + "/contents/images/");
        if (!xf.isEmpty()) {
            pClass.package = true;
        } else {
            xf = KStandardDirs::locate("wallpaper", fileName);
            if (xf.isEmpty()) {
                kWarning() << "Cannot find wallpaper" << fileName;
                return;
            }
        }
        pClass.fullpath = xf;
    }

    QString aspect = el.attribute("scalemode", "free");
    pClass.aspectMode =
            (aspect == "fit") ? Qt::KeepAspectRatio :
            (aspect == "crop") ? Qt::KeepAspectRatioByExpanding :
            Qt::IgnoreAspectRatio;

    pClass.svgImage = fileName.endsWith(".svg") || fileName.endsWith(".svgz");
    if (pClass.svgImage)
        pClass.svgElement = el.attribute("element");
}

QString
KdmPixmap::findBestPixmap(const QString &dir, const QString &pat,
                          const QRect &area, Qt::AspectRatioMode aspectMode)
{
    int tw = area.width(), th = area.height();
    float tar = 1.0 * tw / th;
    float tpn = tw * th;
    QString best;
    float bestPenalty = 0;
    QRegExp rx(QRegExp::escape(dir) + pat);
    QDirIterator dit(dir);
    while (dit.hasNext()) {
        QString fn = dit.next();
        if (rx.exactMatch(fn)) {
            int w = rx.cap(1).toInt(), h = rx.cap(2).toInt();
            // This algorithm considers need for zooming and distortion of
            // aspect ratio / discarded pixels / pixels left free. The weighting
            // gives good results in the tested cases, but is pretty arbitrary
            // by all practical means.
            float ar = 1.0 * w / h;
            float pn = w * h;
            float rawAspect = ((tar > ar) ? tar / ar : ar / tar);
            float penalty = rawAspect - 1;
            if (aspectMode != Qt::IgnoreAspectRatio) {
                bool exp = (aspectMode == Qt::KeepAspectRatioByExpanding);
                // Give an advantage to non-zooming cases.
                if ((w == tw && (h > th) == exp) || (h == th && (w > tw) == exp))
                    goto skipSize;
                else
                    penalty *= 5;
                // Dropped pixels don't contribute to the input area.
                if (exp)
                    pn /= rawAspect;
            } else {
                // This mode does not preserve aspect ratio, so give an
                // advantage to pics with a better ratio to start with.
                penalty *= 10;
            }
            // Too small is worse than too big - within limits.
            penalty += sqrt((tpn > pn) ? (tpn / pn - 1) * 2 : pn / tpn - 1);
            skipSize:
            if (best.isEmpty() || penalty < bestPenalty) {
                best = fn;
                bestPenalty = penalty;
            }
        }
    }
    return best;
}

bool
KdmPixmap::loadPixmap(PixmapStruct::PixmapClass &pClass)
{
    if (!pClass.image.isNull())
        return true;
    if (pClass.fullpath.isEmpty())
        return false;
    QString fn;
    if (pClass.package) {
        // Always find best fit from package.
        fn = findBestPixmap(pClass.fullpath, "(\\d+)x(\\d+)\\.[^.]+",
                            area.isValid() ? area : QRect(0, 0, 1600, 1200),
                            pClass.aspectMode);
    } else {
        if (area.isValid()) {
            if (QFile::exists(pClass.fullpath)) {
                // If base file exists, use only a perfect match.
                int dot = pClass.fullpath.lastIndexOf('.');
                fn = pClass.fullpath.left(dot);
                fn += QString("-%1x%2").arg(area.width()).arg(area.height());
                fn += pClass.fullpath.mid(dot);
                if (!QFile::exists(fn))
                    fn = pClass.fullpath;
            } else {
                // Otherwise find best match.
                int sep = pClass.fullpath.lastIndexOf('/');
                int dot = pClass.fullpath.lastIndexOf('.');
                if (dot < sep)
                    dot = pClass.fullpath.length();
                QString f = QRegExp::escape(pClass.fullpath.mid(sep + 1, dot - sep - 1));
                f += "-(\\d+)x(\\d+)";
                f += QRegExp::escape(pClass.fullpath.mid(dot));
                fn = findBestPixmap(pClass.fullpath.left(sep + 1), f, area,
                                    pClass.aspectMode);
            }
        } else {
            fn = pClass.fullpath;
        }
    }
    if (!pClass.image.load(fn)) {
        kWarning() << "failed to load" << fn;
        pClass.fullpath.clear();
        return false;
    }
    if (pClass.image.format() != QImage::Format_ARGB32)
        pClass.image = pClass.image.convertToFormat(QImage::Format_ARGB32);
    applyTint(pClass, pClass.image);
    return true;
}

bool
KdmPixmap::loadSvg(PixmapStruct::PixmapClass &pClass)
{
    if (pClass.svgRenderer)
        return true;
    if (pClass.fullpath.isEmpty())
        return false;
    pClass.svgRenderer = new QSvgRenderer(pClass.fullpath, this);
    if (!pClass.svgRenderer->isValid()) {
        delete pClass.svgRenderer;
        pClass.svgRenderer = 0;
        kWarning() << "failed to load " << pClass.fullpath ;
        pClass.fullpath.clear();
        return false;
    }
    if (pClass.svgRenderer->animated()) {
        if (!qsm) {
            qsm = new QSignalMapper(this);
            connect(qsm, SIGNAL(mapped(int)), SLOT(slotAnimate(int)));
        }
        qsm->setMapping(pClass.svgRenderer, state); // assuming we only load the current state
        connect(pClass.svgRenderer, SIGNAL(repaintNeeded()), qsm, SLOT(map()));
    }
    pClass.svgSizeHint = pClass.svgElement.isEmpty() ?
        pClass.svgRenderer->defaultSize() :
        pClass.svgRenderer->boundsOnElement(pClass.svgElement).size().toSize();
    return true;
}

QSize
KdmPixmap::sizeHint()
{
    // use the pixmap size as the size hint
    if (!pixmap.normal.svgImage) {
        if (loadPixmap(pixmap.normal))
            return pixmap.normal.image.size();
    } else {
        if (loadSvg(pixmap.normal))
            return pixmap.normal.svgSizeHint;
    }
    return KdmItem::sizeHint();
}

void
KdmPixmap::setGeometry(QStack<QSize> &parentSizes, const QRect &newGeometry, bool force)
{
    KdmItem::setGeometry(parentSizes, newGeometry, force);
    pixmap.active.targetArea = QRect();
    pixmap.prelight.targetArea = QRect();
    pixmap.normal.targetArea = QRect();
}

bool
KdmPixmap::calcTargetArea(PixmapStruct::PixmapClass &pClass, const QSize &sh)
{
    QSize sz = sh;
    sz.scale(area.size(), pClass.aspectMode);
    pClass.targetArea.setSize(sz);
    pClass.targetArea.moveCenter(area.center());
    return pClass.targetArea.size() != pClass.readyPixmap.size();
}

void
KdmPixmap::drawContents(QPainter *p, const QRect &r)
{
    PixmapStruct::PixmapClass &pClass = getCurClass();

    if (pClass.targetArea.isEmpty()) {
        QImage scaledImage;

        if (pClass.svgImage) {
            if (loadSvg(pClass)) {
                if (!calcTargetArea(pClass, pClass.svgSizeHint))
                    goto noop;
                scaledImage = QImage(pClass.targetArea.size(), QImage::Format_ARGB32);
                scaledImage.fill(0);
                QPainter pa(&scaledImage);
                if (pClass.svgElement.isEmpty())
                    pClass.svgRenderer->render(&pa);
                else
                    pClass.svgRenderer->render(&pa, pClass.svgElement);
                applyTint(pClass, scaledImage);
            }
        } else {
            if (loadPixmap(pClass)) {
                if (!calcTargetArea(pClass, pClass.image.size()))
                    goto noop;
                scaledImage =
                    (area.size() != pClass.image.size()) ?
                        pClass.image.scaled(pClass.targetArea.size(),
                                            Qt::IgnoreAspectRatio, Qt::SmoothTransformation) :
                        pClass.image;
            }
        }

        if (scaledImage.isNull()) {
            p->fillRect(r, Qt::black);
            return;
        }

        pClass.readyPixmap = QPixmap::fromImage(scaledImage);
    }
  noop:
    QRect tr = r.intersected(pClass.targetArea);
    p->drawPixmap(tr.topLeft(), pClass.readyPixmap,
                  QRect(tr.topLeft() - pClass.targetArea.topLeft(), tr.size()));
}

void
KdmPixmap::applyTint(PixmapStruct::PixmapClass &pClass, QImage &img)
{
    if (pClass.tint.rgba() == 0xFFFFFFFF)
        return;

    int w = img.width();
    int h = img.height();
    int tint_red = pClass.tint.red();
    int tint_green = pClass.tint.green();
    int tint_blue = pClass.tint.blue();
    int tint_alpha = pClass.tint.alpha();

    for (int y = 0; y < h; ++y) {
        QRgb *ls = (QRgb *)img.scanLine(y);
        for (int x = 0; x < w; ++x) {
            QRgb l = ls[x];
            int r = qRed(l) * tint_red / 255;
            int g = qGreen(l) * tint_green / 255;
            int b = qBlue(l) * tint_blue / 255;
            int a = qAlpha(l) * tint_alpha / 255;
            ls[x] = qRgba(r, g, b, a);
        }
    }
}

KdmPixmap::PixmapStruct::PixmapClass &
KdmPixmap::getClass(ItemState sts)
{
    return
        (sts == Sprelight && pixmap.prelight.present) ?
            pixmap.prelight :
            (sts == Sactive && pixmap.active.present) ?
                pixmap.active :
                pixmap.normal;
}

void
KdmPixmap::slotAnimate(int sts)
{
    PixmapStruct::PixmapClass &pClass = getClass(ItemState(sts));
    pClass.readyPixmap = QPixmap();
    if (&pClass == &getCurClass())
        needUpdate();
}

void
KdmPixmap::statusChanged(bool descend)
{
    KdmItem::statusChanged(descend);
    if (!pixmap.active.present && !pixmap.prelight.present)
        return;
    if ((state == Sprelight && !pixmap.prelight.present) ||
        (state == Sactive && !pixmap.active.present))
        return;
    needUpdate();
}

#include "kdmpixmap.moc"
