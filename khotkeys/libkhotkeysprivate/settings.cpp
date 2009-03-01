/****************************************************************************

 KHotKeys

 Copyright (C) 1999-2001 Lubos Lunak <l.lunak@kde.org>

 Distributed under the terms of the GNU General Public License version 2.

****************************************************************************/

#define _SETTINGS_CPP_

#include "settings.h"

#include "action_data/action_data.h"
#include "action_data/menuentry_shortcut_action_data.h"
#include "action_data/command_url_shortcut_action_data.h"

#include "triggers/triggers.h"
#include "conditions/conditions.h"
#include "conditions/conditions_list.h"

#include "windows_helper/window_selection_list.h"

#include <kconfig.h>
#include <kconfiggroup.h>
#include <kdebug.h>
#include <khotkeysglobal.h>
#include <klocale.h>
#include <kglobal.h>
#include <kmessagebox.h>

namespace KHotKeys
{

const int Settings::CurrentFileVersion = 2;

// Settings

Settings::Settings()
    : m_actions( NULL ),
      gestures_disabled(false),
      gesture_mouse_button(0),
      gesture_timeout(0),
      gestures_exclude(NULL),
      daemon_disabled(false)
    {
    }


Settings::~Settings()
    {
    delete m_actions; m_actions = 0;
    }


ActionDataGroup *Settings::actions()
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


void Settings::exportTo(ActionDataBase *element, KConfigBase &config)
    {
    if (!element)
        {
        Q_ASSERT(element);
        return;
        }

    // Clean the file
    QStringList groups = config.groupList();
    Q_FOREACH (QString name, config.groupList())
        {
        config.deleteGroup(name);
        }

    KConfigGroup mainGroup(&config, "Main");
    mainGroup.writeEntry("Version", CurrentFileVersion);

    // The root group contains nothing but the datacount!
    KConfigGroup dataGroup(&config,  "Data");
    dataGroup.writeEntry("DataCount", 1);

    // The group for the element to export
    KConfigGroup data1Group(&config,  "Data_1");
    element->cfg_write(data1Group);

    ActionDataGroup *group = dynamic_cast<ActionDataGroup*>(element);
    if (group)
        {
        write_actions_recursively_v2(data1Group, group, true);
        }
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


void Settings::setActions( ActionDataGroup *actions )
    {
    delete m_actions;
    m_actions = actions;
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

    // List of already imported configuration files
    already_imported = mainGroup.readEntry( "AlreadyImported",QStringList() );

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
            already_imported.append( import_id );
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

    return read_settings(element, config, true);
    }


void Settings::initialize()
    {
    // Create the KMenuEdit group
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
                        ActionDataGroup::SYSTEM_MENUENTRIES,
                        true);
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

    // Rereading settings. First delete what we have
    setActions(NULL);

    // Initialize m_actions
    m_actions = new ActionDataGroup(
            NULL,
            "should never see",
            "should never see",
            NULL,
            ActionDataGroup::SYSTEM_ROOT,
            true );

    // If we read the main settings and there is no main. Initialize the file
    // and return
    KConfigGroup mainGroup( &config, "Main" ); // main group
    if (!mainGroup.exists())
        {
        initialize();
        return false;
        }

    // ### Read the global configurations
    daemon_disabled = mainGroup.readEntry( "Disabled", false);

    // ### Gestures
    KConfigGroup gesturesConfig( &config, "Gestures" );
    gestures_disabled = gesturesConfig.readEntry( "Disabled", false);
    gesture_mouse_button = gesturesConfig.readEntry( "MouseButton", 2 );
    gesture_mouse_button = qBound( 2, gesture_mouse_button, 9 );
    gesture_timeout = gesturesConfig.readEntry( "Timeout", 300 );

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
    initialize();
    return rc;
    }


bool Settings::read_settings(ActionDataGroup *root, KConfigBase const &config, bool include_disabled)
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
            kDebug() << "Version 1 File!";
            read_settings_v1(root, config);
            break;

        case 2:
            kDebug() << "Version 2 File!";
            read_settings_v2(root, config, include_disabled);
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


void Settings::write_settings()
    {
    KConfig cfg( KHOTKEYS_CONFIG_FILE );

// CHECKME    smazat stare sekce ?
    QStringList groups = cfg.groupList();
    for( QStringList::ConstIterator it = groups.constBegin();
         it != groups.constEnd();
         ++it )
        cfg.deleteGroup( *it );
    KConfigGroup mainGroup( &cfg, "Main" ); // main group
    mainGroup.writeEntry( "Version", 2 ); // now it's version 2 cfg. file
    mainGroup.writeEntry( "AlreadyImported", already_imported );
    KConfigGroup dataGroup( &cfg,  "Data" );
    int cnt = write_actions_recursively_v2( dataGroup, m_actions, true );
    mainGroup.writeEntry( "Autostart", cnt != 0 && !daemon_disabled );
    mainGroup.writeEntry( "Disabled", daemon_disabled );
    KConfigGroup gesturesConfig( &cfg, "Gestures" );
    gesturesConfig.writeEntry( "Disabled", gestures_disabled );
    gesturesConfig.writeEntry( "MouseButton", gesture_mouse_button );
    gesturesConfig.writeEntry( "Timeout", gesture_timeout );
    if( gestures_exclude != NULL )
        {
        KConfigGroup gesturesExcludeConfig( &cfg, "GesturesExclude" );
        gestures_exclude->cfg_write( gesturesExcludeConfig );
        }
    else
        cfg.deleteGroup( "GesturesExclude" );
    KConfigGroup voiceConfig( &cfg, "Voice" );
    voiceConfig.writeEntry("Shortcut" , voice_shortcut.toString() );

    }


// return value means the number of enabled actions written in the cfg file
// i.e. 'Autostart' for value > 0 should be on
int Settings::write_actions_recursively_v2(KConfigGroup& cfg_P, ActionDataGroup* parent_P, bool enabled_P)
    {
    int enabled_cnt = 0;
    QString save_cfg_group = cfg_P.name();
    int cnt = 0;
    if( parent_P )
        {
        Q_FOREACH(ActionDataBase *child,parent_P->children())
            {
            ++cnt;
            if( enabled_P && child->enabled( true ))
                ++enabled_cnt;
            KConfigGroup itConfig( cfg_P.config(), save_cfg_group + '_' + QString::number( cnt ));
            child->cfg_write( itConfig );
            ActionDataGroup* grp = dynamic_cast< ActionDataGroup* >(child);
            if( grp != NULL )
                enabled_cnt += write_actions_recursively_v2( itConfig, grp, enabled_P && child->enabled( true ));
            }
        }
    cfg_P.writeEntry( "DataCount", cnt );
    return enabled_cnt;
    }


void Settings::read_settings_v2(ActionDataGroup *root, KConfigBase const& config, bool include_disabled)
    {
    KConfigGroup dataGroup(&config, "Data");
    read_actions_recursively_v2(dataGroup, root, include_disabled);
    }


void Settings::read_actions_recursively_v2(
        KConfigGroup& cfg_P,
        ActionDataGroup* parent_P,
        bool include_disabled_P )
    {
    QString save_cfg_group = cfg_P.name();
    int cnt = cfg_P.readEntry( "DataCount",0 );
    for( int i = 1;
         i <= cnt;
         ++i )
        {
        KConfigGroup itConfig( cfg_P.config(), save_cfg_group + '_' + QString::number( i ));
        if( include_disabled_P || ActionDataBase::cfg_is_enabled( itConfig ))
            {
            ActionDataBase* new_action = ActionDataBase::create_cfg_read( itConfig, parent_P );
            ActionDataGroup* grp = dynamic_cast< ActionDataGroup* >( new_action );
            if( grp != NULL )
                read_actions_recursively_v2( itConfig, grp, include_disabled_P );
            }
        }
    }

// backward compatibility
void Settings::read_settings_v1(ActionDataGroup *root, KConfigBase const& config)
    {
    KConfigGroup mainGroup( &config, "Main" );
    int sections = mainGroup.readEntry( "Num_Sections", 0 );
    ActionDataGroup* menuentries = NULL;
    Q_FOREACH(ActionDataBase *child, root->children())
        {
        ActionDataGroup* tmp = dynamic_cast< ActionDataGroup* >(child);
        if( tmp == NULL )
            continue;
        if( tmp->system_group() == ActionDataGroup::SYSTEM_MENUENTRIES )
            {
            menuentries = tmp;
            break;
            }
        }
    for( int sect = 1;
         sect <= sections;
         ++sect )
        {
        QString group = QString( "Section%1" ).arg( sect );
        if( !config.hasGroup( group ))
            continue;
        KConfigGroup sectionConfig( &config, group );
        QString name = sectionConfig.readEntry( "Name" );
        if( name.isNull() )
            continue;
        QString shortcut = sectionConfig.readEntry( "Shortcut" );
        if( shortcut.isNull() )
            continue;
        QString run = sectionConfig.readEntry( "Run" );
        if( run.isNull() )
            continue;
        bool menuentry = sectionConfig.readEntry( "MenuEntry", false);
        // CHECKME tohle pridavani az pak je trosku HACK
        if( menuentry )
            {
            if( menuentries == NULL )
                {
                menuentries = new ActionDataGroup( root,
                    i18n( MENU_EDITOR_ENTRIES_GROUP_NAME ),
                    i18n( "These entries were created using Menu Editor." ), NULL,
                    ActionDataGroup::SYSTEM_MENUENTRIES, true );
                menuentries->set_conditions( new Condition_list( "", menuentries ));
                }
            ( void ) new MenuEntryShortcutActionData( menuentries, name, "",
                KShortcut( shortcut ), run );
            }
        else
            {
            ( void ) new CommandUrlShortcutActionData( root, name, "",
                KShortcut( shortcut ), run );
            }
        }
    }

} // namespace KHotKeys
