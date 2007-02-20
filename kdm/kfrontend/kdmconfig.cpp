/*

Config for kdm

Copyright (C) 1997, 1998 Steffen Hansen <hansen@kde.org>
Copyright (C) 2000-2003 Oswald Buddenhagen <ossi@kde.org>


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
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/

#include "kdmconfig.h"
#include "kdm_greet.h"

#include <kconfig.h>
#include <kglobal.h>
#include <klocale.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/utsname.h>

CONF_GREET_DEFS

QString _stsFile;
bool _isLocal;
bool _authorized;

static QString
GetCfgQStr( int id )
{
	char *tmp = GetCfgStr( id );
	QString qs = QString::fromUtf8( tmp );
	free( tmp );
	return qs;
}

static QStringList
GetCfgQStrList( int id )
{
	int i, len;
	char **tmp = GetCfgStrArr( id, &len );
	QStringList qsl;
	for (i = 0; i < len - 1; i++) {
		qsl.append( QString::fromUtf8( tmp[i] ) );
		free( tmp[i] );
	}
	free( tmp );
	return qsl;
}

// Based on kconfigbase.cpp
static QFont *
Str2Font( const QString &aValue )
{
	uint nFontBits;
	QString chStr;
	QFont *aRetFont = new QFont();

	QStringList sl = aValue.split( QString::fromLatin1(","), QString::SkipEmptyParts );

	if (sl.count() == 1) {
		/* X11 font spec */
		aRetFont->setRawMode( true );
		aRetFont->setRawName( aValue );
	} else if (sl.count() == 10) {
		/* qt3 font spec */
		aRetFont->fromString( aValue );
	} else if (sl.count() == 6) {
		/* backward compatible kde2 font spec */
		aRetFont->setFamily( sl[0] );
		aRetFont->setPointSize( sl[1].toInt() );
		aRetFont->setWeight( sl[4].toUInt() );

		aRetFont->setStyleHint( (QFont::StyleHint)sl[2].toUInt() );

		nFontBits = sl[5].toUInt();
		aRetFont->setItalic( (nFontBits & 0x01) != 0 );
		aRetFont->setUnderline( (nFontBits & 0x02) != 0 );
		aRetFont->setStrikeOut( (nFontBits & 0x04) != 0 );
		aRetFont->setFixedPitch( (nFontBits & 0x08) != 0 );
		aRetFont->setRawMode( (nFontBits & 0x20) != 0 );
	}
	aRetFont->setStyleStrategy( (QFont::StyleStrategy)
	   (QFont::PreferMatch |
	    (_antiAliasing ? QFont::PreferAntialias : QFont::NoAntialias)) );

	return aRetFont;
}

extern "C"
void init_config( void )
{
	CONF_GREET_INIT

	_isLocal = GetCfgInt( C_isLocal );
	_hasConsole = _hasConsole && _isLocal && GetCfgInt( C_hasConsole );
	_authorized = GetCfgInt( C_isAuthorized );

	_stsFile = _dataDir + "/kdmsts";

	// Greet String
	char hostname[256], *ptr;
	hostname[0] = '\0';
	if (!gethostname( hostname, sizeof(hostname) ))
		hostname[sizeof(hostname)-1] = '\0';
	struct utsname tuname;
	uname( &tuname );
	QString gst = _greetString;
	_greetString.clear();
	int i, j, l = gst.length();
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
			default: _greetString += i18n("[fix kdmrc]"); continue;
			}
			_greetString += QString::fromLocal8Bit( ptr );
		} else
			_greetString += gst[i];
	}
}

void init_config_qapp( void )
{
	CONF_GREET_INIT_QAPP

	KConfigGroup cfg(KGlobal::config(), "General");
	cfg.writeEntry( "nopaletteChange", true );
	cfg.writeEntry( "font", *_normalFont );
	if (!_GUIStyle.isEmpty())
		cfg.writeEntry( "widgetStyle", _GUIStyle );
}

