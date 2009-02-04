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
 * The WeatherUtils namespace contains helper functions for weather applets and ions.
 * It includes functions for converting temperatures, speeds, distances and pressures between different units and also allows one to convert degrees to a cardinal.
 *
 * For an example an ion developer is required to specify the formats used by their datasource. For this the developer has to use enums provided for different kinds of units.
 *
 * For applet developers this namespace offers both the enums and functions to convert the data to the format user requests. To achieve this functions convertTemperature(), convertSpeed(), convertDistance() and convertPressure() can be used. For example converting from m/s to kmh can be done like this:
 * float speedInKmh = WeatherUtils::convertSpeed( speedInMeters, WeatherUtils::MetersPerSecond, KilometersPerHour );
*/
namespace WeatherUtils
{

enum TemperatureUnit { NoUnit = 0, DegreeUnit, Celsius, Fahrenheit, Kelvin };
enum SpeedUnit { KilometersPerHour = 100, MetersPerSecond, MilesPerHour, Knots, Beaufort };
enum DistanceUnit { Centimeters = 200, Millimeters, Inches, Kilometers, Miles };
enum PressureUnit { Kilopascals = 300, InchesHG, Millibars, Hectopascals }; // FIXME deprecate millibars?

enum Errors { InvalidConversion = -100, NoSuchUnit = -110 };

/**
 * Convert between temperature units. See WeatherUtils::TemperatureUnit for available units.
 * @param value value to convert
 * @param srcUnit the source unit
 * @param destUnit the destination unit
 * @return converted value or WeatherUtils::InvalidConversion when trying to convert between invalid units
*/
ION_EXPORT float convertTemperature(float value, int srcUnit, int destUnit);

/**
 * Convert between pressure units. See WeatherUtils::PressureUnit for available units.
 * @param value value to convert
 * @param srcUnit the source unit
 * @param destUnit the destination unit
 * @return converted value or WeatherUtils::InvalidConversion when trying to convert between invalid units
*/
ION_EXPORT float convertPressure(float value, int srcUnit, int destUnit);

/**
 * Convert between distance units. See WeatherUtils::DistanceUnit for available units.
 * @param value value to convert
 * @param srcUnit the source unit
 * @param destUnit the destination unit
 * @return converted value or WeatherUtils::InvalidConversion when trying to convert between invalid units
*/
ION_EXPORT float convertDistance(float value, int srcUnit, int destUnit);

/**
 * Convert between speed units. See WeatherUtils::SpeedUnit for available units.
 * @param value value to convert
 * @param srcUnit the source unit
 * @param destUnit the destination unit
 * @return converted value or WeatherUtils::InvalidConversion when trying to convert between invalid units
*/
ION_EXPORT float convertSpeed(float value, int srcUnit, int destUnit);

/**
 * Convert from unit to another. See WeatherUtils::Unit for available units.
 * @param value float to convert
 * @param srcUnit from which unit to convert
 * @param destUnit to which unit to convert
 * @return converted value or WeatherUtils::NoSuchUnit when trying to convert from an unknown unit
 * @deprecated Use convertTemperature(), convertDistance(), convertPressure() or convertSpeed() instead.
*/
ION_EXPORT float convert(float value, int srcUnit, int destUnit);

/**
* Returns a string presentation of of WeatherUtils::Unit. Set plain to true in case you don't want a localized version of it.
 * @param unit unit to convert.
 * @param plain if true, returned string is not localized. defaults to false.
 * @return a string presentation of WeatherUtils::Unit. Empty string if called for invalid unit.
*/
ION_EXPORT QString getUnitString(int unit, bool plain = false);

/**
 * Converts wind/sun direction given in degrees to the nearest cardinal direction.
 * @param degrees wind direction in degrees.
 * @return a cardinal if available, empty string on error.
*/
ION_EXPORT QString degreesToCardinal(float degrees);

/**
 * @internal
*/
int knotsToBeaufort(float knots);
int milesToBeaufort(float miles);
int kilometersToBeaufort(float km);
int metersPerSecondToBeaufort(float ms);

} // WeatherUtils namespace

#endif
