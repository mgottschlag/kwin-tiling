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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "localdomainurifilter.h"

#include <qregexp.h>

#include <kprocess.h>
#include <kstandarddirs.h>
#include <config.h>
#include <kdebug.h>

#define HOSTPORT_PATTERN "[a-zA-Z][a-zA-Z0-9-]*:[0-9]+"

LocalDomainURIFilter::LocalDomainURIFilter( QObject *parent, const char *name,
                                            const QStringList & /*args*/ )
    : KURIFilterPlugin( parent, name ? name : "localdomainurifilter", 1.0 ),
      DCOPObject( "LocalDomainURIFilterIface" ),
      last_time( 0 ),
      m_hostPortPattern( QString::fromLatin1(HOSTPORT_PATTERN) )
{
    configure();
}

bool LocalDomainURIFilter::filterURI( KURIFilterData& data ) const
{
    KURL url = data.uri();
    QString cmd = url.url();

    if( cmd[ 0 ] != '#' && cmd[ 0 ] != '~' && cmd[ 0 ] != '/'
        && !cmd.contains( ' ' ) && !cmd.contains( '.' )
        && !cmd.contains( '$' ) // env. vars could resolve to anything
        // most of these are taken from KShortURIFilter
        && cmd[ cmd.length() - 1 ] != '&'
        && !cmd.contains( QString::fromLatin1("||"))
        && !cmd.contains( QString::fromLatin1("&&")) // must not look like shell
        && !cmd.contains( QRegExp( QString::fromLatin1( "[ ;<>]" )))
        && KStandardDirs::findExe( cmd ).isNull()
        && ( url.isMalformed() || m_hostPortPattern.exactMatch( cmd ) )
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
    // find() returns -1 when no match -> left()/truncate() are noops then
    QString host( cmd.left( cmd.find( '/' ) ) );
    host.truncate( host.find( ':' ) ); // Remove port number

    if( host == last_host && last_time > time( NULL ) - 5 )
        return last_result;

    QString helper = KStandardDirs::findExe(QString::fromLatin1( "klocaldomainurifilterhelper" ));
    if( helper.isEmpty())
        return last_result = false;

    KProcess proc;
    proc << helper << host;
    if( !proc.start( KProcess::NotifyOnExit ))
        return last_result = false;

    last_host = host;
    last_time = time( (time_t *)0 );

    if( proc.wait( 1 ) && proc.normalExit() && proc.exitStatus() == 0)
        return last_result = true;

    return last_result = false;
}

void LocalDomainURIFilter::configure()
{
    // nothing
}

K_EXPORT_COMPONENT_FACTORY( liblocaldomainurifilter,
	                    KGenericFactory<LocalDomainURIFilter>( "localdomainurifilter" ) );

#include "localdomainurifilter.moc"
