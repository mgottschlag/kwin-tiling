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
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/


#ifndef KDMCONFIG_H
#define KDMCONFIG_H

#include <config.h>

#include "config.ci"

#ifdef __cplusplus

#include <QString>
#include <qstringlist.h>
#include <QFont>

extern QString _stsFile;
extern bool _isLocal;
extern bool _authorized;

CONF_GREET_CPP_DECLS

// this file happens to be included everywhere, so just put it here
struct dpySpec;
void decodeSess( dpySpec *sess, QString &user, QString &loc );

extern "C"
#endif
void init_config( void );

CONF_GREET_C_DECLS

#endif /* KDMCONFIG_H */
