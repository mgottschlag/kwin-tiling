/*
 *  Copyright 2007 Andreas Pakulat <apaku@gmx.de>
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

#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDBusMetaType>
#include <QMetaType>
#include <QVBoxLayout>
#include <QPointer>

#include <kpluginfactory.h>
#include <kactioncollection.h>
#include <kaction.h>
#include <kglobalshortcutseditor.h>
#include <kdebug.h>
#include <qcombobox.h>
#include <kglobalaccel.h>

K_PLUGIN_FACTORY(GlobalShortcutsModuleFactory, registerPlugin<GlobalShortcutsModule>();)
K_EXPORT_PLUGIN(GlobalShortcutsModuleFactory("kcmkeys"))

Q_DECLARE_METATYPE( QList<int> )

GlobalShortcutsModule::GlobalShortcutsModule( QWidget * parent, const QVariantList & args )
    : KCModule(GlobalShortcutsModuleFactory::componentData(), parent, args),
      editor(0)
{
    KCModule::setButtons( KCModule::Buttons(KCModule::Default|KCModule::Apply) );
    editor = new KGlobalShortcutsEditor(this, KShortcutsEditor::GlobalAction);
    setLayout( new QVBoxLayout );
    layout()->addWidget(editor);
    connect(editor, SIGNAL(changed()), this, SLOT(changed()) );
    load();
}

GlobalShortcutsModule::~GlobalShortcutsModule()
{}

void GlobalShortcutsModule::load()
{
    // Undo all changes not yet applied
    editor->clear();

    qRegisterMetaType<QList<int> >();
    qDBusRegisterMetaType<QList<int> >();
    QDBusConnection bus = QDBusConnection::sessionBus();
    QPointer<QDBusInterface> iface = new QDBusInterface( "org.kde.kded", "/KdedGlobalAccel", "org.kde.KdedGlobalAccel", bus, this );

    QDBusReply<QStringList> components = iface->call("allComponents");

    QHash<QString,KActionCollection*> actionCollections;

    foreach(const QString &component, components.value() )
    {
        // kDebug() << "component:" << component;
        KGlobalAccel::self()->overrideMainComponentData(KComponentData(component.toAscii()));
        KActionCollection* col = new KActionCollection(this);
        actionCollections[component] = col;

        QDBusReply<QStringList> actions = iface->call("allActionsForComponent", qVariantFromValue(component) );
        foreach(const QString &actionText, actions.value() )
        {
            // kDebug() << "- action:" << actionText;
            QString actionName = QString("%1_%2").arg(component).arg(col->count());
            KAction *action = col->addAction(actionName);
            // see KAction::~KAction
            action->setProperty("isConfigurationAction", QVariant(true));
            action->setText(actionText);
            QStringList actionId;
            actionId << component << actionText;
            QDBusReply<QList<int> > defaultShortcut = iface->call("defaultShortcut", qVariantFromValue(actionId));
            QDBusReply<QList<int> > shortcut = iface->call("shortcut", qVariantFromValue(actionId));
            if (!defaultShortcut.value().empty())
            {
                int key = defaultShortcut.value().first();
                // kDebug() << "-- defaultShortcut" << KShortcut(key).toString();
                action->setGlobalShortcut(KShortcut(key), KAction::DefaultShortcut, KAction::NoAutoloading);
            }
            if (!shortcut.value().empty())
            {
                int key = shortcut.value().first();
                // kDebug() << "-- shortcut" << KShortcut(key).toString();
                action->setGlobalShortcut(KShortcut(key), KAction::ActiveShortcut, KAction::NoAutoloading);
            }
        }
    }

    // Add components and their keys to the editor
    Q_FOREACH (QString component, actionCollections.keys() )
    {
        // kDebug() << "Adding collection " << component;
        editor->addCollection( actionCollections[component], component );
    }

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
