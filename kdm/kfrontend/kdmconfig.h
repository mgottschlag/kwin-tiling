    /*

    Configuration for kdm

    Copyright (C) 1997, 1998, 2000 Steffen Hansen <hansen@kde.org>
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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    */


#ifndef KDMCONFIG_H
#define KDMCONFIG_H

#include <config.h>

#include "kdm_config.h"

#ifdef __cplusplus

#include <qstring.h>
#include <qstringlist.h>
#include <qfont.h>

extern QString	_stsFile;
extern bool	_isLocal;

extern QString	_GUIStyle;
extern QString	_colorScheme;

extern QFont	_normalFont;
extern QFont	_failFont;
extern QFont	_greetFont;

extern int	_logoArea;
extern QString	_logo;
extern QString	_greetString;
extern bool	_greeterPosFixed;
extern int	_greeterPosX, _greeterPosY;
extern int	_greeterScreen;

extern bool	_userCompletion;
extern bool	_userList;
extern int	_showUsers;
extern int	_preselUser;
extern QString	_defaultUser;
extern bool	_focusPasswd;
extern bool	_sortUsers;
extern char	**_users;
extern char	**_noUsers;
extern int	_lowUserId, _highUserId;
extern int	_showRoot;
extern int	_faceSource;
extern QString	_faceDir;
extern int	_echoMode;

extern char	**_sessionsDirs;

extern int	_allowShutdown, _allowNuke, _defSdMode;
extern bool	_interactiveSd;

extern int	_numLockStatus;

#if defined(__linux__) && ( defined(__i386__)  || defined(__amd64__) )
extern bool	_useLilo;
extern QString	_liloCmd;
extern QString	_liloMap;
#endif

#ifdef XDMCP
extern int	_loginMode;
#endif

extern int	_forgingSeed;

#ifdef WITH_KDM_XCONSOLE
extern bool	_showLog;
extern char	*_logSource;
#endif

extern bool	_allowClose;

extern QStringList	_pluginsLogin;
extern QStringList	_pluginsShutdown;
extern QStringList	_pluginOptions;

extern bool	_useBackground;
extern char	*_backgroundCfg;

extern bool	_hasConsole;

extern "C"
#endif
void init_config( void );

extern int	_pingInterval;
extern int	_pingTimeout;

extern int	_grabServer;
extern int	_grabTimeout;

extern int	_antiAliasing;

extern char 	*_language;

#endif /* KDMCONFIG_H */
