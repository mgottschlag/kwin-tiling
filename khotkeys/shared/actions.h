/****************************************************************************

 KHotKeys
 
 Copyright (C) 1999-2001 Lubos Lunak <l.lunak@kde.org>

 Distributed under the terms of the GNU General Public License version 2.
 
****************************************************************************/

#ifndef _ACTIONS_H_
#define _ACTIONS_H_


#include <qstring.h>
#include <q3ptrlist.h>
#include <qtimer.h>

#include <kservice.h>

class KConfig;

#include "khotkeysglobal.h"

namespace KHotKeys
{

class Action_data;
class Windowdef_list;

// this one is a base for all "real" resulting actions, e.g. running a command,
// Action_data instances usually contain at least one Action
class Action
    {
    public:
        Action( Action_data* data_P );
        Action( KConfig& cfg_P, Action_data* data_P );
        virtual ~Action();
        virtual void execute() = 0;
        virtual const QString description() const = 0;
        virtual void cfg_write( KConfig& cfg_P ) const;
        virtual Action* copy( Action_data* data_P ) const = 0;
        static Action* create_cfg_read( KConfig& cfg_P, Action_data* data_P );
    protected:
        Action_data* const data;
    KHOTKEYS_DISABLE_COPY( Action );
    };

class Action_list
    : public Q3PtrList< Action >
    {
    public:
        Action_list( const QString& comment_P ); // CHECKME nebo i data ?
        Action_list( KConfig& cfg_P, Action_data* data_P );
        void cfg_write( KConfig& cfg_P ) const;
        typedef Q3PtrListIterator< Action > Iterator;
        const QString& comment() const;
    private:
        QString _comment;
    KHOTKEYS_DISABLE_COPY( Action_list );
    };

class Command_url_action
    : public Action
    {
    typedef Action base;
    public:
        Command_url_action( Action_data* data_P, const QString& command_url_P );
        Command_url_action( KConfig& cfg_P, Action_data* data_P );
        virtual void cfg_write( KConfig& cfg_P ) const;
        virtual void execute();
        virtual const QString description() const;
        const QString& command_url() const;
        virtual Action* copy( Action_data* data_P ) const;
    protected:
        QTimer timeout;
    private:
        QString _command_url;
    };
    
class Menuentry_action
    : public Command_url_action
    {
    typedef Command_url_action base;
    public:
        Menuentry_action( Action_data* data_P, const QString& menuentry_P );
        Menuentry_action( KConfig& cfg_P, Action_data* data_P );
        virtual void cfg_write( KConfig& cfg_P ) const;
        virtual void execute();
        virtual const QString description() const;
        virtual Action* copy( Action_data* data_P ) const;
        KService::Ptr service() const;
    private:
        KService::Ptr _service;
    };
    
class Dcop_action
    : public Action
    {
    typedef Action base;
    public:
        Dcop_action( Action_data* data_P, const QString& app_P, const QString& obj_P,
            const QString& call_P, const QString& args_P );
        Dcop_action( KConfig& cfg_P, Action_data* data_P );
        virtual void cfg_write( KConfig& cfg_P ) const;
        virtual void execute();
        const QString& remote_application() const;
        const QString& remote_object() const;
        const QString& called_function() const;
        const QString& arguments() const;
        virtual const QString description() const;
        virtual Action* copy( Action_data* data_P ) const;
    private:
        QString app; // CHECKME QCString ?
        QString obj;
        QString call;
        QString args;
    };
        
class Keyboard_input_action
    : public Action
    {
    typedef Action base;
    public:
        Keyboard_input_action( Action_data* data_P, const QString& input_P, 
            const Windowdef_list* dest_window_P, bool active_window_P );
        Keyboard_input_action( KConfig& cfg_P, Action_data* data_P );
        virtual ~Keyboard_input_action();
        virtual void cfg_write( KConfig& cfg_P ) const;
        virtual void execute();
        const QString& input() const;
        // send to specific window: dest_window != NULL
        // send to active window: dest_window == NULL && activeWindow() == true
        // send to action window: dest_window == NULL && activeWindow() == false
        const Windowdef_list* dest_window() const;
        bool activeWindow() const;
        virtual const QString description() const;
        virtual Action* copy( Action_data* data_P ) const;
    private:
        QString _input;
        const Windowdef_list* _dest_window;
        bool _active_window;
    };

class Activate_window_action
    : public Action
    {
    typedef Action base;
    public:
        Activate_window_action( Action_data* data_P, const Windowdef_list* window_P );
        Activate_window_action( KConfig& cfg_P, Action_data* data_P );
        virtual ~Activate_window_action();
        virtual void cfg_write( KConfig& cfg_P ) const;
        virtual void execute();
        const Windowdef_list* window() const;
        virtual const QString description() const;
        virtual Action* copy( Action_data* data_P ) const;
    private:
        const Windowdef_list* _window;
    };
        
//***************************************************************************
// Inline
//***************************************************************************

// Action
    
inline
Action::Action( Action_data* data_P )
    : data( data_P )
    {
    }
    
inline
Action::Action( KConfig&, Action_data* data_P )
    : data( data_P )
    {
    }
    
inline
Action::~Action()
    {
    }

// Action_list
        
inline
Action_list::Action_list( const QString& comment_P )
    : Q3PtrList< Action >(), _comment( comment_P )
    {
    setAutoDelete( true );
    }

inline
const QString& Action_list::comment() const
    {
    return _comment;
    }

// Command_url_action
    
inline
Command_url_action::Command_url_action( Action_data* data_P, const QString& command_url_P )
    : Action( data_P ), _command_url( command_url_P )
    {
    }

inline
const QString& Command_url_action::command_url() const
    {
    return _command_url;
    }
    
// Menuentry_action

inline
Menuentry_action::Menuentry_action( Action_data* data_P, const QString& menuentry_P )
    : Command_url_action( data_P, menuentry_P )
    {
    }
    
inline
Menuentry_action::Menuentry_action( KConfig& cfg_P, Action_data* data_P )
    : Command_url_action( cfg_P, data_P )
    {
    }

// DCOP_action

inline
Dcop_action::Dcop_action( Action_data* data_P, const QString& app_P, const QString& obj_P,
    const QString& call_P, const QString& args_P )
    : Action( data_P ), app( app_P ), obj( obj_P ), call( call_P ), args( args_P )
    {
    }

inline
const QString& Dcop_action::remote_application() const
    {
    return app;
    }
    
inline
const QString& Dcop_action::remote_object() const
    {
    return obj;
    }
    
inline
const QString& Dcop_action::called_function() const
    {
    return call;
    }
    
inline
const QString& Dcop_action::arguments() const
    {
    return args;
    }

// Keyboard_input_action

inline
Keyboard_input_action::Keyboard_input_action( Action_data* data_P, const QString& input_P,
    const Windowdef_list* dest_window_P, bool active_window_P )
    : Action( data_P ), _input( input_P ), _dest_window( dest_window_P ), _active_window( active_window_P )
    {
    }
    
inline
const QString& Keyboard_input_action::input() const
    {
    return _input;
    }
    
inline
const Windowdef_list* Keyboard_input_action::dest_window() const
    {
    return _dest_window;
    }

inline
bool Keyboard_input_action::activeWindow() const
    {
    return _active_window;
    }

// Activate_window_action

inline
Activate_window_action::Activate_window_action( Action_data* data_P,
    const Windowdef_list* window_P )
    : Action( data_P ), _window( window_P )
    {
    }
    
inline
const Windowdef_list* Activate_window_action::window() const
    {
    return _window;
    }

} // namespace KHotKeys
    
#endif
