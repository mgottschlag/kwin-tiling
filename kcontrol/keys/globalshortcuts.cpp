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

#include <QCoreApplication>
#include <QDBusConnection>
#include <QDBusError>
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

//### copied over from kdedglobalaccel.cpp (figure out a header file to put it in!)
enum actionIdFields
{
    ComponentUnique = 0,
    ActionUnique = 1,
    ComponentFriendly = 2,
    ActionFriendly = 3
};


K_PLUGIN_FACTORY(GlobalShortcutsModuleFactory, registerPlugin<GlobalShortcutsModule>();)
K_EXPORT_PLUGIN(GlobalShortcutsModuleFactory("kcmkeys"))

Q_DECLARE_METATYPE(QList<int>)

GlobalShortcutsModule::GlobalShortcutsModule(QWidget *parent, const QVariantList &args)
 : KCModule(GlobalShortcutsModuleFactory::componentData(), parent, args),
   editor(0)
{
    KCModule::setButtons(KCModule::Buttons(KCModule::Default | KCModule::Apply));

    // Add import scheme button
    KPushButton *importButton = new KPushButton(this);
    importButton->setText(i18n("Import scheme..."));
    connect(importButton, SIGNAL(clicked()), this, SLOT(importScheme()));

    // Add export scheme button
    KPushButton *exportButton = new KPushButton(this);
    exportButton->setText(i18n("Export scheme..."));
    connect(exportButton, SIGNAL(clicked()), this, SLOT(exportScheme()));

    // Layout for the buttons
    QHBoxLayout *hbox = new QHBoxLayout;
    hbox->addWidget(exportButton);
    hbox->addWidget(importButton);

    // Create the kglobaleditor
    editor = new KGlobalShortcutsEditor(this, KShortcutsEditor::GlobalAction);
    connect(editor, SIGNAL(changed(bool)), this, SIGNAL(changed(bool)));

    // Layout the hole bunch
    QVBoxLayout *global = new QVBoxLayout;
    global->addLayout(hbox);
    global->addWidget(editor);
    setLayout(global);
}

GlobalShortcutsModule::~GlobalShortcutsModule()
{}


void GlobalShortcutsModule::load()
{
    // Undo all changes not yet applied
    editor->clear();

    // Connect to kdedglobalaccel. If that fails there is no need to continue.
    qRegisterMetaType<QList<int> >();
    qDBusRegisterMetaType<QList<int> >();
    QDBusConnection bus = QDBusConnection::sessionBus();
    QPointer<QDBusInterface> iface = new QDBusInterface("org.kde.kded", "/KdedGlobalAccel",
                                                        "org.kde.KdedGlobalAccel", bus, this);
    if (!iface->isValid()) {
        QString errorString;
        QDBusError error = iface->lastError();
        if (error.isValid()) {
            errorString = i18n("Message: %1\nError: %2", error.message(), error.name());
        }
        KMessageBox::sorry(
            this,
            i18n("Failed to contact the KDE global shortcuts daemon\n")
                + errorString );
        return;
    }

    KGlobalAccel *kga = KGlobalAccel::self();
    QList<QStringList> components = kga->allMainComponents();

    QHash<QString, KActionCollection *> actionCollections;

    foreach (const QStringList &componentId, components) {
        // kDebug() << "component:" << componentId;
        const QString &componentUnique = componentId[ComponentUnique];
        kga->overrideMainComponentData(KComponentData(componentUnique.toAscii()));
        KActionCollection* col = new KActionCollection(this);
        actionCollections[componentUnique] = col;

        QList<QStringList> actions = kga->allActionsForComponent(componentId);
        foreach (const QStringList &actionId, actions) {
            const QString &objectName = actionId[ActionUnique];
            KAction *action = col->addAction(objectName);
            action->setProperty("isConfigurationAction", QVariant(true)); // see KAction::~KAction
            action->setText(actionId[ActionFriendly]);

            QDBusReply<QList<int> > defaultShortcut = iface->call("defaultShortcut", qVariantFromValue(actionId));
            QDBusReply<QList<int> > shortcut = iface->call("shortcut", qVariantFromValue(actionId));
            if (!defaultShortcut.value().isEmpty()) {
                int key = defaultShortcut.value().first();
                action->setGlobalShortcut(KShortcut(key), KAction::DefaultShortcut, KAction::NoAutoloading);
            }
            int key = 0;
            if (!shortcut.value().isEmpty()) {
                key = shortcut.value().first();
            }
            // Always call this to enable global shortcuts for the action. The editor widget checks it.
            action->setGlobalShortcut(KShortcut(key), KAction::ActiveShortcut, KAction::NoAutoloading);
        }
    }

    // Add components and their keys to the editor
    foreach (const QStringList &componentId, components) {
        // kDebug() << "Adding collection " << component;
        editor->addCollection(actionCollections[componentId[ComponentUnique]], componentId);
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
                         KGuiItem(i18n("Load")));
        if (choice != KMessageBox::Continue) {
            return;
        }
    }

    SelectSchemeDialog dialog(this);
    if (dialog.exec() != KDialog::Accepted) {
        return;
    }

    KUrl url = dialog.selectedScheme();
    if (!url.isLocalFile()) {
        KMessageBox::sorry(this, i18n("This file (%1) does not exist. You can only select local files.",
                           url.url()));
        return;
    }
    kDebug() << url.path();
    KConfig config(url.path());
    editor->importConfiguration(&config);
}


void GlobalShortcutsModule::exportScheme()
{
    KUrl url = KFileDialog::getSaveFileName(KUrl(), "*.kksrc", parentWidget());
    if (!url.isEmpty()) {
        KConfig config(url.path());
        config.deleteGroup("Shortcuts");
        config.deleteGroup("Global Shortcuts");
        editor->exportConfiguration(&config);
    }
}

#include "globalshortcuts.moc"
