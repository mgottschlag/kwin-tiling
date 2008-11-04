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
#include "kdedglobalaccel_interface.h"
#include "kdedglobalaccel_component_interface.h"
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
#include <KLocale>
#include <KMessageBox>
#include <KPluginFactory>
#include <KPushButton>

K_PLUGIN_FACTORY(GlobalShortcutsModuleFactory, registerPlugin<GlobalShortcutsModule>();)
K_EXPORT_PLUGIN(GlobalShortcutsModuleFactory("kcmkeys"))

GlobalShortcutsModule::GlobalShortcutsModule(QWidget *parent, const QVariantList &args)
 : KCModule(GlobalShortcutsModuleFactory::componentData(), parent, args),
   editor(0)
{
    KCModule::setButtons(KCModule::Buttons(KCModule::Default | KCModule::Apply));

    // Add import scheme button
    KPushButton *importButton = new KPushButton(this);
    importButton->setText(i18n("Import Scheme..."));
    connect(importButton, SIGNAL(clicked()), this, SLOT(importScheme()));

    // Add export scheme button
    KPushButton *exportButton = new KPushButton(this);
    exportButton->setText(i18n("Export Scheme..."));
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
    // Connect to kdedglobalaccel. If that fails there is no need to continue.
    qRegisterMetaType<QList<int> >();
    qDBusRegisterMetaType<QList<int> >();
    qDBusRegisterMetaType<QList<KGlobalShortcutInfo> >();
    qDBusRegisterMetaType<KGlobalShortcutInfo>();
    QDBusConnection bus = QDBusConnection::sessionBus();
    org::kde::KdedGlobalAccel kdedglobalaccel(
            "org.kde.kded",
            "/modules/kdedglobalaccel",
            bus);

    if (!kdedglobalaccel.isValid()) {
        QString errorString;
        QDBusError error = kdedglobalaccel.lastError();
        // The global shortcuts DBus service manages all global shortcuts and we
        // can't do anything useful without it.
        if (error.isValid()) {
            errorString = i18n("Message: %1\nError: %2", error.message(), error.name());
        }

        KMessageBox::sorry(
            this,
            i18n("Failed to contact the KDE global shortcuts daemon\n")
                + errorString );
        return;
    }

    // Undo all changes not yet applied
    editor->clear();

    QDBusReply< QList<QDBusObjectPath> > componentsRc = kdedglobalaccel.allComponents();
    if (!componentsRc.isValid())
        {
        kDebug() << "allComponents() failed!";
        return;
        }
    QList<QDBusObjectPath> components = componentsRc;

    Q_FOREACH(QDBusObjectPath componentPath, components) {

        // Get the component
        org::kde::kdedglobalaccel::Component component(
                "org.kde.kded",
                componentPath.path(),
                bus);
        if (!component.isValid()) {
            kDebug() << "Component " << componentPath.path() << "not valid! Skipping!";
            continue;
        }

        // Get the shortcut contexts.
        QDBusReply<QStringList> shortcutContextsRc = component.getShortcutContexts();
        if (!shortcutContextsRc.isValid()) {
            kDebug() << "Failed to get contexts for component "
                     << componentPath.path() <<"! Skipping!";
            continue;
        }
        QStringList shortcutContexts = shortcutContextsRc;

        // We add shortcut all shortcut context to the editor. Thias way the
        // user keep full control of it's shortcuts.
        Q_FOREACH (QString shortcutContext, shortcutContexts) {

            QDBusReply< QList<KGlobalShortcutInfo> > shortcutsRc =
                component.allShortcutInfos(shortcutContext);
            if (!shortcutsRc.isValid())
                {
                kDebug() << "allShortcutInfos() failed for " << componentPath.path() << shortcutContext;
                continue;
                }
            QList<KGlobalShortcutInfo> shortcuts = shortcutsRc;
            // Shouldn't happen. But you never know
            if (shortcuts.isEmpty()) {
                kDebug() << "Got shortcut context" << shortcutContext << "without shortcuts for"
                    << componentPath.path();
                continue;
            }

            // It's safe now
            const QString componentUnique = shortcuts[0].componentUniqueName();
            QString componentContextId = componentUnique;
            // kdedglobalaccel knows that '|' is our separator between
            // component and context
            if (shortcutContext != "default") {
                componentContextId += QString("|") + shortcutContext;
            }

            // Create a action collection for our current component:context
            KActionCollection* col = new KActionCollection(
                    this,
                    KComponentData(componentContextId.toAscii()));

            // Now add the shortcuts.
            foreach (const KGlobalShortcutInfo &shortcut, shortcuts) {

                const QString &objectName = shortcut.uniqueName();
                KAction *action = col->addAction(objectName);
                action->setProperty("isConfigurationAction", QVariant(true)); // see KAction::~KAction
                action->setText(shortcut.friendlyName());

                // Always call this to enable global shortcuts for the action. The editor widget
                // checks it.
                // Also actually loads the shortcut using the KAction::Autoloading mechanism.
                // Avoid setting the default shortcut; it would just be written to the global
                // configuration so we would not get the real one below.
                action->setGlobalShortcut(KShortcut(), KAction::ActiveShortcut);

                // The default shortcut will never be loaded because it's pointless in a real
                // application. There are no scarce resources [i.e. physical keys] to manage
                // so applications can set them at will and there's no autoloading.
                QList<QKeySequence> sc = shortcut.defaultKeys();
                if (sc.count()>0) {
                    action->setGlobalShortcut(KShortcut(sc[0]), KAction::DefaultShortcut);
                }
            } // Q_FOREACH(shortcut)

            QString componentFriendlyName = shortcuts[0].componentFriendlyName();

            if (shortcuts[0].contextUniqueName() != "default")
                {
                componentFriendlyName +=
                    QString('[') + shortcuts[0].contextFriendlyName() + QString(']');
                }

            editor->addCollection(col, componentContextId, componentFriendlyName);

        } // Q_FOREACH(context)
    } // Q_FOREACH(component)
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
                         i18n("Load Shortcurt Scheme"),
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
