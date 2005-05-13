////////////////////////////////////////////////////////////////////////////////
//
// Class Name    : KFI::CSettingsDialog
// Author        : Craig Drummond
// Project       : K Font Installer
// Creation Date : 10/05/2005
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
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
////////////////////////////////////////////////////////////////////////////////
// (C) Craig Drummond, 2005
////////////////////////////////////////////////////////////////////////////////

#include "SettingsDialog.h"
#include "KfiConstants.h"
#include "Misc.h"
#include <qlayout.h>
#include <qcheckbox.h>
#include <qvbox.h>
#include <qwhatsthis.h>
#include <klocale.h>
#include <kconfig.h>
#include <kmessagebox.h>
#include <kio/job.h>
#include <kio/netaccess.h>

namespace KFI
{

CSettingsDialog::CSettingsDialog(QWidget *parent)
               : KDialogBase(parent, "settingsdialog", true, i18n("Settings"),
                             KDialogBase::Ok|KDialogBase::Cancel, KDialogBase::Ok, true)
{
    QVBox *page = makeVBoxMainWidget();

    itsDoX=new QCheckBox(i18n("Configure fonts for legacy X applications"), page);
    QWhatsThis::add(itsDoX, i18n("<p>Modern applications use a system called \"FontConfig\" to obtain the list of fonts. "
                                 "Older applications, such as OpenOffice 1.x, GIMP 1.x, etc. use the previous \"core X fonts\" mechanism for "
                                 "this.</p><p>Selecting this option will inform the installer to create the necessary files so that these "
                                 "older applications can use the fonts you install.</p><p>Please note, however, that this will slow down "
                                 "the installation process.<p>"));
    itsDoGs=new QCheckBox(i18n("Configure fonts for Ghostscript"), page);
    QWhatsThis::add(itsDoGs, i18n("<p>When printing, most applications create what is know as PostScript. This is then sent to a special "
                                  "application, named Ghostscript, which can interpret the PostScript and send the appropriate instructions "
                                  "to your printer. If your application does not embed whatever fonts it uses into the PostScript, then "
                                  "Ghostscript needs to be informed as to which fonts you have installed, and where they are located.</p>"
                                  "<p>Selecting this option will create the necessary Ghostscript config files.</p><p>Please note, however, "
                                  "that this will also slow down the installation process.</p><p>As most applications can, and do, embed "
                                  "the fonts into the PostScript before sending this to Ghostscript, this option can safely be disabled."));

    KConfig cfg(Misc::root() ? KFI_ROOT_CFG_FILE : KFI_CFG_FILE);

    itsDoX->setChecked(cfg.readBoolEntry(KFI_CFG_X_KEY, KFI_DEFAULT_CFG_X));
    itsDoGs->setChecked(cfg.readBoolEntry(KFI_CFG_GS_KEY, KFI_DEFAULT_CFG_GS));
}

void CSettingsDialog::slotOk()
{
    KConfig cfg(Misc::root() ? KFI_ROOT_CFG_FILE : KFI_CFG_FILE);

    bool oldDoX=cfg.readBoolEntry(KFI_CFG_X_KEY, KFI_DEFAULT_CFG_X),
         oldDoGs=cfg.readBoolEntry(KFI_CFG_GS_KEY, KFI_DEFAULT_CFG_GS);

    cfg.writeEntry(KFI_CFG_X_KEY, itsDoX->isChecked());
    cfg.writeEntry(KFI_CFG_GS_KEY, itsDoGs->isChecked());
    cfg.sync();

    if( ((!oldDoX && itsDoX->isChecked()) || (!oldDoGs && itsDoGs->isChecked())) &&
        KMessageBox::Yes==KMessageBox::questionYesNo(this, i18n("You have enabled a previously disabled option. Would you like the config "
                                                                "files updated now? (Normally they are only updated upon installing, or "
                                                                "removing, a font.)")))
    {
        QByteArray  packedArgs;
        QDataStream stream(packedArgs, IO_WriteOnly);

        stream << KFI::SPECIAL_RECONFIG;

        KIO::NetAccess::synchronousRun(KIO::special(KFI_KIO_FONTS_PROTOCOL ":/", packedArgs), this);
    }

    hide();
}

}
