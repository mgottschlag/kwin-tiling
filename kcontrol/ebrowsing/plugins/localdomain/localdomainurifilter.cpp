/*
    kshorturifilter.h

    This file is part of the KDE project
    Copyright (C) 2002 Lubos Lunak <llunak@suse.cz>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2
    as published by the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/


/*
 
 This filter takes care of hostnames in the local search domain.
 If you're in domain domain.org which has a host intranet.domain.org
 and the typed URI is just intranet, check if there's a host
 intranet.domain.org and if yes, it's a network URI.

*/

#include "localdomainurifilter.h"

#include <time.h>

#include <qregexp.h>

#include <kprocess.h>
#include <kstandarddirs.h>

LocalDomainURIFilter::LocalDomainURIFilter( QObject *parent, const char *name,
    const QStringList & /*args*/ )
    : KURIFilterPlugin( parent, name ? name : "localdomainurifilter", 1.0 ),
      DCOPObject( "LocalDomainURIFilterIface" ), last_time( 0 )
    {
    configure();
    }

bool LocalDomainURIFilter::filterURI( KURIFilterData& data ) const
    {
    KURL url = data.uri();
    QString cmd = url.url();

    if( cmd[ 0 ] != '#' && cmd[ 0 ] != '~' && cmd[ 0 ] != '/'
	&& !cmd.contains( ' ' ) && !cmd.contains( '.' )
	&& !cmd.contains( ':' ) // KShortURIFilter takes care of these
	// most of these are taken from KShortURIFilter
	&& cmd[ cmd.length() - 1 ] != '&'
	&& !cmd.contains( QString::fromLatin1("||"))
	&& !cmd.contains( QString::fromLatin1("&&")) // must not look like shell
	&& !cmd.contains( QRegExp( QString::fromLatin1( "[ ;<>]" )))
	&& KStandardDirs::findExe( cmd ).isNull()
	&& url.isMalformed()
	&& isLocalDomainHost( cmd ))
        {
        cmd.insert( 0, QString::fromLatin1( "http://" ));
        setFilteredURI( data, cmd );
        setURIType( data, KURIFilterData::NET_PROTOCOL );
        return true;
        }
    return false;
    }

// if it's e.g. just 'www', try if it's a hostname in the local search domain
bool LocalDomainURIFilter::isLocalDomainHost( const QString& cmd ) const
    {
    QString host( cmd.contains( '/' ) ? cmd.left( cmd.find( '/' )) : cmd );
    if( host == last_host && last_time > time( NULL ) - 5 )
	return last_result;

    pid_t pid;

	{
	QString helper = KStandardDirs::findExe(
	    QString::fromLatin1( "klocaldomainurifilterhelper" ));
	if( helper.isEmpty())
	    return false;
        KProcess proc;
        proc << helper << host;
        if( !proc.start( KProcess::DontCare ))
	    return false;
	pid = proc.getPid();
	} // destroy 'proc', so that KProcessController now won't do waitpid()
	  // on the process immediatelly

    last_host = host;
    last_time = time( NULL );
    for( int rounds = 0;
	 rounds < 50; // 50 * 20ms = 1s
	 ++rounds )
	{
	int status;
	int ret = waitpid( pid, &status, WNOHANG );
	if( ret < 0 )
	    return false;
	if( ret > 0 )
	    {
	    bool last_result = WIFEXITED( status ) && WEXITSTATUS( status ) == 0;
	    return last_result;
	    }
	struct timespec tm;
	tm.tv_sec = 0;
	tm.tv_nsec = 20 * 1000 * 1000; // 20ms
	nanosleep( &tm, NULL );
	}
    kill( pid, SIGTERM );
    last_result = false;
    return false;
    }

void LocalDomainURIFilter::configure()
    {
    // nothing
    }

K_EXPORT_COMPONENT_FACTORY( liblocaldomainurifilter, 
	                    KGenericFactory<LocalDomainURIFilter>( "localdomainurifilter" ) );

#include "localdomainurifilter.moc"
