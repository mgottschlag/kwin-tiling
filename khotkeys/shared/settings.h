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

enum ImportType
    {
    ImportNone, // no import is done
    ImportAsk,  // if already imported before, ask (called from GUI)
    ImportSilent // if already imported before, ignore (called from the update script)
    };

class KDE_EXPORT Settings
    {
    public:
        Settings();
        bool read_settings( bool include_disabled_P );
        void write_settings();
        bool import( KConfig& cfg_P, bool ask_P );
        Action_data_group* actions;
        bool gestures_disabled_globally;
        int gesture_mouse_button;
        int gesture_timeout;
        bool daemon_disabled;
        Windowdef_list* gestures_exclude;
		KShortcut voice_shortcut;
    protected:
        bool read_settings( KConfig& cfg_P, bool include_disabled_P, ImportType import_P );
        void read_settings_v1( KConfig& cfg_P );
        void read_settings_v2( KConfig& cfg_P, bool include_disabled_P );
        int write_actions_recursively_v2( KConfig& cfg_P, Action_data_group* parent_P, bool enabled_P );
        void read_actions_recursively_v2( KConfig& cfg_P, Action_data_group* parent_P,
            bool include_disabled_P );
    private:
        QStringList already_imported;
    KHOTKEYS_DISABLE_COPY( Settings );
    };

//***************************************************************************
// Inline
//***************************************************************************

} // namespace KHotKeys

#endif
