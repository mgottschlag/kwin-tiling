/****************************************************************************

 KHotKeys
 
 Copyright (C) 1999-2001 Lubos Lunak <l.lunak@kde.org>

 Distributed under the terms of the GNU General Public License version 2.
 
****************************************************************************/

#ifndef _WINDOWS_H_
#define _WINDOWS_H_

#include <sys/types.h>

#include <QObject>
#include <QString>
#include <q3ptrlist.h>
#include <qwindowdefs.h>

#include <netwm_def.h>

#include "khotkeysglobal.h"

class KConfig;
class KWinModule;

namespace KHotKeys
{

const int SUPPORTED_WINDOW_TYPES_MASK = NET::NormalMask | NET::DesktopMask | NET::DockMask
    | NET::ToolbarMask | NET::MenuMask | NET::DialogMask | NET::OverrideMask | NET::TopMenuMask
    | NET::UtilityMask | NET::SplashMask;

class Windowdef_list;
/*class Action_data_base;*/

class Windows
    : public QObject
    {
    Q_OBJECT
    public:
        Windows( bool enable_signals_P, QObject* parent_P );
        virtual ~Windows();
        QString get_window_class( WId id_P );
        QString get_window_role( WId id_P );
        WId active_window();
        void set_action_window( WId window );
        WId action_window();
        WId find_window( const Windowdef_list* window_P );
        static WId window_at_position( int x, int y );
        static void activate_window( WId id_P );
    Q_SIGNALS:
        void window_added( WId window_P );
        void window_removed( WId window_P );
        void active_window_changed( WId window_P );
        void window_changed( WId window_P );
        void window_changed( WId window_P, unsigned int flags_P );
    protected Q_SLOTS:
        void window_added_slot( WId window_P );
        void window_removed_slot( WId window_P );
        void active_window_changed_slot( WId window_P );
        void window_changed_slot( WId window_P );
        void window_changed_slot( WId window_P, unsigned int flags_P );
    private:
        bool signals_enabled;
        KWinModule* kwin_module;
        WId _action_window;
    };
    
struct Window_data
    {
    Window_data( WId id_P );
    QString title; // _NET_WM_NAME or WM_NAME
    QString role; // WM_WINDOW_ROLE
    QString wclass; // WM_CLASS
    NET::WindowType type;
    };
    
class Windowdef
    {
    public:
        Windowdef( const QString& comment_P );
        Windowdef( KConfig& cfg_P );
        virtual ~Windowdef();
        const QString& comment() const;
        virtual bool match( const Window_data& window_P ) = 0;
        static Windowdef* create_cfg_read( KConfig& cfg_P/*, Action_data_base* data_P*/ );
        virtual void cfg_write( KConfig& cfg_P ) const = 0;
        virtual Windowdef* copy( /*Action_data_base* data_P*/ ) const = 0;
        virtual const QString description() const = 0;
    private:
        QString _comment;
    KHOTKEYS_DISABLE_COPY( Windowdef ); // CHECKME asi pak udelat i pro vsechny potomky, at se nezapomene
    };

class Windowdef_list
    : public Q3PtrList< Windowdef >
    {
    public:
        Windowdef_list( const QString& comment_P );
        Windowdef_list( KConfig& cfg_P/*, Action_data_base* data_P*/ );
        void cfg_write( KConfig& cfg_P ) const;
        bool match( const Window_data& window_P ) const;
        Windowdef_list* copy( /*Action_data_base* data_P*/ ) const;
        typedef Q3PtrListIterator< Windowdef > Iterator;
        const QString& comment() const;
    private:
        QString _comment;
    KHOTKEYS_DISABLE_COPY( Windowdef_list );
    };

class Windowdef_simple
    : public Windowdef
    {
    typedef Windowdef base;
    public:
        enum substr_type_t
            {
            NOT_IMPORTANT,
            CONTAINS,
            IS,
            REGEXP,
            CONTAINS_NOT,
            IS_NOT,
            REGEXP_NOT
            };
        enum window_type_t
            {
            WINDOW_TYPE_NORMAL     = ( 1 << NET::Normal ),
            WINDOW_TYPE_DESKTOP    = ( 1 << NET::Desktop ),
            WINDOW_TYPE_DOCK       = ( 1 << NET::Dock ),
//            WINDOW_TYPE_TOOL       = ( 1 << NET::Tool ),
//            WINDOW_TYPE_MENU       = ( 1 << NET::Menu ),
            WINDOW_TYPE_DIALOG     = ( 1 << NET::Dialog )
            };
        Windowdef_simple( const QString& comment_P, const QString& title_P,
            substr_type_t title_type_P, const QString& wclass_P, substr_type_t wclass_type_P,
            const QString& role_P, substr_type_t role_type_P, int window_types_P );
        Windowdef_simple( KConfig& cfg_P );
        virtual bool match( const Window_data& window_P );
        virtual void cfg_write( KConfig& cfg_P ) const;
        const QString& title() const;
        substr_type_t title_match_type() const;
        const QString& wclass() const;
        substr_type_t wclass_match_type() const;
        const QString& role() const;
        substr_type_t role_match_type() const;
        int window_types() const;
        bool type_match( window_type_t type_P ) const;
        bool type_match( NET::WindowType type_P ) const;
        virtual Windowdef* copy( /*Action_data_base* data_P*/ ) const;
        virtual const QString description() const;
    protected:
        bool is_substr_match( const QString& str1_P, const QString& str2_P,
            substr_type_t type_P );
    private:
        QString _title; 
        substr_type_t title_type;
        QString _wclass;
        substr_type_t wclass_type;
        QString _role;
        substr_type_t role_type;
        int _window_types;
    };

//***************************************************************************
// Inline
//***************************************************************************

// Windowdef

inline
Windowdef::Windowdef( const QString& comment_P )
    : _comment( comment_P )
    {
    }
    
inline
const QString& Windowdef::comment() const
    {
    return _comment;
    }
        
inline
Windowdef::~Windowdef()
    {
    }

// Windowdef_list

inline
Windowdef_list::Windowdef_list( const QString& comment_P )
    : Q3PtrList< Windowdef >(), _comment( comment_P )
    {
    setAutoDelete( true );
    }

inline
const QString& Windowdef_list::comment() const
    {
    return _comment;
    }

// Windowdef_simple

inline
const QString& Windowdef_simple::title() const
    {
    return _title;
    }

inline
Windowdef_simple::substr_type_t Windowdef_simple::title_match_type() const
    {
    return title_type;
    }
    
inline
const QString& Windowdef_simple::wclass() const
    {
    return _wclass;
    }
    
inline
Windowdef_simple::substr_type_t Windowdef_simple::wclass_match_type() const
    {
    return wclass_type;
    }
    
inline
const QString& Windowdef_simple::role() const
    {
    return _role;
    }
    
inline
Windowdef_simple::substr_type_t Windowdef_simple::role_match_type() const
    {
    return role_type;
    }
    
inline
int Windowdef_simple::window_types() const
    {
    return _window_types;
    }
    
inline
bool Windowdef_simple::type_match( window_type_t type_P ) const
    {
    return window_types() & type_P;
    }
    
inline
bool Windowdef_simple::type_match( NET::WindowType type_P ) const
    {
    return ( window_types() & ( 1 << type_P ))
        || ( type_P == NET::Unknown && ( window_types() & WINDOW_TYPE_NORMAL ));
        // CHECKME HACK haaaack !
    }
    
} // namespace KHotKeys
    
#endif
