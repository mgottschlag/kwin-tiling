#ifndef __FONT_PREVIEW_H__
#define __FONT_PREVIEW_H__

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

#include <qstring.h>
#include <qpixmap.h>
#include <qsize.h>
#include <qwidget.h>
#include <kurl.h>
#include <kio/job.h>
#include <kio/previewjob.h>
#include <kfileitem.h>

class CFontPreview : public QWidget
{
    Q_OBJECT

    public:

    CFontPreview(QWidget *parent, const char *name=NULL, const QString &str=QString::null);
    virtual ~CFontPreview() {}

    void  paintEvent(QPaintEvent *);
    QSize sizeHint() const;
    QSize minimumSizeHint() const;

    public slots:

    void showFont(const QString &file);
    void result(KIO::Job *job);
    void gotPreview(const KFileItem *item, const QPixmap &pix);
    void failed(const KFileItem *item);

    signals:

    void status(bool);

    private:

    void showFont();

    private:

    QPixmap         itsPixmap;
    KURL            itsCurrentUrl;
    int             itsLastWidth,
                    itsLastHeight;
    KIO::PreviewJob *itsJob;
    QString         itsString;
};

#endif
