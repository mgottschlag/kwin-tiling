#ifndef __DISPLAY_SETTINGS_WIDGET_H__
#define __DISPLAY_SETTINGS_WIDGET_H__

////////////////////////////////////////////////////////////////////////////////
//
// Class Name    : CDisplaySettingsWidget
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

#include "DisplaySettingsWidgetData.h"

class CDisplaySettingsWidget : public CDisplaySettingsWidgetData
{
    public:

    CDisplaySettingsWidget(QWidget *parent, const char *name=NULL);

    virtual ~CDisplaySettingsWidget() {}

    void advancedSelected(bool on);
    void topAndBottomSelected(bool on);
    void textChanged(const QString &str);
    void customStrChecked(bool on);
};

#endif
