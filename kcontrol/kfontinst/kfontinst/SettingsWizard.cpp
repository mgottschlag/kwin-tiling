////////////////////////////////////////////////////////////////////////////////
//
// Class Name    : CSettingsWizard
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
 
#include "SettingsWizard.h"
#include "StarOfficeSettingsWidget.h"
#include "KfiGlobal.h"
#include "Config.h"
#include "Misc.h"
#include <qwizard.h>
#include <qlabel.h>

CSettingsWizard::CSettingsWizard(QWidget *parent, const char *name)
               : CSettingsWizardData(parent, name, true)
{
    if(CMisc::root())
        itsNonRootText->hide();

    itsSOWidget->hideNote();

#ifndef HAVE_XFT
    removePage(itsAAPage);
#endif

/*
    if(CConfig::constNotFound!=CKfiGlobal::cfg().getFontsDir() && CConfig::constNotFound!=CKfiGlobal::cfg().getGhostscriptFile() &&
       CConfig::constNotFound!=CKfiGlobal::cfg().getXConfigFile())
        removePage(itsDirsAndFilesPage);
*/

    this->setFinishEnabled(itsStarOfficePage, true);
}
