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
QFont KDMConfig::Str2Font (const QString &aValue)
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
    }

    return aRetFont;
}

// XXX gallium, fix this :)
QPalette KDMConfig::Str2Palette (const QString &aValue)
{
    QStringList list = KGlobal::dirs()->findAllResources("data",
            "kdisplay/color-schemes/*.kcsrc", false, true);
    QStringList::ConstIterator it;

    KSimpleConfig *config;
    for (it = list.begin(); it != list.end(); it++) {
       config = new KSimpleConfig(*it, true);
       config->setGroup("Color Scheme");
       QString str = config->readEntry("Name");
       if (str.isEmpty()) {
          str =  config->readEntry("name");
          if (str.isEmpty())
             continue;
       }
       if (str == aValue)
	  goto haveit;

       delete config;
    }
    return kapp->palette();

  haveit:
    QColor kde2Blue;
    if (QPixmap::defaultDepth() > 8)
      kde2Blue.setRgb(10, 95, 137);
    else
      kde2Blue.setRgb(0, 0, 192);

    QColor widget(220, 220, 220);

    QColor _button;
    if (QPixmap::defaultDepth() > 8)
      _button.setRgb(228, 228, 228);
    else
      _button.setRgb(220, 220, 220);

    QColor background = config->readColorEntry("background", &widget);
    QColor foreground = config->readColorEntry("foreground", &Qt::black);
    QColor button = config->readColorEntry("buttonBackground", &_button);
    QColor buttonText = config->readColorEntry("buttonForeground", &Qt::black);
    QColor highlight = config->readColorEntry("selectBackground", &kde2Blue);
    QColor highlightedText = config->readColorEntry("selectForeground", &Qt::white);
    QColor base = config->readColorEntry("windowBackground", &Qt::white);
    QColor baseText = config->readColorEntry("windowForeground", &Qt::black);

    int contrast = config->readNumEntry("contrast", 7);

    delete config;


    // The following code was more or less stolen from KApplication
    // TODO: Default colors

    int highlightVal, lowlightVal;
    highlightVal = 100 + (2*contrast+4)*16/10;
    lowlightVal = 100 + (2*contrast+4)*10;

    QColor disfg = foreground;

    int h, s, v;
    disfg.hsv( &h, &s, &v );
    if (v > 128)
        // dark bg, light fg - need a darker disabled fg
        disfg = disfg.dark(lowlightVal);
    else if (disfg != Qt::black)
        // light bg, dark fg - need a lighter disabled fg - but only if !black
        disfg = disfg.light(highlightVal);
    else
        // black fg - use darkgrey disabled fg
        disfg = Qt::darkGray;

    QColorGroup disabledgrp(disfg, background,
                            background.light(highlightVal),
                            background.dark(lowlightVal),
                            background.dark(120),
                            background.dark(120), base);

    QColorGroup colgrp(foreground, background, background.light(highlightVal),
                       background.dark(lowlightVal),
                       background.dark(120),
                       baseText, base);

    int inlowlightVal = lowlightVal-25;
    if (inlowlightVal < 120) inlowlightVal = 120;

    colgrp.setColor(QColorGroup::Highlight, highlight);
    colgrp.setColor(QColorGroup::HighlightedText, highlightedText);
    colgrp.setColor(QColorGroup::Button, button);
    colgrp.setColor(QColorGroup::ButtonText, buttonText);
    colgrp.setColor(QColorGroup::Midlight, background.light(110));

    disabledgrp.setColor(QColorGroup::Button, button);
    disabledgrp.setColor(QColorGroup::ButtonText, buttonText);
    disabledgrp.setColor(QColorGroup::Midlight, background.light(110));

    return QPalette(colgrp, disabledgrp, colgrp);
}


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
    kapp->setPalette (Str2Palette (GetCfgStr (C_ColorScheme)));

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
    _showRoot = GetCfgInt (C_allowRootLogin);
    _faceSource = GetCfgInt (C_FaceSource);

    _sessionTypes = GetCfgQStrList (C_SessionTypes);

    _echoMode = GetCfgInt (C_EchoMode);

    _normalFont = Str2Font (GetCfgQStr (C_StdFont));
    _failFont = Str2Font (GetCfgQStr (C_FailFont));
    _greetFont = Str2Font (GetCfgQStr (C_GreetFont));

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
