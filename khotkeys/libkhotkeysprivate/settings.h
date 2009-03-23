/****************************************************************************

 KHotKeys

 Copyright (C) 1999-2001 Lubos Lunak <l.lunak@kde.org>

 Distributed under the terms of the GNU General Public License version 2.

****************************************************************************/

#ifndef _SETTINGS_H_
#define _SETTINGS_H_

#include "actions/actions.h"
#include "action_data/action_data_group.h"
#include <kshortcut.h>

class KConfig;

namespace KHotKeys
{

class ActionDataGroup;

/**
 * How to handle imports.
 */
enum ImportType
    {
    ImportNone,  //!< no import is done
    ImportAsk,   //!< if already imported before, ask (called from GUI)
    ImportSilent //!< if already imported before, ignore (called from the update script)
    };


/**
 * Handles KHotKeys Settings.
 *
 * Settings are saved to the khotkeysrc file.
 */
class KDE_EXPORT Settings
{
    Q_DISABLE_COPY( Settings )

public:

    static const int CurrentFileVersion;

    Settings();
    ~Settings();

    /**
     * Get the system group.
     */
    ActionDataGroup *get_system_group(ActionDataGroup::system_group_t group_id);

    /**
     * Read the settings.
     *
     * \param include_disabled_P Load disabled shortcuts?
     */
    bool reread_settings(bool include_disabled = true);

    /**
     * Update the settings.
     *
     * Checks if updates are available and imports them if not yet done.
     */
    bool update();

    /**
     * Write the settings.
     */
    void write_settings();

    /**
     * Export settings to @a config
     */
    void exportTo(ActionDataBase *what, KConfigBase &config);

    /**
     * Import settings from \a cfg_P.
     */
    bool import(KConfig& cfg_P, bool ask = true);

    bool importFrom(ActionDataGroup *parent, KConfigBase const &config, bool ask=false);

    /**
     * Get all actions
     */
    ActionDataGroup *actions();

    /**
     * Take the actions. 
     *
     * \note Ownership is transfered to you. Subsequent calls to action() will
     * return 0
     */
    ActionDataGroup *takeActions();

    /**
     * Set the actions. 
     *
     * \note Ownership is taken. The current action list will be deleted. If
     * \@a actions is NULL the method will create a new ActionDataGroup
     */
    void setActions( ActionDataGroup *actions );

    /**
     * @name KHotkeys Daemon
     */
    //@{
    /**
     * Disable the daemon.
     */
    void disableDaemon();

    /**
     * Enable the daemon.
     */
    void enableDaemon();

    /**
     * Is the daemon disabled?
     */
    bool isDaemonDisabled() const;
    //@}

    /**
     * Load the default settings
     */
    bool loadDefaults();

    /**
     * @name Gestures
     */
    //@{
    void disableGestures();
    void enableGestures();
    bool areGesturesDisabled() const;

    int gestureMouseButton() const;
    void setGestureMouseButton( int );

    int gestureTimeOut() const;
    void setGestureTimeOut(int);

    void setGesturesExclude( Windowdef_list *gestures );
    Windowdef_list *gesturesExclude();
    const Windowdef_list *gesturesExclude() const;
    //@}

    /**
     * @name Voice Commands
     */
    //@{
    void setVoiceShortcut( const KShortcut &shortcut );
    KShortcut voiceShortcut() const;
    //@}

protected:

    /**
     * Read settings from \a cfg_P.
     *
     * @param root the group to import to
     * @param config config object to read from
     * @param include_disabled should we read disabled actions?
     * @param disable_actions disable the imported actions?
     */
    bool read_settings(
            ActionDataGroup *root,
            KConfigBase const &config,
            bool include_disabled,
            bool disable_actions = false);

    /**
     * Read settings in the v1 legacy format from \a cfg_P .
     */
    void read_settings_v1(ActionDataGroup *root, KConfigBase const& cfg);

    /**
     * Read settings in the v2 format from \a cfg_P .
     */
    void read_settings_v2(
            ActionDataGroup *root,
            KConfigBase const& cfg,
            bool include_disabled,
            bool disable_actions = false);

    /**
     * Write \a parent_P recursively to \a cfg_P
     *
     * The return value specifies the number of active input actions written.
     */
    int write_actions_recursively_v2(
        KConfigGroup& cfg_P,
        ActionDataGroup* parent_P,
        bool enabled_P );

    /**
     * Read \a parent_P recursively to \a cfg_P
     */
    void read_actions_recursively_v2(
        KConfigGroup& cfg_P,
        ActionDataGroup* parent_P,
        bool include_disabled_P,
        bool disable_actions = false);

    /**
     * Make sure all System Groups exists
     */
    void validate();

private:

    // Reset all values. No defaults are loaded
    void reinitialize();

    /**
     * TODO
     */
    ActionDataGroup* m_actions;

    /**
     * @name Gestures
     */
    //@{
    /**
     * Gestures globally disabled?
     */
    bool gestures_disabled;

    /**
     * Mouse button used for gestures.
     */
    int gesture_mouse_button;

    /**
     * Gesture timeout
     */
    int gesture_timeout;

    /**
     * Windows to exclude from gestures
     */
    Windowdef_list* gestures_exclude;
    //@}

    /**
     * KHotKeys daemon disabled?
     */
    bool daemon_disabled;

    /**
     * The shortcut that triggers a voice command
     */
    KShortcut voice_shortcut;

    /**
     * List of id's for all imported files.
     */
    QStringList already_imported;
};

} // namespace KHotKeys

#endif
