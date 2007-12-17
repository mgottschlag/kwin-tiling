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
    
    QDBusReply<QList<int> > l = iface->call("allKeys");
    QHash<QString,int> shortcuts;
    foreach( int i, l.value() )
    {
        QDBusReply<QStringList> actionid = iface->call("action", qVariantFromValue(i));
        QStringList actionlist = actionid.value();
        if( !actionCollections.contains( actionlist.first() ) ) 
        {
            actionCollections[actionlist.first()] = new KActionCollection(this);
        }
        QDBusReply<QList<int> > shortcut = iface->call("shortcut", qVariantFromValue(actionid.value()));
        KAction *action = new KAction(actionlist.at(1), this);
        KActionCollection* col = actionCollections[actionlist.first()];
        QString actionname = QString("%1_%2").arg(actionlist.first()).arg(col->count());
        shortcuts[actionname] = shortcut.value().first();
        col->addAction(actionname, action);
    }
    KComponentData curcomp = KGlobal::mainComponent();
    foreach(QString title, actionCollections.keys())
    {
        KGlobalAccel::self()->overrideMainComponentData(KComponentData(title.toAscii()));
        KActionCollection* ac = actionCollections[title];
        foreach( QString s, shortcuts.keys() )
        {
            if( s.startsWith(title) )
            {
                qobject_cast<KAction*>(ac->action(s))->setGlobalShortcut(KShortcut(shortcuts[s]));
                qobject_cast<KAction*>(ac->action(s))->setGlobalShortcutAllowed(true,KAction::NoAutoloading);
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
