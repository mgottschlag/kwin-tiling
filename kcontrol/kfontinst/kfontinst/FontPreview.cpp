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
#include "Misc.h"
#include <kapplication.h>
#include <klocale.h>
#include <qpainter.h>

#include <stdlib.h>

CFontPreview::CFontPreview(QWidget *parent, const char *name, const QString &str)
            : QWidget(parent, name),
              itsLastWidth(0),
              itsLastHeight(0),
              itsJob(NULL),
              itsString(QString::null==str ? i18n(" No preview available") : str)
{
}

void CFontPreview::showFont(const QString &file)
{
    KURL url;

    url.setPath(CMisc::getDir(file));
    url.setFileName(CMisc::getFile(file));
    itsCurrentUrl=url;
    showFont();
}

void CFontPreview::showFont()
{
    KURL::List urls;

    urls.append(itsCurrentUrl);
    itsLastWidth=width();
    itsLastHeight=height();
    itsJob=KIO::filePreview(urls, width(), height(), 0, 0, true, false);
    connect(itsJob, SIGNAL(result(KIO::Job *)), SLOT(result(KIO::Job *)));
    connect(itsJob, SIGNAL(gotPreview(const KFileItem *, const QPixmap &)), 
                    SLOT(gotPreview(const KFileItem *, const QPixmap &)));
    connect(itsJob, SIGNAL(failed(const KFileItem *)), SLOT(failed(const KFileItem *)));
}

void CFontPreview::result(KIO::Job *job)
{
    disconnect(job, SIGNAL(result(KIO::Job *)), this, SLOT(result(KIO::Job *)));
    disconnect(job, SIGNAL(gotPreview(const KFileItem *, const QPixmap &)), this,
                    SLOT(gotPreview(const KFileItem *, const QPixmap &)));
    disconnect(job, SIGNAL(failed(const KFileItem *)), this, SLOT(failed(const KFileItem *)));

    if(job==itsJob)
        itsJob=NULL;
}

void CFontPreview::gotPreview(const KFileItem *item, const QPixmap &pix)
{
    if(item->url()==itsCurrentUrl)
    {
        itsPixmap=pix;
        update();
        emit status(true);
    }
}

void CFontPreview::failed(const KFileItem *item)
{
    if(item->url()==itsCurrentUrl)
    {
        QPixmap nullPix;

        itsPixmap=nullPix;
        update();
        emit status(false);
    }
}

void CFontPreview::paintEvent(QPaintEvent *)
{
    QPainter paint( this );

    if(itsPixmap.isNull())
    {
        QRect r(rect());

        r.setX(r.x()+1);
        r.setY(r.y()+((height()-fontMetrics().height())/2));
        paint.setPen(kapp->palette().active().text());
        paint.drawText(r, AlignLeft, itsString);
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
    return QSize(192, 64);
}

QSize CFontPreview::minimumSizeHint() const
{
    return QSize(32, 32);
}

#include "FontPreview.moc"
