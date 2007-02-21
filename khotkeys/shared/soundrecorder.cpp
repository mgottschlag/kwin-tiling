/***************************************************************************
 *   Copyright (C) 2005 by Olivier Goffart   *
 *   ogoffart@kde.org   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.             *
 ***************************************************************************/

#include "config-khotkeys.h"

#include "soundrecorder.h"

#include <kdebug.h>
#include <klocale.h>
#include <qtimer.h>
#include <klibloader.h>

#include "khotkeysglobal.h"

namespace KHotKeys
{

SoundRecorder::create_ptr SoundRecorder::create_fun = NULL;

bool SoundRecorder::init( KLibrary* lib )
{
#ifdef HAVE_ARTS
    if( create_fun == NULL && lib != NULL )
        create_fun = (create_ptr) lib->symbol( "khotkeys_soundrecorder_create" );
#endif
//    kdDebug( 1217 ) << "soundrecorder:" << create_fun << ":" << lib << endl;
    return create_fun != NULL;
}

SoundRecorder* SoundRecorder::create( QObject* parent )
{
#ifdef HAVE_ARTS
    if( create_fun != NULL )
        return create_fun( parent, name );
#endif
    return new SoundRecorder( parent );
}

SoundRecorder::SoundRecorder(QObject *parent)  : QObject(parent) {}

SoundRecorder::~SoundRecorder()
{
}

void SoundRecorder::start()
{
}

void SoundRecorder::stop()
{
}

void SoundRecorder::abort()
{
}


Sound SoundRecorder::sound()
{
	Sound s;
	return s;
}

}

#include "soundrecorder.moc"
