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

void calc(const QDateTime &dt, double longitude, double latitude,
          double zone, double *jd, double *century, double *eqTime,
          double *solarDec, double *azimuth, double *zenith);
double radToDeg(double angleRad);
double degToRad(double angleDeg);
double calcJD(double year, double month, double day);
QDateTime calcDateFromJD(double jd, double minutes, double zone);
double calcTimeJulianCent(double jd);
double calcJDFromJulianCent(double t);
double calcGeomMeanLongSun(double t);
double calcGeomMeanAnomalySun(double t);
double calcEccentricityEarthOrbit(double t);
double calcSunEqOfCenter(double t);
double calcSunTrueLong(double t);
double calcSunTrueAnomaly(double t);
double calcSunRadVector(double t);
double calcSunApparentLong(double t);
double calcMeanObliquityOfEcliptic(double t);
double calcObliquityCorrection(double t);
double calcSunRtAscension(double t);
double calcSunDeclination(double t);
double calcEquationOfTime(double t);
void   calcAzimuthAndZenith(QDateTime now, double eqTime, double zone,
                            double solarDec, double latitude, double longitude,
                            double *zenith, double *azimuth);
double calcElevation(double zenith);
double calcHourAngle(double zenith, double solarDec, double latitude);
void   calcTimeUTC(double zenith, bool rise, double *jd, double *minutes,
                   double latitude, double longitude);
double calcSolNoonUTC(double t, double longitude);
}


#endif
