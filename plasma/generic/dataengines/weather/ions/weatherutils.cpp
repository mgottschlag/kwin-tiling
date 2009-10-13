/*****************************************************************************
 * Copyright (C) 2007-2009 by Shawn Starr <shawn.starr@rogers.com>           *
 * Copyright (C) 2008-2009 by Teemu Rytilahti <tpr@d5k.net>                  *
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

#include "weatherutils.h"
#include <QString>

namespace WeatherUtils
{

QString degreesToCardinal(float degrees)
{
    QString direction;
    if ((degrees >= 348.75 && degrees <= 360) || (degrees > 0 && degrees <= 11.25))
        direction = 'N';
    else if (degrees >= 11.25 && degrees < 33.75)
        direction = "NNE";
    else if (degrees >= 33.75 && degrees < 56.25)
        direction = "NE";
    else if (degrees >= 56.25 && degrees < 78.75)
        direction = "ENE";
    else if (degrees >= 78.75 && degrees < 101.25)
        direction = 'E';
    else if (degrees >= 101.25 && degrees < 123.75)
        direction = "ESE";
    else if (degrees >= 123.75 && degrees < 146.25)
        direction = "SE";
    else if (degrees >= 146.25 && degrees < 168.75)
        direction = "SSE";
    else if (degrees >= 168.75 && degrees < 191.25)
        direction = 'S';
    else if (degrees >= 191.25 && degrees < 213.75)
        direction = "SSW";
    else if (degrees >= 213.75 && degrees < 236.25)
        direction = "SW";
    else if (degrees >= 236.25 && degrees < 258.75)
        direction = "WSW";
    else if (degrees >= 258.75 && degrees < 281.25)
        direction = 'W';
    else if (degrees >= 281.25 && degrees < 303.75)
        direction = "WNW";
    else if (degrees >= 303.75 && degrees < 326.25)
        direction = "NW";
    else if (degrees >= 326.25 && degrees < 248.75)
        direction = "NNW";

    if (!direction.isEmpty())
        return direction;
    else
        return QString();
}

} // namespace WeatherUtils

