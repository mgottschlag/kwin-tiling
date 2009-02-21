/*
 *  Copyright 2007 Andreas Pakulat <apaku@gmx.de>
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

#include "globalshortcuts.h"

#include "kglobalshortcutseditor.h"

#include <KDE/KPluginFactory>

#include <QtGui/QLayout>


K_PLUGIN_FACTORY(GlobalShortcutsModuleFactory, registerPlugin<GlobalShortcutsModule>();)
K_EXPORT_PLUGIN(GlobalShortcutsModuleFactory("kcmkeys"))


GlobalShortcutsModule::GlobalShortcutsModule(QWidget *parent, const QVariantList &args)
 : KCModule(GlobalShortcutsModuleFactory::componentData(), parent, args),
   editor(0)
{
    KCModule::setButtons(KCModule::Buttons(KCModule::Default | KCModule::Apply));


    // Create the kglobaleditor
    editor = new KGlobalShortcutsEditor(this, KShortcutsEditor::GlobalAction);
    connect(editor, SIGNAL(changed(bool)), this, SIGNAL(changed(bool)));

    // Layout the hole bunch
    QVBoxLayout *global = new QVBoxLayout;
    global->addWidget(editor);
    setLayout(global);
}

GlobalShortcutsModule::~GlobalShortcutsModule()
{}


void GlobalShortcutsModule::load()
{
    editor->load();
}


void GlobalShortcutsModule::defaults()
{
    editor->allDefault();
}


void GlobalShortcutsModule::save()
{
    editor->save();
}


#include "globalshortcuts.moc"
