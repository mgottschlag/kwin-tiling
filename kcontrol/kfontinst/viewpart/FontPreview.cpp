////////////////////////////////////////////////////////////////////////////////
//
// Class Name    : CFontPreview
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
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
////////////////////////////////////////////////////////////////////////////////
// (C) Craig Drummond, 2001, 2002, 2003
////////////////////////////////////////////////////////////////////////////////

#include "FontPreview.h"
#include "Misc.h"
#include "Global.h"
#include "FontEngine.h"
#include <kapplication.h>
#include <klocale.h>
#include <qpainter.h>
#include <qimage.h>
#include <stdlib.h>

CFontPreview::CFontPreview(QWidget *parent, const char *name, const QString &str)
            : QWidget(parent, name),
              itsCurrentFace(1),
              itsLastWidth(0),
              itsLastHeight(0),
              itsString(str.isNull() ? i18n(" No preview available") : str),
              itsBgndCol(eraseColor())
{
}

void CFontPreview::showFont(const QString &file, int face)
{
    KURL url;

    url.setPath(CMisc::getDir(file));
    url.setFileName(CMisc::getFile(file));
    showFont(url, face);
}

void CFontPreview::showFont(const KURL &url, int face)
{
    itsCurrentUrl=url;
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

    if(CGlobal::fe().openKioFont(itsCurrentUrl.path(), CFontEngine::NAME, true, itsCurrentFace-1))
    {
        setEraseColor(Qt::white);
        CGlobal::fe().createPreview(itsLastWidth, itsLastHeight, itsPixmap, itsCurrentFace-1);
        update();
        emit status(true);
        CGlobal::fe().closeFont();
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
        paint.drawText(rect(), AlignCenter, itsString);
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
    return QSize(160, 200);
}

QSize CFontPreview::minimumSizeHint() const
{
    return QSize(32, 32);
}

#include "FontPreview.moc"
