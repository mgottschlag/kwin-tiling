/*
 *   Copyright (C) 2000 Matthias Elter <elter@kde.org>
 *   Copyright (C) 2001-2002 Raffaele Sandrini <sandrini@kde.org>
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
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */
#include "kmenuedit.h"

#include <QSplitter>

#include <kaction.h>
#include <kactioncollection.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kedittoolbar.h>
#include <kglobal.h>
#include <kicon.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kservice.h>
#include <kstandardaction.h>
#include <kstandardshortcut.h>
#include <kxmlguifactory.h>
#include <sonnet/configdialog.h>
#include "treeview.h"
#include "basictab.h"
#include "preferencesdlg.h"
#include "kmenueditadaptor.h"

#include "kmenuedit.moc"

KMenuEdit::KMenuEdit ()
  : KXmlGuiWindow (0), m_tree(0), m_basicTab(0), m_splitter(0)
{
    ( void )new KmenueditAdaptor(this);
    QDBusConnection::sessionBus().registerObject("/KMenuEdit", this);
    KConfigGroup grp( KGlobal::config(), "General" );
    m_showHidden = grp.readEntry("ShowHidden", false);

    // setup GUI
    setupActions();
    slotChangeView();
}

KMenuEdit::~KMenuEdit()
{
    KConfigGroup config(KGlobal::config(), "General");
    config.writeEntry("SplitterSizes", m_splitter->sizes());

    config.sync();
}

void KMenuEdit::setupActions()
{
    KAction *action = actionCollection()->addAction("newsubmenu");
    action->setIcon(KIcon("menu_new"));
    action->setText(i18n("&New Submenu..."));
    action = actionCollection()->addAction("newitem");
    action->setIcon(KIcon("document-new")) ;
    action->setText(i18n("New &Item..."));
    action->setShortcuts(KStandardShortcut::openNew());
    action = actionCollection()->addAction("newsep");
    action->setIcon(KIcon("menu_new_sep"));
    action->setText(i18n("New S&eparator"));

    m_actionDelete = 0;

    actionCollection()->addAction(KStandardAction::Save, this, SLOT( slotSave() ));
    actionCollection()->addAction(KStandardAction::Quit, this, SLOT( close() ));
    actionCollection()->addAction(KStandardAction::Cut);
    actionCollection()->addAction(KStandardAction::Copy);
    actionCollection()->addAction(KStandardAction::Paste);

    action = new KAction( i18n("Restore to System Menu"), this );
    actionCollection()->addAction( "restore_system_menu", action );
    connect( action, SIGNAL(triggered(bool) ), SLOT(slotRestoreMenu()) );
    KStandardAction::preferences( this, SLOT( slotConfigure() ), actionCollection() );
}

void KMenuEdit::slotConfigure()
{
    PreferencesDialog dlg( this );
    if ( dlg.exec() )
    {
        KConfigGroup grp( KGlobal::config(), "General" );
        bool newShowHiddenValue = grp.readEntry("ShowHidden", false);
        if ( newShowHiddenValue != m_showHidden )
        {
            m_showHidden = newShowHiddenValue;
            m_tree->updateTreeView(m_showHidden);
            m_basicTab->updateHiddenEntry( m_showHidden );
        }
    }
}

void KMenuEdit::setupView()
{
    m_splitter = new QSplitter;
    m_splitter->setOrientation(Qt::Horizontal);
    m_tree = new TreeView(actionCollection());
    m_splitter->addWidget(m_tree);
    m_basicTab = new BasicTab;
    m_splitter->addWidget(m_basicTab);

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
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup generalGroup(config, "General");
    QList<int> sizes = generalGroup.readEntry("SplitterSizes",QList<int>());

    if (sizes.isEmpty())
        sizes << 1 << 3;
    m_splitter->setSizes(sizes);
    m_tree->setFocus();

    setCentralWidget(m_splitter);
}

void KMenuEdit::slotChangeView()
{
    guiFactory()->removeClient( this );

    delete m_actionDelete;

    m_actionDelete = actionCollection()->addAction("delete");
    m_actionDelete->setIcon(KIcon("edit-delete"));
    m_actionDelete->setText(i18n("&Delete"));
    m_actionDelete->setShortcut(QKeySequence(Qt::Key_Delete));

    if (!m_splitter)
       setupView();
    setupGUI(KXmlGuiWindow::ToolBar|Keys|Save|Create, "kmenueditui.rc");

    m_tree->setViewMode(m_showHidden);
    m_basicTab->updateHiddenEntry( m_showHidden );
}

void KMenuEdit::slotSave()
{
    m_tree->save();

}

bool KMenuEdit::queryClose()
{
    if (!m_tree->dirty()) return true;


    int result;
    result = KMessageBox::warningYesNoCancel(this,
                                             i18n("You have made changes to the menu.\n"
                         "Do you want to save the changes or discard them?"),
                                             i18n("Save Menu Changes?"),
                                             KStandardGuiItem::save(), KStandardGuiItem::discard() );

    switch(result)
    {
      case KMessageBox::Yes:
         return m_tree->save();

      case KMessageBox::No:
         return true;

      default:
         break;
    }
    return false;
}

void KMenuEdit::slotConfigureToolbars()
{
    KEditToolBar dlg( factory() );
    dlg.exec();
}

void KMenuEdit::slotRestoreMenu()
{
    m_tree->restoreMenuSystem();
}

void KMenuEdit::restoreSystemMenu()
{
    m_tree->restoreMenuSystem();
}
