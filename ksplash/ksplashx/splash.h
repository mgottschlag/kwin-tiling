/********************************************************************

Copyright (C) 2007 Lubos Lunak <l.lunak@kde.org>

Please see file LICENSE for the licensing terms of ksplashx as a whole.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*********************************************************************/

#ifndef _SPLASH_H
#define _SPLASH_H

#include <stdio.h>
#include "qcolor.h"

static inline QRgb blend(const QRgb& c, const QRgb& background )
{
    if( qAlpha( c ) == 255 )
        return c;
    return qRgb( ( qRed( background ) * ( 255 - qAlpha( c ) ) + qRed( c ) * qAlpha( c ) ) / 255,
                 ( qGreen( background ) * ( 255 - qAlpha( c ) ) + qGreen( c ) * qAlpha( c ) ) / 255,
                 ( qBlue( background ) * ( 255 - qAlpha( c ) ) + qBlue( c ) * qAlpha( c ) ) / 255 );
}

void runSplash( const char* theme, bool test, int pipe );

#endif
