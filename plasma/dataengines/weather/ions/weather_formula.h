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

#include <kdemacros.h>

class KDE_EXPORT WeatherFormula
{
public:
    WeatherFormula();
    ~WeatherFormula();
    // Convert Temperatures, pressures
    float celsiusToF(float temperature) const;
    float fahrenheitToC(float temperature) const;
    float milesToKM(float miles) const;
    float kilometersToMI(float km) const;
    float kilopascalsToInches(float kpa) const;
    float inchesToKilopascals(float inches) const;
    float millibarsToKilopascals(float milibar) const;
    float millibarsToInches(float milibar) const;
    float centimetersToIN(float cm) const;
    float inchesToCM(float inch) const;
    float millimetersToIN(float mm) const;
    float inchesToMM(float inch) const;

    // Winds measured in meters per second
    float kilometersToMS(float km) const;
    float milesToMS(float miles) const;
    float knotsToMS(float knots) const;

    // Winds measured in knots
    float kilometersToKT(float km) const;
    float milesToKT(float miles) const;
    float knotsToKM(float knots) const;
    float knotsToMI(float knots) const;

    // Winds measured in beaufort scale value
    int knotsToBF(float knots) const;
    int milesToBF(float miles) const;
    int kilometersToBF(float km) const;
};

#endif
