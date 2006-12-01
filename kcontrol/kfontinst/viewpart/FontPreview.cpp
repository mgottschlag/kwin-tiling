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

#include "FontPreview.h"
#include "FcEngine.h"
#include <kapplication.h>
#include <klocale.h>
#include <QPainter>
#include <QImage>
#include <QPixmap>
#include <QPaintEvent>
#include <stdlib.h>

namespace KFI
{

CFontPreview::CFontPreview(QWidget *parent)
            : QWidget(parent),
              itsCurrentFace(1),
              itsLastWidth(0),
              itsLastHeight(0),
              itsUnicodeStart(-1),
              itsStyleInfo(KFI_NO_STYLE_INFO)
{
    QPalette p(palette());
    p.setColor(backgroundRole(), CFcEngine::bgndCol());
    setPalette(p);
}

void CFontPreview::showFont(const KUrl &url, const QString &name, unsigned long styleInfo,
                            int face)
{
    itsFontName=name;
    itsCurrentUrl=url;
    itsStyleInfo=styleInfo;
    showFace(face);
}

void CFontPreview::showFace(int face)
{
    itsCurrentFace=face;
    showFont();
}

void CFontPreview::showFont()
{
    itsLastWidth=width();
    itsLastHeight=height();

    if(!itsCurrentUrl.isEmpty() &&
       CFcEngine::instance()->draw(itsCurrentUrl, itsLastWidth, itsLastHeight, itsPixmap,
                                   itsCurrentFace-1, false, itsUnicodeStart, itsFontName,
                                   itsStyleInfo))
    {
        update();
        emit status(true);
    }
    else
    {
        QPixmap nullPix;

        itsPixmap=nullPix;
        update();
        emit status(false);
    }
}

void CFontPreview::paintEvent(QPaintEvent *)
{
    QPainter paint(this);

    paint.fillRect(rect(), CFcEngine::bgndCol());
    if(!itsPixmap.isNull())
    {
        static const int constStepSize=16;

        if(abs(width()-itsLastWidth)>constStepSize || abs(height()-itsLastHeight)>constStepSize)
            showFont();
        else
            paint.drawPixmap(0, 0, itsPixmap);
    }
}

QSize CFontPreview::sizeHint() const
{
    return QSize(132, 132);
}

QSize CFontPreview::minimumSizeHint() const
{
    return QSize(32, 32);
}

}

#include "FontPreview.moc"
