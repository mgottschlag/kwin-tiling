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
#include <klocale.h>
#include <kprotocolmanager.h>
#include <kinstance.h>
#include <kglobal.h>

#include "kshorturifilter.h"
#include "kshorturifilter.moc"

KInstance *KShortURIFilterFactory::s_instance = 0L;

KShortURIFilter::KShortURIFilter( QObject *parent, const char *name )
                :KURIFilterPlugin( parent, name ? name : "KShortURIFilter", "shorturi", 1.0),
                 DCOPObject("KShortURIFilterIface")
{
    // TODO : Make this configurable.  Should go into control module...
    m_urlHints.insert("www", "http://");
    m_urlHints.insert("ftp", "ftp://");
    m_urlHints.insert("news", "news://");
    m_pCmdType = KShortURIFilter::Unknown;
    m_strError = QString::null;
}

KShortURIFilter::~KShortURIFilter()
{
}

bool KShortURIFilter::filterURI( KURL &uri )
{
    if( uri.isMalformed() )
    {
        debug( "About to filter the URL" );
        QString url = uri.url();
        m_pCmdType = parseCmd( url );
        if( m_pCmdType != KShortURIFilter::Unknown ||
            m_pCmdType != KShortURIFilter::Error )
        {
            debug( "URL Filtered to : %s", url.latin1() );
            uri = url;
            return true;
        }
    }
    return false;
}

bool KShortURIFilter::isExecutable( const QString &name ) const
{
    QStringList path = QStringList::split( ':', getenv("PATH") );
    for( QStringList::ConstIterator it = path.begin(); it != path.end(); it++ )
    {
        if( access(*it + '/' + name, X_OK) == 0 )
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

KShortURIFilter::CmdType KShortURIFilter::parseCmd( QString& cmd )
{
    // First look if we've got a known protocol prefix
    // and assume a qualified URL
    QStringList protocols = KProtocolManager::self().protocols();
    for( QStringList::ConstIterator it = protocols.begin(); it != protocols.end(); it++ )
    {
        if( cmd.left((*it).length()).lower() == *it )
            return KShortURIFilter::URL;
    }

    // See if the beginning of cmd looks like a valid FQDN
    QString host;
    if( QRegExp("[a-z][a-z0-9-]*\\.[a-z]", false).match(cmd) == 0 )
    {
        host = cmd.left(cmd.find('.'));
        // Check if it's one of the urlHints and qualify the URL
        QString proto = m_urlHints[host.lower()];
        if( !proto.isEmpty() )
        {
            cmd.insert(0, proto);
            return KShortURIFilter::URL;
        }
    }

    // Assume http if cmd starts with <IP>/
    // Will QRegExp support ([0-9]{1,3}\.){3}[0-9]{1,3} some time?
    if ( QRegExp("[0-9][0-9]?[0-9]?\\.[0-9][0-9]?[0-9]?\\.[0-9][0-9]?[0-9]?\\.[0-9][0-9]?[0-9]?/").match(cmd) == 0 )
    {
        cmd.insert(0, "http://");
        return KShortURIFilter::URL;
    }

    // Is it to be invoked with kdehelp? btw: how to invoke the man/info browser in KDE 2?
    /*
    if( cmd.left(5) == "info:" || cmd.left(4) == "man:" || cmd[0] == '#' )
    {
        if( cmd.left(2) == "##" )
            cmd = "info:" + ( cmd.length() == 2 ? QString("(dir)" ) : cmd.mid(2));
        else if ( cmd[0] == '#' )
            cmd = "man" + (cmd.length() == 1 ? QString("(index)") : cmd.mid(1));
        return KShortURIFilter::Help;
    }
    */
    
    // Local file and directory processing.
    // TODO: also detect executables when arguments are given
    QString uri = QDir::cleanDirPath( cmd );

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
                m_strError = dir ? i18n("%1 doesn't have a home directory!").arg(user) : i18n("There is no user called %1.").arg(user);
                return KShortURIFilter::Error;
            }
        }
    }

    // Now for any local resource
    struct stat buff;

    // Determine if "uri" is an absolute path to a local resource
    if( (uri[0] == '/') && ( stat( uri.latin1() , &buff ) == 0 ) )
    {
        if( !S_ISDIR(buff.st_mode) && access (uri.latin1(), X_OK) == 0 )
        {
            cmd = uri;
            return KShortURIFilter::Executable;
        }
        // Open "uri" as file:/xxx if it is a non-executable local resource.
        if( S_ISDIR( buff.st_mode ) || S_ISREG( buff.st_mode ) )
        {
            cmd = "file:" + uri;
            return KShortURIFilter::URL;
        }
    }

    // If "uri" is not the absolute path to a file or a directory,
    // see if it is executable under the user's $PATH variable.
    if( isExecutable ( uri ) )
    {
        cmd = uri;
        return KShortURIFilter::Executable;
    }

    // If cmd is NOT a local resource check for a  valid "shortURL"
    // candidate, append "http://" as the default protocol.
    // FIXME: Make this option configurable !! (Dawit A.)
    if( (!host.isEmpty() && isValidShortURL ( cmd )) || cmd == "localhost" )
    {
        cmd.insert(0, "http://");
        return KShortURIFilter::URL;
    }
    return KShortURIFilter::Shell;
}

KCModule *KShortURIFilter::configModule(QWidget *parent, const char *name) const
{
    return 0; // TODO : Pending control-panel module.
}

QString KShortURIFilter::configName() const
{
    return i18n("&Short URI");
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

QObject *KShortURIFilterFactory::create( QObject *parent, const char *, const char*, const QStringList & )
{
    QObject *obj = new KShortURIFilter( parent );
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

