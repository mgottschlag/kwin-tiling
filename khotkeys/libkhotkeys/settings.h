/****************************************************************************

 KHotKeys

 Copyright (C) 1999-2001 Lubos Lunak <l.lunak@kde.org>

 Distributed under the terms of the GNU General Public License version 2.

****************************************************************************/

#ifndef _SETTINGS_H_
#define _SETTINGS_H_

#include "actions.h"
#include <kshortcut.h>

class KConfig;

namespace KHotKeys
{

class Action_data_group;

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
    Q_DISABLE_COPY( Settings );

public:

    Settings();
    ~Settings();

    /**
     * Read the settings.
     *
     * \param include_disabled_P Load disabled shortcuts?
     */
    bool read_settings( bool include_disabled_P );

    /**
     * Write the settings.
     */
    void write_settings();

    /**
     * Import settings from \a cfg_P.
     */
    bool import( KConfig& cfg_P, bool ask_P );

    /**
     * Get all actions
     */
    Action_data_group *actions();

    /**
     * Take the actions. 
     *
     * \note Ownership is transfered to you. Subsequent calls to action() will
     * return 0
     */
    Action_data_group *takeActions();

    /**
     * Set the actions. 
     *
     * \note Ownership is taken. The current action list will be deleted.
     */
    void setActions( Action_data_group *actions );

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
     * Read settings from \a cfg_P. \a include_disabled_P controls wether
     * disabled actions should be discarded.
     */
    bool read_settings( KConfig& cfg_P, bool include_disabled_P, ImportType import_P );

    /**
     * Read settings in the v1 legacy format from \a cfg_P .
     */
    void read_settings_v1( KConfig& cfg_P );

    /**
     * Read settings in the v2 format from \a cfg_P .
     */
    void read_settings_v2( KConfig& cfg_P, bool include_disabled_P );

    /**
     * Write \a parent_P recursively to \a cfg_P
     */
    int write_actions_recursively_v2(
        KConfigGroup& cfg_P,
        Action_data_group* parent_P,
        bool enabled_P );

    /**
     * Read \a parent_P recursively to \a cfg_P
     */
    void read_actions_recursively_v2(
        KConfigGroup& cfg_P,
        Action_data_group* parent_P,
        bool include_disabled_P );

private:

    /**
     * TODO
     */
    Action_data_group* m_actions;

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
