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
 *   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */

#include <kglobal.h>
#include <kconfig.h>
#include <klocale.h>
#include <kaction.h>
#include <kstdaction.h>
#include <kstdaccel.h>
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
    m_view = 0;
    // restore size
    KConfig *config = KGlobal::config();
    config->setGroup("General");
    int w = config->readNumEntry("Width",640);
    int h = config->readNumEntry("Height", 480);
    resize(w, h);
    m_showHidden = config->readBoolEntry("ShowHidden");

    // setup GUI
    setupActions();
    slotChangeView();

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
    (void)new KAction(i18n("&New Submenu..."), "menu_new", 0, actionCollection(), "newsubmenu");
    (void)new KAction(i18n("New &Item..."), "filenew", KStdAccel::key(KStdAccel::New), actionCollection(), "newitem");

    m_actionDelete = 0;
    m_actionUndelete = 0;

    m_actionShowHidden = new KToggleAction(i18n("Show &Hidden Items"), KShortcut(), this, SLOT( slotChangeView()), actionCollection(), "show_hidden");
    m_actionShowHidden->setChecked(m_showHidden);

    KStdAction::quit(this, SLOT( slotClose() ), actionCollection());
    KStdAction::cut(0, 0, actionCollection());
    KStdAction::copy(0, 0, actionCollection());
    KStdAction::paste(0, 0, actionCollection());
}

void KMenuEdit::setupView()
{
    m_view = new MenuEditView(actionCollection(), this);
    setCentralWidget(m_view);
}

void KMenuEdit::slotChangeView()
{
    m_showHidden = m_actionShowHidden->isChecked();

    // disabling the updates prevents unnecessary redraws
    setUpdatesEnabled( false );
    guiFactory()->removeClient( this );

    if (m_actionDelete)
    {
       delete m_actionDelete;
       m_actionDelete = 0;
    }
    if (m_actionUndelete)
    {
       delete m_actionUndelete;
       m_actionUndelete = 0;
    }

    m_actionDelete = new KAction(i18n("&Delete"), "editdelete", Key_Delete, actionCollection(), "delete");
    if (m_showHidden)
    {
       m_actionUndelete = new KAction(i18n("&Re-add"), "undo", KStdAccel::key(KStdAccel::Undo), actionCollection(), "undelete");
    }

    if (!m_view)
       setupView();
    createGUI("kmenueditui.rc");

    // make it look nice
    toolBar(0)->setIconText(KToolBar::IconTextBottom);

    m_view->setViewMode(m_showHidden);
}

void KMenuEdit::slotClose()
{
    DCOPClient *dcc = kapp->dcopClient();
    if ( !dcc->isAttached() )
        dcc->attach();
    dcc->send("kded", "kbuildsycoca", "recreate()", QByteArray());
    close();
}
