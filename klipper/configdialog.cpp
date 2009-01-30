// -*- Mode: C++; c-basic-offset: 4; indent-tabs-mode: nil; tab-width: 8; -*-
/* This file is part of the KDE project
   Copyright (C) 2000 by Carsten Pfeiffer <pfeiffer@kde.org>
   Copyright (C) 2008 by Dmitry Suzdalev <dimsuz@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "configdialog.h"

#include <KLocale>
#include <KMenu>
#include <KShortcutsDialog>
#include <KDebug>
#include <KEditListBox>

#include <QHeaderView>

#include "klipper.h"

GeneralWidget::GeneralWidget(QWidget* parent)
    : QWidget(parent)
{
    m_ui.setupUi(this);
    connect(m_ui.rbSynchronize, SIGNAL(toggled(bool)), SLOT(onSyncronizeToggled(bool)));
}

void GeneralWidget::onSyncronizeToggled(bool toggled)
{
    m_ui.kcfg_IgnoreSelection->setEnabled(!toggled);
}

ActionsWidget::ActionsWidget(QWidget* parent)
    : QWidget(parent)
{
    m_ui.setupUi(this);

    m_ui.pbAddAction->setIcon(KIcon("list-add"));
    m_ui.pbDelAction->setIcon(KIcon("list-remove"));
    m_ui.pbAdvanced->setIcon(KIcon("configure"));

    m_ui.kcfg_ActionList->header()->resizeSection(0, 250);

#if 0
    if ( /*KServiceTypeTrader::self()->query("KRegExpEditor/KRegExpEditor").isEmpty()*/ true) // see notice in configdialog.cpp about KRegExpEditor
    {
        cbUseGUIRegExpEditor->hide();
        cbUseGUIRegExpEditor->setChecked( false );
    }
#endif

    connect(m_ui.kcfg_ActionList, SIGNAL(itemSelectionChanged()), SLOT(onSelectionChanged()));
    connect(m_ui.kcfg_ActionList, SIGNAL(customContextMenuRequested(const QPoint &)), SLOT(onContextMenu(const QPoint &)));
    connect(m_ui.kcfg_ActionList, SIGNAL(itemChanged(QTreeWidgetItem*, int)), SLOT(onItemChanged(QTreeWidgetItem*, int)));

    connect(m_ui.pbAddAction, SIGNAL(clicked()), SLOT(onAddAction()));
    connect(m_ui.pbDelAction, SIGNAL(clicked()), SLOT(onDeleteAction()));
    connect(m_ui.pbAdvanced, SIGNAL(clicked()), SLOT(onAdvanced()));

    onSelectionChanged();
}

void ActionsWidget::setActionList(const ActionList& list)
{
    m_ui.kcfg_ActionList->clear();
    foreach (ClipAction* action, list) {
        if (!action) {
            kDebug() << "action is null!";
            continue;
        }
        QStringList actionProps;
        actionProps << action->regExp() << action->description();
        QTreeWidgetItem *item = new QTreeWidgetItem(m_ui.kcfg_ActionList, actionProps);
        item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled);

        foreach (ClipCommand* command, action->commands()) {
            QStringList cmdProps;
            cmdProps << command->command << command->description;
            QTreeWidgetItem *child = new QTreeWidgetItem(item, cmdProps);
            child->setIcon(0, KIcon(command->pixmap.isEmpty() ? "system-run" : command->pixmap));
            child->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled);
        }
    }

    // after all actions loaded, reset modified state of tree widget.
    // Needed because tree widget reacts on item changed events to tell if its changed
    // this will ensure that apply button state will be correctly changed
    m_ui.kcfg_ActionList->resetModifiedState();
}

void ActionsWidget::setExcludedWMClasses(const QStringList& excludedWMClasses)
{
    m_exclWMClasses = excludedWMClasses;
}

QStringList ActionsWidget::excludedWMClasses() const
{
    return m_exclWMClasses;
}

ActionList ActionsWidget::actionList() const
{
    ClipAction *action = 0;
    ActionList list;

    QTreeWidgetItemIterator it(m_ui.kcfg_ActionList);
    while (*it) {
        if (!(*it)->parent()) {
            if (action) {
                list.append(action);
                action = 0;
            }
            action = new ClipAction((*it)->text(0), (*it)->text(1));
        } else {
            if (action)
                action->addCommand((*it)->text(0), (*it)->text(1), true);
        }
        it++;
    }
    if (action)
        list.append(action);

    return list;
}

void ActionsWidget::resetModifiedState()
{
    m_ui.kcfg_ActionList->resetModifiedState();
}

void ActionsWidget::onSelectionChanged()
{
    m_ui.pbDelAction->setEnabled(!m_ui.kcfg_ActionList->selectedItems().isEmpty());
}

void ActionsWidget::onContextMenu(const QPoint& pos)
{
    QTreeWidgetItem *item = m_ui.kcfg_ActionList->itemAt(pos);
    if ( !item )
        return;

    KMenu *menu = new KMenu;
    QAction *addCmd = menu->addAction(KIcon("list-add"), i18n("Add Command"));
    QAction *rmCmd = menu->addAction(KIcon("list-remove"), i18n("Remove Command"));
    if ( !item->parent() ) {// no "command" item
        rmCmd->setEnabled( false );
        item->setExpanded ( true );
    }

    QAction *executed = menu->exec(mapToGlobal(pos));
    if ( executed == addCmd ) {
        QTreeWidgetItem *child = new QTreeWidgetItem(item->parent() ? item->parent() : item, QStringList()
                                                     << i18n("Double-click here to set the command to be executed")
                                                     << i18n("<new command>"));
        child->setIcon(0, KIcon("system-run"));
        child->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled);
    }
    else if ( executed == rmCmd )
        delete item;

    delete menu;
}

void ActionsWidget::onItemChanged(QTreeWidgetItem *item, int column)
{
    if (!item->parent() || column != 0)
        return;
    ClipCommand command( 0, item->text(0), item->text(1) );

    m_ui.kcfg_ActionList->blockSignals(true); // don't lead in infinite recursion...
    item->setIcon(0, KIcon(command.pixmap.isEmpty() ? "system-run" : command.pixmap));
    m_ui.kcfg_ActionList->blockSignals(false);
}

void ActionsWidget::onAddAction()
{
    QTreeWidgetItem *item = new QTreeWidgetItem(m_ui.kcfg_ActionList,
                                                QStringList() << i18n("Double-click here to set the regular expression")
                                                              << i18n("<new action>"));
    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled);
}


void ActionsWidget::onDeleteAction()
{
    QTreeWidgetItem *item = m_ui.kcfg_ActionList->currentItem();
    if ( item && item->parent() )
        item = item->parent();
    delete item;
}

void ActionsWidget::onAdvanced()
{
    KDialog dlg(this);
    dlg.setModal(true);
    dlg.setCaption( i18n("Advanced Settings") );
    dlg.setButtons( KDialog::Ok | KDialog::Cancel );

    AdvancedWidget *widget = new AdvancedWidget(&dlg);
    widget->setWMClasses( m_exclWMClasses );

    dlg.setMainWidget(widget);

    if ( dlg.exec() == KDialog::Accepted ) {
        m_exclWMClasses = widget->wmClasses();
    }
}

ConfigDialog::ConfigDialog(QWidget *parent, KConfigSkeleton *skeleton, const Klipper* klipper, KActionCollection *collection,
                           bool isApplet)
    : KConfigDialog(parent, "preferences", skeleton), m_klipper(klipper)
{
    if ( isApplet )
        setHelp( QString(), "klipper" );

    m_actionsPage = new ActionsWidget(this);

    addPage(new GeneralWidget(this), i18nc("General Config", "General"), "klipper", i18n("General Config"));
    addPage(m_actionsPage, i18nc("Actions Config", "Actions"), "system-run", i18n("Actions Config"));

    QWidget* w = new QWidget(this);
    m_shortcutsWidget = new KShortcutsEditor( collection, w, KShortcutsEditor::GlobalAction );
    addPage(m_shortcutsWidget, i18nc("Shortcuts Config", "Shortcuts"), "configure-shortcuts", i18n("Shortcuts Config"));
}


ConfigDialog::~ConfigDialog()
{
}


void ConfigDialog::updateSettings()
{
    // user clicked Ok or Apply

    if (!m_klipper) {
        kDebug() << "Klipper object is null";
        return;
    }

    m_shortcutsWidget->save();

    m_actionsPage->resetModifiedState();

    m_klipper->urlGrabber()->setActionList(m_actionsPage->actionList());
    m_klipper->urlGrabber()->setExcludedWMClasses(m_actionsPage->excludedWMClasses());
    m_klipper->saveSettings();
}

void ConfigDialog::updateWidgets()
{
    // settings were updated, update widgets

    if (m_klipper && m_klipper->urlGrabber() ) {
        m_actionsPage->setActionList(m_klipper->urlGrabber()->actionList());
        m_actionsPage->setExcludedWMClasses(m_klipper->urlGrabber()->excludedWMClasses());
    } else {
        kDebug() << "Klipper or grabber object is null";
        return;
    }
}

void ConfigDialog::updateWidgetsDefault()
{
    // default widget values requested

    m_shortcutsWidget->allDefault();
}

// it does not make sense to port / enable this since KRegExpEditor is in a very bad shape. just keep this
// code here because it will probably help at a later point to port it when KRegExpEditor is again usable.
// 2007-10-20, uwolfer
#if 0
void ListView::rename( Q3ListViewItem* item, int c )
{
  bool gui = false;
  if ( item->childCount() != 0 && c == 0) {
    // This is the regular expression
    if ( _configWidget->useGUIRegExpEditor() ) {
      gui = true;
    }
  }

  if ( gui ) {
    if ( ! _regExpEditor )
      _regExpEditor = KServiceTypeTrader::createInstanceFromQuery<QDialog>( "KRegExpEditor/KRegExpEditor", QString(), this );
    KRegExpEditorInterface *iface = qobject_cast<KRegExpEditorInterface *>(_regExpEditor);

    Q_ASSERT( iface );
    iface->setRegExp( item->text( 0 ) );

    bool ok = _regExpEditor->exec();
    if ( ok )
      item->setText( 0, iface->regExp() );
  }
  else
    K3ListView::rename( item ,c );
}
#endif

AdvancedWidget::AdvancedWidget( QWidget *parent )
    : QWidget(parent)
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    editListBox = new KEditListBox(i18n("D&isable Actions for Windows of Type WM_CLASS"), this);

    editListBox->setButtons(KEditListBox::Add | KEditListBox::Remove);
    editListBox->setCheckAtEntering(true);

    editListBox->setWhatsThis(i18n("<qt>This lets you specify windows in which Klipper should "
                                   "not invoke \"actions\". Use<br /><br />"
                                   "<center><b>xprop | grep WM_CLASS</b></center><br />"
                                   "in a terminal to find out the WM_CLASS of a window. "
                                   "Next, click on the window you want to examine. The "
                                   "first string it outputs after the equal sign is the one "
                                   "you need to enter here.</qt>"));
    mainLayout->addWidget(editListBox);

    editListBox->setFocus();
}

AdvancedWidget::~AdvancedWidget()
{
}

void AdvancedWidget::setWMClasses( const QStringList& items )
{
    editListBox->setItems(items);
}

QStringList AdvancedWidget::wmClasses() const
{
    return editListBox->items();
}

#include "configdialog.moc"
