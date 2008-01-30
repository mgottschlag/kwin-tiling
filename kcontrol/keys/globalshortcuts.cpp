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

#include "ui_globalshortcuts.h"

#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDBusMetaType>
#include <QMetaType>
#include <QVBoxLayout>

#include <kpluginfactory.h>
#include <kactioncollection.h>
#include <kaction.h>
#include <kshortcutseditor.h>
#include <kdebug.h>
#include <qcombobox.h>
#include <kglobalaccel.h>

K_PLUGIN_FACTORY(GlobalShortcutsModuleFactory, registerPlugin<GlobalShortcutsModule>();)
K_EXPORT_PLUGIN(GlobalShortcutsModuleFactory("kcmkeys"))

Q_DECLARE_METATYPE( QList<int> )

GlobalShortcutsModule::GlobalShortcutsModule( QWidget * parent, const QVariantList & args )
    : KCModule(GlobalShortcutsModuleFactory::componentData(), parent, args), ui(0),
      editor(0), saved(false)
{
    ui = new Ui::GlobalShortcuts();
    ui->setupUi(this);
    layout()->setMargin(0);

    KCModule::setButtons( KCModule::Buttons(KCModule::Default) );

    editor = new KShortcutsEditor(this, KShortcutsEditor::GlobalAction);
    layout()->addWidget(editor);

    connect(editor, SIGNAL(keyChange()), this, SLOT(changed()));
    connect(ui->components, SIGNAL(activated(const QString&)),
            this, SLOT(componentChanged(const QString&)));
    load();
}

GlobalShortcutsModule::~GlobalShortcutsModule()
{
    if(!saved)
        editor->undoChanges();
    delete ui;
}

void GlobalShortcutsModule::load()
{
    ui->components->clear();
    qRegisterMetaType<QList<int> >();
    qDBusRegisterMetaType<QList<int> >();
    QDBusConnection bus = QDBusConnection::sessionBus();
    QDBusInterface* iface = new QDBusInterface( "org.kde.kded", "/KdedGlobalAccel", "org.kde.KdedGlobalAccel", bus, this );

    QDBusReply<QStringList> components = iface->call("allComponents");
    KComponentData curcomp = KGlobal::mainComponent();
    foreach(const QString &component, components.value() )
    {
        kDebug() << "component:" << component;
        KGlobalAccel::self()->overrideMainComponentData(KComponentData(component.toAscii()));
        KActionCollection* col = new KActionCollection(this);
        actionCollections[component] = col;

        QDBusReply<QStringList> actions = iface->call("allActionsForComponent", qVariantFromValue(component) );
        foreach(const QString &actionText, actions.value() )
        {
            kDebug() << "- action:" << actionText;
            QString actionName = QString("%1_%2").arg(component).arg(col->count());
            KAction *action = col->addAction(actionName);
            action->setProperty("isConfigurationAction", QVariant(true));
            action->setText(actionText);
            QStringList actionId;
            actionId << component << actionText;
            QDBusReply<QList<int> > defaultShortcut = iface->call("defaultShortcut", qVariantFromValue(actionId));
            QDBusReply<QList<int> > shortcut = iface->call("shortcut", qVariantFromValue(actionId));
            if (!defaultShortcut.value().empty())
            {
                int key = defaultShortcut.value().first();
                kDebug() << "-- defaultShortcut" << KShortcut(key).toString();
                action->setGlobalShortcut(KShortcut(key), KAction::DefaultShortcut, KAction::NoAutoloading);
            }
            if (!shortcut.value().empty())
            {
                int key = shortcut.value().first();
                kDebug() << "-- shortcut" << KShortcut(key).toString();
                action->setGlobalShortcut(KShortcut(key), KAction::ActiveShortcut, KAction::NoAutoloading);
            }
        }
    }
    KGlobalAccel::self()->overrideMainComponentData(curcomp);
    ui->components->addItems( actionCollections.keys() );
    componentChanged(ui->components->currentText());
    delete iface;
}

void GlobalShortcutsModule::save()
{
    kDebug() << "saving shortcuts";
    editor->save();
    saved = true;
}

void GlobalShortcutsModule::defaults()
{
    kDebug() << "undoing shortcuts";
    editor->allDefault();
}

void GlobalShortcutsModule::componentChanged(const QString &str)
{
    kDebug() << "Switching to component:" << str;
    if( actionCollections.contains(str) )
    {
        KGlobalAccel::self()->overrideMainComponentData(KComponentData(str.toAscii()));
        editor->clearCollections();
        editor->addCollection( actionCollections[str] );
    }
}

#include "globalshortcuts.moc"
