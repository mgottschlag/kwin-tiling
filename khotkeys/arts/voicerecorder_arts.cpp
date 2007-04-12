/****************************************************************************

 KHotKeys
 
 Copyright (C) 2005 Olivier Goffgart <ogoffart @ kde.org>

 Distributed under the terms of the GNU General Public License version 2.
 
****************************************************************************/

#include <QtGui/QColor>
#include <QtGui/QActionEvent>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "voicerecorder.h"

#ifdef HAVE_ARTS

#include <arts/kplayobject.h>
#include <arts/kartsserver.h>
#include <arts/kartsdispatcher.h>
#include <arts/kplayobjectfactory.h>

extern "C"
KDE_EXPORT
void khotkeys_voicerecorder_arts_play( const QString& file )
{
        KHotKeys::VoiceRecorder::arts_play_fun check = khotkeys_voicerecorder_arts_play; // check the type matches
        ( void ) check;
	KArtsDispatcher dispatcher;
	KArtsServer server;
	KDE::PlayObjectFactory factory( server.server() );
	KDE::PlayObject* playobj = factory.createPlayObject( file, true );
	playobj->play();
}

#endif
