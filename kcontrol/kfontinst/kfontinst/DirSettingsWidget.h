#ifndef __DIR_SETTINGS_WIDGET_H__
#define __DIR_SETTINGS_WIDGET_H__

////////////////////////////////////////////////////////////////////////////////
//
// Class Name    : CDirSettingsWidget
// Author        : Craig Drummond
// Project       : K Font Installer (kfontinst-kcontrol)
// Creation Date : 29/04/2001
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

#include "DirSettingsWidgetData.h"

class CDirSettingsWidget : public CDirSettingsWidgetData
{
    Q_OBJECT

    public:

    CDirSettingsWidget(QWidget *parent, const char *name=NULL);
    virtual ~CDirSettingsWidget() {}

    void encodingsDirButtonPressed();
    void gsFontmapButtonPressed();
    void xDirButtonPressed();
    void xConfigButtonPressed();
    void xftConfigFileButtonPressed();
    void t1SubDir(const QString &str);
    void ttSubDir(const QString &str);
    void ghostscriptChecked(bool on);
    void xftChecked(bool on);
    void setGhostscriptFile(const QString &f);
    void setXConfigFile(const QString &f);

    signals:

    void encodingsDirChanged();

    private:

    void setupSubDirCombos();
};

#endif
