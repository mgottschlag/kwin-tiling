    /*

    Config for kdm
    $Id$

    Copyright (C) 1997, 1998 Steffen Hansen <hansen@kde.org>
    Copyright (C) 2000-2002 Oswald Buddenhagen <ossi@kde.org>


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
#include <stdlib.h>
#include <unistd.h>
#include <sys/utsname.h>

//#include <qfile.h>
#include <qmotifstyle.h>
#include <qmotifplusstyle.h>
#include <qcdestyle.h>
#include <qsgistyle.h>
#include <qwindowsstyle.h>
#include <qplatinumstyle.h>
#include <qtextcodec.h>	//XXX
#include <qsettings.h> 

#include <kapplication.h>
#include <kglobal.h>
#include <klocale.h>
#include <kcharsets.h>
#include <kstandarddirs.h>

#include "kdmconfig.h"
#include "kdm_greet.h"

KDMConfig *kdmcfg = 0;

QString GetCfgQStr (int id)
{
    char *tmp = GetCfgStr (id);
    QString qs = QString::fromUtf8 (tmp);
    free (tmp);
    return qs;
}

QStringList GetCfgQStrList (int id)
{
    int i, len;
    char **tmp = GetCfgStrArr (id, &len);
    QStringList qsl;
    for (i = 0; i < len - 1; i++) {
	qsl.append (QString::fromUtf8 (tmp[i]));
	free (tmp[i]);
    }
    free (tmp);
    return qsl;
}

// Based on kconfigbase.cpp
QFont KDMConfig::Str2Font (QString aValue)
{
    uint nFontBits;
    QFont aRetFont;
    QString chStr;

    QStringList sl = QStringList::split (QString::fromLatin1(","), aValue);

    if (sl.count() == 1) {
	/* X11 font spec */
	aRetFont = QFont (aValue);
	aRetFont.setRawMode (true);
    } else if (sl.count() == 10) {
	/* qt3 font spec */
	aRetFont.fromString( aValue );
    } else if (sl.count() == 6) {
	/* backward compatible kde2 font spec */
	aRetFont = QFont (sl[0], sl[1].toInt(), sl[4].toUInt() );

	aRetFont.setStyleHint( (QFont::StyleHint)sl[2].toUInt() );

	nFontBits = sl[5].toUInt();
	aRetFont.setItalic( nFontBits & 0x01 != 0);
	aRetFont.setUnderline( nFontBits & 0x02 != 0 );
	aRetFont.setStrikeOut( nFontBits & 0x04 != 0 );
	aRetFont.setFixedPitch( nFontBits & 0x08 != 0 );
	aRetFont.setRawMode( nFontBits & 0x20 != 0 );

#if QT_VERSION < 300
	QFont::CharSet chId = (QFont::CharSet)sl[3].toUInt(&chOldEntry);
	if (chOldEntry)
	    aRetFont.setCharSet( chId );
	else if (kapp) {
	    if (sl[3] == QString::fromLatin1("default"))
		chStr = KGlobal::locale() ? 
			KGlobal::locale()->charset() : 
			"iso-8859-1";
	    else
		chStr = sl[3];
	    KGlobal::charsets()->setQFont(aRetFont, chStr);
	}
#endif
    }

    return aRetFont;
}

/*
QColor KDMConfig::Str2Color (QString aValue)
{
    QColor aRetColor;
    QStringList sl;

    if ( aValue.at(0) == '#' )
	aRetColor.setNamedColor(aValue);
    else {
	sl = QStringList::split (QString::fromLatin1(","), aValue);
	if (sl.count() == 3)
	    aRetColor.setRgb( sl[0].toInt(), sl[1].toInt(), sl[2].toInt() );
    }
    return aRetColor;
}
*/

KDMConfig::KDMConfig()
{
    KGlobal::locale()->setLanguage (GetCfgQStr (C_Language));
    qApp->setDefaultCodec(QTextCodec::codecForName(KGlobal::locale()->language().latin1()));

    _allowShutdown = GetCfgInt (C_allowShutdown);
    _allowNuke = GetCfgInt (C_allowNuke);
    _defSdMode = GetCfgInt (C_defSdMode);
    _interactiveSd = GetCfgInt (C_interactiveSd);

    if (GetCfgInt (C_GreeterPosFixed)) {
	_greeterPosX = GetCfgInt (C_GreeterPosX);
	_greeterPosY = GetCfgInt (C_GreeterPosY);
    } else
	_greeterPosX = -1;
    _greeterScreen = GetCfgInt (C_GreeterScreen);

    kapp->setStyle (GetCfgStr (C_GUIStyle));

    _logoArea = GetCfgInt (C_LogoArea);

    _logo = GetCfgQStr (C_LogoPixmap);
    if( _logo.isEmpty())
	_logo = locate("data", QString::fromLatin1("kdm/pics/kdelogo.png") );

    _showUsers = GetCfgInt (C_ShowUsers);
    _users = GetCfgQStrList (C_SelectedUsers);
    _noUsers = GetCfgQStrList (C_HiddenUsers);
    _lowUserId = GetCfgInt (C_MinShowUID);
    _highUserId = GetCfgInt (C_MaxShowUID);
    _sortUsers = GetCfgInt (C_SortUsers);

    _sessionTypes = GetCfgQStrList (C_SessionTypes);

    _echoMode = GetCfgInt (C_EchoMode);

    _normalFont = Str2Font (GetCfgQStr (C_StdFont));
    _failFont = Str2Font (GetCfgQStr (C_FailFont));
    _greetFont = Str2Font (GetCfgQStr (C_GreetFont));

    QSettings().writeEntry("/qt/useXft", (bool) GetCfgInt (C_AntiAliasing));

    // Greet String
    char hostname[256], *ptr;
    gethostname (hostname, 255);
    struct utsname tuname;
    uname (&tuname);
    QString gst = GetCfgQStr (C_GreetString);
    int i, j, l = gst.length ();
    for (i = 0; i < l; i++) {
	if (gst[i] == '%') {
	    switch (gst[++i].cell()) {
		case '%': _greetString += gst[i]; continue;
		case 'd': ptr = dname; break;
		case 'h': ptr = hostname; break;
		case 'n': ptr = tuname.nodename;
		    for (j = 0; ptr[j]; j++)
			if (ptr[j] == '.') {
			    ptr[j] = 0;
			    break;
			}
		    break;
		case 's': ptr = tuname.sysname; break;
		case 'r': ptr = tuname.release; break;
		case 'm': ptr = tuname.machine; break;
		default: _greetString += i18n ("[fix kdmrc!]"); continue;
	    }
	    _greetString += QString::fromLocal8Bit (ptr);
	} else
	    _greetString += gst[i];
    }

    _preselUser = GetCfgInt (C_PreselectUser);
    _defaultUser = GetCfgQStr (C_DefaultUser);
    _focusPasswd = GetCfgInt (C_FocusPasswd);

    _numLockStatus = GetCfgInt (C_NumLock);

#if defined(__linux__) && defined(__i386__)
    if ((_useLilo = GetCfgInt (C_useLilo))) {
	_liloCmd = GetCfgQStr (C_liloCmd);
	_liloMap = GetCfgQStr (C_liloMap);
    }
#endif
}
