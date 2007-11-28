/****************************************************************************
**
** This file is based on sources of the Qt GUI Toolkit, used under the terms
** of the GNU General Public License version 2 (see the original copyright
** notice below).
** All further contributions to this file are (and are required to be)
** licensed under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** The original Qt license header follows:
**
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "defs.h"
#include <string.h>

bool freadline( char* buf, int bufsize, FILE* datafile )
    {
    if( fgets( buf, bufsize, datafile ) == 0 )
        {
        buf[ 0 ] = '\0';
        return false;
        }
    char* nl = strchr( buf, '\n' );
    if( nl != NULL )
        *nl = '\0';
    return true;
    }

void strip_whitespace( char* line )
    {
    char* dst = line;
    char* src = line;
    while( *src == ' ' )
        ++src;
    while( ( *dst++ = *src++ ) != '\0' )
        ;
    --dst;
    while( dst >= line && *dst == ' ' )
        *dst-- = '\0';
    }

bool begins_with( const char* line, const char* str )
    {
    int len = strlen( str );
    return strncmp( line, str, len );
    }
