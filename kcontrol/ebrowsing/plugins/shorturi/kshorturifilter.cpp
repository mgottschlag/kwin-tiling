/*
    kshorturifilter.h

    This file is part of the KDE project
    Copyright (C) 2000 Dawit Alemayehu <adawit@earthlink.net>
    Copyright (C) 2000 Malte Starostik <starosti@zedat.fu-berlin.de>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include <stdlib.h>
#include <unistd.h>
#include <pwd.h>
#include <sys/stat.h>

#include <qdir.h>
#include <qlist.h>

#include <kurl.h>
#include <kdebug.h>
#include <kprotocolmanager.h>
#include <kinstance.h>
#include <kglobal.h>
#include <kstddirs.h>

//#include "kshorturiopts.h"
#include "kshorturifilter.h"
#include "kshorturifilter.moc"

#define FQDN_PATTERN    "[a-zA-Z][a-zA-Z0-9-]*\\.[a-zA-Z]"
#define ENV_VAR_PATTERN "$[a-zA-Z_][a-zA-Z0-9_]*"
#define IPv4_PATTERN    "[0-9][0-9]?[0-9]?\\.[0-9][0-9]?[0-9]?\\.[0-9][0-9]?[0-9]?\\.[0-9][0-9]?[0-9]?:?[0-9]?[0-9]?[0-9]?/?"

KInstance *KShortURIFilterFactory::s_instance = 0;

KShortURIFilter::KShortURIFilter( QObject *parent, const char *name )
                :KURIFilterPlugin( parent, name ? name : "shorturi", 1.0),
                 DCOPObject("KShortURIFilterIface")
{
    // TODO: Make this configurable.  Should go into control module...
    m_urlHints.insert("www.", "http://");
    m_urlHints.insert("ftp.", "ftp://");
    m_urlHints.insert("news.", "news://");
}

bool KShortURIFilter::isValidShortURL( const QString& cmd ) const
{
  // TODO: configurability like always treat foobar/XXX as a shortURL
  if ( QRegExp( "[ ;<>]" ).match( cmd ) >= 0
    || QRegExp( "||" ).match( cmd ) >= 0
    || QRegExp( "&&" ).match( cmd ) >= 0
    || cmd.at( cmd.length()-1 ) == '&' ) return false;

  return true;
}

bool KShortURIFilter::expandEnivVar( QString& cmd ) const
{
    // ENVIRONMENT variable expansion
    int env_len = 0;
    int env_loc = 0;
    while( 1 )
    {
        env_loc = QRegExp( ENV_VAR_PATTERN ).match( cmd, env_loc, &env_len );
        if( env_loc == -1 ) break;
        const char* exp = getenv( cmd.mid( env_loc + 1, env_len - 1 ).local8Bit().data() );
	cmd.replace( env_loc, env_len, QString::fromLocal8Bit(exp) );
    }
    return ( env_len ) ? true : false;
}

bool KShortURIFilter::filterURI( KURIFilterData& data ) const
{
    QString cmd = data.uri().url();

    // We process SMB first because it can be
    // represented with a special format.
    int match = cmd.lower().find( "smb:" );
    if (  match == 0 || cmd.find( "\\\\" ) == 0 )
    {
        if( match == 0 )
            cmd = QDir::cleanDirPath( cmd.mid( 4 ) );
        else
        {
            match = 0;
            while( cmd[match] == '\\' ) match++;
            cmd = cmd.mid( match );
        }
        for (uint i=0; i < cmd.length(); i++)
        {
            if (cmd[i]=='\\')
                cmd[i]='/';
        }
        cmd[0] == '/' ? cmd.prepend( "smb:" ) : cmd.prepend( "smb:/" );
        setFilteredURI( data, cmd );
        setURIType( data, KURIFilterData::NET_PROTOCOL );
        return data.hasBeenFiltered();
    }

    // Process the url for known and supported protocols. If it is
    // a match and a valid url, we return immediately w/o filtering
    // except if the protocol is "file".  The reason for this is that
    // more filtering such as environment expansion might be required.
    QStringList protocols = KProtocolManager::self().protocols();
    for( QStringList::ConstIterator it = protocols.begin(); it != protocols.end(); it++ )
    {
        if( (cmd.left((*it).length()).lower() == *it) &&
            !data.uri().isMalformed() && !data.uri().isLocalFile() )
        {
            setFilteredURI( data, cmd );
            if ( *it == "man" || *it == "help" )
                setURIType( data, KURIFilterData::HELP );
            else
                setURIType( data, KURIFilterData::NET_PROTOCOL );
            return data.hasBeenFiltered();
        }
    }

    // See if the beginning of cmd looks like a valid FQDN.
    QString host;
    if( QRegExp( FQDN_PATTERN ).match(cmd) == 0 )
    {
        host = cmd.left(cmd.find('.'));
        // Check if it's one of the urlHints and qualify the URL
        QString proto = m_urlHints[host.lower()];
        if( proto.length() != 0 )
        {
            cmd.insert(0, proto);
            setFilteredURI( data, cmd );
            setURIType( data, KURIFilterData::NET_PROTOCOL );
            return data.hasBeenFiltered();
        }
    }
    // Assume http if cmd starts with <IP>/
    // Will QRegExp support ([0-9]{1,3}\.){3}[0-9]{1,3} some time?
    // TODO : ADD LITERAL IPv6 support - See RFC 2373 Appendix B
    if ( QRegExp( IPv4_PATTERN ).match(cmd) == 0 )
    {
        cmd.insert(0, "http://");
        setFilteredURI( data, cmd );
        setURIType( data, KURIFilterData::NET_PROTOCOL );
        return data.hasBeenFiltered();
    }

    if( cmd[0] == '#' )
    {
        if( cmd.left(2) == "##" )
            cmd = "info:/" + ( cmd.length() == 2 ? QString::fromLatin1("dir" ) : cmd.mid(2));
        else if ( cmd[0] == '#' )
            cmd = "man:/" + cmd.mid(1);
        setFilteredURI( data, cmd );
        setURIType( data, KURIFilterData::HELP );
        return data.hasBeenFiltered();
    }

    if( !data.uri().isMalformed() && data.uri().isLocalFile() )
    {
      // Local file and directory processing.
      // Strip off "file:/" in order to expand local
      // URLs if necessary.
      // Use KURL::path, which does the right thing.
      // The previous hack made "/" an empty string. (David)
      cmd = QDir::cleanDirPath( data.uri().path() );
    }

    // HOME directory ?
    if( cmd[0] == '~' )
    {
        int len = cmd.find('/');
        if( len == -1 )
            len = cmd.length();
        if( len == 1 )
            cmd.replace ( 0, 1, QDir::homeDirPath() );
        else
        {
            QString user = cmd.mid( 1, len-1 );
            struct passwd *dir = getpwnam(user.latin1());
            if( dir && strlen(dir->pw_dir) )
                cmd.replace (0, len, dir->pw_dir);
            else
            {
                QString msg = dir ? i18n("<qt><b>%1</b> doesn't have a home directory!</qt>").arg(user) :
                                    i18n("<qt>There is no user called <b>%1</b>.</qt>").arg(user);
                setErrorMsg( data, msg );
                setURIType( data, KURIFilterData::ERROR );
                return data.hasBeenFiltered();
            }
        }
    }

    // Expand any environment variables
    expandEnivVar( cmd );

    // Now check for local resource match
    struct stat buff;

    // Determine if "uri" is an absolute path to a local resource
    if( (cmd[0] == '/') && ( stat( cmd.latin1() , &buff ) == 0 ) )
    {
        bool isDir = S_ISDIR( buff.st_mode );
        if( !isDir && access (cmd.latin1(), X_OK) == 0 )
        {
            setFilteredURI( data, cmd );
            setURIType( data, KURIFilterData::EXECUTABLE );
            return data.hasBeenFiltered();
        }
        // Open "uri" as file:/xxx if it is a non-executable local resource.
        if( isDir || S_ISREG( buff.st_mode ) )
        {
            cmd.insert( 0, "file:" );
            setFilteredURI( data, cmd );
            setURIType( data, ( isDir ) ? KURIFilterData::LOCAL_DIR : KURIFilterData::LOCAL_FILE );
            return data.hasBeenFiltered();
        }
    }

    // If "uri" is not the absolute path to a file or a directory,
    // see if it is executable under the user's $PATH variable.
    if( !KStandardDirs::findExe( cmd ).isNull() )
    {
        setFilteredURI( data, cmd );
        setURIType( data, KURIFilterData::EXECUTABLE );
        return data.hasBeenFiltered();
    }

    // If cmd is NOT a local resource, check for a valid "shortURL"
    // candidate and append "http://" as the default protocol.
    // FIXME: Make this option configurable !! (Dawit A.)
    if( (!host.isEmpty() && isValidShortURL ( cmd )) || cmd == "localhost" )
    {
        cmd.insert( 0, "http://" );
        setFilteredURI( data, cmd );
        setURIType( data, KURIFilterData::NET_PROTOCOL );
        return data.hasBeenFiltered();
    }
    // TODO: Detect executables when arguments are given
    setURIType( data, KURIFilterData::UNKNOWN );
    return data.hasBeenFiltered();
}

KCModule* KShortURIFilter::configModule( QWidget*, const char* ) const
{
	return 0; //new KShortURIOptions( parent, name );
}

QString KShortURIFilter::configName() const
{
    return i18n("&ShortURLs");
}

void KShortURIFilter::configure()
{
}

/***************************************** KShortURIFilterFactory *******************************************/

KShortURIFilterFactory::KShortURIFilterFactory( QObject *parent, const char *name )
                       :KLibFactory( parent, name )
{
    s_instance = new KInstance( "kshorturifilter" );
}

KShortURIFilterFactory::~KShortURIFilterFactory()
{
    delete s_instance;
}

QObject *KShortURIFilterFactory::create( QObject *parent, const char *name, const char*, const QStringList & )
{
    QObject *obj = new KShortURIFilter( parent, name );
    emit objectCreated( obj );
    return obj;
}

KInstance *KShortURIFilterFactory::instance()
{
    return s_instance;
}

extern "C"
{
    void *init_libkshorturifilter()
    {
        return new KShortURIFilterFactory;
    }
}

