/*
   Copyright (c) 2000 Matthias Elter <elter@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.

*/

#include <qhbox.h>
#include <qcursor.h>

#include <klocale.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <klibloader.h>
#include <global.h>

#include "kecdialog.h"
#include "kecdialog.moc"
#include "kcmodule.h"
#include "moduleinfo.h"
#include "modloader.h"

KExtendedCDialog::KExtendedCDialog(QWidget *parent, const char *name, bool modal)
  : KDialogBase(IconList, i18n("Settings"), Help | User1 |Cancel | Apply | Ok, Ok,
                parent, name, modal, true, i18n("&Defaults"))
{
    enableButton(Apply, false);
    connect(this, SIGNAL(aboutToShowPage(QWidget *)), this, SLOT(aboutToShow(QWidget *)));
    setInitialSize(QSize(640,480));
}

KExtendedCDialog::~KExtendedCDialog()
{
    moduleDict.setAutoDelete(true);
}

void KExtendedCDialog::slotUser1()
{
    int curPageIndex = activePageIndex();
    for (KCModule* module = modules.first(); module != 0; module = modules.next())
    {
       if (pageIndex((QWidget *)module->parent()) == curPageIndex)
       {
          module->defaults();
          clientChanged(true);
          return;
       }
    }
}

void KExtendedCDialog::slotApply()
{
    for (KCModule* module = modules.first(); module != 0; module = modules.next())
        module->save();
    clientChanged(false);
}


void KExtendedCDialog::slotOk()
{
    for (KCModule* module = modules.first(); module != 0; module = modules.next())
        module->save();
    accept();
}

void KExtendedCDialog::clientChanged(bool state)
{
    enableButton(Apply, state);
}

void KExtendedCDialog::addModule(const QString& path, bool withfallback)
{
    kdDebug(1208) << "KExtendedCDialog::addModule " << path << endl;

    ModuleInfo info(path);

    QHBox* page = addHBoxPage(info.name(), info.comment(),
                              KGlobal::iconLoader()->loadIcon(info.icon(), KIcon::Desktop, KIcon::SizeMedium));
    if(!page) {
        ModuleLoader::unloadModule(info);
        return;
    }
    moduleDict.insert(page, new LoadInfo(path, withfallback));
    if (modules.isEmpty())
       aboutToShow(page);
}

void KExtendedCDialog::aboutToShow(QWidget *page)
{
    LoadInfo *loadInfo = moduleDict[page];
    if (!loadInfo)
       return;

    QApplication::setOverrideCursor(Qt::WaitCursor);

    moduleDict.remove(page);

    ModuleInfo info(loadInfo->path);

    KCModule *module = ModuleLoader::loadModule(info, loadInfo->withfallback);


    if (!module)
    {
        QApplication::restoreOverrideCursor();
        KMessageBox::error(this, i18n("There was an error loading module\n'%1'\nThe diagnostics is:\n%2")
                           .arg(loadInfo->path).arg(KLibLoader::self()->lastErrorMessage()));
        delete loadInfo;
        return;
    }

    module->reparent(page,0,QPoint(0,0),true);
    connect(module, SIGNAL(changed(bool)), this, SLOT(clientChanged(bool)));
    //setHelp( docpath, QString::null );
    modules.append(module);

    KCGlobal::repairAccels( topLevelWidget() );

    delete loadInfo;

    QApplication::restoreOverrideCursor();
}
