#ifndef __SYS_CONFIGURER_H__
#define __SYS_CONFIGURER_H__

////////////////////////////////////////////////////////////////////////////////
//
// Class Name    : CSysConfigurer
// Author        : Craig Drummond
// Project       : K Font Installer (kfontinst-kcontrol)
// Creation Date : 03/05/2001
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

#include "AfmCreator.h"
#include "FontmapCreator.h"
#include "StarOfficeConfig.h"
#include <qobject.h>
#include <qstring.h>

class QStringList;
class QWidget;

class CSysConfigurer : public QObject
{
    Q_OBJECT

    public:

    CSysConfigurer(QWidget *parent=NULL);
    ~CSysConfigurer() {}

    static void getTTandT1Dirs(QStringList &list);
    void        go();
    void        status(const QString &str, const QString &errorMsg=QString::null, bool error=false);
    QWidget *   parent() { return itsParent; }

    public slots:

    void        step(const QString &) { status(QString::null); }

    signals:

    void        initProgress(const QString &title, int numSteps);
    void        progress(const QString &step);
    void        stopProgress();
    void        successful();

    private:

    QWidget           *itsParent;
    CAfmCreator       itsAfm;
    CStarOfficeConfig itsSo;
    CFontmapCreator   itsFm;
};

#endif
