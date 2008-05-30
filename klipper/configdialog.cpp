// -*- Mode: C++; c-basic-offset: 4; indent-tabs-mode: nil; tab-width: 8; -*-
/* This file is part of the KDE project
   Copyright (C) 2000 by Carsten Pfeiffer <pfeiffer@kde.org>

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
#include <QLabel>
#include <QLayout>
#include <QPushButton>
#include <QHeaderView>

#include <klocale.h>
#include <kmenu.h>
#include <kshortcutsdialog.h>
#include <KServiceTypeTrader>

#include "configdialog.h"

ConfigDialog::ConfigDialog(QWidget *parent, KConfigSkeleton *skeleton, const ActionList *list, KActionCollection *collection,
                           bool isApplet)
    : KConfigDialog(parent, "preferences", skeleton)
{
    if ( isApplet )
        setHelp( QString(), "klipper" );

    QWidget *w = 0; // the parent for the widgets

    w = new QWidget(this);
    generalWidget = new GeneralWidget(w);
    addPage(generalWidget, i18nc("General Config", "General"), "klipper", i18n("General Config"));

    w = new QWidget(this);
    actionWidget = new ActionWidget(list, w);
    addPage(actionWidget, i18nc("Actions Config", "Actions"), "system-run", i18n("Actions Config"));

    w = new QWidget(this);
    shortcutsWidget = new KShortcutsEditor( collection, w, KShortcutsEditor::GlobalAction );
    addPage(shortcutsWidget, i18nc("Shortcuts Config", "Shortcuts"), "configure-shortcuts", i18n("Shortcuts Config"));
}


ConfigDialog::~ConfigDialog()
{
}

void ConfigDialog::commitShortcuts()
{
    //keysWidget->commitChanges();
}

GeneralWidget::GeneralWidget( QWidget *parent )
    : QWidget( parent )
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    cbMousePos = new QCheckBox(i18n("&Popup menu at mouse-cursor position"), this);
    mainLayout->addWidget(cbMousePos);

    cbSaveContents = new QCheckBox(i18n("Save clipboard contents on e&xit"), this);
    mainLayout->addWidget(cbSaveContents);

    cbStripWhitespace = new QCheckBox(i18n("Remove whitespace when executing actions"), this);
    cbStripWhitespace->setWhatsThis(i18n("Sometimes, the selected text has some whitespace at the end, which, "
                                         "if loaded as URL in a browser would cause an error. Enabling this option "
                                         "removes any whitespace at the beginning or end of the selected string (the original "
                                         "clipboard contents will not be modified)."));
    mainLayout->addWidget(cbStripWhitespace);

    cbReplayAIH = new QCheckBox(i18n("&Replay actions on an item selected from history"), this);
    mainLayout->addWidget(cbReplayAIH);

    cbNoNull = new QCheckBox(i18n("Pre&vent empty clipboard"), this);
    cbNoNull->setWhatsThis(i18n("Selecting this option has the effect, that the "
                                "clipboard can never be emptied. E.g. when an application "
                                "exits, the clipboard would usually be emptied."));
    mainLayout->addWidget(cbNoNull);

    cbIgnoreSelection = new QCheckBox(i18n("&Ignore selection"), this);
    cbIgnoreSelection->setWhatsThis(i18n("This option prevents the selection being recorded "
                                         "in the clipboard history. Only explicit clipboard "
                                         "changes are recorded."));
    mainLayout->addWidget(cbIgnoreSelection);

    QGroupBox *group = new QGroupBox(i18n("Clipboard/Selection Behavior"), this);
    group->setWhatsThis(i18n("<qt>There are two different clipboard buffers available:<br /><br />"
                             "<b>Clipboard</b> is filled by selecting something "
                             "and pressing Ctrl+C, or by clicking \"Copy\" in a toolbar or "
                             "menubar.<br /><br />"
                             "<b>Selection</b> is available immediately after "
                             "selecting some text. The only way to access the selection "
                             "is to press the middle mouse button.<br /><br />"
                             "You can configure the relationship between Clipboard and Selection."
                             "</qt>"));
    mainLayout->addWidget(group);

    QVBoxLayout *groupLayout = new QVBoxLayout(group);

    cbSynchronize = new QRadioButton(i18n("Sy&nchronize contents of the clipboard and the selection"), group);
    cbSynchronize->setWhatsThis(i18n("Selecting this option synchronizes these two buffers."));
    connect(cbSynchronize, SIGNAL(clicked()), SLOT(slotClipConfigChanged()));
    groupLayout->addWidget(cbSynchronize);

    cbSeparate = new QRadioButton(i18n("Separate clipboard and selection"), group);
    cbSeparate->setWhatsThis(i18n("Using this option will only set the selection when highlighting "
                                  "something and the clipboard when choosing e.g. \"Copy\" "
                                  "in a menubar."));
    connect(cbSeparate, SIGNAL(clicked()), SLOT(slotClipConfigChanged()));
    groupLayout->addWidget(cbSeparate);

    cbSeparate->setChecked(!cbSynchronize->isChecked());

    popupTimeout = new KIntNumInput(this);
    popupTimeout->setLabel(i18n("Tim&eout for action popups:"));
    popupTimeout->setRange(0, 200);
    popupTimeout->setSuffix(i18n(" sec"));
    popupTimeout->setToolTip(i18n("A value of 0 disables the timeout"));
    mainLayout->addWidget(popupTimeout);

    maxItems = new KIntNumInput(this);
    maxItems->setLabel(i18n("C&lipboard history size:"));
    maxItems->setRange(2, 2048);
    connect(maxItems, SIGNAL(valueChanged(int)), SLOT(historySizeChanged(int)));
    mainLayout->addWidget(maxItems);

    slotClipConfigChanged();
}

GeneralWidget::~GeneralWidget()
{
}

void GeneralWidget::historySizeChanged( int value )
{
    // Note there is no %n in this string, because value is not supposed
    // to be put into the suffix of the spinbox.
    maxItems->setSuffix( i18np( " entry", " entries", value ) );
}

void GeneralWidget::slotClipConfigChanged()
{
    cbIgnoreSelection->setEnabled( !cbSynchronize->isChecked() );
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

ActionWidget::ActionWidget( const ActionList *list, QWidget *parent )
    : QWidget(parent),
      advancedWidget(0)
{
    Q_ASSERT(list);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    QLabel *lblAction = new QLabel(i18n("Action &list (right click to add/remove commands):"), this);
    mainLayout->addWidget(lblAction);

    treeWidget = new QTreeWidget(this);
    lblAction->setBuddy(treeWidget);

    treeWidget->setHeaderLabels(QStringList() << i18n("Regular Expression") << i18n("Description"));
    treeWidget->header()->resizeSection(0, 250);
    treeWidget->setSelectionBehavior(QTreeWidget::SelectRows);
    treeWidget->setSelectionMode(QTreeWidget::SingleSelection);

    mainLayout->addWidget(treeWidget);

    ClipAction *action   = 0L;
    ClipCommand *command = 0L;

    ActionListIterator it( *list );

    while (it.hasNext()) {
        action = it.next();

        QTreeWidgetItem *item = new QTreeWidgetItem(treeWidget,
                                                    QStringList() << action->regExp() << action->description());
        item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled);

        QListIterator<ClipCommand*> it2( action->commands() );
        while (it2.hasNext()) {
            command = it2.next();

            QTreeWidgetItem *child = new QTreeWidgetItem(item, QStringList()
                                                         << command->command << command->description);
            child->setIcon(0, KIcon(command->pixmap.isEmpty() ? "system-run" : command->pixmap));
            child->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled);
        }
    }

    connect(treeWidget, SIGNAL(itemSelectionChanged()), SLOT(selectionChanged()));

    treeWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(treeWidget, SIGNAL(customContextMenuRequested(const QPoint &)), SLOT(slotContextMenu(const QPoint &)));

    connect(treeWidget, SIGNAL(itemChanged(QTreeWidgetItem*, int)), SLOT(slotItemChanged(QTreeWidgetItem*, int)));

    treeWidget->setSortingEnabled(false);

    cbUseGUIRegExpEditor = new QCheckBox(i18n("&Use graphical editor for editing regular expressions" ), this);
    if ( /*KServiceTypeTrader::self()->query("KRegExpEditor/KRegExpEditor").isEmpty()*/ true) // see notice above about KRegExpEditor
    {
        cbUseGUIRegExpEditor->hide();
        cbUseGUIRegExpEditor->setChecked( false );
    }
    mainLayout->addWidget(cbUseGUIRegExpEditor);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    mainLayout->addLayout(buttonLayout);

    QPushButton *button = new QPushButton(KIcon("list-add"), i18n("&Add Action"), this);
    connect(button, SIGNAL(clicked()), SLOT(slotAddAction()));
    buttonLayout->addWidget(button);

    delActionButton = new QPushButton(KIcon("list-remove"), i18n("&Delete Action"), this);
    connect(delActionButton, SIGNAL(clicked()), SLOT(slotDeleteAction()));
    buttonLayout->addWidget(delActionButton);

    buttonLayout->addStretch();

    QPushButton *advanced = new QPushButton(KIcon("configure"), i18n("Advanced..."), this);
    connect(advanced, SIGNAL(clicked()), SLOT(slotAdvanced()));
    buttonLayout->addWidget(advanced);

    QLabel *label = new QLabel(i18n("Click on a highlighted item's column to change it. \"%s\" in a "
                                    "command will be replaced with the clipboard contents."), this);
    label->setWordWrap(true);
    mainLayout->addWidget(label);

    QLabel *labelRegexp = new QLabel(i18n("For more information about regular expressions, you could have a look at the "
                                          "<a href=\"http://en.wikipedia.org/wiki/Regular_expression\">Wikipedia entry "
                                          "about this topic</a>."), this);
    labelRegexp->setOpenExternalLinks(true);
    labelRegexp->setWordWrap(true);
    mainLayout->addWidget(labelRegexp);

    delActionButton->setEnabled(treeWidget->currentItem());
}

ActionWidget::~ActionWidget()
{
}

void ActionWidget::selectionChanged()
{
    delActionButton->setEnabled(!treeWidget->selectedItems().isEmpty());
}

void ActionWidget::slotContextMenu(const QPoint& pos)
{
    QTreeWidgetItem *item = treeWidget->itemAt(pos);
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

void ActionWidget::slotItemChanged(QTreeWidgetItem *item, int column)
{
    if (!item->parent() || column != 0)
        return;
    ClipCommand command( 0, item->text(0), item->text(1) );

    treeWidget->blockSignals(true); // don't lead in infinite recursion...
    item->setIcon(0, KIcon(command.pixmap.isEmpty() ? "system-run" : command.pixmap));
    treeWidget->blockSignals(false);
}

void ActionWidget::slotAddAction()
{
    QTreeWidgetItem *item = new QTreeWidgetItem(treeWidget,
                                                QStringList() << i18n("Double-click here to set the regexp")
                                                              << i18n("<new action>"));
    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled);
}


void ActionWidget::slotDeleteAction()
{
    QTreeWidgetItem *item = treeWidget->currentItem();
    if ( item && item->parent() )
        item = item->parent();
    delete item;
}


ActionList * ActionWidget::actionList()
{
    ClipAction *action = 0;
    ActionList *list = new ActionList;

    QTreeWidgetItemIterator it(treeWidget);
    while (*it) {
        if (!(*it)->parent()) {
            if (action) {
                list->append(action);
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
        list->append(action);

    return list;
}

void ActionWidget::slotAdvanced()
{
    KDialog dlg(this);
    dlg.setModal(true);
    dlg.setCaption( i18n("Advanced Settings") );
    dlg.setButtons( KDialog::Ok | KDialog::Cancel );

    AdvancedWidget *widget = new AdvancedWidget(&dlg);
    widget->setWMClasses( m_wmClasses );

    dlg.setMainWidget(widget);

    if ( dlg.exec() == KDialog::Accepted ) {
        m_wmClasses = widget->wmClasses();
    }
}

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

#include "configdialog.moc"
