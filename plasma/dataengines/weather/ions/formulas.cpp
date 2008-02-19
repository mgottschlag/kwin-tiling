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

#include "formulas.h"
#include <math.h>

namespace WeatherFormula
{

float celsiusToF(float temperature)
{
    return (temperature * 9 / 5 + 32);
}

float fahrenheitToC(float temperature)
{
    return (temperature - 32) * 5 / 9;
}

float milesToKM(float miles)
{
    return (1.609344 * miles);
}

float kilometersToMI(float km)
{
    return (0.621371192 * km);
}

float kilopascalsToInches(float kpa)
{
    return ((0.02952997 * kpa) * 10);
}

float inchesToKilopascals(float inches)
{
    return (inches * 3.386389);
}

float millibarsToInches(float millibar)
{
    return (millibar * 0.0295301);
}

float millibarsToKilopascals(float millibar)
{
    return (millibar * 0.10);
}

float centimetersToIN(float cm)
{
    return (cm * 0.393700787);
}

float inchesToCM(float inch)
{
    return (inch * 2.54);
}

float millimetersToIN(float mm)
{
    return (mm * 0.0393700787);
}

float inchesToMM(float inch)
{
    return (inch * 25.4);
}

float kilometersToMS(float km)
{
    return (km * 0.277778);
}

float milesToMS(float miles)
{
    return (miles * 0.44704);
}

float knotsToMS(float knots)
{
    return (knots * 1.9438);
}

float knotsToKM(float knots)
{
    return floor(knots * 1.852 + 0.5);
}

float kilometersToKT(float km)
{
    return (km * 0.539956803);
}

float milesToKT(float miles)
{
    return (miles * 0.868976242);
}
float knotsToMI(float knots)
{
    return (knots * 1.507794);
}

int knotsToBF(float knots)
{
    if (knots < 1) {
        return 0;
    } else if (knots >= 1 && knots < 4) {
        return 1;
    } else if (knots >= 4 && knots < 7) {
        return 2;
    } else if (knots >= 7 && knots < 11) {
        return 3;
    } else if (knots >= 11 && knots < 16) {
        return 4;
    } else if (knots >= 16 && knots < 22) {
        return 5;
    } else if (knots >= 22 && knots < 28) {
        return 6;
    } else if (knots >= 28 && knots < 34) {
        return 7;
    } else if (knots >= 34 && knots < 41) {
        return 8;
    } else if (knots >= 41 && knots < 48) {
        return 9;
    } else if (knots >= 48 && knots < 56) {
        return 10;
    } else if (knots >= 56 && knots < 64) {
        return 11;
    } else {
        return 12;
    }
}

int milesToBF(float miles)
{
    return knotsToBF(miles / 1.1507794);
}

int kilometersToBF(float km)
{
    return knotsToBF(km / 1.852);
}

} // namespace WeatherFormula

