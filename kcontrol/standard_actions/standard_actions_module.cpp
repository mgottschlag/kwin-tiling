/*
 *  Copyright 2008 Michael Jansen <kde@michael-jansen.biz>
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */
#include "standard_actions_module.h"


#include <KAboutData>
#include <KAction>
#include <KActionCollection>
#include <KConfigGroup>
#include <KDebug>
#include <KPluginFactory>
#include <KShortcutsEditor>
#include <KStandardAction>
#include <KMessageBox>
#include <KLocale>

#include <QVBoxLayout>
#include <QSet>

K_PLUGIN_FACTORY(StandardActionsModuleFactory, registerPlugin<StandardActionsModule>();)
K_EXPORT_PLUGIN(StandardActionsModuleFactory("kcm_standard_actions"))

static void dressUpAction(KAction *action, KStandardShortcut::StandardShortcut shortcutId)
    {
    // Remember the shortcutId so we know where to save changes.
    action->setData(shortcutId);
    // We have to manually adjust the action. We want to show the
    // hardcoded default and the user set shortcut. But action currently
    // only contain the active shortcuts as default shortcut. So we
    // have to fill it correctly
    KShortcut hardcoded = KStandardShortcut::hardcodedDefaultShortcut(shortcutId);
    KShortcut active    = KStandardShortcut::shortcut(shortcutId);
    // Set the hardcoded default shortcut as default shortcut
    action->setShortcut(hardcoded, KAction::DefaultShortcut);
    // Set the user defined values as active shortcuts. If the user only
    // has overwritten the primary shortcut make sure the alternate one
    // still get's shown
    if (active.alternate()==QKeySequence())
        {
        active.setAlternate(hardcoded.alternate());
        }
    action->setShortcut(active, KAction::ActiveShortcut);
    }

StandardActionsModule::StandardActionsModule(
        QWidget *parent,
        const QVariantList &args )
    : KCModule(StandardActionsModuleFactory::componentData(), parent, args )
      ,m_editor(NULL)
      ,m_actionCollection(NULL)
    {
    KAboutData about("kcm_standard_actions", 0, ki18n("Standard Shortcuts"), "0.1");
    StandardActionsModuleFactory::componentData().setAboutData(about);

    // Configure the KCM
    KCModule::setButtons(KCModule::Buttons(KCModule::Default | KCModule::Apply | KCModule::Help));

    // Create and configure the editor
    m_editor = new KShortcutsEditor(this, KShortcutsEditor::WidgetAction | KShortcutsEditor::WindowAction | KShortcutsEditor::ApplicationAction); // there will be no global actions, so make sure that column is hidden
    connect(m_editor, SIGNAL(keyChange()), this, SLOT(keyChanged()));

    // Make a layout
    QVBoxLayout *global = new QVBoxLayout;
    global->addWidget(m_editor);
    setLayout(global);
    }


StandardActionsModule::~StandardActionsModule()
    {}


void StandardActionsModule::defaults()
    {
    m_editor->allDefault();
    }


void StandardActionsModule::keyChanged()
    {
    emit changed(true);
    }


void StandardActionsModule::load()
    {
    // Create a collection to handle the shortcuts
    m_actionCollection = new KActionCollection(
            this,
            StandardActionsModuleFactory::componentData());

    // Keeps track of which shortcut IDs have been added
    QSet<int> shortcutIdsAdded;

    // Put all shortcuts for standard actions into the collection
    Q_FOREACH(KStandardAction::StandardAction id, KStandardAction::actionIds())
        {
        KStandardShortcut::StandardShortcut shortcutId = KStandardAction::shortcutForActionId(id);
        // If the StandardShortcutId is AccelNone skip configuration for this
        // action.
        if (shortcutId == KStandardShortcut::AccelNone || shortcutIdsAdded.contains(shortcutId))
            {
            continue;
            }
        // Create the action
        KAction *action = KStandardAction::create(id, NULL, NULL, m_actionCollection);
        dressUpAction(action, shortcutId);
        shortcutIdsAdded << shortcutId;
        }

    // Put in the remaining standard shortcuts too...
    for(int i = int(KStandardShortcut::AccelNone) + 1; i < KStandardShortcut::StandardShortcutCount; ++i)
        {
        KStandardShortcut::StandardShortcut shortcutId = static_cast<KStandardShortcut::StandardShortcut>(i);
        if(!shortcutIdsAdded.contains(shortcutId))
            {
            KAction *action = new KAction(KStandardShortcut::label(shortcutId), this);
            action->setWhatsThis(KStandardShortcut::whatsThis(shortcutId));
            dressUpAction(action, shortcutId);
            m_actionCollection->addAction(KStandardShortcut::name(shortcutId), action);
            }
        }

    // Hand the collection to the editor
    m_editor->addCollection(m_actionCollection, i18n("Standard Shortcuts"));
    }


void StandardActionsModule::save()
    {
    m_editor->commit();

    Q_FOREACH(QAction* action, m_actionCollection->actions())
        {
        KAction *kaction = qobject_cast<KAction*>(action);

        KStandardShortcut::saveShortcut(
                static_cast<KStandardShortcut::StandardShortcut>(action->data().toInt())
                , kaction->shortcut());
        }

    KGlobal::config()->sync();
    KConfigGroup cg(KGlobal::config(), "Shortcuts");
    cg.sync();

    QString title = i18n("Standard Actions successfully saved");
    QString message = i18n(
        "The changes have been saved. Please note that:"
        "<ul><li>Applications need to be restarted to see the changes.</li>"
        "    <li>This change could introduce shortcut conflicts in some applications.</li>"
        "</ul>" );
    KMessageBox::information(this, message, title, "shortcuts_saved_info");
    }

#include "standard_actions_module.moc"
