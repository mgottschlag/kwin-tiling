////////////////////////////////////////////////////////////////////////////////
//
// Class Name    : CKfiMainWidget
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "KfiMainWidget.h"
#include "KfiGlobal.h"
#include "Config.h"
#include "FontsWidget.h"
#include "SettingsWidget.h"
#include "XftConfigSettingsWidget.h"
#include <qtabwidget.h>

CKfiMainWidget::CKfiMainWidget(QWidget *parent, const char *)
              : CKfiMainWidgetData(parent)
{
    connect(itsTab, SIGNAL(currentChanged(QWidget*)), this, SLOT(tabChanged(QWidget *)));
    connect(itsFonts, SIGNAL(progressActive(bool)), itsSettings, SLOT(setDisabled(bool)));
    connect(itsSettings, SIGNAL(madeChanges()), itsFonts, SLOT(enableCfgButton()));
#ifdef HAVE_XFT
    connect(itsAA, SIGNAL(madeChanges()), itsFonts, SLOT(enableCfgButton()));
    connect(itsAA, SIGNAL(savedChanges()), itsFonts, SLOT(setCfgButton()));
    connect(itsFonts, SIGNAL(configuredSystem()), itsAA, SLOT(disableSaveButton()));
#else
    itsTab->removePage(itsAATab);
#endif
}

void CKfiMainWidget::tabChanged(QWidget *tab)
{
    if(tab==itsFontsTab)
    {
        itsFonts->setOrientation(CKfiGlobal::cfg().getFontListsOrientation());
        itsFonts->rescan();
    }
}

void CKfiMainWidget::scanFonts()
{
    itsFonts->scanDirs();
}

void CKfiMainWidget::configureSystem()
{
    itsTab->showPage(itsFontsTab);
    itsFonts->configureSystem();
}
