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
#include <kprotocolinfo.h>
#include <kinstance.h>
#include <kglobal.h>
#include <kstddirs.h>

//#include "kshorturiopts.h"
#include "kshorturifilter.h"
#include "kshorturifilter.moc"

#define FQDN_PATTERN    "[a-zA-Z][a-zA-Z0-9-]*\\.[a-zA-Z]"
#define IPv4_PATTERN    "[0-9][0-9]?[0-9]?\\.[0-9][0-9]?[0-9]?\\.[0-9][0-9]?[0-9]?\\.[0-9][0-9]?[0-9]?:[[0-9][0-9]?[0-9]?]?/?"

#define ENV_VAR_PATTERN "$[a-zA-Z_][a-zA-Z0-9_]*"
#define QFL1(x) QString::fromLatin1(x)

KInstance *KShortURIFilterFactory::s_instance = 0;

KShortURIFilter::KShortURIFilter( QObject *parent, const char *name )
                :KURIFilterPlugin( parent, name ? name : "shorturi", 1.0),
                 DCOPObject("KShortURIFilterIface")
{
    // TODO: Make this configurable.  Should go into control module...
    // Note: the order is important (FQDN_PATTERN should be last)
    m_urlHints.append(URLHint(QFL1("www"), QFL1("http://")));
    m_urlHints.append(URLHint(QFL1("ftp"), QFL1("ftp://")));
    m_urlHints.append(URLHint(QFL1("news"), QFL1("news://")));
    m_urlHints.append(URLHint(QFL1(IPv4_PATTERN), QFL1("http://")));
    m_urlHints.append(URLHint(QFL1(FQDN_PATTERN), QFL1("http://")));
    m_strDefaultProtocol = QFL1("http://");
}

bool KShortURIFilter::isValidShortURL( const QString& cmd ) const
{
  // Loose many of the QRegExp matches as they tend
  // to slow things down.  They are also unnecessary!! (DA)
  if ( cmd.find( QFL1("||") ) >= 0 || cmd.find( QFL1("&&") ) >= 0 ||
       cmd.at( cmd.length()-1 ) == '&' || cmd.find( '.') == -1 ||
       QRegExp( QFL1("[ ;<>]") ).match( cmd ) >= 0 )
       return false;

  return true;
}

bool KShortURIFilter::expandEnvVar( QString& cmd ) const
{
  // ENVIRONMENT variable expansion
  int env_len = 0;
  int env_loc = 0;
  while( 1 )
  {
    env_loc = QRegExp( QFL1(ENV_VAR_PATTERN) ).match( cmd, env_loc, &env_len );
    if( env_loc == -1 ) break;
    const char* exp = getenv( cmd.mid( env_loc + 1, env_len - 1 ).local8Bit().data() );
    cmd.replace( env_loc, env_len, QString::fromLocal8Bit(exp) );
  }
  return ( env_len ) ? true : false;
}

bool KShortURIFilter::filterURI( KURIFilterData& data ) const
{
  KURL url = data.uri();
  QString cmd = url.url();

 /*
  * Here is a description of how the shortURI deals with the supplied
  * data.  First it expands any environment variable settings and then
  * deals with special shortURI cases. These special cases are the "smb:"
  * URL scheme which is very specific to KDE, "#" and "##" which are
  * shortcuts for man:/ and info:/ protocols respectively abd local files.
  * Then it checks to see if URL is valid and one that is supported by KDE's
  * IO system.  If all the above check fails, it simply lookups the URL in
  * the user-defined list and returns without filtering if it is not found.
  * In the future versions, we might want to make it so that everything
  * with the exception of ENV expansion is configurable by the user.
  */
  if ( expandEnvVar( cmd ) )
    url = cmd;  // Update the url

  // Handle SMB Protocol shortcuts ...
  int loc = cmd.lower().find( QFL1("smb:") );
  if (  loc == 0 || cmd.find( QFL1("\\\\") ) == 0 )
  {
    if( loc == 0 )
      cmd = QDir::cleanDirPath( cmd.mid( 4 ) );
    else
    {
      loc = 0;
      while( cmd[loc] == '\\' ) loc++;
      cmd = cmd.mid( loc );
    }

    for (uint i=0; i < cmd.length(); i++)
    {
      if (cmd[i]=='\\')
        cmd[i]='/';
    }
    cmd[0] == '/' ? cmd.prepend( QFL1("smb:") ) : cmd.prepend( QFL1("smb:/") );
    setFilteredURI( data, cmd );
    setURIType( data, KURIFilterData::NET_PROTOCOL );
    return data.hasBeenFiltered();
  }

  // Handle MAN & INFO pages shortcuts...
  if( cmd[0] == '#' ||
      cmd.find( QFL1("man:"), 0, true ) == 0 ||
      cmd.find( QFL1("info:"), 0, true ) == 0 )
  {
    if( cmd.left(2) == QFL1("##") )
      cmd = QFL1("info:/") + ( cmd.length() == 2 ? QFL1("dir") : cmd.mid(2));
    else if ( cmd[0] == '#' )
      cmd = QFL1("man:/") + cmd.mid(1);
    else if ( cmd.lower() == QFL1( "man:" ) )
      cmd += '/';
    else if ( cmd.lower() == QFL1( "info:" ) )
      cmd += QFL1( "/dir" );
    setFilteredURI( data, cmd );
    setURIType( data, KURIFilterData::HELP );
    return data.hasBeenFiltered();
  }

  // Handle all LOCAL URLs cases...
  // Clean up the URL. That is remove "file:" and
  // any excess slashes from it.
  if( !url.isMalformed() && url.isLocalFile() )
  {
    bool hasEndSlash = ( cmd[cmd.length()-1] == '/' );
    cmd = QDir::cleanDirPath( url.path() );
    if ( hasEndSlash && cmd.right(1) != QFL1("/") )
      cmd += '/';
    url = cmd;      // update the URL...
  }

  // Expanding shortcut to HOME URL...
  if( cmd[0] == '~' )
  {
    int len = cmd.find('/');
    if( len == -1 )
      len = cmd.length();
    if( len == 1 )
    {
      cmd.replace ( 0, 1, QDir::homeDirPath() );
      url = cmd;
    }
    else
    {
      QString user = cmd.mid( 1, len-1 );
      struct passwd *dir = getpwnam(user.local8Bit().data());
      if( dir && strlen(dir->pw_dir) )
      {
        cmd.replace (0, len, QString::fromLocal8Bit(dir->pw_dir));
        url = cmd;          // update the URL...
      }
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

  // Checking for local resource match...
  struct stat buff;
  // Determine if "uri" is an absolute path to a local resource
  if( (cmd[0] == '/') && ( stat( cmd.local8Bit().data() , &buff ) == 0 ) )
  {
    bool isDir = S_ISDIR( buff.st_mode );
    if( !isDir && access (cmd.local8Bit().data(), X_OK) == 0 )
    {
      setFilteredURI( data, cmd );
      setURIType( data, KURIFilterData::EXECUTABLE );
      return data.hasBeenFiltered();
    }
    // Open "uri" as file:/xxx if it is a non-executable local resource.
    if( isDir || S_ISREG( buff.st_mode ) )
    {
      cmd.insert( 0, QFL1("file:") );
      setFilteredURI( data, cmd );
      setURIType( data, ( isDir ) ? KURIFilterData::LOCAL_DIR : KURIFilterData::LOCAL_FILE );
      return data.hasBeenFiltered();
    }
  }

  // If "uri" is not the absolute path to a file or
  // a directory, see if it is executable under the
  // user's $PATH variable.
  if( !KStandardDirs::findExe( cmd ).isNull() )
  {
    setFilteredURI( data, cmd );
    setURIType( data, KURIFilterData::EXECUTABLE );
    return data.hasBeenFiltered();
  }

  // Process URLs of known and supported protocols so we don't have
  // to resort to the pattern matching scheme below which can possibly
  // be slow things down...
  QStringList protocols = KProtocolInfo::protocols();
  for( QStringList::ConstIterator it = protocols.begin(); it != protocols.end(); it++ )
  {
    if( (cmd.left((*it).length()).lower() == *it) &&
        !url.isMalformed() && !url.isLocalFile() )
    {
      setFilteredURI( data, cmd );
      if ( *it == QFL1("man") || *it == QFL1("help") )
        setURIType( data, KURIFilterData::HELP );
      else
        setURIType( data, KURIFilterData::NET_PROTOCOL );
      return data.hasBeenFiltered();
    }
  }

  // Okay this is the code that allows users to supply custom
  // matches for specific URLs using Qt's regexp class.  This
  // is hard-coded for now in the constructor, but will soon be
  // moved to the config dialog so that people can configure this
  // stuff.  This is perhaps one of those unecessary but somewhat
  // useful features that usually makes people go WHOO and WHAAA.
  QRegExp match;
  QValueList<URLHint>::ConstIterator it;
  for( it = m_urlHints.begin(); it != m_urlHints.end(); ++it )
  {
    int len = 0;      // Future use for allowing replacement
    match = (*it).regexp;
    //kdDebug() << "KShortURIFilter::filterURI match=" << (*it).regexp << endl;
    //kdDebug() << "KShortURIFilter::filterURI match returned " << match.match( cmd, 0, &len ) << endl;
    if( match.match( cmd, 0, &len ) == 0 )
    {
      cmd.prepend( (*it).prepend );
      setFilteredURI( data, cmd );
      setURIType( data, KURIFilterData::NET_PROTOCOL );
      return data.hasBeenFiltered();
    }
  }

  // If cmd is NOT a local resource, check if it
  // is a valid "shortURL" candidate and append
  // the default protocol the user supplied. (DA)
  if( !url.isLocalFile() && isValidShortURL( cmd ) )
  {
    cmd.insert( 0, m_strDefaultProtocol );
    setFilteredURI( data, cmd );
    setURIType( data, KURIFilterData::NET_PROTOCOL );
    return data.hasBeenFiltered();
  }

  // If we reach this point, we cannot filter
  // this thing so simply return the default
  // value of the filter data object which is
  // false...
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

