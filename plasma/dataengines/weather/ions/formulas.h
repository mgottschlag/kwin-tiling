/***************************************************************************
 *   Copyright (C) 2007, 2008 by Shawn Starr <shawn.starr@rogers.com>      *
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
    
    // Enumerations for unit types
    enum Unit { NoUnit = 0, Celsius, Fahrenheit, Kelvin, Kilometers, MetersPerSecond, Miles, Kilopascals, 
                InchesHG, Millibars, Hectopascals, Centimeters, Millimeters, Inches, 
                Knots, Beaufort };

    // Convert Units
    ION_EXPORT float convert(float value, int srcUnit, int destUnit);
    ION_EXPORT QString getUnitString(int unit, bool plain=false);
    
    int knotsToBeaufort(float knots);
    int milesToBeaufort(float miles);
    int kilometersToBeaufort(float km);
    int metersPerSecondToBeaufort(float ms);

} // WeatherFormula namespace

#endif
