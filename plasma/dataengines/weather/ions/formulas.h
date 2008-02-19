/***************************************************************************
 *   Copyright (C) 2007 by Shawn Starr <shawn.starr@rogers.com>            *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA          *
 ***************************************************************************/

/* Meteorological formula class */

#ifndef _WEATHERFORMULA_H
#define _WEATHERFORMULA_H

#include "ion_export.h"

namespace WeatherFormula
{
    // Convert Temperatures, pressures
    ION_EXPORT float celsiusToF(float temperature);
    ION_EXPORT float fahrenheitToC(float temperature);
    ION_EXPORT float milesToKM(float miles);
    ION_EXPORT float kilometersToMI(float km);
    ION_EXPORT float kilopascalsToInches(float kpa);
    ION_EXPORT float inchesToKilopascals(float inches);
    ION_EXPORT float millibarsToKilopascals(float milibar);
    ION_EXPORT float millibarsToInches(float milibar);
    ION_EXPORT float centimetersToIN(float cm);
    ION_EXPORT float inchesToCM(float inch);
    ION_EXPORT float millimetersToIN(float mm);
    ION_EXPORT float inchesToMM(float inch);

    // Winds measured in meters per second
    ION_EXPORT float kilometersToMS(float km);
    ION_EXPORT float milesToMS(float miles);
    ION_EXPORT float knotsToMS(float knots);

    // Winds measured in knots
    ION_EXPORT float kilometersToKT(float km);
    ION_EXPORT float milesToKT(float miles);
    ION_EXPORT float knotsToKM(float knots);
    ION_EXPORT float knotsToMI(float knots);

    // Winds measured in beaufort scale value
    ION_EXPORT int knotsToBF(float knots);
    ION_EXPORT int milesToBF(float miles);
    ION_EXPORT int kilometersToBF(float km);

} // WeatherFormula namespace

#endif
