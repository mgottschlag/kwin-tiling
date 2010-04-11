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

#include <QtGui/QApplication>

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


void Settings::exportTo(
        ActionDataBase *what,
        KConfigBase &config,
        const QString &id,
        KHotKeys::ActionState state,
        bool allowMerging)
    {
    SettingsWriter writer(this, state, id, allowMerging);
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
    if (read_settings(m_actions, file, true, Enabled))
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


bool Settings::import(KConfig& config, ImportType ask, ActionState state)
    {
    return importFrom(m_actions, config, ask, state);
    }


bool Settings::isConfigFileValid(KConfigBase const &config, ImportType ask)
    {
    bool valid = false;

    // The file is only valid if it has a main group
    KConfigGroup mainGroup( &config, "Main" );
    if (mainGroup.isValid())
        {
        // Now check the version
        int version = mainGroup.readEntry( "Version", -1234576 );
        switch (version)
            {
            case 2:
                valid = true;
                break;

            case 1:         // Version 1 files no longer supported
                kDebug() << "Version 1 file encountered.";
                break;

            case -1234576:  // No Version entry -> invalid file
                kDebug() << "No version specified in file:";
                valid = false;
                break;

            default:
                kDebug() << "Invalid Version found:" << version;
                valid = false;
                break;
            }
        }

    // if it's valid we are finished.
    if (valid) return valid;

    // See if we should inform the user.
    switch (ask)
        {
        case ImportAsk:
                {
                KMessageBox::information(
                        QApplication::activeWindow(),
                        "The specified file is empty or not a configuration file",
                        "Import actions");
                }
            break;

        case ImportSilent:
            break;

        default:
            Q_ASSERT(false);
        }

    return valid;
    }


bool Settings::importFrom(ActionDataGroup *element, KConfigBase const &config, ImportType ask, ActionState state)
    {
    // Make sure the given file is valid
    if (!isConfigFileValid(config, ask)) return false;

    KConfigGroup mainGroup(&config, "Main");
    // A file can have a import id.
    QString import_id = mainGroup.readEntry( "ImportId" );
    if (!import_id.isEmpty())
        {
        // File has a id. Check for a previous import.
        if (already_imported.contains( import_id ))
            {
            switch (ask)
                {
                case ImportAsk:
                    // Ask the user?
                    if( ask == ImportSilent
                            || ( ask == ImportAsk && KMessageBox::warningContinueCancel(
                                    NULL,
                                    i18n( "This \"actions\" file has already been imported before. "
                                          "Are you sure you want to import it again?" )) != KMessageBox::Continue ) )
                        {
                        return true; // import "successful"
                        }
                    break;

                case ImportSilent:
                    return true;

                default:
                    // Unknown ImportType. Most likely None.
                    Q_ASSERT(false);
                    return true;
                }
            }
        else
            {
            already_imported.append(import_id);
            }
        }
    else
        {
        switch (ask)
            {
            case ImportAsk:
                if (KMessageBox::warningContinueCancel(
                                NULL,
                                i18n( "This \"actions\" file has no ImportId field and therefore it cannot be determined "
                                      "whether or not it has been imported already. Are you sure you want to import it?" ))
                        == KMessageBox::Cancel )
                    {
                    return true;
                    }
                break;

            case ImportSilent:
                return true;

            default:
                // Unknown ImportType. Most likely None.
                Q_ASSERT(false);
                return true;
            }
        }

    // Include Disabled, Disable the imported actions
    return read_settings(element, config, true, state);
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

    bool rc = read_settings(m_actions, config, include_disabled, Retain);
    // Ensure the system groups exist
    validate();
    return rc;
    }


bool Settings::read_settings(ActionDataGroup *root, KConfigBase const &config, bool include_disabled, ActionState stateStrategy)
    {
    // Make sure the given file is valid
    if (!isConfigFileValid(config, ImportSilent)) return false;

    KConfigGroup mainGroup( &config, "Main" ); // main group
    int version = mainGroup.readEntry( "Version", -1234576 );
    QString import_id = mainGroup.readEntry( "ImportId" );
    switch (version)
        {
        case 2:
                {
                kDebug() << "Version 2 File!";
                SettingsReaderV2 reader(this, include_disabled, stateStrategy, import_id);
                reader.read(config, root);
                }
            break;

        default:
            // All other values are impossible because of the
            // isConfigFileValid() call above.
            Q_ASSERT(false);
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
        if (import(file, ImportSilent, Retain))
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
    SettingsWriter writer(this, Retain);
    writer.writeTo(cfg);
    }

} // namespace KHotKeys
