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

#include "weather_formula.h"
#include <math.h>

WeatherFormula::WeatherFormula()
{
}

WeatherFormula::~WeatherFormula()
{
}

float WeatherFormula::celsiusToF(float temperature) const
{
    return (temperature * 9 / 5 + 32);
}

float WeatherFormula::fahrenheitToC(float temperature) const
{
    return (temperature - 32) * 5 / 9;
}

float WeatherFormula::milesToKM(float miles) const
{
    return (1.609344 * miles);
}

float WeatherFormula::kilometersToMI(float km) const
{
    return (0.621371192 * km);
}

float WeatherFormula::kilopascalsToInches(float kpa) const
{
    return ((0.02952997 * kpa) * 10);
}

float WeatherFormula::inchesToKilopascals(float inches) const
{
    return (inches * 3.386389);
}

float WeatherFormula::millibarsToInches(float millibar) const
{
    return (millibar * 0.0295301);
}

float WeatherFormula::millibarsToKilopascals(float millibar) const
{
    return (millibar * 0.10);
}

float WeatherFormula::centimetersToIN(float cm) const
{
    return (cm * 0.393700787);
}

float WeatherFormula::inchesToCM(float inch) const
{
    return (inch * 2.54);
}

float WeatherFormula::millimetersToIN(float mm) const
{
    return (mm * 0.0393700787);
}

float WeatherFormula::inchesToMM(float inch) const
{
    return (inch * 25.4);
}

float WeatherFormula::kilometersToMS(float km) const
{
    return (km * 0.277778);
}

float WeatherFormula::milesToMS(float miles) const
{
    return (miles * 0.44704);
}

float WeatherFormula::knotsToMS(float knots) const
{
    return (knots * 1.9438);
}

float WeatherFormula::knotsToKM(float knots) const
{
    return floor(knots * 1.852 + 0.5);
}

float WeatherFormula::kilometersToKT(float km) const
{
    return (km * 0.539956803);
}

float WeatherFormula::milesToKT(float miles) const
{
    return (miles * 0.868976242);
}
float WeatherFormula::knotsToMI(float knots) const
{
    return (knots * 1.507794);
}

int WeatherFormula::knotsToBF(float knots) const
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

int WeatherFormula::milesToBF(float miles) const
{
    return knotsToBF(miles / 1.1507794);
}

int WeatherFormula::kilometersToBF(float km) const
{
    return knotsToBF(km / 1.852);
}

