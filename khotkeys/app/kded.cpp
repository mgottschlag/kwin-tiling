/****************************************************************************

 KHotKeys

 Copyright (C) 1999-2001 Lubos Lunak <l.lunak@kde.org>

 Distributed under the terms of the GNU General Public License version 2.

****************************************************************************/

#include "kded.h"

#include "action_data/action_data_group.h"
#include "action_data/menuentry_shortcut_action_data.h"
#include "actions/actions.h"

#include "shortcuts_handler.h"

#include "triggers/gestures.h"


#include <kaboutdata.h>
#include <kdebug.h>
#include <kglobalaccel.h>
#include <kpluginfactory.h>
#include <kpluginloader.h>

#include <unistd.h>

#define COMPONENT_NAME "khotkeys"

K_PLUGIN_FACTORY(KHotKeysModuleFactory,
                 registerPlugin<KHotKeysModule>();
    )
K_EXPORT_PLUGIN(KHotKeysModuleFactory(COMPONENT_NAME))

using namespace KHotKeys;

// KhotKeysModule

KHotKeysModule::KHotKeysModule(QObject* parent, const QList<QVariant>&)
    : KDEDModule(parent)
    , actions_root(NULL)
    , _settings()
    ,_initialized(false)
    {
    // initialize
    kDebug() << "Installing the delayed initialization callback.";
    QMetaObject::invokeMethod( this, "initialize", Qt::QueuedConnection);
    }


void KHotKeysModule::initialize()
    {
    if (_initialized)
        {
        return;
        }

    kDebug() << "Delayed initialization.";
    _initialized = true;

    // Initialize the global data, grab keys
    KHotKeys::init_global_data( true, this );

    // If a shortcut was changed (global shortcuts kcm), save
    connect(
            keyboard_handler.data(), SIGNAL(shortcutChanged()),
            this, SLOT(save()));

    // Read the configuration from file khotkeysrc
    reread_configuration();

    KGlobalAccel::cleanComponent(COMPONENT_NAME);

    if (_settings.update())
        {
        save();
        }

    }


KHotKeysModule::~KHotKeysModule()
    {
    // actions_root belongs to _settings.
    actions_root = NULL;
    }


void KHotKeysModule::reread_configuration()
    {
    kDebug() << "Reloading the khotkeys configuration";

    // Stop listening
    actions_root = NULL; // Disables the dbus interface effectively
    KHotKeys::khotkeys_set_active( false );

    // Load the settings
    _settings.reread_settings(true);

    KHotKeys::gesture_handler->set_mouse_button( _settings.gestureMouseButton() );
    KHotKeys::gesture_handler->set_timeout( _settings.gestureTimeOut() );
    kDebug() << _settings.areGesturesDisabled();
    KHotKeys::gesture_handler->enable( !_settings.areGesturesDisabled() );
    KHotKeys::gesture_handler->set_exclude( _settings.gesturesExclude() );
    // FIXME: SOUND
    // KHotKeys::voice_handler->set_shortcut( _settings.voice_shortcut );
    actions_root = _settings.actions();
    KHotKeys::khotkeys_set_active( true );
    }




SimpleActionData* KHotKeysModule::menuentry_action(const QString &storageId)
    {
    ActionDataGroup *menuentries = _settings.get_system_group(
            ActionDataGroup::SYSTEM_MENUENTRIES);

    // Now try to find the action
    Q_FOREACH(ActionDataBase* element, menuentries->children())
        {
        SimpleActionData *actionData = dynamic_cast<SimpleActionData*>(element);

        if (actionData && actionData->action())
            {
            MenuEntryAction *action = dynamic_cast<MenuEntryAction*>(actionData->action());
            if (action && action->service() && (action->service()->storageId() == storageId))
                {
                return actionData;
                }
            }
        }

    return NULL;
    }


QString KHotKeysModule::get_menuentry_shortcut(const QString &storageId)
    {
    SimpleActionData* actionData = menuentry_action(storageId);

    // No action found
    if (actionData == NULL) return "";

    // The action must have a shortcut trigger. but don't assume to much
    ShortcutTrigger* shortcutTrigger = dynamic_cast<ShortcutTrigger*>(actionData->trigger());

    Q_ASSERT(shortcutTrigger);
    if (shortcutTrigger == NULL) return "";

    return shortcutTrigger->shortcut().primary();
    }


QString KHotKeysModule::register_menuentry_shortcut(
        const QString &storageId,
        const QString &sequence)
    {
    kDebug() << storageId << "(" << sequence << ")";

    // Check the service we got. If it is invalid there is no need to
    // continue.
    KService::Ptr wantedService = KService::serviceByStorageId(storageId);
    if (wantedService.isNull())
        {
        kError() << "Storage Id " << storageId << "not valid";
        return "";
        }

    // Look for the action
    SimpleActionData* actionData = menuentry_action(storageId);

    // No action found. Create on if sequence is != ""
    if (actionData == NULL)
        {
        kDebug() << "No action found";

        // If the sequence is empty there is no need to create a action.
        if (sequence.isEmpty()) return "";

        kDebug() << "Creating a new action";

        // Create the action
        ActionDataGroup *menuentries = _settings.get_system_group(
                ActionDataGroup::SYSTEM_MENUENTRIES);

        MenuEntryShortcutActionData *newAction = new MenuEntryShortcutActionData(
                menuentries,
                wantedService->name(),
                storageId,
                KShortcut(sequence),
                storageId);

        newAction->enable();

        _settings.write();

        // Return the real shortcut
        return newAction->trigger()->shortcut().primary();
        }
    // We found a action
    else
        {
        if (sequence.isEmpty())
            {
            kDebug() << "Deleting the action";
            actionData->aboutToBeErased();
            delete actionData;
            _settings.write();
            return "";
            }
        else
            {
            kDebug() << "Changing the action";
            // The action must have a shortcut trigger. but don't assume to much
            ShortcutTrigger* shortcutTrigger =
                    dynamic_cast<ShortcutTrigger*>(actionData->trigger());
            Q_ASSERT(shortcutTrigger);
            if (shortcutTrigger == NULL) return "";

            // Change the actionData
            shortcutTrigger->set_key_sequence(sequence);
            _settings.write();

            // Remove the resulting real shortcut
            return shortcutTrigger->shortcut().primary();
            }
        }

    Q_ASSERT(false);
    return "";
    }


void KHotKeysModule::quit()
    {
    deleteLater();
    }


void KHotKeysModule::save()
    {
    KHotKeys::khotkeys_set_active( false );
    _settings.write();
    KHotKeys::khotkeys_set_active( true );
    }

#include "kded.moc"
