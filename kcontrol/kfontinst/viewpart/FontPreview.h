#ifndef __FONT_PREVIEW_H__
#define __FONT_PREVIEW_H__

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

#include <qstring.h>
#include <qpixmap.h>
#include <qsize.h>
#include <qwidget.h>
#include <qcolor.h>
#include <kurl.h>

class CFontPreview : public QWidget
{
    Q_OBJECT

    public:

    CFontPreview(QWidget *parent, const char *name=NULL, int size=-1, bool wf=false);
    virtual ~CFontPreview() {}

    void            paintEvent(QPaintEvent *);
    QSize           sizeHint() const;
    QSize           minimumSizeHint() const;
    int             currentSize() const { return itsSize; }
    bool            waterfall() const   { return itsWaterfall; }

    void showFont(const KURL &url);
    void showSize(int size);
    void showWaterfall(bool wf);
    void showFont();

    public slots:

    void showFace(int face);

    signals:

    void status(bool);

    private:

    QPixmap itsPixmap;
    KURL    itsCurrentUrl;
    int     itsCurrentFace,
            itsLastWidth,
            itsLastHeight,
            itsSize;
    QColor  itsBgndCol;
    bool    itsWaterfall;
};

#endif
