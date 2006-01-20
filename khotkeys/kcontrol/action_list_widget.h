/****************************************************************************

 KHotKeys
 
 Copyright (C) 1999-2001 Lubos Lunak <l.lunak@kde.org>

 Distributed under the terms of the GNU General Public License version 2.
 
****************************************************************************/

#ifndef _ACTION_LIST_WIDGET_H_
#define _ACTION_LIST_WIDGET_H_

#include <qlistview.h>

#include <actions.h>
#include <kdialogbase.h>

#include <action_list_widget_ui.h>

#include "activate_window_widget.h"

namespace KHotKeys
{

class Action_data;
class Command_url_widget;
class Menuentry_widget;
class Dcop_widget;
class Keyboard_input_widget;

class Action_list_item;

class Action_list_widget
    : public Action_list_widget_ui
    {
    Q_OBJECT
    public:
        Action_list_widget( QWidget* parent_P = NULL, const char* name_P = NULL );
        virtual ~Action_list_widget();
        void set_data( const Action_list* data_P );
        Action_list* get_data( Action_data* data_P ) const;
    public Q_SLOTS:
        void clear_data();
    protected:
        Action_list_item* create_listview_item( Action* action_P, Q3ListView* parent1_P,
            Q3ListViewItem* parent2_P, Q3ListViewItem* after_P, bool copy_P );
        void edit_listview_item( Action_list_item* item_P );
        enum type_t { TYPE_COMMAND_URL_ACTION, TYPE_MENUENTRY_ACTION, TYPE_DCOP_ACTION,
            TYPE_KEYBOARD_INPUT_ACTION, TYPE_ACTIVATE_WINDOW_ACTION };
    protected Q_SLOTS:
        void new_selected( int type_P );
        virtual void copy_pressed();
        virtual void delete_pressed();
        virtual void modify_pressed();
        virtual void current_changed( Q3ListViewItem* item_P );
    protected:
        Action_list_item* selected_item;
    };

typedef Action_list_widget Action_list_tab;

class Action_list_item
    : public Q3ListViewItem
    {
    public:
        Action_list_item( Q3ListView* parent_P, Action* action_P );
        Action_list_item( Q3ListViewItem* parent_P, Action* action_P );
        Action_list_item( Q3ListView* parent_P, Q3ListViewItem* after_P, Action* action_P );
        Action_list_item( Q3ListViewItem* parent_P, Q3ListViewItem* after_P, Action* action_P );
        virtual ~Action_list_item();
        virtual QString text( int column_P ) const;
        Action* action() const;
        void set_action( Action* action_P );
    protected:
        Action* _action; // owns it
    };
        
class Action_dialog
    {
    public:
        virtual Action* edit_action() = 0;
        virtual ~Action_dialog();
    };
    
class Command_url_action_dialog
    : public KDialogBase, public Action_dialog
    {
    Q_OBJECT
    public:
        Command_url_action_dialog( Command_url_action* action_P );
        virtual Action* edit_action();
    protected:
        virtual void accept();
        Command_url_widget* widget;
        Command_url_action* action;
    };
        
class Menuentry_action_dialog
    : public KDialogBase, public Action_dialog
    {
    Q_OBJECT
    public:
        Menuentry_action_dialog( Menuentry_action* action_P );
        virtual Action* edit_action();
    protected:
        virtual void accept();
        Menuentry_widget* widget;
        Menuentry_action* action;
    };
        
class Dcop_action_dialog
    : public KDialogBase, public Action_dialog
    {
    Q_OBJECT
    public:
        Dcop_action_dialog( Dcop_action* action_P );
        virtual Action* edit_action();
    protected:
        virtual void accept();
        Dcop_widget* widget;
        Dcop_action* action;
    };
        
class Keyboard_input_action_dialog
    : public KDialogBase, public Action_dialog
    {
    Q_OBJECT
    public:
        Keyboard_input_action_dialog( Keyboard_input_action* action_P );
        virtual Action* edit_action();
    protected:
        virtual void accept();
        Keyboard_input_widget* widget;
        Keyboard_input_action* action;
    };
        
class Activate_window_action_dialog
    : public KDialogBase, public Action_dialog
    {
    Q_OBJECT
    public:
        Activate_window_action_dialog( Activate_window_action* action_P );
        virtual Action* edit_action();
    protected:
        virtual void accept();
        Activate_window_widget* widget;
        Activate_window_action* action;
    };
        
//***************************************************************************
// Inline
//***************************************************************************

// Action_list_item

inline
Action_list_item::Action_list_item( Q3ListView* parent_P, Action* action_P )
    : Q3ListViewItem( parent_P ), _action( action_P )
    {
    }
    
inline
Action_list_item::Action_list_item( Q3ListViewItem* parent_P, Action* action_P )
    : Q3ListViewItem( parent_P ), _action( action_P )
    {
    }

inline
Action_list_item::Action_list_item( Q3ListView* parent_P, Q3ListViewItem* after_P,
    Action* action_P )
    : Q3ListViewItem( parent_P, after_P ), _action( action_P )
    {
    }

inline
Action_list_item::Action_list_item( Q3ListViewItem* parent_P, Q3ListViewItem* after_P,
    Action* action_P )
    : Q3ListViewItem( parent_P, after_P ), _action( action_P )
    {
    }

inline
Action* Action_list_item::action() const
    {
    return _action;
    }
    
inline
void Action_list_item::set_action( Action* action_P )
    {
    delete _action;
    _action = action_P;
    }

// Action_dialog

inline
Action_dialog::~Action_dialog()
    {
    }

} // namespace KHotKeys

#endif
