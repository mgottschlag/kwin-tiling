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

#include "FontPreview.h"
#include "FcEngine.h"
#include "CharTip.h"
#include <kapplication.h>
#include <klocale.h>
#include <QPainter>
#include <QImage>
#include <QPixmap>
#include <QPaintEvent>
#include <QMouseEvent>
#include <stdlib.h>

namespace KFI
{

CFontPreview::CFontPreview(QWidget *parent)
            : QWidget(parent),
              itsCurrentFace(1),
              itsLastWidth(0),
              itsLastHeight(0),
              itsStyleInfo(KFI_NO_STYLE_INFO),
              itsLastChar(itsChars.end()),
              itsTip(NULL)
{
    QPalette p(palette());
    p.setColor(backgroundRole(), CFcEngine::bgndCol());
    setPalette(p);
}

CFontPreview::~CFontPreview()
{
    delete itsTip;
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
                                   itsCurrentFace-1, false, itsRange, &itsChars, itsFontName,
                                   itsStyleInfo))
    {
        setMouseTracking(itsChars.count()>0);
        update();
        emit status(true);
    }
    else
    {
        QPixmap nullPix;

        itsPixmap=nullPix;
        setMouseTracking(false);
        update();
        emit status(false);
    }
    itsLastChar=itsChars.end();
}

void CFontPreview::setUnicodeRange(const QList<CFcEngine::TRange> &r)
{
    itsRange=r;
    showFont();
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

void CFontPreview::mouseMoveEvent(QMouseEvent *event)
{
    QList<CFcEngine::TChar>::ConstIterator end(itsChars.end());

    if(itsLastChar==end || !(*itsLastChar).contains(event->pos()))
        for(QList<CFcEngine::TChar>::ConstIterator it(itsChars.begin()); it!=end; ++it)
            if((*it).contains(event->pos()))
            {
                if(!itsTip)
                    itsTip=new CCharTip(this);

                itsTip->setItem(*it);
                itsLastChar=it;
                break;
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
