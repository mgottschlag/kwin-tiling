/*
 * kshorturifilter.cpp
 * Copyright (c) 1999-2000 Dawit Alemayehu <adawit@earthlink.net>
 */

#include <stdlib.h>
#include <unistd.h>
#include <pwd.h>
#include <sys/stat.h>

#include <qdir.h>
#include <qlist.h>

#include <kurl.h>
#include <kprotocolmanager.h>
#include <kinstance.h>
#include <kglobal.h>
#include <kstddirs.h>

#include "kshorturifilter.h"
#include "kshorturifilter.moc"

KInstance *KShortURIFilterFactory::s_instance = 0L;

KShortURIFilter::KShortURIFilter( QObject *parent, const char *name )
                :KURIFilterPlugin( parent, name ? name : "shorturi", 1.0),
                 DCOPObject("KShortURIFilterIface")
{
    // TODO : Make this configurable.  Should go into control module...
    m_urlHints.insert("www", "http://");
    m_urlHints.insert("ftp", "ftp://");
    m_urlHints.insert("news", "news://");
}

bool KShortURIFilter::filterURI( KURIFilterData& data ) const
{
    if( data.uri().isMalformed() )
    {
        parseURI( data );
        if( data.uriType() != KURIFilterData::ERROR ||
            data.uriType() != KURIFilterData::UNKNOWN )
            return true;
    }
    return false;
}

bool KShortURIFilter::isValidShortURL( const QString& cmd ) const
{
  // NOTE : By design, this check disqualifies some valid
  // URL's that contain queries and *nix command characters.
  // This is an intentional trade off to best match the URL
  // with a local resource first.  This also allows KShortURIFilter
  // to behave consistently with the way it does now. (Dawit A.)
  // Some tests removed since this is now called only if the start of cmd qualifies for a host name (malte)

  // TODO: configurability like always treat foobar/XXX as a shortURL
  if ( QRegExp( "[ ;<>]" ).match( cmd ) >= 0
    || QRegExp( "||" ).match( cmd ) >= 0
    || QRegExp( "&&" ).match( cmd ) >= 0
    || cmd.at( cmd.length()-1 ) == '&' ) return false;

  return true;
}

void KShortURIFilter::parseURI( KURIFilterData& data ) const
{
    QString cmd = data.uri().url().lower();
    QStringList protocols = KProtocolManager::self().protocols();

    // First look if we've got a known protocol prefix
    // and assume a qualified URL
    for( QStringList::ConstIterator it = protocols.begin(); it != protocols.end(); it++ )
    {
        if( cmd.left((*it).length()).lower() == *it )
        {
            setURIType( data, KURIFilterData::NET_PROTOCOL );
            return;
        }
    }

    // See if the beginning of cmd looks like a valid FQDN
    QString host;
    if( QRegExp("[a-z][a-z0-9-]*\\.[a-z]", false).match(cmd) == 0 )
    {
        host = cmd.left(cmd.find('.'));
        // Check if it's one of the urlHints and qualify the URL
        QString proto = m_urlHints[host.lower()];
        if( proto.length() != 0 )
        {
            cmd.insert(0, proto);
            setFilteredURI( data, cmd );
            setURIType( data, KURIFilterData::NET_PROTOCOL );
            return;
        }
    }

    // Assume http if cmd starts with <IP>/
    // Will QRegExp support ([0-9]{1,3}\.){3}[0-9]{1,3} some time?
    // TODO : ADD LITERAL IPv6 support - See RFC 2373 Appendix B
    if ( QRegExp("[0-9][0-9]?[0-9]?\\.[0-9][0-9]?[0-9]?\\.[0-9][0-9]?[0-9]?\\.[0-9][0-9]?[0-9]?/").match(cmd) == 0 )
    {
        cmd.insert(0, "http://");
        setFilteredURI( data, cmd );
        setURIType( data, KURIFilterData::NET_PROTOCOL );
        return;
    }

    // Is it to be invoked with kdehelp? btw: how to invoke the man/info browser in KDE 2?
    /*
    if( cmd.left(5) == "info:" || cmd.left(4) == "man:" || cmd[0] == '#' )
    {
        if( cmd.left(2) == "##" )
            cmd = "info:" + ( cmd.length() == 2 ? QString("(dir)" ) : cmd.mid(2));
        else if ( cmd[0] == '#' )
            cmd = "man" + (cmd.length() == 1 ? QString("(index)") : cmd.mid(1));
        return KURIFilter::HELP;
    }
    */
    // Local file and directory processing.
    // TODO: also detect executables when arguments are given
    QString uri = QDir::cleanDirPath( cmd );

    // This will translate "smb:" into "smb:/"
    // for kio_smb.
    if( cmd == "smb:" )
    {
        cmd += '/';
        setFilteredURI( data, cmd );
        setURIType( data, KURIFilterData::NET_PROTOCOL );
        return;
    }

    // Strip off "file:/" in order to expand local
    // URLs if necessary.
    if( uri.lower().find("file:/") == 0 )
        uri.remove ( 0, uri[6] == '/' ? 6 : 5 );

    // HOME directory ?
    if( uri[0] == '~' )
    {
        int len = uri.find('/');
        if( len == -1 )
            len = uri.length();
        if( len == 1 )
            uri.replace ( 0, 1, QDir::homeDirPath() );
        else
        {
            QString user = uri.mid( 1, len-1 );
            struct passwd *dir = getpwnam(user.latin1());
            if( dir && strlen(dir->pw_dir) )
                uri.replace (0, len, dir->pw_dir);
            else
            {
                QString msg = dir ? i18n("%1 doesn't have a home directory!").arg(user) : i18n("There is no user called %1.").arg(user);
                setErrorMsg( data, msg );
                setURIType( data, KURIFilterData::ERROR );
                return;
            }
        }
    }

    // Now for any local resource
    struct stat buff;

    // Determine if "uri" is an absolute path to a local resource
    if( (uri[0] == '/') && ( stat( uri.latin1() , &buff ) == 0 ) )
    {
        bool isDir = S_ISDIR( buff.st_mode );
        if( isDir && access (uri.latin1(), X_OK) == 0 )
        {
            cmd = uri;
            setFilteredURI( data, cmd );
            setURIType( data, KURIFilterData::EXECUTABLE );
            return;
        }
        // Open "uri" as file:/xxx if it is a non-executable local resource.
        if( isDir || S_ISREG( buff.st_mode ) )
        {
            cmd = "file:" + uri;
            setFilteredURI( data, cmd );
            setURIType( data, ( isDir ) ? KURIFilterData::LOCAL_DIR : KURIFilterData::LOCAL_FILE );
            return;
        }
    }

    // If "uri" is not the absolute path to a file or a directory,
    // see if it is executable under the user's $PATH variable.
    if( !KStandardDirs::findExe( uri ).isNull() )
    {
        cmd = uri;
        setFilteredURI( data, cmd );
        setURIType( data, KURIFilterData::EXECUTABLE );
        return;
    }

    // If cmd is NOT a local resource check for a  valid "shortURL"
    // candidate, append "http://" as the default protocol.
    // FIXME: Make this option configurable !! (Dawit A.)
    if( (!host.isEmpty() && isValidShortURL ( cmd )) || cmd == "localhost" )
    {
        cmd.insert(0, "http://");
        setFilteredURI( data, cmd );
        setURIType( data, KURIFilterData::NET_PROTOCOL );
        return;
    }
    setURIType( data, KURIFilterData::NET_PROTOCOL );
}

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

