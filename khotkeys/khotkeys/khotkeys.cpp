/****************************************************************************

 KHotKeys -  (C) 2000 Lubos Lunak <l.lunak@email.cz>

 khotkeys.cpp  -
 
 $Id$

****************************************************************************/

#define __khotkeys_CPP

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <krun.h>
#include <ksimpleconfig.h>
#include <kurifilter.h>

#include "khotkeys.h"

KHotKeysApp::KHotKeysApp()
    : KUniqueApplication( false, true ) // no styles
    {
    accel = new KHKGlobalAccel();
    reread_configuration();
    }

KHotKeysApp::~KHotKeysApp()
    {
    delete accel;
    }

void KHotKeysApp::accel_activated( const QString& action, const QString&, int )
    {
    KHotData* current = data[ action ]; // CHECKME should always find
    if( current->timeout.isActive()) // a little timeout after running
        return;
    const QString& run = current->run.stripWhiteSpace();
    if( run == "" )
        return;
    KURIFilterData uri = run;
    KURIFilter::self()->filterURI( uri );
    switch( uri.uriType())
        {
        case KURIFilterData::LOCAL_FILE:
        case KURIFilterData::LOCAL_DIR:
        case KURIFilterData::NET_PROTOCOL:
        case KURIFilterData::HELP:
            {
            ( void ) new KRun( uri.uri());
          break;
            }
        case KURIFilterData::EXECUTABLE:
        case KURIFilterData::SHELL:
        default:  // try simply executing the command if unknown
            {
            int space_pos = run.find( ' ' );
            QString icon_name = uri.iconName();
            if( icon_name.isNull())
                icon_name = QString::fromLatin1( "go" );
            if( run[ 0 ] != '\'' && run[ 0 ] != '"' && space_pos > -1
                && run[ space_pos - 1 ] != '\\' )
                {
                if( !KRun::runCommand( run, run.left( space_pos ), icon_name ))
                    ;// CHECKME error 
                }
            else
                {
                if( !KRun::runCommand( run, run, icon_name ))
                    ; // CHECKME error
                }
          break;
            }
        }
    current->timeout.start( 1000, true ); // 1sec timeout
    }

void KHotKeysApp::reread_configuration()
    {
    accel->clear();
    data.clear();    
    KSimpleConfig cfg( "khotkeysrc", true );
    data.read_config( cfg );
    for( KHotData_dict::Iterator it( data );
         it.current();
         ++it )
        {
        if( !accel->insertItem( it.currentKey(), it.currentKey(),
            it.current()->shortcut ))
            continue; // invalid shortcut
        accel->connectItem( it.currentKey(), this,
            SLOT( accel_activated( const QString&, const QString&, int )));
        }
    }
    
bool KHotKeysApp::x11EventFilter(XEvent * ev) 
    {
    if( accel->x11EventFilter( ev ))
        return true;
    return KUniqueApplication::x11EventFilter( ev );
    }

#include "khotkeys.moc"
