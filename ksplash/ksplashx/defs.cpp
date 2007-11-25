/*
 *   Copyright (C) 2007 Luboš Luňák <l.lunak@kde.org>
 *
 *   This program is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU General Public
 *   License as published by the Free Software Foundation; either
 *   version 2 of the License, or (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

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
