/****************************************************************************

 KHotKeys
 
 Copyright (C) 1999-2001 Lubos Lunak <l.lunak@kde.org>

 Distributed under the terms of the GNU General Public License version 2.
 
****************************************************************************/

#ifndef _KCMKHOTKEYS_H_
#define _KCMKHOTKEYS_H_

#include <kcmodule.h>

#include <actions.h>

namespace KHotKeys
{

class Actions_listview_widget;
class Tab_widget;
class Action_data_base;
class Main_buttons_widget;

class Module
    : public KCModule
    {
    Q_OBJECT
    public:
        Module( QWidget *parent_P, const char *name_P );
        virtual ~Module();
        virtual void load();
        virtual void save();
//        virtual void defaults(); not used
        QString quickHelp() const;
        int buttons();
        void set_current_action_data( Action_data_base* data_P );
        Action_data_base* current_action_data();
        void action_name_changed( const QString& name_P );
        Action_data_group* actions_root() const;
    public slots:
        void changed();
    protected:
        void set_new_current_action( bool save_old_P );
        Actions_listview_widget* actions_listview_widget;
        Tab_widget* tab_widget;
        Main_buttons_widget* buttons_widget;
        Action_data_group* _actions_root; // vlastni vsechny Action_data
        Action_data_base* _current_action_data;
        bool listview_is_changed;
        bool deleting_action;
    protected slots:
        void listview_current_action_changed();
        void new_action();
        void new_action_group();
        void delete_action();
        void global_settings();
    };
    
extern Module* module;

//***************************************************************************
// Inline
//***************************************************************************

// Module

inline
Action_data_group* Module::actions_root() const
    {
    return _actions_root;
    }

inline
Action_data_base* Module::current_action_data()
    {
    return _current_action_data;
    }
    
} // namespace KHotKeys

#endif
