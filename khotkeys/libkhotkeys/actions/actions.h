/****************************************************************************

 KHotKeys

 Copyright (C) 1999-2001 Lubos Lunak <l.lunak@kde.org>

 Distributed under the terms of the GNU General Public License version 2.

****************************************************************************/

#ifndef _ACTIONS_H_
#define _ACTIONS_H_


#include <QtCore/QList>
#include <QtCore/QTimer>

#include <kservice.h>

class KConfig;

#include "khotkeysglobal.h"

namespace KHotKeys
{

class Action_data;
class Windowdef_list;

// this one is a base for all "real" resulting actions, e.g. running a command,
// Action_data instances usually contain at least one Action
class KDE_EXPORT Action
    {
    Q_DISABLE_COPY( Action );

    public:

        enum Type
            {
            ActivateWindowAction,
            CommandUrlActionType,
            DBusActionType,
            KeyboardInputActionType,
            MenuEntryActionType
            };

        Action( Action_data* data_P );
        Action( KConfigGroup& cfg_P, Action_data* data_P );
        virtual ~Action();
        virtual void execute() = 0;
        virtual Type type() = 0;
        virtual const QString description() const = 0;
        virtual void cfg_write( KConfigGroup& cfg_P ) const;
        virtual Action* copy( Action_data* data_P ) const = 0;
        static Action* create_cfg_read( KConfigGroup& cfg_P, Action_data* data_P );
    protected:
        Action_data* const data;
    };

class KDE_EXPORT ActionList
    : public QList< Action* >
    {
    Q_DISABLE_COPY( ActionList )
    public:
        ActionList( const QString& comment_P ); // CHECKME nebo i data ?
        ActionList( KConfigGroup& cfg_P, Action_data* data_P );
        ~ActionList();
        void cfg_write( KConfigGroup& cfg_P ) const;
        //! Some convenience typedef
        typedef QList<Action*>::Iterator Iterator;
        typedef QList<Action*>::ConstIterator ConstIterator;
        const QString& comment() const;

    private:
        QString _comment;
    };

class KDE_EXPORT CommandUrlAction
    : public Action
    {
    typedef Action base;
    public:
        CommandUrlAction( Action_data* data_P, const QString& command_url_P = QString() );
        CommandUrlAction( KConfigGroup& cfg_P, Action_data* data_P );
        virtual void cfg_write( KConfigGroup& cfg_P ) const;
        virtual void execute();
        virtual const QString description() const;

        //! The command url to trigger
        void set_command_url( const QString &command_url );
        QString command_url() const;

        virtual Type type() { return CommandUrlActionType; }
        virtual Action* copy( Action_data* data_P ) const;
    protected:
        QTimer timeout;
    private:
        QString _command_url;
    };

class KDE_EXPORT MenuEntryAction
    : public CommandUrlAction
    {
    typedef CommandUrlAction base;
    public:
        MenuEntryAction( Action_data* data_P, const QString& menuentry_P = QString() );
        MenuEntryAction( KConfigGroup& cfg_P, Action_data* data_P );
        virtual void cfg_write( KConfigGroup& cfg_P ) const;
        virtual void execute();

        // The service we trigger
        KService::Ptr service() const;
        void set_service( KService::Ptr );

        virtual const QString description() const;
        virtual Action* copy( Action_data* data_P ) const;
        virtual Type type() { return Action::MenuEntryActionType; }
    private:
        KService::Ptr _service;
    };

class KDE_EXPORT DBusAction
    : public Action
    {
    typedef Action base;
    public:
        DBusAction( 
            Action_data* data_P,
            const QString& app_P = QString(),
            const QString& obj_P = QString(),
            const QString& call_P= QString(),
            const QString& args_P= QString() );

        DBusAction( KConfigGroup& cfg_P, Action_data* data_P );
        virtual void cfg_write( KConfigGroup& cfg_P ) const;
        virtual void execute();
        const QString remote_application() const;
        const QString remote_object() const;
        const QString called_function() const;
        const QString arguments() const;

        void set_remote_application( const QString &application );
        void set_remote_object( const QString &object );
        void set_called_function( const QString &function );
        void set_arguments( const QString &args );

        virtual const QString description() const;
        virtual Action* copy( Action_data* data_P ) const;
        virtual Type type() { return DBusActionType; }
    private:
        QString _application; // CHECKME QCString ?
        QString _object;
        QString _function;
        QString _arguments;
    };

class KDE_EXPORT Keyboard_input_action
    : public Action
    {
    typedef Action base;
    public:
        Keyboard_input_action( Action_data* data_P, const QString& input_P,
            const Windowdef_list* dest_window_P, bool active_window_P );
        Keyboard_input_action( KConfigGroup& cfg_P, Action_data* data_P );
        virtual ~Keyboard_input_action();
        virtual void cfg_write( KConfigGroup& cfg_P ) const;
        virtual void execute();
        const QString& input() const;
        // send to specific window: dest_window != NULL
        // send to active window: dest_window == NULL && activeWindow() == true
        // send to action window: dest_window == NULL && activeWindow() == false
        const Windowdef_list* dest_window() const;
        bool activeWindow() const;
        virtual const QString description() const;
        virtual Action* copy( Action_data* data_P ) const;
        virtual Type type() { return KeyboardInputActionType; }
    private:
        QString _input;
        const Windowdef_list* _dest_window;
        bool _active_window;
    };

class KDE_EXPORT Activate_window_action
    : public Action
    {
    typedef Action base;
    public:
        Activate_window_action( Action_data* data_P, const Windowdef_list* window_P );
        Activate_window_action( KConfigGroup& cfg_P, Action_data* data_P );
        virtual ~Activate_window_action();
        virtual void cfg_write( KConfigGroup& cfg_P ) const;
        virtual void execute();
        const Windowdef_list* window() const;
        virtual const QString description() const;
        virtual Action* copy( Action_data* data_P ) const;
        virtual Type type() { return ActivateWindowAction; }
    private:
        const Windowdef_list* _window;
    };

} // namespace KHotKeys

#endif
