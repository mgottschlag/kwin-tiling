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
#include "SettingsWidget.h"
#include "KfiGlobal.h"
#include "Config.h"
#include "Misc.h"
#include <klocale.h>
#include <qwizard.h>
#include <qtextedit.h>
#include <qlabel.h>
#include <fstream.h>
#include <stdio.h>

CSettingsWizard::CSettingsWizard(QWidget *parent, const char *name)
               : CSettingsWizardData(parent, name, true)
{
    if(CMisc::root())
    {
        itsNonRootText->hide();

        QString genTxt=itsGenText->text();

        itsGenText->setText(genTxt+i18n("\n\nIf \"%1\" is listed as the CUPS folder, it is probable that you are not using the CUPS"
                                        " printing system - in which case just ensure that the checkbox is not selected.").arg(
                            i18n(CConfig::constNotFound.utf8())));
    }

    this->setFinishEnabled(itsCompletePage, true);
}
