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

#include "formulas.h"
#include <math.h>
#include <KLocalizedString>

namespace WeatherFormula
{


float convert(float value, int srcUnit, int destUnit)
{
    switch (srcUnit) {
    case WeatherFormula::Celsius:
        switch (destUnit) {
        case WeatherFormula::Fahrenheit:
            return (value * 9 / 5 + 32);
        case WeatherFormula::Kelvin:
            return (value + 273.15);
        };

    case WeatherFormula::Fahrenheit:
        switch (destUnit) {
        case WeatherFormula::Celsius:
            return (value - 32) * 5 / 9;
        case WeatherFormula::Kelvin:
            return (5 / 9 * (value - 32) + 273.15);
        };

    case WeatherFormula::Kelvin:
        switch (destUnit) {
        case WeatherFormula::Celsius:
            return (value - 273.15);
        case WeatherFormula::Fahrenheit:
            return ((value - 273.15) * 1.8) + 32;
        };
  
    case WeatherFormula::Kilometers:
        switch (destUnit) {
        case WeatherFormula::Miles:
            return (0.621371192 * value);
        case WeatherFormula::MetersPerSecond:
            return (value * 0.277778);
        case WeatherFormula::Knots:
            return (value * 0.539956803);
        case WeatherFormula::Beaufort:
            return kilometersToBeaufort(value);
        };

    case WeatherFormula::MetersPerSecond:
        switch (destUnit) {
        case WeatherFormula::Miles:
            return (value * 2.23693629);
        case WeatherFormula::Kilometers:
            return (value * 3.6);
        case WeatherFormula::Knots:
            return (value * 1.943845);
        case WeatherFormula::Beaufort:
            return metersPerSecondToBeaufort(value);
        };
    
    case WeatherFormula::Miles:
        switch (destUnit) {
        case WeatherFormula::Kilometers:
            return (1.609344 * value);
        case WeatherFormula::MetersPerSecond:
            return (value * 0.44704);
        case WeatherFormula::Knots:
            return (value * 0.868976242);
        case WeatherFormula::Beaufort:
            return milesToBeaufort(value);
        };

    case WeatherFormula::Kilopascals:
        switch (destUnit) {
        case WeatherFormula::InchesHG:
            return ((0.02952997 * value) * 10);
        case WeatherFormula::Millibars:
        case WeatherFormula::Hectopascals:
            return (value / 0.10);
        };
   
    case WeatherFormula::InchesHG:
        switch (destUnit) {
        case WeatherFormula::Kilopascals:
            return (value * 3.386389);
        case WeatherFormula::Millibars:
        case WeatherFormula::Hectopascals:
            return (value * 33.8637526);
        };
    
    case WeatherFormula::Millibars:
        switch (destUnit) {
        case WeatherFormula::Kilopascals:
            return (value * 0.10);
        case WeatherFormula::InchesHG:
	    return (value * 0.0295333727);
        };

    case WeatherFormula::Centimeters:
        switch (destUnit) {
        case WeatherFormula::Millimeters:
            return (value / 0.1);
        case WeatherFormula::Inches:
            return (value * 0.393700787);
        };
    
    case WeatherFormula::Millimeters:
        switch (destUnit) {
        case WeatherFormula::Centimeters: 
            return (value * 0.1);           
        case WeatherFormula::Inches:
            return (value * 0.0393700787);
        };
   
    case WeatherFormula::Inches:
        switch (destUnit) {
        case WeatherFormula::Centimeters:
            return (value * 2.54);
        case WeatherFormula::Millimeters:
            return (value * 25.4);
        };
  
    case WeatherFormula::Knots:
        switch (destUnit) {
        case WeatherFormula::Kilometers:
            return floor(value * 1.852 + 0.5);
        case WeatherFormula::Miles:
            return (value * 1.507794);
        case WeatherFormula::MetersPerSecond:
            return (value * 1.9438);
        case WeatherFormula::Beaufort:
            return knotsToBeaufort(value);
        };
    };
   return 0;
}

QString getUnitString(int unit, bool plain)
{
    switch (unit) {
        case WeatherFormula::Celsius:
            if(plain)
                return QString("C");
            else
                return i18nc("Celsius, temperature unit", "⁰C");
                
        case WeatherFormula::Fahrenheit:
            if(plain)
                return QString("F");
            else
                return i18nc("Fahrenheit, temperature unit", "⁰F");
            
        case WeatherFormula::Kelvin:
            if(plain)
                return QString("K");
            else
                return i18nc("Kelvin, temperature unit", "K");
            
        case WeatherFormula::Kilometers:
            if(plain)
                return QString("kmh");
            else
                return i18nc("kilometers per hour, windspeed unit", "km/h");
            
        case WeatherFormula::MetersPerSecond:
            if(plain)
                return QString("ms");
            else
                return i18nc("meters per second, windspeed unit", "m/s");
            
        case WeatherFormula::Miles:
            if(plain)
                return QString("mph");
            else
                return i18nc("miles per hour, windspeed unit", "mph");
            
        case WeatherFormula::Kilopascals:
            if(plain)
                return QString("kpa");
            else
                return i18nc("kilopascals, airpressure unit", "kPa");
            
        case WeatherFormula::InchesHG:
            if(plain)
                return QString("in");
            else
                return i18nc("inches hg, airpressure unit", "inHg");
            
        case WeatherFormula::Millibars:
            if(plain)
                return QString("mbar");
            else
                return i18nc("millibars, airpressure unit", "mbar");
            
        case WeatherFormula::Hectopascals:
            if(plain)
                return QString("hpa");
            else
                return i18nc("hectopascals, airpressure unit", "hPa");
            
        case WeatherFormula::Centimeters:
            if(plain)
                return QString("cm");
            else
                return i18nc("centimeters, length unit", "cm");
            
        case WeatherFormula::Millimeters:
            if(plain)
                return QString("mm");
            else
                return i18nc("millimeters, length unit", "mm");
            
        case WeatherFormula::Inches:
            if(plain)
                return QString("in");
            else
                return i18nc("inches, length unit", "in");
            
        case WeatherFormula::Knots:
            if(plain)
                return QString("kt");
            else
                return i18nc("knots, wind speed unit", "kt");
            
        case WeatherFormula::Beaufort:
            if(plain)
                return QString("bft");
            else
                return i18nc("beaufort, wind speed unit", "Bft");
            
        default:
            return QString();
    }
}

QString windDegreesToCardinal(float degrees)
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

} // namespace WeatherFormula

