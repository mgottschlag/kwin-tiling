/*
 * commandShortcuts.h
 *
 * Copyright (c) 2003 Aaron J. Seigo
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Steet, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "commandShortcuts.h"
#include "treeview.h"

#include <q3buttongroup.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qradiobutton.h>

//Added by qt3to4:
#include <QVBoxLayout>
#include <QHBoxLayout>

#include <kactivelabel.h>
#include <kapplication.h>
#include <kmessagebox.h>
#include <kdialog.h>
#include <khotkeys.h>
#include <kkeybutton.h>
#include <klocale.h>

static bool treeFilled = false;
CommandShortcutsModule::CommandShortcutsModule( QWidget *parent, const char *name )
: QWidget( parent, name )
{
    treeFilled = false;
    initGUI();
}

CommandShortcutsModule::~CommandShortcutsModule()
{
}

// Called when [Reset] is pressed
void CommandShortcutsModule::load()
{
    defaults();
}

void CommandShortcutsModule::save()
{
    for (treeItemListIterator it(m_changedItems); it.current(); ++it)
    {
        KHotKeys::changeMenuEntryShortcut(it.current()->storageId(), it.current()->accel());
    }
    m_changedItems.clear();
}

void CommandShortcutsModule::defaults()
{
    m_tree->clear();
    m_tree->fill();
}

QString CommandShortcutsModule::quickHelp() const
{
  return i18n("<h1>Command Shortcuts</h1> Using key bindings you can configure applications "
    "and commands to be triggered when you press a key or a combination of keys.");
}

void CommandShortcutsModule::initGUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this, KDialog::marginHint());
    mainLayout->addSpacing( KDialog::marginHint() );

    KActiveLabel* label = new KActiveLabel(this);
    label->setText(i18n("<qt>Below is a list of known commands which you may assign keyboard shortcuts to. "
                        "To edit, add or remove entries from this list use the "
                        "<a href=\"launchMenuEditor\">KDE menu editor</a>.</qt>"));
    label->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
    disconnect(label, SIGNAL(linkClicked(const QString &)), label, SLOT(openLink(const QString &)));
    connect(label, SIGNAL(linkClicked(const QString &)), this, SLOT(launchMenuEditor()));
    mainLayout->addWidget(label);

    m_tree = new AppTreeView(this, "appTreeView");
    m_tree->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    mainLayout->setStretchFactor(m_tree, 10);
    mainLayout->addWidget(m_tree);
    m_tree->setWhatsThis(
                    i18n("This is a list of all the desktop applications and commands "
                         "currently defined on this system. Click to select a command to "
                         "assign a keyboard shortcut to. Complete management of these "
                         "entries can be done via the menu editor program."));
    connect(m_tree, SIGNAL(entrySelected(const QString&, const QString &, bool)),
            this, SLOT(commandSelected(const QString&, const QString &, bool)));
    connect(m_tree, SIGNAL(doubleClicked(Q3ListViewItem *, const QPoint &, int)),
            this, SLOT(commandDoubleClicked(Q3ListViewItem *, const QPoint &, int)));
    m_shortcutBox = new Q3ButtonGroup(i18n("Shortcut for Selected Command"), this);
    mainLayout->addWidget(m_shortcutBox);
    QHBoxLayout* buttonLayout = new QHBoxLayout(m_shortcutBox, KDialog::marginHint() * 2);
    buttonLayout->addSpacing( KDialog::marginHint() );

    m_noneRadio = new QRadioButton(i18n("no key", "&None"), m_shortcutBox);
    m_noneRadio->setWhatsThis( i18n("The selected command will not be associated with any key."));
    buttonLayout->addWidget(m_noneRadio);
    m_customRadio = new QRadioButton(i18n("C&ustom"), m_shortcutBox);
    m_customRadio->setWhatsThis(
                    i18n("If this option is selected you can create a customized key binding for the"
                         " selected command using the button to the right.") );
    buttonLayout->addWidget(m_customRadio);
    m_shortcutButton = new KKeyButton(m_shortcutBox);
    m_shortcutButton->setWhatsThis(
                    i18n("Use this button to choose a new shortcut key. Once you click it, "
                         "you can press the key-combination which you would like to be assigned "
                         "to the currently selected command."));
    buttonLayout->addSpacing(KDialog::spacingHint() * 2);
    buttonLayout->addWidget(m_shortcutButton);
    connect(m_shortcutButton, SIGNAL(capturedShortcut(const KShortcut&)),
            this, SLOT(shortcutChanged(const KShortcut&)));
    connect(m_customRadio, SIGNAL(toggled(bool)), m_shortcutButton, SLOT(setEnabled(bool)));
    connect(m_noneRadio, SIGNAL(toggled(bool)), this, SLOT(shortcutRadioToggled(bool)));
    buttonLayout->addStretch(1);
}

void CommandShortcutsModule::launchMenuEditor()
{
    if ( KApplication::startServiceByDesktopName( "kmenuedit",
                                                  QString::null /*url*/,
                                                  0 /*error*/,
                                                  0 /*dcopservice*/,
                                                  0 /*pid*/,
                                                  "" /*startup_id*/,
                                                  true /*nowait*/ ) != 0 )
    {
        KMessageBox::error(this,
                           i18n("The KDE menu editor (kmenuedit) could not be launched.\n"
                           "Perhaps it is not installed or not in your path."),
                           i18n("Application Missing"));
    }
}


void CommandShortcutsModule::shortcutRadioToggled(bool remove)
{
    AppTreeItem *item = static_cast<AppTreeItem*>(m_tree->currentItem());
    if (!item || item->isDirectory())
    {
        return;
    }

    if (remove)
    {
        m_shortcutButton->setShortcut(QString(), false);
        item->setAccel(QString::null);
        if (m_changedItems.findRef(item) == -1)
        {
            m_changedItems.append(item);
        }
        emit changed(true);
    }
    else
    {
        m_shortcutButton->captureShortcut();
    }
}

void CommandShortcutsModule::shortcutChanged(const KShortcut& shortcut)
{
    AppTreeItem *item = static_cast<AppTreeItem*>(m_tree->currentItem());
    if (!item || item->isDirectory())
    {
        return;
    }

    QString accel = shortcut.toString();
    bool hasAccel = !accel.isEmpty();
    m_noneRadio->blockSignals(true);
    m_noneRadio->setChecked(!hasAccel);
    m_customRadio->setChecked(hasAccel);
    m_shortcutButton->setShortcut(accel, false);
    item->setAccel(accel);
    m_noneRadio->blockSignals(false);
    if (m_changedItems.findRef(item) == -1)
    {
       m_changedItems.append(item);
    }

    emit changed( true );
}

void CommandShortcutsModule::showing(QWidget* w)
{
    if (w != this || treeFilled)
    {
        return;
    }

    m_tree->fill();
    if (m_tree->firstChild())
    {
        m_tree->setSelected(m_tree->firstChild(), true);
    }
    else
    {
        m_shortcutBox->setEnabled(false);
    }
    treeFilled = true;
}

void CommandShortcutsModule::commandSelected(const QString& /* path */, const QString & accel, bool isDirectory)
{
    m_noneRadio->blockSignals(true);
    m_shortcutBox->setEnabled(!isDirectory);
    if (!isDirectory)
    {
        bool hasAccel = !accel.isEmpty();
        m_noneRadio->setChecked(!hasAccel);
        m_customRadio->setChecked(hasAccel);
        m_shortcutButton->setShortcut(accel, false);
    }
    m_noneRadio->blockSignals(false);
}

void CommandShortcutsModule::commandDoubleClicked(Q3ListViewItem *item, const QPoint &, int)
{
    if (!item)
    {
        return;
    }
    AppTreeItem *rl_item = static_cast<AppTreeItem*>(item);
    if ( rl_item->isDirectory())
        return;

    m_shortcutButton->captureShortcut();
}

#include "commandShortcuts.moc"
