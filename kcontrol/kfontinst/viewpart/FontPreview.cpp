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

#include "FontPreview.h"
#include "FcEngine.h"
#include "CharTip.h"
#include <KDE/KApplication>
#include <KDE/KLocale>
#include <QtGui/QPainter>
#include <QtGui/QPaintEvent>
#include <QtGui/QMouseEvent>
#include <stdlib.h>

namespace KFI
{

static const int constBorder=4;
static const int constStepSize=16;

CFontPreview::CFontPreview(QWidget *parent)
            : QWidget(parent),
              itsCurrentFace(1),
              itsLastWidth(0),
              itsLastHeight(0),
              itsStyleInfo(KFI_NO_STYLE_INFO),
              itsLastChar(itsChars.end()),
              itsTip(NULL)
{
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
    itsLastWidth=width()+constStepSize;
    itsLastHeight=height()+constStepSize;

    if(!itsCurrentUrl.isEmpty() &&
       CFcEngine::instance()->draw(itsCurrentUrl, itsLastWidth, itsLastHeight, itsPixmap,
                                   itsCurrentFace, false, itsRange, &itsChars, itsFontName,
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

    paint.fillRect(rect(), palette().base());
    if(!itsPixmap.isNull())
    {

        if(abs(width()-itsLastWidth)>constStepSize || abs(height()-itsLastHeight)>constStepSize)
            showFont();
        else
            paint.drawPixmap(QPoint(constBorder, constBorder), itsPixmap,
                             QRect(0, 0, width()-(constBorder*2), height()-(constBorder*2)));
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
