/*
    $Id$

    Copyright (C) 2000 Carsten Pfeiffer <pfeiffer@kde.org>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.

*/

#include <qstringlist.h>

#include <kconfig.h>
#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>
#include <kstddirs.h>

#include "events.h"

// simple access to all knotify-handled applications
Events::Events()
{
    m_apps.setAutoDelete( true );
}


void Events::load()
{
    m_apps.clear();
    m_apps.append( new KNApplication( new KConfig("eventsrc", false, false)));

    QStringList fullpaths(KGlobal::dirs()->findAllResources("data", "*/eventsrc", false, true));
    QString path;
    for (QStringList::Iterator it=fullpaths.begin(); it!=fullpaths.end(); ++it) {
        path = makeRelative( *it );
	if ( !path.isEmpty() ) {
	    KConfig *kc = new KConfig( path, false, false, "data" );
	    m_apps.append( new KNApplication( kc ));
	}
    }
}

void Events::save()
{
    kdDebug() << "save\n";

    KNApplicationListIterator it( m_apps );
    while ( it.current() ) {
	(*it)->save();
	++it;
    }
}

// returns e.g. "kwin/eventsrc" from a given path
// "/opt/kde2/share/apps/kwin/eventsrc"
QString Events::makeRelative( const QString& fullPath )
{
  int slash = fullPath.findRev( '/' ) - 1;
  slash = fullPath.findRev( '/', slash );

  if ( slash < 0 )
    return QString::null;

  return fullPath.mid( slash+1 );
}

//////////////////////////////////////////////////////////////////////



// ownership of the KConfig is transferred
KNApplication::KNApplication( KConfig *config )
{
    m_events = 0L;
    kc = config;
    kc->setGroup( QString::fromLatin1("!Global!") );
    m_icon = kc->readEntry(QString::fromLatin1("IconName"),
                           QString::fromLatin1("misc"));
    m_description = kc->readEntry( QString::fromLatin1("Comment"),
				   i18n("No description available") );
}

KNApplication::~KNApplication()
{
    delete kc;
    delete m_events;
}


EventList * KNApplication::eventList()
{
    if ( !m_events ) {
	m_events = new EventList;
	m_events->setAutoDelete( true );
	loadEvents();
    }

    return m_events;
}


void KNApplication::save()
{
    if ( !m_events )
	return;

    QString presentation = QString::fromLatin1("presentation");
    QString soundfile = QString::fromLatin1("soundfile");
    QString logfile = QString::fromLatin1("logfile");

    KNEventListIterator it( *m_events );
    KNEvent *e;
    while ( (e = it.current()) ) {
	kc->setGroup( e->configGroup );
	kc->writeEntry( presentation, e->presentation );
	kc->writeEntry( soundfile, e->soundfile );
	kc->writeEntry( logfile, e->logfile );

	++it;
    }
    kc->sync();
}


void KNApplication::loadEvents()
{
    KNEvent *e = 0L;

    QString global = QString::fromLatin1("!Global!");
    QString default_group = QString::fromLatin1("<default>");
    QString name = QString::fromLatin1("Name");
    QString comment = QString::fromLatin1("Comment");
    QString unknown = i18n("Unknown Title");
    QString nodesc = i18n("No Description");

    QString presentation = QString::fromLatin1("presentation");
    QString defpresentation = QString::fromLatin1("default_presentation");
    QString nopresentation = QString::fromLatin1("nopresentation");
    QString soundfile = QString::fromLatin1("soundfile");
    QString defsoundfile = QString::fromLatin1("default_sound");
    QString logfile = QString::fromLatin1("logfile");
    QString deflogfile = QString::fromLatin1("default_logfile");

    QStringList conflist = kc->groupList();
    QStringList::Iterator it = conflist.begin();

    while ( it != conflist.end() ) {
	if ( (*it) != global && (*it) != default_group ) { // event group
	    kc->setGroup( *it );

	    e = new KNEvent;
	    e->name = kc->readEntry( name, unknown );
	    e->description = kc->readEntry( comment, nodesc );
	    e->configGroup = *it;

	    if ( e->name.isEmpty() || e->description.isEmpty() )
		delete e;

	    else { // load the event
		int default_rep = kc->readNumEntry( defpresentation, 0 );
		e->presentation = kc->readNumEntry( presentation, default_rep);
		e->dontShow = kc->readNumEntry( nopresentation, 0 );
		e->logfile = kc->readEntry(logfile, kc->readEntry(deflogfile));
		e->soundfile = kc->readEntry( soundfile,
					      kc->readEntry( defsoundfile ));

		m_events->append( e );
	    }
	}

	++it;
    }

    return;
}
