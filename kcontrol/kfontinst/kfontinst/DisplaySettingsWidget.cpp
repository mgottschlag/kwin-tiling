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

#include "DisplaySettingsWidget.h"
#include "KfiGlobal.h"
#include "Config.h"
#include <qradiobutton.h>
#include <qcheckbox.h>
#include <qlineedit.h>

CDisplaySettingsWidget::CDisplaySettingsWidget(QWidget *parent, const char *name)
                      : CDisplaySettingsWidgetData(parent, name)
{
    itsAdvanced->setChecked(CKfiGlobal::cfg().getAdvancedMode());
    if(CKfiGlobal::cfg().getFontListsOrientation()==Qt::Vertical)
        itsTopAndBottom->setChecked(true);
    else
        itsLeftAndRight->setChecked(true);

    itsCustomCheck->setChecked(CKfiGlobal::cfg().getUseCustomPreviewStr());
    itsCustomText->setText(CKfiGlobal::cfg().getCustomPreviewStr());
}

void CDisplaySettingsWidget::advancedSelected(bool on)
{
    CKfiGlobal::cfg().setAdvancedMode(on);
}

void CDisplaySettingsWidget::topAndBottomSelected(bool on)
{ 
    Qt::Orientation newOrient=on ? Qt::Vertical : Qt::Horizontal;

    if(newOrient!=CKfiGlobal::cfg().getFontListsOrientation())
        CKfiGlobal::cfg().setFontListsOrientation(newOrient); 
}

void CDisplaySettingsWidget::textChanged(const QString &str)
{
    CKfiGlobal::cfg().setCustomPreviewStr(str);
}

void CDisplaySettingsWidget::customStrChecked(bool on)
{
    CKfiGlobal::cfg().setUseCustomPreviewStr(on);
}
