/***************************************************************************
 *   Copyright (C) 2007, 2008 by Shawn Starr <shawn.starr@rogers.com>      *
 *   Copyright (C) 2008 by Teemu Rytilahti <tpr@d5k.net>                   *
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

#include "weatherutils.h"
#include <math.h>
#include <KLocalizedString>

namespace WeatherUtils
{


float convert(float value, int srcUnit, int destUnit)
{
    switch (srcUnit) {
    case WeatherUtils::Celsius:
        switch (destUnit) {
        case WeatherUtils::Fahrenheit:
            return (value * 9 / 5 + 32);
        case WeatherUtils::Kelvin:
            return (value + 273.15);
        };

    case WeatherUtils::Fahrenheit:
        switch (destUnit) {
        case WeatherUtils::Celsius:
            return (value - 32) * 5 / 9;
        case WeatherUtils::Kelvin:
            return (5 / 9 * (value - 32) + 273.15);
        };

    case WeatherUtils::Kelvin:
        switch (destUnit) {
        case WeatherUtils::Celsius:
            return (value - 273.15);
        case WeatherUtils::Fahrenheit:
            return ((value - 273.15) * 1.8) + 32;
        };
  
    case WeatherUtils::Kilometers:
        switch (destUnit) {
        case WeatherUtils::Miles:
            return (0.621371192 * value);
        case WeatherUtils::MetersPerSecond:
            return (value * 0.277778);
        case WeatherUtils::Knots:
            return (value * 0.539956803);
        case WeatherUtils::Beaufort:
            return kilometersToBeaufort(value);
        };

    case WeatherUtils::MetersPerSecond:
        switch (destUnit) {
        case WeatherUtils::Miles:
            return (value * 2.23693629);
        case WeatherUtils::Kilometers:
            return (value * 3.6);
        case WeatherUtils::Knots:
            return (value * 1.943845);
        case WeatherUtils::Beaufort:
            return metersPerSecondToBeaufort(value);
        };
    
    case WeatherUtils::Miles:
        switch (destUnit) {
        case WeatherUtils::Kilometers:
            return (1.609344 * value);
        case WeatherUtils::MetersPerSecond:
            return (value * 0.44704);
        case WeatherUtils::Knots:
            return (value * 0.868976242);
        case WeatherUtils::Beaufort:
            return milesToBeaufort(value);
        };

    case WeatherUtils::Kilopascals:
        switch (destUnit) {
        case WeatherUtils::InchesHG:
            return ((0.02952997 * value) * 10);
        case WeatherUtils::Millibars:
        case WeatherUtils::Hectopascals:
            return (value / 0.10);
        };
   
    case WeatherUtils::InchesHG:
        switch (destUnit) {
        case WeatherUtils::Kilopascals:
            return (value * 3.386389);
        case WeatherUtils::Millibars:
        case WeatherUtils::Hectopascals:
            return (value * 33.8637526);
        };
    
    case WeatherUtils::Millibars:
        switch (destUnit) {
        case WeatherUtils::Kilopascals:
            return (value * 0.10);
        case WeatherUtils::InchesHG:
	    return (value * 0.0295333727);
        };

    case WeatherUtils::Centimeters:
        switch (destUnit) {
        case WeatherUtils::Millimeters:
            return (value / 0.1);
        case WeatherUtils::Inches:
            return (value * 0.393700787);
        };
    
    case WeatherUtils::Millimeters:
        switch (destUnit) {
        case WeatherUtils::Centimeters: 
            return (value * 0.1);           
        case WeatherUtils::Inches:
            return (value * 0.0393700787);
        };
   
    case WeatherUtils::Inches:
        switch (destUnit) {
        case WeatherUtils::Centimeters:
            return (value * 2.54);
        case WeatherUtils::Millimeters:
            return (value * 25.4);
        };
  
    case WeatherUtils::Knots:
        switch (destUnit) {
        case WeatherUtils::Kilometers:
            return floor(value * 1.852 + 0.5);
        case WeatherUtils::Miles:
            return (value * 1.507794);
        case WeatherUtils::MetersPerSecond:
            return (value * 1.9438);
        case WeatherUtils::Beaufort:
            return knotsToBeaufort(value);
        };
    };
   return 0;
}

QString getUnitString(int unit, bool plain)
{
    switch (unit) {
        case WeatherUtils::Celsius:
            if(plain)
                return QString("C");
            else
                return i18nc("Celsius, temperature unit", "⁰C");
                
        case WeatherUtils::Fahrenheit:
            if(plain)
                return QString("F");
            else
                return i18nc("Fahrenheit, temperature unit", "⁰F");
            
        case WeatherUtils::Kelvin:
            if(plain)
                return QString("K");
            else
                return i18nc("Kelvin, temperature unit", "K");
            
        case WeatherUtils::Kilometers:
            if(plain)
                return QString("kmh");
            else
                return i18nc("kilometers per hour, windspeed unit", "km/h");
            
        case WeatherUtils::MetersPerSecond:
            if(plain)
                return QString("ms");
            else
                return i18nc("meters per second, windspeed unit", "m/s");
            
        case WeatherUtils::Miles:
            if(plain)
                return QString("mph");
            else
                return i18nc("miles per hour, windspeed unit", "mph");
            
        case WeatherUtils::Kilopascals:
            if(plain)
                return QString("kpa");
            else
                return i18nc("kilopascals, airpressure unit", "kPa");
            
        case WeatherUtils::InchesHG:
            if(plain)
                return QString("in");
            else
                return i18nc("inches hg, airpressure unit", "inHg");
            
        case WeatherUtils::Millibars:
            if(plain)
                return QString("mbar");
            else
                return i18nc("millibars, airpressure unit", "mbar");
            
        case WeatherUtils::Hectopascals:
            if(plain)
                return QString("hpa");
            else
                return i18nc("hectopascals, airpressure unit", "hPa");
            
        case WeatherUtils::Centimeters:
            if(plain)
                return QString("cm");
            else
                return i18nc("centimeters, length unit", "cm");
            
        case WeatherUtils::Millimeters:
            if(plain)
                return QString("mm");
            else
                return i18nc("millimeters, length unit", "mm");
            
        case WeatherUtils::Inches:
            if(plain)
                return QString("in");
            else
                return i18nc("inches, length unit", "in");
            
        case WeatherUtils::Knots:
            if(plain)
                return QString("kt");
            else
                return i18nc("knots, wind speed unit", "kt");
            
        case WeatherUtils::Beaufort:
            if(plain)
                return QString("bft");
            else
                return i18nc("beaufort, wind speed unit", "Bft");
            
        default:
            return QString();
    }
}

QString degreesToCardinal(float degrees)
{
    QString direction;
    if((degrees >= 348.75 && degrees <= 360) || (degrees > 0 && degrees <= 11.25))
        direction = "N";
    else if(degrees >= 11.25 && degrees < 33.75)
        direction = "NNE";
    else if(degrees >= 33.75 && degrees < 56.25)
        direction = "NE";
    else if(degrees >= 56.25 && degrees < 78.75)
        direction = "ENE";
    else if(degrees >= 78.75 && degrees < 101.25)
        direction = "E";
    else if(degrees >= 101.25 && degrees < 123.75)
        direction = "ESE";
    else if(degrees >= 123.75 && degrees < 146.25)
        direction = "SE";
    else if(degrees >= 146.25 && degrees < 168.75)
        direction = "SSE";
    else if(degrees >= 168.75 && degrees < 191.25)
        direction = "S";
    else if(degrees >= 191.25 && degrees < 213.75) 
        direction = "SSW";
    else if(degrees >= 213.75 && degrees < 236.25)
        direction = "SW";
    else if(degrees >= 236.25 && degrees < 258.75) 
        direction = "WSW";
    else if(degrees >= 258.75 && degrees < 281.25)
        direction = "W";
    else if(degrees >= 281.25 && degrees < 303.75) 
        direction = "WNW";
    else if(degrees >= 303.75 && degrees < 326.25)
        direction = "NW";
    else if(degrees >= 326.25 && degrees < 248.75) 
        direction = "NNW";
    
    if(!direction.isEmpty())
        return direction;
    else
        return QString();
}


int knotsToBeaufort(float knots)
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

int milesToBeaufort(float miles)
{
    return knotsToBeaufort(miles / 1.1507794);
}

int kilometersToBeaufort(float km)
{
    return knotsToBeaufort(km / 1.852);
}

int metersPerSecondToBeaufort(float ms)
{
    return knotsToBeaufort(ms * 1.943845);
}

} // namespace WeatherUtils

