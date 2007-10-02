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

#include <math.h>
#include "weather_formula.h"

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
