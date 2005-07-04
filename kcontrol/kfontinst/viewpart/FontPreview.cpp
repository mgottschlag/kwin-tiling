////////////////////////////////////////////////////////////////////////////////
//
// Class Name    : KFI::CFontPreview
// Author        : Craig Drummond
// Project       : K Font Installer
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
// Foundation, Inc., 51 Franklin Steet, Fifth Floor, Boston, MA  02110-1301, USA.
//
////////////////////////////////////////////////////////////////////////////////
// (C) Craig Drummond, 2001, 2002, 2003, 2004
////////////////////////////////////////////////////////////////////////////////

#include "FontPreview.h"
#include <kapplication.h>
#include <klocale.h>
#include <qpainter.h>
#include <qimage.h>
#include <stdlib.h>

namespace KFI
{

CFontPreview::CFontPreview(QWidget *parent, const char *name)
            : QWidget(parent, name),
              itsCurrentFace(1),
              itsLastWidth(0),
              itsLastHeight(0),
              itsBgndCol(eraseColor())
{
}

void CFontPreview::showFont(const KURL &url)
{
    itsCurrentUrl=url;
    showFace(1);
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
       itsEngine.draw(itsCurrentUrl, itsLastWidth, itsLastHeight, itsPixmap, itsCurrentFace-1, false))
    {
        setEraseColor(Qt::white);
        update();
        emit status(true);
    }
    else
    {
        QPixmap nullPix;

        setEraseColor(itsBgndCol);
        itsPixmap=nullPix;
        update();
        emit status(false);
    }
}

void CFontPreview::paintEvent(QPaintEvent *)
{
    QPainter paint(this);
 
    if(itsPixmap.isNull())
    {
        paint.setPen(kapp->palette().active().text());
        paint.drawText(rect(), AlignCenter, i18n(" No preview available"));
    }
    else
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
