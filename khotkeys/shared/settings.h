/****************************************************************************

 KHotKeys
 
 Copyright (C) 1999-2001 Lubos Lunak <l.lunak@kde.org>

 Distributed under the terms of the GNU General Public License version 2.
 
****************************************************************************/

#ifndef _SETTINGS_H_
#define _SETTINGS_H_

#include "actions.h"

class KConfig;

namespace KHotKeys
{

class Settings
    {
    public:
        Settings();
        void read_settings( bool include_disabled_P );
        void write_settings();
        Action_data_group* actions;
    protected:
        void read_settings_v1( KConfig& cfg_P );
        void read_settings_v2( KConfig& cfg_P, bool include_disabled_P );
        int write_actions_recursively_v2( KConfig& cfg_P, Action_data_group* parent_P, bool enabled_P );
        void read_actions_recursively_v2( KConfig& cfg_P, Action_data_group* parent_P,
            bool include_disabled_P );
    KHOTKEYS_DISABLE_COPY( Settings );
    };

//***************************************************************************
// Inline
//***************************************************************************

} // namespace KHotKeys

#endif
