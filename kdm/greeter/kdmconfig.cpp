    /*

    Config for kdm
    $Id$

    Copyright (C) 1997, 1998 Steffen Hansen <hansen@kde.org>
    Copyright (C) 2000 Oswald Buddenhagen <ossi@kde.org>


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
 

#include <stdio.h>
#include <unistd.h>
#include "kdmconfig.h"
//#include <qpixmap.h>
#include <qfile.h>
/*#include <kapp.h>
#include <pwd.h>
#include <sys/types.h>
#include <iostream.h>
*/
#include <kglobal.h>
#include <kstddirs.h>
#include <kpassdlg.h>

#define defEchoMode	KPasswordEdit::OneStar

#define defShowMode	UsrAll

KDMConfig::KDMConfig() :
    KSimpleConfig(*KGlobal::dirs()->resourceDirs("config").begin() + 
		  QString::fromLatin1("kdmrc"))
{
    setGroup( "KDM" );

    QString tmp = readEntry( "ShutdownButton" );
    if( tmp == QString::fromLatin1("All") )
	_shutdownButton = SdAll;
    else if( tmp == QString::fromLatin1("RootOnly") )
	_shutdownButton = SdRootOnly;
    else if( tmp == QString::fromLatin1("ConsoleOnly") )
	_shutdownButton = SdConsoleOnly;
    else
	_shutdownButton = SdNone;

    _shutdown = readEntry( "Shutdown", QString::fromLatin1(SHUTDOWN_CMD) );
    _restart = readEntry( "Restart", QString::fromLatin1(REBOOT_CMD) );
#ifndef BSD
    if (readBoolEntry("AllowConsoleMode", true))
	_consoleMode = readEntry( "ConsoleMode", 
				  QString::fromLatin1("/sbin/init 3") );
#endif

    _useChooser = readBoolEntry("UseChooser", false);

    /* TODO: to be ported to QStyle
    if( readEntry( "GUIStyle") == "Windows")
	_style = Qt::WindowsStyle;
    else
	_style = MotifStyle;
    */

    QString logoArea = readEntry( "LogoArea", QString::fromLatin1("KdmLogo") );
    _useLogo = logoArea == QString::fromLatin1( "KdmLogo");

    QString logo_string = readEntry( "LogoPixmap" );
    if( logo_string.isEmpty())
	_logo = locate("data", QString::fromLatin1("kdm/pics/kdelogo.png") );
    else
	_logo = logo_string;

    QString uv = readEntry( "ShowUsers");
    if( uv == QString::fromLatin1("All") )
	_showUsers = UsrAll;
    else if( uv == QString::fromLatin1("Selected") )
	_showUsers = UsrSel;
    else if( uv == QString::fromLatin1("None") )
	_showUsers = UsrNone;
    else
	_showUsers = defShowMode;
    _users = readListEntry( "Users");
    _noUsers = readListEntry( "NoUsers");
    _lowUserId = readNumEntry( "UserIDLow" );
    _sortUsers = readBoolEntry("SortUsers", true);

    _sessionTypes = readListEntry( "SessionTypes" );
     // Defaults for session types
    if( _sessionTypes.isEmpty() ) {
	_sessionTypes.append( QString::fromLatin1("kde") );
	_sessionTypes.append( QString::fromLatin1("failsafe") );
    }

    QString val = readEntry("EchoMode").lower();
    if (val == QString::fromLatin1("OneStar"))
	_echoMode = KPasswordEdit::OneStar;
    else if (val == QString::fromLatin1("ThreeStars"))
	_echoMode = KPasswordEdit::ThreeStars;
    else if ((val == QString::fromLatin1("NoStars")) || 
	     (val == QString::fromLatin1("NoEcho")))
	_echoMode = KPasswordEdit::NoEcho;
    else
	_echoMode = defEchoMode;

    QString normal_font = readEntry( "StdFont" );
    if( !normal_font.isEmpty()) {
	if(normal_font.contains(','))
	    _normalFont = new QFont( readFontEntry( "StdFont"));
	else {
	    _normalFont = new QFont( normal_font);
            _normalFont->setRawMode( true);
	}
    } else
	_normalFont = new QFont;

    QString fail_font = readEntry( "FailFont" );
    if( !fail_font.isEmpty()) {
	if(fail_font.contains(','))
	    _failFont = new QFont( readFontEntry( "FailFont"));
	else {
	    _failFont = new QFont( fail_font);
	    _failFont->setRawMode( true);
	}
    } else {
	_failFont = new QFont( *_normalFont);
	_failFont->setBold( true);
    }

    QString greet_font = readEntry( "GreetFont" );
    if( !greet_font.isEmpty()) {
	if(greet_font.contains(','))
	    _greetFont = new QFont( readFontEntry( "GreetFont"));
	else {
	    _greetFont = new QFont( greet_font);
	    _greetFont->setRawMode( true);
	}
    } else
	_greetFont = new QFont( QString::fromLatin1("times"), 24, QFont::Black);

    // Greet String
    char buf[256];
    gethostname( buf, 255);
    // Reading hostname with same encoding as filenames.
    // most likely this doesn't really matter because the standard says (?)
    // that it has to be in US-ASCII only.
    QString longhostname = QFile::decodeName(buf);
    QString hostname;
    // Remove domainname, because it's generally
    // too long to look nice in the title:
    int dot = longhostname.find('.');
    if( dot != -1)
	hostname = longhostname.left( dot);
    else
	hostname = longhostname;
    QString greet_string = readEntry( "GreetString", QString::fromLatin1("KDE System at [HOSTNAME]") );
    QRegExp rx( QString::fromLatin1("HOSTNAME") );
    greet_string.replace( rx, hostname);
    _greetString = greet_string;

    if (readBoolEntry( "AutoLoginEnable", false))
	_autoUser = readEntry( "AutoLoginUser" );
    _autoLogin1st = readBoolEntry( "AutoLogin1st", true);

    if (readBoolEntry( "NoPassEnable", false))
	_noPassUsers = readListEntry( "NoPassUsers" );

    _autoReLogin = readBoolEntry( "AutoReLogin", false);
    _showPrevious = readBoolEntry( "ShowPrevious", false);

    // Lilo options
    setGroup( QString::fromLatin1("Lilo") );

    _liloCmd = readEntry("LiloCommand", QString::fromLatin1("/sbin/lilo"));
    _liloMap = readEntry("LiloMap", QString::fromLatin1("/boot/map"));
    _useLilo = readBoolEntry("Lilo", false);

    setGroup( QString::fromLatin1("Previous") );	// for lastUser entries
}

KDMConfig::~KDMConfig(void)
{
    delete _normalFont;
    delete _failFont;
    delete _greetFont;
}
