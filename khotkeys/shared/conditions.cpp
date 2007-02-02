/****************************************************************************

 KHotKeys

 Copyright (C) 1999-2001 Lubos Lunak <l.lunak@kde.org>

 Distributed under the terms of the GNU General Public License version 2.

****************************************************************************/

#define _CONDITIONS_CPP_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "conditions.h"

#ifdef KHOTKEYS_DEBUG
#include <typeinfo>
#endif

#include <kconfig.h>
#include <kdebug.h>
#include <klocale.h>
#include <assert.h>

#include <X11/Xlib.h>
#include <fixx11h.h>

#include "action_data.h"
//Added by qt3to4:
#include <Q3PtrList>

namespace KHotKeys
{

// Condition

Condition::Condition( Condition_list_base* parent_P )
    : _parent( parent_P )
    {
    if( _parent )
        _parent->append( this );
    }

Condition::Condition( KConfig&, Condition_list_base* parent_P )
    : _parent( parent_P )
    {
    if( _parent )
        _parent->append( this );
    }

Condition* Condition::create_cfg_read( KConfig& cfg_P, Condition_list_base* parent_P )
    {
    QString type = cfg_P.readEntry( "Type" );
    if( type == "ACTIVE_WINDOW" )
        return new Active_window_condition( cfg_P, parent_P );
    if( type == "EXISTING_WINDOW" )
        return new Existing_window_condition( cfg_P, parent_P );
    if( type == "NOT" )
        return new Not_condition( cfg_P, parent_P );
    if( type == "AND" )
        return new And_condition( cfg_P, parent_P );
    if( type == "OR" )
        return new Or_condition( cfg_P, parent_P );
    kWarning( 1217 ) << "Unknown Condition type read from cfg file\n";
    return NULL;
    }

Condition::~Condition()
    {
    if( _parent )
        _parent->remove( this );
    }


void Condition::cfg_write( KConfig& cfg_P ) const
    {
    cfg_P.writeEntry( "Type", "ERROR" );
    }

void Condition::updated() const
    {
    if( !khotkeys_active())
        return;
    assert( _parent != NULL );
    _parent->updated();
    }

#ifdef KHOTKEYS_DEBUG
void Condition::debug( int depth_P )
    {
    char tmp[ 1024 ];
    int i;
    for( i = 0;
         i < depth_P;
         ++i )
        tmp[ i ] = ' ';
    tmp[ i ] = '\0';
    kDebug( 1217 ) << tmp << description() << ":(" << this << ")" << endl;
    }

void Condition::debug_list( const Q3PtrList< Condition >& list_P, int depth_P )
    {
    char tmp[ 1024 ];
    int i;
    for( i = 0;
         i < depth_P;
         ++i )
        tmp[ i ] = ' ';
    tmp[ i ] = '\0';
    for( Q3PtrListIterator< Condition > it( list_P );
         it;
         ++it )
        (*it)->debug( depth_P + 1 );
    }
#endif


// Condition_list_base

Condition_list_base::Condition_list_base( KConfig& cfg_P, Condition_list_base* parent_P )
    : Condition( parent_P )
    {
    QString save_cfg_group = cfg_P.group();
    int cnt = cfg_P.readEntry( "ConditionsCount", 0 );
    for( int i = 0;
         i < cnt;
         ++i )
        {
        cfg_P.setGroup( save_cfg_group + QString::number( i ));
        (void) Condition::create_cfg_read( cfg_P, this );
        }
    cfg_P.setGroup( save_cfg_group );
    }

Condition_list_base::~Condition_list_base()
    {
    while( !isEmpty())
        {
        Condition* c = getFirst();
        remove( c );
        delete c;
        }
    }
    
void Condition_list_base::cfg_write( KConfig& cfg_P ) const
    {
    QString save_cfg_group = cfg_P.group();
    int i = 0;
    for( Iterator it( *this );
         it;
         ++it, ++i )
        {
        cfg_P.setGroup( save_cfg_group + QString::number( i ));
        it.current()->cfg_write( cfg_P );
        }
    cfg_P.setGroup( save_cfg_group );
    cfg_P.writeEntry( "ConditionsCount", i );
    }

bool Condition_list_base::accepts_children() const
    {
    return true;
    }

#ifdef KHOTKEYS_DEBUG
void Condition_list_base::debug( int depth_P )
    {
    char tmp[ 1024 ];
    int i;
    for( i = 0;
         i < depth_P;
         ++i )
        tmp[ i ] = ' ';
    tmp[ i ] = '\0';
    kDebug( 1217 ) << tmp << typeid( *this ).name() << ":(" << this << ")" << endl;
    debug_list( *this, depth_P + 1 );
    }
#endif

// Condition_list

Condition_list::Condition_list( KConfig& cfg_P, Action_data_base* data_P )
    : Condition_list_base( cfg_P, NULL ), data( data_P )
    {
    _comment = cfg_P.readEntry( "Comment" );
    }

void Condition_list::cfg_write( KConfig& cfg_P ) const
    {
    base::cfg_write( cfg_P );
    cfg_P.writeEntry( "Comment", comment());
    }

Condition_list* Condition_list::copy( Action_data_base* data_P ) const
    {
    Condition_list* ret = new Condition_list( comment(), data_P );
    for( Iterator it( *this );
         it;
         ++it )
        ret->append( it.current()->copy( ret ));
    return ret;
    }


bool Condition_list::match() const
    {
    if( count() == 0 ) // no conditions to match => ok
        return true;
    for( Iterator it( *this );
         it;
         ++it )
        if( it.current()->match()) // OR
            return true;
    return false;
    }

void Condition_list::updated() const
    {
    if( !khotkeys_active())
        return;
    data->update_triggers();
//    base::updated(); no need to, doesn't have parent
    }

// CHECKME tohle je drobet hack, jeste to zvazit
void Condition_list::set_data( Action_data_base* data_P )
    {
    assert( data == NULL || data == data_P );
    data = data_P;
    }

const QString Condition_list::description() const
    {
    assert( false );
    return QString();
    }

Condition_list* Condition_list::copy( Condition_list_base* ) const
    {
    assert( false );
    return NULL;
    }

// Active_window_condition

Active_window_condition::Active_window_condition( KConfig& cfg_P, Condition_list_base* parent_P )
    : Condition( cfg_P, parent_P )
    {
    QString save_cfg_group = cfg_P.group();
    cfg_P.setGroup( save_cfg_group + "Window" );
    _window = new Windowdef_list( cfg_P );
    cfg_P.setGroup( save_cfg_group );
    init();
    set_match();
    }

void Active_window_condition::init()
    {
    connect( windows_handler, SIGNAL( active_window_changed( WId )),
        this, SLOT( active_window_changed( WId )));
    }

bool Active_window_condition::match() const
    {
    return is_match;
    }

void Active_window_condition::set_match()
    {
    is_match = window()->match( Window_data( windows_handler->active_window()));
    kDebug( 1217 ) << "Active_window_condition::set_match :" << is_match << endl;
    updated();
    }

void Active_window_condition::cfg_write( KConfig& cfg_P ) const
    {
    base::cfg_write( cfg_P );
    QString save_cfg_group = cfg_P.group();
    cfg_P.setGroup( save_cfg_group + "Window" );
    window()->cfg_write( cfg_P );
    cfg_P.setGroup( save_cfg_group );
    cfg_P.writeEntry( "Type", "ACTIVE_WINDOW" ); // overwrites value set in base::cfg_write()
    }

#ifdef HAVE_COVARIANT_RETURN
Active_window_condition* Active_window_condition::copy( Condition_list_base* parent_P ) const
#else
Condition* Active_window_condition::copy( Condition_list_base* parent_P ) const
#endif
    {
    return new Active_window_condition( window()->copy(), parent_P );
    }

const QString Active_window_condition::description() const
    {
    return i18n( "Active window: " ) + window()->comment();
    }

void Active_window_condition::active_window_changed( WId )
    {
    set_match();
    }

Active_window_condition::~Active_window_condition()
    {
    disconnect( windows_handler, NULL, this, NULL );
    delete _window;
    }

// Existing_window_condition

Existing_window_condition::Existing_window_condition( KConfig& cfg_P, Condition_list_base* parent_P )
    : Condition( cfg_P, parent_P )
    {
    QString save_cfg_group = cfg_P.group();
    cfg_P.setGroup( save_cfg_group + "Window" );
    _window = new Windowdef_list( cfg_P );
    cfg_P.setGroup( save_cfg_group );
    init();
    set_match();
    }

void Existing_window_condition::init()
    {
    connect( windows_handler, SIGNAL( window_added( WId )), this, SLOT( window_added( WId )));
    connect( windows_handler, SIGNAL( window_removed( WId )), this, SLOT( window_removed( WId )));
    }

bool Existing_window_condition::match() const
    {
    return is_match;
    }

void Existing_window_condition::set_match( WId w_P )
    {
    if( w_P != None && !is_match )
        is_match = window()->match( Window_data( w_P ));
    else
        is_match = windows_handler->find_window( window()) != None;
    kDebug( 1217 ) << "Existing_window_condition::set_match :" << is_match << endl;
    updated();
    }

void Existing_window_condition::cfg_write( KConfig& cfg_P ) const
    {
    base::cfg_write( cfg_P );
    QString save_cfg_group = cfg_P.group();
    cfg_P.setGroup( save_cfg_group + "Window" );
    window()->cfg_write( cfg_P );
    cfg_P.setGroup( save_cfg_group );
    cfg_P.writeEntry( "Type", "EXISTING_WINDOW" ); // overwrites value set in base::cfg_write()
    }

#ifdef HAVE_COVARIANT_RETURN
Existing_window_condition* Existing_window_condition::copy( Condition_list_base* parent_P ) const
#else
Condition* Existing_window_condition::copy( Condition_list_base* parent_P ) const
#endif
    {
    return new Existing_window_condition( window()->copy(), parent_P );
    }

const QString Existing_window_condition::description() const
    {
    return i18n( "Existing window: " ) + window()->comment();
    }

void Existing_window_condition::window_added( WId w_P )
    {
    set_match( w_P );
    }

void Existing_window_condition::window_removed( WId )
    {
    set_match();
    }

Existing_window_condition::~Existing_window_condition()
    {
    disconnect( windows_handler, NULL, this, NULL );
    delete _window;
    }

// Not_condition

Not_condition::Not_condition( KConfig& cfg_P, Condition_list_base* parent_P )
    : Condition_list_base( cfg_P, parent_P )
    {
    // CHECKME kontrola poctu ?
    }

bool Not_condition::match() const
    {
    return condition() ? !condition()->match() : false;
    }

void Not_condition::cfg_write( KConfig& cfg_P ) const
    {
    base::cfg_write( cfg_P );
    cfg_P.writeEntry( "Type", "NOT" ); // overwrites value set in base::cfg_write()
    }

Not_condition* Not_condition::copy( Condition_list_base* parent_P ) const
    {
    Not_condition* ret = new Not_condition( parent_P );
    if( condition())
        ret->append( condition()->copy( ret ));
    return ret;
    }

const QString Not_condition::description() const
    {
    return i18nc( "Not_condition", "Not" );
    }

bool Not_condition::accepts_children() const
    {
    return count() == 0;
    }

// And_condition

And_condition::And_condition( KConfig& cfg_P, Condition_list_base* parent_P )
    : Condition_list_base( cfg_P, parent_P )
    {
    // CHECKME kontrola poctu ?
    }

bool And_condition::match() const
    {
    for( Iterator it( *this );
         it;
         ++it )
        if( !it.current()->match()) // AND
            return false;
    return true; // all true (or empty)
    }

void And_condition::cfg_write( KConfig& cfg_P ) const
    {
    base::cfg_write( cfg_P );
    cfg_P.writeEntry( "Type", "AND" ); // overwrites value set in base::cfg_write()
    }

And_condition* And_condition::copy( Condition_list_base* parent_P ) const
    {
    And_condition* ret = new And_condition( parent_P );
    for( Iterator it( *this );
         it;
         ++it )
        ret->append( (*it)->copy( ret ));
    return ret;
    }

const QString And_condition::description() const
    {
    return i18nc( "And_condition", "And" );
    }

// Or_condition

Or_condition::Or_condition( KConfig& cfg_P, Condition_list_base* parent_P )
    : Condition_list_base( cfg_P, parent_P )
    {
    // CHECKME kontrola poctu ?
    }

bool Or_condition::match() const
    {
    if( count() == 0 ) // empty => ok
        return true;
    for( Iterator it( *this );
         it;
         ++it )
        if( it.current()->match()) // OR
            return true;
    return false;
    }

void Or_condition::cfg_write( KConfig& cfg_P ) const
    {
    base::cfg_write( cfg_P );
    cfg_P.writeEntry( "Type", "OR" ); // overwrites value set in base::cfg_write()
    }

Or_condition* Or_condition::copy( Condition_list_base* parent_P ) const
    {
    Or_condition* ret = new Or_condition( parent_P );
    for( Iterator it( *this );
         it;
         ++it )
        ret->append( (*it)->copy( ret ));
    return ret;
    }

const QString Or_condition::description() const
    {
    return i18nc( "Or_condition", "Or" );
    }

} // namespace KHotKeys

#include "conditions.moc"
