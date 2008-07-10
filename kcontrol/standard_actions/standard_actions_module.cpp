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

#include <QLayout>

#include <KActionCollection>
#include <KDebug>
#include <KPluginFactory>
#include <KShortcutsEditor>
#include <KStandardAction>

K_PLUGIN_FACTORY(StandardActionsModuleFactory, registerPlugin<StandardActionsModule>();)
K_EXPORT_PLUGIN(StandardActionsModuleFactory("kcmstandard_actions"))

StandardActionsModule::StandardActionsModule(
        QWidget *parent,
        const QVariantList &args )
    : KCModule(StandardActionsModuleFactory::componentData(), parent, args )
      ,m_editor(NULL)
    {
    // Configure the KCM
    KCModule::setButtons(KCModule::Buttons(KCModule::Default | KCModule::Apply));

    // Create and configure the editor
    m_editor = new KShortcutsEditor(this, KShortcutsEditor::AllActions);
    connect(m_editor, SIGNAL(keyChange()), this, SIGNAL(changed()));

    // Make a layout
    QVBoxLayout *global = new QVBoxLayout;
    global->addWidget(m_editor);
    setLayout(global);
    }

StandardActionsModule::~StandardActionsModule()
    {}

void StandardActionsModule::defaults()
    {
    kDebug();
    m_editor->allDefault();
    }

void StandardActionsModule::load()
    {
    kDebug();

    // Create a collection to handle the shortcuts
    KActionCollection* col = new KActionCollection(
            this,
            StandardActionsModuleFactory::componentData());

    // Put all standard shortcuts into the collection
    Q_FOREACH(KStandardAction::StandardAction id, KStandardAction::actionIds())
        {
        KAction *action = KStandardAction::create(id, NULL, NULL, col);
        }

    m_editor->addCollection(col);
    }

void StandardActionsModule::save()
    {
    kDebug();
    }

#include "standard_actions_module.moc"
