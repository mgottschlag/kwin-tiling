#ifndef __DISK_FONT_LIST_WIDGET_H__
#define __DISK_FONT_LIST_WIDGET_H__
 
////////////////////////////////////////////////////////////////////////////////
//
// Class Name    : CDiskFontListWidget
// Author        : Craig Drummond
// Project       : K Font Installer (kfontinst-kcontrol)
// Creation Date : 21/04/2001
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
 
#include "FontListWidget.h"

class CDiskFontListWidget : public CFontListWidget
{
    Q_OBJECT

    public:

    enum EStatus
    {
        SUCCESS,
        PERMISSION_DENIED,
        ALREADY_INSTALLED,
        HAS_SUB_DIRS,
        COULD_NOT_CREATE_DIR
    };

    CDiskFontListWidget(QWidget *parent, const char *name=NULL);

    virtual ~CDiskFontListWidget() {}

    void rescan(bool advancedMode);

    public slots:

    void changeDirectory();
    void install();
    void setDestDir(const QString &dir);
    void setDestDirFromFontSel(const QString &dir, const QString &) { setDestDir(dir); }
    void selectionChanged();
    void fontMoved(const QString &font, const QString &from, const QString &to);
    void dirMoved(const QString &top, const QString &sub);

    signals:

    void installFont(const QString &dir, const QString &file);
    void installDir(const QString &top, const QString &sub);

    private:

    EStatus install(const QString &sourceDir, const QString &destDir, const QString &fname);
    EStatus installDir(const QString &sourceDir, const QString &destDir, const QString &sub);
    QString statusToStr(EStatus status);

    private:

    QString itsDestDir;
};

#endif
