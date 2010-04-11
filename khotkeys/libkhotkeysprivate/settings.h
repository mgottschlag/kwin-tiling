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

enum ActionState
    {
    Retain,      //!< Keep the current state
    Enabled,     //!< Enable all actions
    Disabled     //!< Disable all actions
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
    void write();

    /**
     * Export settings to @a config.
     *
     * @param id    use id for the exported file.
     * @param state state to use for exported actions
     */
    void exportTo(
            ActionDataBase *what,
            KConfigBase &config,
            const QString &id,
            ActionState state,
            bool allowMerging);

    /**
     * Import settings from \a cfg_P.
     */
    bool import(KConfig& cfg_P, ImportType ask, ActionState state);

    bool importFrom(
            ActionDataGroup *parent,
            KConfigBase const &config,
            ImportType ask,
            ActionState state);

    /**
     * Get all actions
     */
    ActionDataGroup *actions();
    const ActionDataGroup *actions() const;

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

    /**
     * Check if the given config file is a valid khotkeys file
     */
    bool isConfigFileValid(KConfigBase const &config, ImportType ask);

protected:

    /**
     * Read settings from \a cfg_P.
     *
     * @param root the group to import to
     * @param config config object to read from
     * @param include_disabled should we read disabled actions?
     * @param state enable, disable or keep the actions enabled state
     */
    bool read_settings(
            ActionDataGroup *root,
            KConfigBase const &config,
            bool include_disabled,
            ActionState state);

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

    friend class SettingsWriter;
};

} // namespace KHotKeys

#endif
