/****************************************************************************

 KHotKeys
 
 Copyright (C) 1999-2001 Lubos Lunak <l.lunak@kde.org>

 Distributed under the terms of the GNU General Public License version 2.
 
****************************************************************************/

#ifndef _TAB_WIDGET_H_
#define _TAB_WIDGET_H_

#include <QTabWidget>

#include <actions.h>

namespace KHotKeys
{

class Tab_widget
    : public QTabWidget
    {
    Q_OBJECT
    public:
        enum action_type_t
            {
            TYPE_FIRST,
            TYPE_GENERIC = TYPE_FIRST,
            TYPE_COMMAND_URL_SHORTCUT,
            TYPE_MENUENTRY_SHORTCUT,
            TYPE_DCOP_SHORTCUT,
            TYPE_KEYBOARD_INPUT_SHORTCUT,
            TYPE_KEYBOARD_INPUT_GESTURE,
            TYPE_ACTIVATE_WINDOW_SHORTCUT,
            TYPE_END
            };
        Tab_widget( QWidget* parent_P = NULL, const char* name_P = NULL );
        virtual ~Tab_widget();
        void set_action_type( action_type_t type_P, bool force_P = false );
        void save_current_action_changes();
        void load_current_action();
        void clear_pages();
        static action_type_t type( const Action_data* data_P );
    Q_SIGNALS: // internal
        void clear_pages_signal();
    protected Q_SLOTS:
        void set_action_type_slot( int type_P );
    protected:
        void check_action_type();
        class Pages_set;
        void show_pages( const Pages_set& pages_P );
        enum tab_pos_t { TAB_FIRST, TAB_INFO = TAB_FIRST, TAB_GENERAL_SETTINGS, TAB_GESTURES_SETTINGS,
            TAB_GENERAL, TAB_GROUP_GENERAL,
            TAB_TRIGGERS, TAB_SHORTCUT_TRIGGER, TAB_GESTURE_TRIGGER, TAB_ACTIONS, TAB_COMMAND_URL,
            TAB_MENUENTRY, TAB_DCOP, TAB_KEYBOARD_INPUT, TAB_WINDOW, TAB_CONDITIONS, TAB_VOICE_SETTINGS, TAB_END };
        QWidget* pages[ TAB_END ];
        enum tab_show_type_t { NONE, DATA, GROUP };
        tab_show_type_t current_type;
        action_type_t current_data_type;
        static const char* const tab_labels[];
        class Pages_set // that main reason for existence of this class is the fact that
            {           // I was very curious if overloading operator, ( = comma ) really
            public:     // works ( it does, but not exactly as I expected :(   )
                Pages_set( tab_pos_t page_P );
                Pages_set& operator,( tab_pos_t page_P );
                bool is_set( tab_pos_t page_P ) const;
            protected:
                bool set[ TAB_END ];
            };
        friend Pages_set operator,( tab_pos_t page1_P, tab_pos_t page2_P ); // CHECKME
        friend tab_pos_t& operator++( tab_pos_t& val_P ); // CHECKME
    };
        
//***************************************************************************
// Inline
//***************************************************************************

// Tab_widget

// grrrr
inline
Tab_widget::tab_pos_t& operator++( Tab_widget::tab_pos_t& val_P )
    {
    val_P = static_cast< Tab_widget::tab_pos_t >( val_P + 1 );
    return val_P;
    }

inline
void Tab_widget::clear_pages()
    {
    emit clear_pages_signal();
    }
            
// Tab_widget::Pages_set

inline
Tab_widget::Pages_set::Pages_set( tab_pos_t page_P )
    {
    for( tab_pos_t i = TAB_FIRST;
         i < TAB_END;
         ++i )
        set[ i ] = false;
    set[ page_P ] = true;
    }
    
inline
bool Tab_widget::Pages_set::is_set( tab_pos_t page_P ) const
    {
    return set[ page_P ];
    }

inline
Tab_widget::Pages_set& Tab_widget::Pages_set::operator,( tab_pos_t page_P )
    {
    set[ page_P ] = true;
    return *this;
    }
        
inline
Tab_widget::Pages_set operator,( Tab_widget::tab_pos_t page1_P, Tab_widget::tab_pos_t page2_P )
    {
    return Tab_widget::Pages_set( page1_P ), page2_P;
    }
        
// grrrr
inline
Tab_widget::action_type_t& operator++( Tab_widget::action_type_t& val_P )
    {
    val_P = static_cast< Tab_widget::action_type_t >( val_P + 1 );
    return val_P;
    }

} // namespace KHotKeys

#endif
