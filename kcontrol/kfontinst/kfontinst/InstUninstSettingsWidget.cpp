////////////////////////////////////////////////////////////////////////////////
//
// Class Name    : CInstUninstSettingsWidget
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

#include "InstUninstSettingsWidget.h"
#include "KfiGlobal.h"
#include "Config.h"
#include <kglobal.h>
#include <kiconloader.h>
#include <qpushbutton.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qradiobutton.h>
#include <qcheckbox.h>
#include <kfiledialog.h>

CInstUninstSettingsWidget::CInstUninstSettingsWidget(QWidget *parent, const char *name)
                         : CInstUninstSettingsWidgetData(parent, name)
{
    itsUninstallDirButton->setPixmap(KGlobal::iconLoader()->loadIcon("fileopen", KIcon::Small));
    itsFixTtfPsNamesUponInstall->setChecked(CKfiGlobal::cfg().getFixTtfPsNamesUponInstall());

    if(CKfiGlobal::cfg().getUninstallIsDelete())
        itsDeleteRadio->setChecked(true);
    else
        itsMoveRadio->setChecked(true);

    itsUninstallDirText->setText(CKfiGlobal::cfg().getUninstallDir());
}
 
void CInstUninstSettingsWidget::fixTtfNamesSelected(bool on)
{
    CKfiGlobal::cfg().setFixTtfPsNamesUponInstall(on);
}

void CInstUninstSettingsWidget::moveToSelected(bool on)
{
    CKfiGlobal::cfg().setUninstallIsDelete(!on);
}

void CInstUninstSettingsWidget::uninstallDirButtonPressed()
{
    QString dir=KFileDialog::getExistingDirectory(itsUninstallDirText->text(), this, "Select Uninstall Folder");
 
    if(QString::null!=dir && dir!=itsUninstallDirText->text())
    {
        itsUninstallDirText->setText(dir);
        CKfiGlobal::cfg().setUninstallDir(dir);
    }
}
