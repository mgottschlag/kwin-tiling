/*****************************************************************************
 * Copyright (C) 2007-2009 by Shawn Starr <shawn.starr@rogers.com>           *
 * Copyright (C) 2008 by Teemu Rytilahti <tpr@d5k.net>                       *
 *                                                                           *
 * This library is free software; you can redistribute it and/or             *
 * modify it under the terms of the GNU Library General Public               *
 * License as published by the Free Software Foundation; either              *
 * version 2 of the License, or (at your option) any later version.          *
 *                                                                           *
 * This library is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU         *
 * Library General Public License for more details.                          *
 *                                                                           *
 * You should have received a copy of the GNU Library General Public License *
 * along with this library; see the file COPYING.LIB.  If not, write to      *
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,      *
 * Boston, MA 02110-1301, USA.                                               *
 *****************************************************************************/

/* Meteorological formula class */

#ifndef _WEATHERUTILS_H
#define _WEATHERUTILS_H

#include "ion_export.h"

/**
 * The WeatherUtils namespace contains helper functions for weather ions.
*/
namespace WeatherUtils
{

/**
 * Converts wind/sun direction given in degrees to the nearest cardinal direction.
 * @param degrees wind direction in degrees.
 * @return a cardinal if available, empty string on error.
*/
ION_EXPORT QString degreesToCardinal(float degrees);

} // WeatherUtils namespace

#endif
