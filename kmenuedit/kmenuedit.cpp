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
    m_showHidden = config->readBoolEntry("ShowHidden");
    m_showRemoved = config->readBoolEntry("ShowRemoved");

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
    (void)new KAction(i18n("&New Submenu"), "menu_new", 0, actionCollection(), "newsubmenu");
    (void)new KAction(i18n("New &Item"), "filenew", 0, actionCollection(), "newitem");

    m_actionDelete = 0;
    m_actionUndelete = 0;
    
    m_actionHide = 0; 
    m_actionUnhide = 0;
    
    m_actionShowRemoved = new KToggleAction(i18n("Show &Removed Items"), KShortcut(), this, SLOT( slotChangeView()), actionCollection(), "show_removed");
    m_actionShowRemoved->setChecked(m_showRemoved);

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
    m_showRemoved = m_actionShowRemoved->isChecked();

    // disabling the updates prevents unnecessary redraws
    setUpdatesEnabled( false );
    guiFactory()->removeClient( this );

    if (m_actionDelete)
    {
//       m_actionDelete->unplugAll();
       delete m_actionDelete; 
       m_actionDelete = 0;
    }
    if (m_actionUndelete)
    {
//       m_actionUndelete->unplugAll();
       delete m_actionUndelete; 
       m_actionUndelete = 0;
    }
    if (m_actionHide)
    {
//       m_actionHide->unplugAll();
       delete m_actionHide; 
       m_actionHide = 0;
    }
    if (m_actionUnhide)
    {
//       m_actionUnhide->unplugAll();
       delete m_actionUnhide; 
       m_actionUnhide = 0;
    }
    
    m_actionDelete = new KAction(i18n("&Remove"), "editdelete", 0, actionCollection(), "delete");
    if (m_showRemoved)
    {
       m_actionUndelete = new KAction(i18n("&Re-add"), "undo", 0, actionCollection(), "undelete");
    }
    
    if (m_showHidden)
    {
       // Need better icons!
       m_actionHide = new KAction(i18n("&Hide"), "editdelete", 0, actionCollection(), "hide");
       m_actionUnhide = new KAction(i18n("Unhid&e"), "redo", 0, actionCollection(), "unhide");
    }

    if (!m_view)
       setupView();
    createGUI("kmenueditui.rc");

    // make it look nice
    toolBar(0)->setIconText(KToolBar::IconTextBottom);
    
    m_view->setViewMode(m_showRemoved, m_showHidden);
}

void KMenuEdit::slotClose()
{
    DCOPClient *dcc = kapp->dcopClient();
    if ( !dcc->isAttached() )
        dcc->attach();
    dcc->send("kded", "kbuildsycoca", "recreate()", QByteArray());
    close();
}
