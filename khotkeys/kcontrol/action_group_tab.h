/****************************************************************************

 KHotKeys
 
 Copyright (C) 1999-2001 Lubos Lunak <l.lunak@kde.org>

 Distributed under the terms of the GNU General Public License version 2.
 
****************************************************************************/

#ifndef _ACTION_GROUP_TAB_H_
#define _ACTION_GROUP_TAB_H_

#include <action_group_tab_ui.h>
#include <action_data.h>

namespace KHotKeys
{

class Action_data_group;
class Windowdef_list;

class Action_group_tab
    : public Action_group_tab_ui
    {
    Q_OBJECT
    public:
        Action_group_tab( QWidget* parent_P = NULL, const char* name_P = NULL );
        void set_data( const Action_data_group* data_P );
        Action_data_group* get_data( Action_data_group* parent_P,
            Condition_list* conditions_P ) const;
    public Q_SLOTS:
        void clear_data();
    protected Q_SLOTS:
        virtual void action_group_name_changed( const QString& name_P );
    protected:
        Action_data_group::system_group_t system_group;
    };
    
//***************************************************************************
// Inline
//***************************************************************************

} // namespace KHotKeys

#endif
