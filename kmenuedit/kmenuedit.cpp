/*
 *   Copyright (C) 2000 Matthias Elter <elter@kde.org>
 *   Copyright (C) 2001-2002 Raffaele Sandrini <sandrini@kde.org)
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include <kglobal.h>
#include <kconfig.h>
#include <klocale.h>
#include <kaction.h>
#include <kstdaction.h>
#include <kdebug.h>
#include <dcopclient.h>
#include <kapplication.h>

#include "menueditview.h"
#include "kmenuedit.h"
#include "kmenuedit.moc"

KMenuEdit::KMenuEdit (QWidget *, const char *name)
  : KMainWindow (0, name)
{
    setCaption(i18n("Edit K Menu"));

    // restore size
    KConfig *config = KGlobal::config();
    config->setGroup("General");
    int w = config->readNumEntry("Width",640);
    int h = config->readNumEntry("Height", 480);
    resize(w, h);

    // setup GUI
    setupActions();
    setupView();
    createGUI("kmenueditui.rc");

    // make it look nice
    toolBar(0)->setIconText(KToolBar::IconTextBottom);
}

KMenuEdit::~KMenuEdit()
{
    // save size
    KConfig *config = KGlobal::config();
    config->setGroup("General");
    config->writeEntry("Width", width());
    config->writeEntry("Height", height());
    config->sync();
}

void KMenuEdit::setupActions()
{
    (void)new KAction(i18n("&New Submenu"), "menu_new", 0, actionCollection(), "newsubmenu");
    (void)new KAction(i18n("New &Item"), "filenew", 0, actionCollection(), "newitem");
    (void)new KAction(i18n("&Delete"), "edittrash", 0, actionCollection(), "delete");
    (void)new KAction(i18n("&Hide"), "editdelete", 0, actionCollection(), "hide");
    (void)new KAction(i18n("&Unhide"), "redo", 0, actionCollection(), "unhide");

    KStdAction::quit(this, SLOT( slotClose() ), actionCollection());
    KStdAction::cut(0, 0, actionCollection());
    KStdAction::copy(0, 0, actionCollection());
    KStdAction::paste(0, 0, actionCollection());
}

void KMenuEdit::setupView()
{
    _view = new MenuEditView(actionCollection(), this);
    setCentralWidget(_view);
}

void KMenuEdit::slotClose()
{
    DCOPClient *dcc = kapp->dcopClient();
    if ( !dcc->isAttached() )
        dcc->attach();
    dcc->send("kded", "kbuildsycoca", "recreate()", QByteArray());
    close();
}
