#ifndef __INSTALLED_FONT_LIST_WIDGET_H__
#define __INSTALLED_FONT_LIST_WIDGET_H__
 
////////////////////////////////////////////////////////////////////////////////
//
// Class Name    : CInstalledFontListWidget
// Author        : Craig Drummond
// Project       : K Font Installer (kfontinst-kcontrol)
// Creation Date : 20/04/2001
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
#include <qpushbutton.h>

class QPopupMenu;
 
class CInstalledFontListWidget : public CFontListWidget
{
    Q_OBJECT

    public:

    enum EStatus
    {
        SUCCESS,
        PERMISSION_DENIED,
        HAS_SUB_DIRS,
        COULD_NOT_CREATE_DIR,
        COULD_NOT_DELETE_DIR
    };

    CInstalledFontListWidget(QWidget *parent, const char *name=NULL);

    virtual ~CInstalledFontListWidget() {}

    QString currentDir();
    void    rescan(bool advancedMode, const QString &dir1);
    void    enableCfgButton() { setCfgButtonState(true); }
    void    setCfgButtonState(bool state);

    public slots:

    void configure();
    void uninstall();
    void addFont(const QString &path, const QString &file);
    void addSubDir(const QString &top, const QString &sub);
    void disableCfgButton()   { setCfgButtonState(false); }
    void popupMenu(QListViewItem *item, const QPoint &point, int column);
    void fixTtfPsNames();
    void toggleDir();
    void touchDir();
    void createDir();
    void deleteDir();
    void toggleUnscaled();
    void setCfgButton();
    void selectionChanged();

    signals:

    void configureSystem();
    void fontMoved(const QString &font, const QString &from, const QString &to);
    void dirMoved(const QString &top, const QString &sub);

    private:

    EStatus uninstall(const QString &dir, const QString &sub, const QString &file, bool deleteAfm=true);
    EStatus uninstallDir(const QString &top, const QString &sub);
    QString statusToStr(EStatus status);

    private:

    QPopupMenu *itsFontsPopup,
               *itsDirsPopup;
    int        itsFixTtfPsNamesME,
               itsEnableDirME,
               itsDisableDirME,
               itsCreateDirME,
               itsDeleteDirME,
               itsSetScaledME,
               itsSetUnscaledME,
               itsTouchME;
};

#endif
