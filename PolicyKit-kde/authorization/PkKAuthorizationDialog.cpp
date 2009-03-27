/***************************************************************************
 *   Copyright (C) 2008 by Daniel Nicoletti                                *
 *   dantti85-pk@yahoo.com.br                                              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#include "PkKAuthorizationDialog.h"

#include <KCmdLineArgs>
#include <KConfig>
#include <KDebug>

#include <KDialog>
#include "PkKAuthorization.h"

namespace PolkitKde
{

PkKAuthorizationDialog::PkKAuthorizationDialog()
        : KUniqueApplication()
{
    dialog = new KDialog;
    dialog->setWindowIcon(KIcon("object-locked"));
    dialog->setButtons(KDialog::Close);
    dialog->setCaption(i18n("Authorizations"));

    PkKAuthorization *pkAuthorization = new PkKAuthorization;
    connect(this, SIGNAL(newAction(QString)), pkAuthorization, SLOT(newAction(QString)));
    dialog->setMainWidget(pkAuthorization);

    //load dialog settings
    KConfig config;
    KConfigGroup dialogSettingsGroup(&config, "DialogSettings");
    dialog->restoreDialogSize(dialogSettingsGroup);
    dialog->show();
}

PkKAuthorizationDialog::~PkKAuthorizationDialog()
{
    KConfig config;
    KConfigGroup dialogSettingsGroup(&config, "DialogSettings");
    dialog->saveDialogSize(dialogSettingsGroup);
//     PkKAuthorization::instance()->setParent(0);//what does this?
    delete dialog;
}

int PkKAuthorizationDialog::newInstance()
{
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

    if (args->count()) {
        // we only suport one item
        emit newAction(args->arg(0));
    }

    args->clear();
    return 0;
}


}

#include "PkKAuthorizationDialog.moc"
