////////////////////////////////////////////////////////////////////////////////
//
// Class Name    : CFontPreview
// Author        : Craig Drummond
// Project       : K Font Installer (kfontinst-kcontrol)
// Creation Date : 04/11/2001
// Version       : $Revision$ $Date$
//
////////////////////////////////////////////////////////////////////////////////
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
////////////////////////////////////////////////////////////////////////////////
// (C) Craig Drummond, 2001
////////////////////////////////////////////////////////////////////////////////

#include "FontPreview.h"
#include <qpainter.h>
#include <qpalette.h>
#include <kapplication.h>

CFontPreview::CFontPreview(QWidget *parent, const char *name=NULL)
            : QWidget(parent, name)
{
}

void CFontPreview::setText(const QString &text)
{
    itsText=text;
    update();
}

void CFontPreview::setPixmap(const QPixmap &pixmap)
{
    QString empty;

    itsText=empty;
    itsPixmap=pixmap;
    update();
}

void CFontPreview::paintEvent(QPaintEvent *)
{
    QRect    r(rect());
    QPainter paint( this );

    r.setX(r.x()+1);

    if(itsText.isEmpty())
        paint.drawPixmap(r, itsPixmap);
    else
    {
        r.setY(r.y()+((height()-fontMetrics().height())/2));
        paint.setPen(kapp->palette().active().text());
        paint.drawText(r, AlignLeft, itsText);
    }
}

QSize CFontPreview::sizeHint() const
{
    return QSize(32, 32);
}

QSize CFontPreview::minimumSizeHint() const
{
    return QSize(32, 32);
}
