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

#include <qsplitter.h>

#include <kaction.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kkeydialog.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kservice.h>
#include <kstdaction.h>
#include <kstdaccel.h>

#include "treeview.h"
#include "basictab.h"
#include "kmenuedit.h"
#include "kmenuedit.moc"

KMenuEdit::KMenuEdit (QWidget *, const char *name)
  : KMainWindow (0, name), m_tree(0), m_basicTab(0), m_splitter(0)
{
    setCaption(i18n("Edit K Menu"));

    // restore size
    KConfig *config = KGlobal::config();
    config->setGroup("General");
    int w = config->readNumEntry("Width",640);
    int h = config->readNumEntry("Height", 480);
    resize(w, h);
#if 0
    m_showHidden = config->readBoolEntry("ShowHidden");
#else
    m_showHidden = false;
#endif

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
    config->writeEntry("SplitterSizes", m_splitter->sizes());

    config->sync();
}

void KMenuEdit::setupActions()
{
    (void)new KAction(i18n("&New Submenu..."), "menu_new", 0, actionCollection(), "newsubmenu");
    (void)new KAction(i18n("New &Item..."), "filenew", KStdAccel::openNew(), actionCollection(), "newitem");

    m_actionDelete = 0;

    KStdAction::save(this, SLOT( slotSave() ), actionCollection());
    KStdAction::quit(this, SLOT( close() ), actionCollection());
    KStdAction::cut(0, 0, actionCollection());
    KStdAction::copy(0, 0, actionCollection());
    KStdAction::paste(0, 0, actionCollection());
    KStdAction::keyBindings( this, SLOT( slotConfigureKeys() ), actionCollection() );
}

void KMenuEdit::slotConfigureKeys()
{
  KKeyDialog::configure( actionCollection(), this );
}

void KMenuEdit::setupView()
{
    m_splitter = new QSplitter(Horizontal, this);
    m_tree = new TreeView(actionCollection(), m_splitter);
    m_basicTab = new BasicTab(m_splitter);

    connect(m_tree, SIGNAL(entrySelected(MenuFolderInfo *)),
            m_basicTab, SLOT(setFolderInfo(MenuFolderInfo *)));
    connect(m_tree, SIGNAL(entrySelected(MenuEntryInfo *)),
            m_basicTab, SLOT(setEntryInfo(MenuEntryInfo *)));
    connect(m_tree, SIGNAL(disableAction()),
            m_basicTab, SLOT(slotDisableAction() ) );

    connect(m_basicTab, SIGNAL(changed(MenuFolderInfo *)),
            m_tree, SLOT(currentChanged(MenuFolderInfo *)));

    connect(m_basicTab, SIGNAL(changed(MenuEntryInfo *)),
            m_tree, SLOT(currentChanged(MenuEntryInfo *)));

    connect(m_basicTab, SIGNAL(findServiceShortcut(const KShortcut&, KService::Ptr &)),
            m_tree, SLOT(findServiceShortcut(const KShortcut&, KService::Ptr &)));

    // restore splitter sizes
    KConfig* config = KGlobal::config();
    QValueList<int> sizes = config->readIntListEntry("SplitterSizes");

    if (sizes.isEmpty())
	sizes << 1 << 3;
    m_splitter->setSizes(sizes);
    m_tree->setFocus();

    setCentralWidget(m_splitter);
}

void KMenuEdit::slotChangeView()
{
#if 0
    m_showHidden = m_actionShowHidden->isChecked();
#else
    m_showHidden = false;
#endif

    // disabling the updates prevents unnecessary redraws
    setUpdatesEnabled( false );
    guiFactory()->removeClient( this );

    delete m_actionDelete;

    m_actionDelete = new KAction(i18n("&Delete"), "editdelete", Key_Delete, actionCollection(), "delete");

    if (!m_splitter)
       setupView();
    createGUI("kmenueditui.rc");

    m_tree->setViewMode(m_showHidden);
}

void KMenuEdit::slotSave()
{
    m_tree->save();
}

bool KMenuEdit::queryClose()
{
    if (!m_tree->dirty()) return true;

    int result = KMessageBox::warningYesNoCancel(this,
                    i18n("You have made changes to the menu.\n"
                         "Do you want to save the changes or discard them?"),
                    i18n("Save Menu Changes?"),
                    KStdGuiItem::save(), KStdGuiItem::discard() );

    switch(result)
    {
      case KMessageBox::Yes:
         m_tree->save();
         return true;

      case KMessageBox::No:
         return true;

      default:
         break;
    }
    return false;
}

