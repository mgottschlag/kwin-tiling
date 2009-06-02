/**
 * Copyright (C) 1999-2001 Lubos Lunak <l.lunak@kde.org>
 * Copyright (C) 2009 Michael Jansen <kde@michael-jansen.biz>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License version 2 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB. If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 **/

#include "settings.h"

#include "action_data/action_data.h"
#include "settings_reader_v2.h"
#include "settings_writer.h"
#include "windows_helper/window_selection_list.h"

#include <KDE/KConfig>
#include <KDE/KDebug>
#include <KDE/KMessageBox>
#include <KDE/KStandardDirs>

namespace KHotKeys
{

// Settings

Settings::Settings()
    : m_actions( NULL ),
      gestures_exclude(NULL)
    {
    reinitialize();
    }


Settings::~Settings()
    {
    delete m_actions; m_actions = 0;
    }


ActionDataGroup *Settings::actions()
    {
    return m_actions;
    }


const ActionDataGroup *Settings::actions() const
    {
    return m_actions;
    }


bool Settings::areGesturesDisabled() const
    {
    return gestures_disabled;
    }


void Settings::disableDaemon()
    {
    daemon_disabled = true;
    }


void Settings::disableGestures()
    {
    gestures_disabled = true;
    }


void Settings::enableDaemon()
    {
    daemon_disabled = false;
    }


void Settings::enableGestures()
    {
    gestures_disabled = false;
    }


void Settings::exportTo(ActionDataBase *what, KConfigBase &config)
    {
    SettingsWriter writer(this);
    writer.exportTo(what, config);
    }


int Settings::gestureMouseButton() const
    {
    return gesture_mouse_button;
    }


Windowdef_list *Settings::gesturesExclude()
    {
    return gestures_exclude;
    }


const Windowdef_list *Settings::gesturesExclude() const
    {
    return gestures_exclude;
    }


int Settings::gestureTimeOut() const
    {
    return gesture_timeout;
    }


bool Settings::isDaemonDisabled() const
    {
    return daemon_disabled;
    }


bool Settings::loadDefaults()
    {
    reinitialize();

    // Load the default set.
    QString installPath = KGlobal::dirs()->installPath("data");

    KConfig file(installPath + "khotkeys/defaults.khotkeys");
    if (read_settings(m_actions, file, true, false))
        {
        kDebug() << "Loaded defaults from" << file.name();
        already_imported.append("defaults");
        return true;
        }
    else
        {
        kDebug() << "Failed to load defaults from" << file.name();
        return false;
        }

    }


void Settings::reinitialize()
    {
    // Rereading settings. First delete what we have
    setActions(NULL);

    gestures_disabled = true;
    gesture_mouse_button = 2;
    gesture_timeout = 300;
    gestures_exclude = NULL,

    daemon_disabled = false;

    // Currently unused
    voice_shortcut = KShortcut();

    already_imported = QStringList();
    }


void Settings::setActions( ActionDataGroup *actions )
    {
    delete m_actions;

    m_actions = actions
        ? actions
        : new ActionDataGroup(
                NULL,
                "should never see",
                "should never see",
                NULL,
                ActionDataGroup::SYSTEM_ROOT);
    m_actions->enable();
    }


void Settings::setGesturesExclude( Windowdef_list *gestures )
    {
    delete gestures_exclude;
    gestures_exclude = gestures;
    }


void Settings::setGestureMouseButton( int mouse_button )
    {
    gesture_mouse_button = mouse_button;
    }


void Settings::setGestureTimeOut(int timeout)
    {
    gesture_timeout = timeout;
    }


void Settings::setVoiceShortcut( const KShortcut &shortcut )
    {
    voice_shortcut = shortcut;
    }


ActionDataGroup *Settings::takeActions()
    {
    ActionDataGroup *res = m_actions;
    m_actions = 0;
    return res;
    }


KShortcut Settings::voiceShortcut() const
    {
    return voice_shortcut;
    }


bool Settings::import( KConfig& config, bool ask )
    {
    return importFrom(m_actions, config, ask);
    }


bool Settings::importFrom(ActionDataGroup *element, KConfigBase const &config, bool ask)
    {
    KConfigGroup mainGroup(&config, "Main");

    // A file can have a import id.
    QString import_id = mainGroup.readEntry( "ImportId" );
    if( !import_id.isEmpty())
        {
        // File has a id. Check for a previous import.
        if( already_imported.contains( import_id ))
            {
            // Ask the user?
            if( !ask || KMessageBox::warningContinueCancel(
                            NULL,
                            i18n( "This \"actions\" file has already been imported before. "
                                  "Are you sure you want to import it again?" )) != KMessageBox::Continue )
                {
                return true; // import "successful"
                }
            }
        else
            {
            already_imported.append(import_id);
            }
        }
    else
        {
        // File has no import id
        if( ask && KMessageBox::warningContinueCancel( NULL,
                i18n( "This \"actions\" file has no ImportId field and therefore it cannot be determined "
                      "whether or not it has been imported already. Are you sure you want to import it?" ))
                == KMessageBox::Cancel )
            return true;
        }

    // Include Disabled, Disable the imported actions
    return read_settings(element, config, true, true);
    }


void Settings::validate()
    {
    // Create the KMenuEdit group if it does not yet exist
    get_system_group(ActionDataGroup::SYSTEM_MENUENTRIES);
    }


ActionDataGroup *Settings::get_system_group(ActionDataGroup::system_group_t group_id)
    {
    Q_ASSERT(m_actions);

    // Search for the menuentries system group.
    ActionDataGroup *system_group = NULL;

    Q_FOREACH(KHotKeys::ActionDataBase* element, m_actions->children())
        {
        ActionDataGroup *group = dynamic_cast<ActionDataGroup*>(element);

        if (group && (group->system_group() == group_id))
            {
            system_group = group;
            break;
            }
        }

    // Check if we found the group
    if (system_group==NULL)
        {
        switch (group_id)
            {
            case ActionDataGroup::SYSTEM_MENUENTRIES:
                system_group = new ActionDataGroup(
                        m_actions,
                        "KMenuEdit",
                        "KMenuEdit Global Shortcuts",
                        NULL,
                        ActionDataGroup::SYSTEM_MENUENTRIES);
                system_group->enable();
                break;

            default:
                Q_ASSERT(false);
                return NULL;
            }
        }

    Q_ASSERT(system_group);
    return system_group;
    }


bool Settings::reread_settings(bool include_disabled)
    {
    KConfig config( KHOTKEYS_CONFIG_FILE );

    // If we read the main settings and there is no main. Initialize the file
    // and return
    KConfigGroup mainGroup( &config, "Main" ); // main group
    if (!mainGroup.exists())
        {
        loadDefaults();
        validate();
        return false;
        }

    // First delete what we have
    reinitialize();

    // ### Read the global configurations. Reinitialize sets the default
    daemon_disabled = mainGroup.readEntry( "Disabled", daemon_disabled);

    // ### List of already imported configuration files
    already_imported = mainGroup.readEntry(
            "AlreadyImported",
            QStringList());

    // ### Gestures
    KConfigGroup gesturesConfig( &config, "Gestures" );
    // ### Read the gesture configurations. Reinitialize sets the default.
    // Keep them
    gestures_disabled = gesturesConfig.readEntry( "Disabled", gestures_disabled);
    gesture_mouse_button = gesturesConfig.readEntry( "MouseButton", gesture_mouse_button );
    gesture_mouse_button = qBound( 2, gesture_mouse_button, 9 );
    gesture_timeout = gesturesConfig.readEntry( "Timeout", gesture_timeout );

    // Somhow gesture_timeout found it's way into my config file. Fix it for
    // everyone else too.
    if (gesture_timeout < 100) gesture_timeout = 300;

    KConfigGroup gesturesExcludeConfig( &config, "GesturesExclude" );
    delete gestures_exclude;
    gestures_exclude = new Windowdef_list( gesturesExcludeConfig );

    // ### Voice
    KConfigGroup voiceConfig( &config, "Voice" );
    voice_shortcut=KShortcut( voiceConfig.readEntry("Shortcut" , "")  );

    bool rc = read_settings(m_actions, config, include_disabled);
    // Ensure the system groups exist
    validate();
    return rc;
    }


bool Settings::read_settings(ActionDataGroup *root, KConfigBase const &config, bool include_disabled, bool disable)
    {
    // If the config group we should read from is empty, return.
    if(config.groupList().count() == 0 )
        {
        kDebug() << "Empty group! Returning";
        return false;
        }

    KConfigGroup mainGroup( &config, "Main" ); // main group
    int version = mainGroup.readEntry( "Version", -1234576 );
    switch (version)
        {
        case 1:
            kError() << "Version 1 file no longer supported!";
            break;

        case 2:
                {
                kDebug() << "Version 2 File!";
                SettingsReaderV2 reader(this, include_disabled, disable);
                reader.read(config, root);
                }
            break;

        case -1234576: // no config file
            kWarning() << "No configuration file!";
            return false;

        default:
            kWarning() << "Unknown cfg. file version\n";
            return false;
        }

    return true;
    }


bool Settings::update()
    {
    QStringList updates(KGlobal::dirs()->findAllResources("data", "khotkeys/*.khotkeys"));
    bool imported(false);

    Q_FOREACH (const QString &path, updates)
        {
        // Import checks if the file was already imported.
        KConfig file(path);
        if (import(file, false))
            {
            kDebug() << "Imported file" << path;
            imported = true;
            }
        }

    if (imported)
        {
        write();
        }
    return false;
    }


void Settings::write()
    {
    KConfig cfg( KHOTKEYS_CONFIG_FILE );
    SettingsWriter writer(this);
    writer.writeTo(cfg);
    }

} // namespace KHotKeys
