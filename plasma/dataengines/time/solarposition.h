/*
 *   Copyright (C) 2009 Petri Damsten <damu@iki.fi>
 *
 *   This program is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU General Public License as
 *   published by the Free Software Foundation; either version 2 of
 *   the License, or (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SOLARPOSITION_H
#define SOLARPOSITION_H

#include "time_solar_export.h"

#include <Plasma/DataEngine>
/*
 *   These function were ported from public domain javascript code:
 *   http://www.srrb.noaa.gov/highlights/solarrise/azel.html
 *   http://www.srrb.noaa.gov/highlights/sunrise/sunrise.html
 *   Calculation details:
 *   http://www.srrb.noaa.gov/highlights/sunrise/calcdetails.html
 */

namespace NOAASolarCalc
{

TIME_SOLAR_EXPORT void calc(const QDateTime &dt, double longitude, double latitude,
                           double zone, double *jd, double *century, double *eqTime,
                           double *solarDec, double *azimuth, double *zenith);
TIME_SOLAR_EXPORT double radToDeg(double angleRad);
TIME_SOLAR_EXPORT double degToRad(double angleDeg);
TIME_SOLAR_EXPORT double calcJD(double year, double month, double day);
TIME_SOLAR_EXPORT QDateTime calcDateFromJD(double jd, double minutes, double zone);
TIME_SOLAR_EXPORT double calcTimeJulianCent(double jd);
TIME_SOLAR_EXPORT double calcJDFromJulianCent(double t);
TIME_SOLAR_EXPORT double calcGeomMeanLongSun(double t);
TIME_SOLAR_EXPORT double calcGeomMeanAnomalySun(double t);
TIME_SOLAR_EXPORT double calcEccentricityEarthOrbit(double t);
TIME_SOLAR_EXPORT double calcSunEqOfCenter(double t);
TIME_SOLAR_EXPORT double calcSunTrueLong(double t);
TIME_SOLAR_EXPORT double calcSunTrueAnomaly(double t);
TIME_SOLAR_EXPORT double calcSunRadVector(double t);
TIME_SOLAR_EXPORT double calcSunApparentLong(double t);
TIME_SOLAR_EXPORT double calcMeanObliquityOfEcliptic(double t);
TIME_SOLAR_EXPORT double calcObliquityCorrection(double t);
TIME_SOLAR_EXPORT double calcSunRtAscension(double t);
TIME_SOLAR_EXPORT double calcSunDeclination(double t);
TIME_SOLAR_EXPORT double calcEquationOfTime(double t);
TIME_SOLAR_EXPORT void   calcAzimuthAndZenith(QDateTime now, double eqTime, double zone,
                                             double solarDec, double latitude, double longitude,
                                             double *zenith, double *azimuth);
TIME_SOLAR_EXPORT double calcElevation(double zenith);
TIME_SOLAR_EXPORT double calcHourAngle(double zenith, double solarDec, double latitude);
TIME_SOLAR_EXPORT void   calcTimeUTC(double zenith, bool rise, double *jd, double *minutes,
                                    double latitude, double longitude);
TIME_SOLAR_EXPORT double calcSolNoonUTC(double t, double longitude);
}


#endif
