    /*

    Configuration for kdm. Class KDMConfig
    $Id$

    Copyright (C) 1997, 1998, 2000 Steffen Hansen <hansen@kde.org>
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
 

#ifndef KDMCONFIG_H
#define KDMCONFIG_H

#include "kdm-config.h"

#include <sys/param.h>	// for BSD
#include <unistd.h>

#include <qstring.h>
#include <qstrlist.h>
//#include <qregexp.h>
#include <qfont.h>
//#include <qcolor.h>
//#include <qfile.h>
//#include <qiconview.h>

#include <ksimpleconfig.h>

#include <qnamespace.h>

class KDMConfig : public KSimpleConfig {

public:
    KDMConfig();
    ~KDMConfig();

    QFont	*_normalFont;
    QFont	*_failFont;
    QFont	*_greetFont;

    // GUIStyle	_style;
    bool	_useLogo;
    QString	_logo;
    QString	_greetString;

    enum	SuModes { UsrNone, UsrSel, UsrAll };
    SuModes	_showUsers;
    bool	_showPrevious;
    bool	_sortUsers;
    QStringList	_users;
    QStringList	_noUsers;
    int		_lowUserId;
    int		_echoMode;
     
    QStringList	_sessionTypes;

    bool	_autoReLogin;
    bool	_autoLogin1st;
    QString	_autoUser;
    QStringList	_noPassUsers;

    enum	SdModes { SdNone, SdAll, SdRootOnly, SdConsoleOnly };
    SdModes	_shutdownButton;
    QString	_shutdown;
    QString	_restart;
#ifndef BSD
    QString	_consoleMode;
#endif

    bool	_useChooser;

    bool	_useLilo;
    QString	_liloCmd;
    QString	_liloMap;
};

#endif /* KDMCONFIG_H */
