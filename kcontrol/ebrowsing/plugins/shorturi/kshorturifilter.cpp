/*
    kshorturifilter.h

    This file is part of the KDE project
    Copyright (C) 2000 Yves Arrouye <yves@realnames.com>
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
    // TODO: Make this configurable.  Should go into control module...
    m_urlHints.insert("www", "http://");
    m_urlHints.insert("ftp", "ftp://");
    m_urlHints.insert("news", "news://");
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

bool KShortURIFilter::filterURI( KURIFilterData& data ) const
{
    QString cmd = data.uri().url();

    // First look if we've got a known protocol prefix
    // and it is a valid URL.  If so return immediately.
    QStringList protocols = KProtocolManager::self().protocols();
    for( QStringList::ConstIterator it = protocols.begin(); it != protocols.end(); it++ )
    {
        if( (cmd.left((*it).length()).lower() == *it) && !data.uri().isMalformed() )
        {
            setURIType( data, KURIFilterData::NET_PROTOCOL );
            return data.hasBeenFiltered();
        }
    }

    // See if the beginning of cmd looks like a valid FQDN
    // QRegExp is buggy, the caseSenstive flag does not work!!
    // Worked around it by adding the signature for CAP letters
    // into QRegExp. (DA)
    QString host;
    if( QRegExp("[a-zA-Z][a-zA-Z0-9-]*\\.[a-zA-Z]").match(cmd) == 0 )
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
    if ( QRegExp("[0-9][0-9]?[0-9]?\\.[0-9][0-9]?[0-9]?\\.[0-9][0-9]?[0-9]?\\.[0-9][0-9]?[0-9]?/").match(cmd) == 0 )
    {
        cmd.insert(0, "http://");
        setFilteredURI( data, cmd );
        setURIType( data, KURIFilterData::NET_PROTOCOL );
        return data.hasBeenFiltered();
    }
/*
    // Is it to be invoked with kdehelp? btw: how to invoke the man/info browser in KDE 2?
    if( cmd.left(5) == "info:" || cmd.left(4) == "man:" || cmd[0] == '#' )
    {
        if( cmd.left(2) == "##" )
            cmd = "info:" + ( cmd.length() == 2 ? QString("(dir)" ) : cmd.mid(2));
        else if ( cmd[0] == '#' )
            cmd = "man" + (cmd.length() == 1 ? QString("(index)") : cmd.mid(1));
        setFilteredURI( data, cmd );
        setURIType( data, KURIFilterData::HELP );
        return;
    }
*/

    cmd = QDir::cleanDirPath( cmd );

    // Translate "smb:", "smb://" or anything that
    // starts with "\\" into "smb:/".
    bool smbURI = ( cmd.lower() == "smb:" || cmd.lower() == "smb:/" );
    if( smbURI || cmd.find( "\\\\" ) == 0 )
    {
        if( smbURI )
            if( cmd.left(1) != '/' ) cmd += '/';
        else
            cmd.replace( 0, 2, "smb:/" );
        setFilteredURI( data, cmd );
        setURIType( data, KURIFilterData::NET_PROTOCOL );
        return data.hasBeenFiltered();
    }

    // Local file and directory processing.

    // Strip off "file:/" in order to expand local
    // URLs if necessary.
    if( cmd.lower().find("file:/") == 0 )
        cmd.remove ( 0, cmd[6] == '/' ? 6 : 5 );

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
                QString msg = dir ? i18n("%1 doesn't have a home directory!").arg(user) : i18n("There is no user called %1.").arg(user);
                setErrorMsg( data, msg );
                setURIType( data, KURIFilterData::ERROR );
                return data.hasBeenFiltered();
            }
        }
    }

    // Now for any local resource
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

    // If cmd is NOT a local resource check for a  valid "shortURL"
    // candidate, append "http://" as the default protocol.
    // FIXME: Make this option configurable !! (Dawit A.)
    if( (!host.isEmpty() && isValidShortURL ( cmd )) || cmd == "localhost" )
    {
        cmd.insert( 0, "http://" );
        setFilteredURI( data, cmd );
        setURIType( data, KURIFilterData::NET_PROTOCOL );
        return data.hasBeenFiltered();
    }
    // TODO: Detect executables when arguments are given
    setURIType( data, KURIFilterData::SHELL );
    return data.hasBeenFiltered();
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

