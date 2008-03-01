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
#include "select_scheme_dialog.h"
#include <kdebug.h>

#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDBusMetaType>
#include <QLayout>
#include <QPointer>

#include <KAction>
#include <KActionCollection>
#include <KConfig>
#include <KConfigGroup>
#include <KFileDialog>
#include <KGlobalAccel>
#include <KLocale>
#include <KMessageBox>
#include <KPluginFactory>
#include <KPushButton>

K_PLUGIN_FACTORY(GlobalShortcutsModuleFactory, registerPlugin<GlobalShortcutsModule>();)
K_EXPORT_PLUGIN(GlobalShortcutsModuleFactory("kcmkeys"))

Q_DECLARE_METATYPE( QList<int> )

GlobalShortcutsModule::GlobalShortcutsModule( QWidget * parent, const QVariantList & args )
    : KCModule(GlobalShortcutsModuleFactory::componentData(), parent, args),
      editor(0)
{
    KCModule::setButtons( KCModule::Buttons(KCModule::Default|KCModule::Apply) );

    // Add import scheme button
    KPushButton *importButton = new KPushButton(this);
    importButton->setText(i18n("Import scheme ..."));
    connect( 
        importButton, SIGNAL(clicked()),
        this,SLOT(importScheme()) );

    // Add export scheme button
    KPushButton *exportButton = new KPushButton(this);
    exportButton->setText(i18n("Export scheme ..."));
    connect( 
        exportButton, SIGNAL(clicked()),
        this,SLOT(exportScheme()) );

    // Layout for the buttons
    QHBoxLayout *hbox = new QHBoxLayout;
    hbox->addWidget(exportButton);
    hbox->addWidget(importButton);

    // Create the kglobaleditor
    editor = new KGlobalShortcutsEditor(this, KShortcutsEditor::GlobalAction);
    connect(editor, SIGNAL(changed(bool)), this, SIGNAL(changed(bool)) );

    // Layout the hole bunch
    QVBoxLayout *global = new QVBoxLayout;
    global->addLayout(hbox);
    global->addWidget(editor);
    setLayout(global);

    // Initialize our content
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
            kWarning() << "FIXME: Get a real objectName from iface!";
            QString objectName = actionText;
            KAction *action = col->addAction(objectName);
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


void GlobalShortcutsModule::importScheme()
{
    // Check for unsaved modifications
    if (editor->isModified()) {
        int choice = KMessageBox::warningContinueCancel(
            parentWidget(),
            i18n("Your current changes will be lost if you load another scheme before saving this one"),
            i18n("Load shortcurt scheme"),
            KGuiItem(i18n("Load")) );
        if (choice != KMessageBox::Continue)
            {
            return;
            }
    }

    SelectSchemeDialog dialog(this);
    if (dialog.exec() != KDialog::Accepted) {
        return;
    }

    KUrl url = dialog.selectedScheme();
    Q_ASSERT(url.isLocalFile());
    kDebug() << url.path();
    KConfig config(url.path());
    editor->importConfiguration(&config);
}


void GlobalShortcutsModule::exportScheme()
{
    KUrl url = KFileDialog::getSaveFileName( KUrl(), "*.kksrc", parentWidget() );
    if (!url.isEmpty()) {
        KConfig config(url.path());
        config.deleteGroup( "Shortcuts" );
        config.deleteGroup( "Global Shortcuts" );
        editor->exportConfiguration(&config);
    }
}

#include "globalshortcuts.moc"
