/****************************************************************************

 KHotKeys
 
 Copyright (C) 1999-2001 Lubos Lunak <l.lunak@kde.org>

 Distributed under the terms of the GNU General Public License version 2.
 
****************************************************************************/

#define _KHOTKEYSGLOBAL_CPP_

#include "khotkeysglobal.h"

#include <assert.h>
#include <kdebug.h>
#include <kstandarddirs.h>
#include <klibloader.h>

#include "input.h"
#include "windows.h"
#include "triggers.h"
#include "gestures.h"
#include "voices.h"

// FIXME: SOUND
// #include "soundrecorder.h"

namespace KHotKeys
{

Kbd* keyboard_handler;
Windows* windows_handler;
static bool _khotkeys_active = false;

void init_global_data( bool active_P, QObject* owner_P )
    {
    assert( keyboard_handler == NULL );
    assert( windows_handler == NULL );
    assert( gesture_handler == NULL );
    static_cast< void >( new Kbd( active_P, owner_P ));
    static_cast< void >( new Windows( active_P, owner_P ));
    static_cast< void >( new Gesture( active_P, owner_P ));
// FIXME: SOUND
//    static_cast< void >( new Voice( active_P, owner_P ));
    khotkeys_set_active( false );
    }
    
void khotkeys_set_active( bool active_P )
    {
    _khotkeys_active = active_P;
    }
    
bool khotkeys_active()
    {
    return _khotkeys_active;
    }
    
// does the opposite of KStandardDirs::findResource() i.e. e.g.
// "/opt/kde2/share/applnk/System/konsole.desktop" -> "System/konsole.desktop"
QString get_menu_entry_from_path( const QString& path_P )
    {
    QStringList dirs = KGlobal::dirs()->resourceDirs( "apps" );
    for( QStringList::ConstIterator it = dirs.begin();
         it != dirs.end();
         ++it )
        if( path_P.indexOf( *it ) == 0 )
            {
            QString ret = path_P;
            ret.remove( 0, (*it).length());
            if( ret[ 0 ] == '/' )
                ret.remove( 0, 1 );
            return ret;
            }
    return path_P;
    }

static int have_arts = -1;

bool haveArts()
    {
    return false;
// FIXME: SOUND
#if 0
    if( have_arts == -1 )
        {
        have_arts = 0;
        KLibrary* arts = KLibLoader::self()->library( QLatin1String("khotkeys_arts") );
        if( arts == NULL )
            kDebug( 1217 ) << "Couldn't load khotkeys_arts:" << KLibLoader::self()->lastErrorMessage();
        if( arts != NULL && SoundRecorder::init( arts ))
            have_arts = 1;
        }
    return have_arts != 0;
#endif
    }

void disableArts()
    {
    have_arts = 0;
    }

} // namespace KHotKeys
