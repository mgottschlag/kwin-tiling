    /*

    Config for kdm
    $Id$

    Copyright (C) 1997, 1998 Steffen Hansen
                             stefh@mip.ou.dk


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
 

#include "kdmconfig.h"
#include <qpixmap.h>
#include <kapp.h>
#include <pwd.h>
#include <sys/types.h>
#include <iostream.h>
#include <kglobal.h>
#include <kstddirs.h>
#include <kpassdlg.h>

#define defEchoMode	KPasswordEdit::OneStar

KDMConfig::KDMConfig( )
{
     getConfig();
}

void
KDMConfig::insertUsers( QIconView *iconview, QStringList s, bool sorted)
{
     kc->setGroup( QString::fromLatin1("KDM") );
     QPixmap default_pix( locate("user_pic", 
				 QString::fromLatin1("default.png")));
     if( default_pix.isNull())
	  printf("Cant get default pixmap from \"default.png\"\n");
     if( s.isEmpty()) {
          QStringList no_users = kc->readListEntry( QString::fromLatin1("NoUsers"));
          struct passwd *ps;
#define CHECK_STRING( x) (x != 0 && x[0] != 0)
          setpwent();
          for( ps = getpwent(); ps ; ) {
	       // usernames are stored in the same encoding as files
	       QString username = QFile::decodeName ( ps->pw_name );
               if( CHECK_STRING(ps->pw_dir) &&
                   CHECK_STRING(ps->pw_shell) &&
				(ps->pw_uid >= _low_user_id || ps->pw_uid == 0) &&
                   ( no_users.find( username ) == no_users.end())){
		    // we might have a real user, insert him/her
		    QPixmap p( locate("user_pic",
				      username + QString::fromLatin1(".png")));
		    if( p.isNull())
			 p = default_pix;
		    QIconViewItem *item = new QIconViewItem( iconview, 
							     username, p);
		    item->setDragEnabled(false);
	       }
	       ps = getpwent();
	  }
	  endpwent();
#undef CHECK_STRING
     } else {
          QStringList::ConstIterator it = s.begin();
          for( ; it != s.end(); ++it) {
               QPixmap p( locate("user_pic",
				 *it + QString::fromLatin1(".png")));
               if( p.isNull())
                    p = default_pix;
	       QIconViewItem *item = new QIconViewItem( iconview, 
							*it, p);
	       item->setDragEnabled(false);
          }
     }
     if( sorted) iconview->sort();
}

void KDMConfig::getConfig()
{
	_low_user_id = 0;
    kc = new KConfig( QString::fromLatin1("kdmrc") ); // kalle
    kc->setGroup( QString::fromLatin1("KDM") );
    
    // Read Entries
    QString normal_font = kc->readEntry( QString::fromLatin1("StdFont") );
    QString fail_font   = kc->readEntry( QString::fromLatin1("FailFont") );
    QString greet_font  = kc->readEntry( QString::fromLatin1("GreetFont") );

	// Read low user ID
   QString low_user_id = kc->readEntry( QString::fromLatin1("UserIDLow") );
	_low_user_id=low_user_id.toInt();

    QString greet_string = kc->readEntry( QString::fromLatin1("GreetString"));
    _sessionTypes = kc->readListEntry( QString::fromLatin1("SessionTypes"));

    QString logoArea = kc->readEntry( QString::fromLatin1("LogoArea"), 
				      QString::fromLatin1("KdmLogo") );
    _useLogo = logoArea == QString::fromLatin1("KdmLogo");

    QString logo_string = kc->readEntry( QString::fromLatin1("LogoPixmap") );
    if( kc->hasKey( QString::fromLatin1("ShutdownButton") ) ) {
	QString tmp = kc->readEntry( QString::fromLatin1("ShutdownButton") );
	if( tmp == QString::fromLatin1("All") )
	    _shutdownButton = All;
	else if( tmp == QString::fromLatin1("RootOnly") )
	    _shutdownButton = RootOnly;
	else if( tmp == QString::fromLatin1("ConsoleOnly") )
	    _shutdownButton = ConsoleOnly;
	else
	    _shutdownButton = KNone;
	_shutdown         = kc->readEntry( QString::fromLatin1("Shutdown") );
	if( _shutdown.isNull())
	    _shutdown = QString::fromLatin1(SHUTDOWN_CMD);
	_restart          = kc->readEntry( QString::fromLatin1("Restart") );
	if( _restart.isNull())
	    _restart = QString::fromLatin1(REBOOT_CMD);
    } else
	_shutdownButton   = KNone;
    
#ifndef BSD
    _consoleMode = kc->readEntry( QString::fromLatin1("ConsoleMode") );
    if (_consoleMode.isNull())
	_consoleMode = QString::fromLatin1("/sbin/init 3");
#endif
    
    /* TODO: to be ported to QStyle
       if( kc->hasKey( "GUIStyle")) {
       if( kc->readEntry( "GUIStyle") == "Windows")
       _style = Qt::WindowsStyle;
       else                        // Added this cause else users couldn't
       _style = MotifStyle;   // explicitly ask for motif-style. Th.
       } else {
       _style = MotifStyle;
       }
    */

     // Logo
     if( logo_string.isNull()) // isEmpty() ?
          _logo = locate("data", QString::fromLatin1("kdm/pics/kdelogo.png"));
     else
          _logo = logo_string;

     // Table of users
     _sorted = kc->readNumEntry( QString::fromLatin1("SortUsers"), 1);
     if( kc->hasKey( QString::fromLatin1("UserView") ) && 
	 kc->readNumEntry( QString::fromLatin1("UserView"))) {
          if( kc->hasKey( QString::fromLatin1("Users") ) ) {
               _users = 
		    kc->readListEntry( QString::fromLatin1("Users"));
          }
	  _show_users = true;
     } else {
          /* no user view */
          _show_users = false;
     }

     // Defaults for session types
     if( _sessionTypes.isEmpty() ) {
          _sessionTypes.append( QString::fromLatin1("kde") );
          _sessionTypes.append( QString::fromLatin1("failsafe") );
     }

     QString val = kc->readEntry(QString::fromLatin1("EchoMode")).lower();
     if (val == QString::fromLatin1("onestar"))
        _echoMode = KPasswordEdit::OneStar;
     else if (val == QString::fromLatin1("threestars"))
        _echoMode = KPasswordEdit::ThreeStars;
     else if ((val == QString::fromLatin1("nostars")) || 
              (val == QString::fromLatin1("noecho")))
        _echoMode = KPasswordEdit::NoEcho;
     else
        _echoMode = defEchoMode;
        
     // Greet String and fonts:
     char buf[256];
     gethostname( buf, 255);
     // Reading hostname with same encoding as filenames.
     // most likely this doesn't really matter because the standards says (?)
     // that it has to be in US-ASCII only.
     QString longhostname = QFile::decodeName(buf);
     QString hostname;
     // Remove domainname, because it's generally
     // too long to look nice in the title:
     int dot = longhostname.find('.');
     if( dot != -1) hostname = longhostname.left( dot);
     else hostname = longhostname;

     if( !normal_font.isEmpty()) { // Rettet til isEmpty. Strengen kan godt være 0-længde
                                   // selvom isNull() giver false.
          if(normal_font.contains(',')) {                           //Th.
            _normalFont = new QFont(
	      kc->readFontEntry( QString::fromLatin1("StdFont"))); //Th.
          }
          else {
            _normalFont = new QFont( normal_font);
            _normalFont->setRawMode( true);
          }
     } else
          _normalFont = new QFont;

     if( !fail_font.isEmpty()) {
          if(fail_font.contains(',')) {                             //Th.
            _failFont = new QFont(
	      kc->readFontEntry( QString::fromLatin1("FailFont")));  //Th.
          }
          else {
            _failFont = new QFont( fail_font);
            _failFont->setRawMode( true);
          }
     } else {
          _failFont = new QFont( *_normalFont);
          _failFont->setBold( true);
     }

     if( !greet_font.isEmpty()) {
          if(greet_font.contains(',')) {                             //Th.
            _greetFont = new QFont(
	      kc->readFontEntry( QString::fromLatin1("GreetFont") )); //Th.
          }
          else {
            _greetFont = new QFont( greet_font);
            _greetFont->setRawMode( true);
          }
     } else
          _greetFont = new QFont( QString::fromLatin1("times"), 24, QFont::Black);

     if( greet_string.isEmpty())
          _greetString = hostname;
     else {
          QRegExp rx( QString::fromLatin1("HOSTNAME") );
          greet_string.replace( rx, hostname);
          _greetString = greet_string;
     }

     // Lilo options
     kc->setGroup( QString::fromLatin1("Lilo") );
     _liloCmd = kc->readEntry(QString::fromLatin1("LiloCommand"),
			      QString::fromLatin1("/sbin/lilo"));
     _liloMap = kc->readEntry(QString::fromLatin1("LiloMap"),
			      QString::fromLatin1("/boot/map"));
     _useLilo = kc->readBoolEntry(QString::fromLatin1("Lilo"), FALSE);
}

KDMConfig::~KDMConfig()
{
     delete _normalFont;
     delete _failFont;
     delete _greetFont;
     delete kc;
}







