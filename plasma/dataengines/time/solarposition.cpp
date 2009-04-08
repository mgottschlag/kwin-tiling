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

#include "solarposition.h"
#include <math.h>
#include <QDateTime>

/*
 *   This class is ported from public domain javascript code:
 *   http://www.srrb.noaa.gov/highlights/solarrise/azel.html
 *   http://www.srrb.noaa.gov/highlights/sunrise/sunrise.html
 *   Calculation details:
 *   http://www.srrb.noaa.gov/highlights/sunrise/calcdetails.html
 */

class NOAASolarCalc
{
    public:
        static void   calc(const QDateTime &dt, double longitude, double latitude,
                           double zone, double *jd, double *century, double *eqTime,
                           double *solarDec, double *azimuth, double *zenith);
        static double radToDeg(double angleRad);
        static double degToRad(double angleDeg);
        static double calcJD(double year, double month, double day);
        static QDateTime calcDateFromJD(double jd, double minutes, double zone);
        static double calcTimeJulianCent(double jd);
        static double calcJDFromJulianCent(double t);
        static double calcGeomMeanLongSun(double t);
        static double calcGeomMeanAnomalySun(double t);
        static double calcEccentricityEarthOrbit(double t);
        static double calcSunEqOfCenter(double t);
        static double calcSunTrueLong(double t);
        static double calcSunTrueAnomaly(double t);
        static double calcSunRadVector(double t);
        static double calcSunApparentLong(double t);
        static double calcMeanObliquityOfEcliptic(double t);
        static double calcObliquityCorrection(double t);
        static double calcSunRtAscension(double t);
        static double calcSunDeclination(double t);
        static double calcEquationOfTime(double t);
        static void   calcAzimuthAndZenith(QDateTime now, double eqTime, double zone,
                                           double solarDec, double latitude, double longitude,
                                           double *zenith, double *azimuth);
        static double calcElevation(double zenith);
        static double calcHourAngle(double zenith, double solarDec, double latitude);
        static void   calcTimeUTC(double zenith, bool rise, double *jd, double *minutes,
                                  double latitude, double longitude);
        static double calcSolNoonUTC(double t, double longitude);
};

SolarPosition::SolarPosition()
{
}

SolarPosition::~SolarPosition()
{
}

void SolarPosition::appendData(Plasma::DataEngine::Data &data)
{
    //QTime time;
    //time.start();
    double longitude = data["Longitude"].toDouble();
    double latitude = data["Latitude"].toDouble();
    double zone = data["Offset"].toDouble() / -3600.0;
    QDateTime dt(data["Date"].toDate(), data["Time"].toTime());

    double jd;
    double century;
    double eqTime;
    double solarDec;
    double azimuth;
    double zenith;
    
    NOAASolarCalc::calc(dt, longitude, latitude, zone, &jd, &century, &eqTime,
                        &solarDec, &azimuth, &zenith);
    data["Equation of Time"] = eqTime;
    data["Solar Declination"] = solarDec;
    data["Azimuth"] = azimuth;
    data["Zenith"] = zenith;
    data["Corrected Elevation"] = NOAASolarCalc::calcElevation(zenith);

    const QString cacheKey = QString("%1|%2|%3")
            .arg(latitude).arg(longitude).arg(dt.date().toString(Qt::ISODate));
    if (!m_cache.contains(cacheKey)) {
        double minutes;
        double jd2;
        
        jd2 = jd;
        NOAASolarCalc::calcTimeUTC(90.833, true, &jd2, &minutes, latitude, longitude);
        m_cache[cacheKey]["Apparent Sunrise"] = NOAASolarCalc::calcDateFromJD(jd2, minutes, zone);
        jd2 = jd;
        NOAASolarCalc::calcTimeUTC(90.833, false, &jd2, &minutes, latitude, longitude);
        m_cache[cacheKey]["Apparent Sunset"] = NOAASolarCalc::calcDateFromJD(jd2, minutes, zone);

        jd2 = jd;
        NOAASolarCalc::calcTimeUTC(90.0, true, &jd2, &minutes, latitude, longitude);
        m_cache[cacheKey]["Sunrise"] = NOAASolarCalc::calcDateFromJD(jd2, minutes, zone);
        jd2 = jd;
        NOAASolarCalc::calcTimeUTC(90.0, false, &jd2, &minutes, latitude, longitude);
        m_cache[cacheKey]["Sunset"] = NOAASolarCalc::calcDateFromJD(jd2, minutes, zone);

        jd2 = jd;
        NOAASolarCalc::calcTimeUTC(96.0, true, &jd2, &minutes, latitude, longitude);
        m_cache[cacheKey]["Civil Dawn"] = NOAASolarCalc::calcDateFromJD(jd2, minutes, zone);
        jd2 = jd;
        NOAASolarCalc::calcTimeUTC(96.0, false, &jd2, &minutes, latitude, longitude);
        m_cache[cacheKey]["Civil Dusk"] = NOAASolarCalc::calcDateFromJD(jd2, minutes, zone);

        jd2 = jd;
        NOAASolarCalc::calcTimeUTC(102.0, true, &jd2, &minutes, latitude, longitude);
        m_cache[cacheKey]["Nautical Dawn"] = NOAASolarCalc::calcDateFromJD(jd2, minutes, zone);
        jd2 = jd;
        NOAASolarCalc::calcTimeUTC(102.0, false, &jd2, &minutes, latitude, longitude);
        m_cache[cacheKey]["Nautical Dusk"] = NOAASolarCalc::calcDateFromJD(jd2, minutes, zone);

        jd2 = jd;
        NOAASolarCalc::calcTimeUTC(108.0, true, &jd2, &minutes, latitude, longitude);
        m_cache[cacheKey]["Astronomical Dawn"] = NOAASolarCalc::calcDateFromJD(jd2, minutes, zone);
        jd2 = jd;
        NOAASolarCalc::calcTimeUTC(108.0, false, &jd2, &minutes, latitude, longitude);
        m_cache[cacheKey]["Astronomical Dusk"] = NOAASolarCalc::calcDateFromJD(jd2, minutes, zone);

        century = NOAASolarCalc::calcTimeJulianCent(jd);
        minutes = NOAASolarCalc::calcSolNoonUTC(century, longitude);
        dt = NOAASolarCalc::calcDateFromJD(jd, minutes, zone);
        NOAASolarCalc::calc(dt, longitude, latitude, zone, &jd, &century, &eqTime,
                            &solarDec, &azimuth, &zenith);
        m_cache[cacheKey]["Solar Noon"] = dt;
        m_cache[cacheKey]["Min Zenith"] = zenith;
        m_cache[cacheKey]["Max Corrected Elevation"] = NOAASolarCalc::calcElevation(zenith);
    }
    data.unite(m_cache[cacheKey]);
    //kDebug() << "Calculation took:" << time.elapsed() << "ms";
    //kDebug() << data;
}

void NOAASolarCalc::calc(const QDateTime &dt, double longitude, double latitude,
                         double zone, double *jd, double *century, double *eqTime,
                         double *solarDec, double *azimuth, double *zenith)
{
    double timenow = dt.time().hour() + dt.time().minute() / 60.0 +
                     dt.time().second() / 3600.0 + zone;
    *jd = NOAASolarCalc::calcJD(dt.date().year(), dt.date().month(), dt.date().day());
    *century = NOAASolarCalc::calcTimeJulianCent(*jd + timenow / 24.0);
    //double earthRadVec = NOAASolarCalc::calcSunRadVector(*century);
    //double alpha = NOAASolarCalc::calcSunRtAscension(*century);
    *eqTime = NOAASolarCalc::calcEquationOfTime(*century);
    *solarDec = NOAASolarCalc::calcSunDeclination(*century);
    NOAASolarCalc::calcAzimuthAndZenith(dt, *eqTime, zone, *solarDec, latitude, longitude,
                                        zenith, azimuth);
}

// Convert radian angle to degrees
double NOAASolarCalc::radToDeg(double angleRad)
{
    return (180.0 * angleRad / M_PI);
}

// Convert degree angle to radians
double NOAASolarCalc::degToRad(double angleDeg)
{
    return (M_PI * angleDeg / 180.0);
}

//***********************************************************************/
//* Name:    calcJD                                                     */
//* Type:    Function                                                   */
//* Purpose: Julian day from calendar day                               */
//* Arguments:                                                          */
//*   year : 4 digit year                                               */
//*   month: January = 1                                                */
//*   day  : 1 - 31                                                     */
//* Return value:                                                       */
//*   The Julian day corresponding to the date                          */
//* Note:                                                               */
//*   Number is returned for start of day.  Fractional days should be   */
//*   added later.                                                      */
//***********************************************************************/
double NOAASolarCalc::calcJD(double year, double month, double day)
{
    if (month <= 2) {
        year -= 1;
        month += 12;
    }
    double A = floor(year / 100.0);
    double B = 2 - A + floor(A / 4.0);

    return floor(365.25 * (year + 4716)) + floor(30.6001 * (month + 1))
           + day + B - 1524.5;
}

//***********************************************************************/
//* Name:    calcDateFromJD                                             */
//* Type:    Function                                                   */
//* Purpose: Calendar date from Julian Day                              */
//* Arguments:                                                          */
//*   jd   : Julian Day                                                 */
//* Return value:                                                       */
//*   QDateTime                                                         */
//* Note:                                                               */
//***********************************************************************/
QDateTime NOAASolarCalc::calcDateFromJD(double jd, double minutes, double zone)
{
    QDateTime result;
    
    minutes -= 60 * zone;
    if (minutes > 1440.0) {
        minutes -= 1440.0;
        jd += 1.0;
    }
    if (minutes < 0.0) {
        minutes += 1440.0;
        jd -= 1.0;
    }
    double floatHour = minutes / 60.0;
    double hour = floor(floatHour);
    double floatMinute = 60.0 * (floatHour - floor(floatHour));
    double minute = floor(floatMinute);
    double floatSec = 60.0 * (floatMinute - floor(floatMinute));
    double second = floor(floatSec + 0.5);
    result.setTime(QTime(hour, minute, second));

    double z = floor(jd + 0.5);
    double f = (jd + 0.5) - z;
    double A;
    if (z < 2299161.0) {
        A = z;
    } else {
        double alpha = floor((z - 1867216.25) / 36524.25);
        A = z + 1.0 + alpha - floor(alpha / 4.0);
    }
    double B = A + 1524.0;
    double C = floor((B - 122.1) / 365.25);
    double D = floor(365.25 * C);
    double E = floor((B - D) / 30.6001);
    double day = B - D - floor(30.6001 * E) + f;
    double month = (E < 14.0) ? E - 1 : E - 13.0;
    double year = (month > 2.0) ? C - 4716.0 : C - 4715.0;
    result.setDate(QDate(year, month, day));

    return result;
}

//***********************************************************************/
//* Name:    calcTimeJulianCent                                         */
//* Type:    Function                                                   */
//* Purpose: convert Julian Day to centuries since J2000.0.             */
//* Arguments:                                                          */
//*   jd : the Julian Day to convert                                    */
//* Return value:                                                       */
//*   the T value corresponding to the Julian Day                       */
//***********************************************************************/
double NOAASolarCalc::calcTimeJulianCent(double jd)
{
    return (jd - 2451545.0) / 36525.0;
}

//***********************************************************************/
//* Name:    calcJDFromJulianCent                                       */
//* Type:    Function                                                   */
//* Purpose: convert centuries since J2000.0 to Julian Day.             */
//* Arguments:                                                          */
//*   t : number of Julian centuries since J2000.0                      */
//* Return value:                                                       */
//*   the Julian Day corresponding to the t value                       */
//***********************************************************************/
double NOAASolarCalc::calcJDFromJulianCent(double t)
{
    return t * 36525.0 + 2451545.0;
}

//***********************************************************************/
//* Name:    calcGeomMeanLongSun                                        */
//* Type:    Function                                                   */
//* Purpose: calculate the Geometric Mean Longitude of the Sun          */
//* Arguments:                                                          */
//*   t : number of Julian centuries since J2000.0                      */
//* Return value:                                                       */
//*   the Geometric Mean Longitude of the Solar in degrees              */
//***********************************************************************/
double NOAASolarCalc::calcGeomMeanLongSun(double t)
{
    double L0 = 280.46646 + t * (36000.76983 + 0.0003032 * t);
    while (L0 > 360.0) {
        L0 -= 360.0;
    }
    while(L0 < 0.0) {
        L0 += 360.0;
    }
    return L0;      // in degrees
}

//***********************************************************************/
//* Name:    calcGeomAnomalySun                                         */
//* Type:    Function                                                   */
//* Purpose: calculate the Geometric Mean Anomaly of the Sun            */
//* Arguments:                                                          */
//*   t : number of Julian centuries since J2000.0                      */
//* Return value:                                                       */
//*   the Geometric Mean Anomaly of the Solar in degrees                */
//***********************************************************************/
double NOAASolarCalc::calcGeomMeanAnomalySun(double t)
{
    return 357.52911 + t * (35999.05029 - 0.0001537 * t); // in degrees
}

//***********************************************************************/
//* Name:    calcEccentricityEarthOrbit                                 */
//* Type:    Function                                                   */
//* Purpose: calculate the eccentricity of earth's orbit                */
//* Arguments:                                                          */
//*   t : number of Julian centuries since J2000.0                      */
//* Return value:                                                       */
//*   the unitless eccentricity                                         */
//***********************************************************************/
double NOAASolarCalc::calcEccentricityEarthOrbit(double t)
{
    return 0.016708634 - t * (0.000042037 + 0.0000001267 * t); // unitless
}

//***********************************************************************/
//* Name:    calcSunEqOfCenter                                          */
//* Type:    Function                                                   */
//* Purpose: calculate the equation of center for the sun               */
//* Arguments:                                                          */
//*   t : number of Julian centuries since J2000.0                      */
//* Return value:                                                       */
//*   in degrees                                                        */
//***********************************************************************/
double NOAASolarCalc::calcSunEqOfCenter(double t)
{
    double m = calcGeomMeanAnomalySun(t);

    double mrad = degToRad(m);
    double sinm = sin(mrad);
    double sin2m = sin(mrad+mrad);
    double sin3m = sin(mrad+mrad+mrad);

    // in degrees
    return sinm * (1.914602 - t * (0.004817 + 0.000014 * t)) +
           sin2m * (0.019993 - 0.000101 * t) + sin3m * 0.000289;
}

//***********************************************************************/
//* Name:    calcSunTrueLong                                            */
//* Type:    Function                                                   */
//* Purpose: calculate the true longitude of the sun                    */
//* Arguments:                                                          */
//*   t : number of Julian centuries since J2000.0                      */
//* Return value:                                                       */
//*   solar's true longitude in degrees                                 */
//***********************************************************************/
double NOAASolarCalc::calcSunTrueLong(double t)
{
    double l0 = calcGeomMeanLongSun(t);
    double c = calcSunEqOfCenter(t);

    return l0 + c; // in degrees
}

//***********************************************************************/
//* Name:    calcSunTrueAnomaly                                         */
//* Type:    Function                                                   */
//* Purpose: calculate the true anamoly of the sun                      */
//* Arguments:                                                          */
//*   t : number of Julian centuries since J2000.0                      */
//* Return value:                                                       */
//*   solar's true anamoly in degrees                                   */
//***********************************************************************/
double NOAASolarCalc::calcSunTrueAnomaly(double t)
{
    double m = calcGeomMeanAnomalySun(t);
    double c = calcSunEqOfCenter(t);

    return m + c; // in degrees
}

//***********************************************************************/
//* Name:    calcSunRadVector                                           */
//* Type:    Function                                                   */
//* Purpose: calculate the distance to the sun in AU                    */
//* Arguments:                                                          */
//*   t : number of Julian centuries since J2000.0                      */
//* Return value:                                                       */
//*   solar radius vector in AUs                                        */
//***********************************************************************/
double NOAASolarCalc::calcSunRadVector(double t)
{
    double v = calcSunTrueAnomaly(t);
    double e = calcEccentricityEarthOrbit(t);

    // in AUs
    return (1.000001018 * (1 - e * e)) / (1 + e * cos(degToRad(v)));
}

//***********************************************************************/
//* Name:    calcSunApparentLong                                        */
//* Type:    Function                                                   */
//* Purpose: calculate the apparent longitude of the sun                */
//* Arguments:                                                          */
//*   t : number of Julian centuries since J2000.0                      */
//* Return value:                                                       */
//*   solar's apparent longitude in degrees                             */
//***********************************************************************/
double NOAASolarCalc::calcSunApparentLong(double t)
{
    double o = calcSunTrueLong(t);

    double omega = 125.04 - 1934.136 * t;
    // in degrees
    return o - 0.00569 - 0.00478 * sin(degToRad(omega));
}

//***********************************************************************/
//* Name:    calcMeanObliquityOfEcliptic                                */
//* Type:    Function                                                   */
//* Purpose: calculate the mean obliquity of the ecliptic               */
//* Arguments:                                                          */
//*   t : number of Julian centuries since J2000.0                      */
//* Return value:                                                       */
//*   mean obliquity in degrees                                         */
//***********************************************************************/
double NOAASolarCalc::calcMeanObliquityOfEcliptic(double t)
{
    double seconds = 21.448 - t * (46.8150 + t * (0.00059 - t * (0.001813)));
    return 23.0 + (26.0 + (seconds/60.0)) / 60.0; // in degrees
}

//***********************************************************************/
//* Name:    calcObliquityCorrection                                    */
//* Type:    Function                                                   */
//* Purpose: calculate the corrected obliquity of the ecliptic          */
//* Arguments:                                                          */
//*   t : number of Julian centuries since J2000.0                      */
//* Return value:                                                       */
//*   corrected obliquity in degrees                                    */
//***********************************************************************/
double NOAASolarCalc::calcObliquityCorrection(double t)
{
    double e0 = calcMeanObliquityOfEcliptic(t);

    double omega = 125.04 - 1934.136 * t;
    return e0 + 0.00256 * cos(degToRad(omega)); // in degrees
}

//***********************************************************************/
//* Name:    calcSunRtAscension                                         */
//* Type:    Function                                                   */
//* Purpose: calculate the right ascension of the sun                   */
//* Arguments:                                                          */
//*   t : number of Julian centuries since J2000.0                      */
//* Return value:                                                       */
//*   solar's right ascension in degrees                                */
//***********************************************************************/
double NOAASolarCalc::calcSunRtAscension(double t)
{
    double e = calcObliquityCorrection(t);
    double lambda = calcSunApparentLong(t);

    double tananum = (cos(degToRad(e)) * sin(degToRad(lambda)));
    double tanadenom = (cos(degToRad(lambda)));
    return radToDeg(atan2(tananum, tanadenom)); // in degrees
}

//***********************************************************************/
//* Name:    calcSunDeclination                                         */
//* Type:    Function                                                   */
//* Purpose: calculate the declination of the sun                       */
//* Arguments:                                                          */
//*   t : number of Julian centuries since J2000.0                      */
//* Return value:                                                       */
//*   solar's declination in degrees                                    */
//***********************************************************************/
double NOAASolarCalc::calcSunDeclination(double t)
{
    double e = calcObliquityCorrection(t);
    double lambda = calcSunApparentLong(t);

    double sint = sin(degToRad(e)) * sin(degToRad(lambda));
    return radToDeg(asin(sint)); // in degrees
}

//***********************************************************************/
//* Name:    calcHourAngle                                              */
//* Type:    Function                                                   */
//* Purpose: calculate the hour angle of the sun at sunset for the      */
//*          latitude                                                   */
//* Arguments:                                                          */
//*   zenith : zenith angle in degrees                                  */
//*   lat : latitude of observer in degrees                             */
//*   solarDec : declination angle of sun in degrees                    */
//* Return value:                                                       */
//*   hour angle of sunset in radians                                   */
//***********************************************************************/
double NOAASolarCalc::calcHourAngle(double zenith, double solarDec, double latitude)
{
    double latRad = degToRad(latitude);
    double sdRad  = degToRad(solarDec);

    //double HAarg = (cos(degToRad(90.833)) / (cos(latRad) * cos(sdRad)) - tan(latRad) * tan(sdRad));

    double HA = (acos(cos(degToRad(zenith)) / (cos(latRad) * cos(sdRad)) - tan(latRad) * tan(sdRad)));

    return HA;     // in radians
}

//***********************************************************************/
//* Name:    calcEquationOfTime                                         */
//* Type:    Function                                                   */
//* Purpose: calculate the difference between true solar time and mean  */
//*     solar time                                                      */
//* Arguments:                                                          */
//*   t : number of Julian centuries since J2000.0                      */
//* Return value:                                                       */
//*   equation of time in minutes of time                               */
//***********************************************************************/
double NOAASolarCalc::calcEquationOfTime(double t)
{
    double epsilon = calcObliquityCorrection(t);
    double l0 = calcGeomMeanLongSun(t);
    double e = calcEccentricityEarthOrbit(t);
    double m = calcGeomMeanAnomalySun(t);

    double y = tan(degToRad(epsilon) / 2.0);
    y *= y;

    double sin2l0 = sin(2.0 * degToRad(l0));
    double sinm   = sin(degToRad(m));
    double cos2l0 = cos(2.0 * degToRad(l0));
    double sin4l0 = sin(4.0 * degToRad(l0));
    double sin2m  = sin(2.0 * degToRad(m));

    double Etime = y * sin2l0 - 2.0 * e * sinm + 4.0 * e * y * sinm * cos2l0
            - 0.5 * y * y * sin4l0 - 1.25 * e * e * sin2m;

    return radToDeg(Etime) * 4.0; // in minutes of time
}

//***********************************************************************/
//* Name:    calcSolNoonUTC                                             */
//* Type:    Function                                                   */
//* Purpose: calculate the Universal Coordinated Time (UTC) of solar    */
//*     noon for the given day at the given location on earth           */
//* Arguments:                                                          */
//*   t : number of Julian centuries since J2000.0                      */
//*   longitude : longitude of observer in degrees                      */
//* Return value:                                                       */
//*   time in minutes from zero Z                                       */
//***********************************************************************/
double NOAASolarCalc::calcSolNoonUTC(double t, double longitude)
{
    // First pass uses approximate solar noon to calculate eqtime
    double tnoon = calcTimeJulianCent(calcJDFromJulianCent(t) + longitude / 360.0);
    double eqTime = calcEquationOfTime(tnoon);
    double solNoonUTC = 720 + (longitude * 4) - eqTime; // min

    double newt = calcTimeJulianCent(calcJDFromJulianCent(t) -0.5 + solNoonUTC / 1440.0);

    eqTime = calcEquationOfTime(newt);
    // double solarNoonDec = calcSunDeclination(newt);
    solNoonUTC = 720 + (longitude * 4) - eqTime; // min
    return solNoonUTC;
}

//***********************************************************************/
//* Name:    calcTimeUTC                                                */
//* Type:    Function                                                   */
//* Purpose: calculate the Universal Coordinated Time (UTC) of sunset   */
//*         for the given day at the given location on earth            */
//* Arguments:                                                          */
//*   zenith : zenith in degrees                                        */
//*   rise : rise or set                                                */
//*   JD  : julian day                                                  */
//*   latitude : latitude of observer in degrees                        */
//*   longitude : longitude of observer in degrees                      */
//* Return value:                                                       */
//*   time in minutes from zero Z                                       */
//***********************************************************************/
void NOAASolarCalc::calcTimeUTC(double zenith, bool rise, double *jd, double *minutes,
                                double latitude, double longitude)
{
    forever {
        double t = calcTimeJulianCent(*jd);
        double m = (rise) ? 1.0 : -1.0;

        // *** Find the time of solar noon at the location, and use
        //     that declination. This is better than start of the
        //     Julian day

        double noonmin = calcSolNoonUTC(t, longitude);
        double tnoon = calcTimeJulianCent(*jd + noonmin / 1440.0);

        // First calculates sunrise and approx length of day

        double eqTime = calcEquationOfTime(tnoon);
        double solarDec = calcSunDeclination(tnoon);
        double hourAngle = m * calcHourAngle(zenith, solarDec, latitude);

        double delta = longitude - radToDeg(hourAngle);
        double timeDiff = 4 * delta;
        *minutes = 720 + timeDiff - eqTime;

        // first pass used to include fractional day in gamma calc

        double newt = calcTimeJulianCent(calcJDFromJulianCent(t) + *minutes / 1440.0);
        eqTime = calcEquationOfTime(newt);
        solarDec = calcSunDeclination(newt);
        hourAngle = m * calcHourAngle(zenith, solarDec, latitude);

        delta = longitude - radToDeg(hourAngle);
        timeDiff = 4 * delta;
        *minutes = 720 + timeDiff - eqTime; // in minutes
        if (isnan(*minutes)) {
            *jd -= m;
        } else {
            return;
        }
    }
}

void NOAASolarCalc::calcAzimuthAndZenith(QDateTime dt, double eqTime, double zone,
                                         double solarDec, double latitude, double longitude,
                                         double *zenith, double *azimuth)
{
    double solarTimeFix = eqTime - 4.0 * longitude + 60.0 * zone;
    double trueSolarTime = dt.time().hour() * 60.0 + dt.time().minute() +
                           dt.time().second() / 60.0 + solarTimeFix;
    // in minutes
    while (trueSolarTime > 1440) {
        trueSolarTime -= 1440;
    }
    //double hourAngle = calcHourAngle(timenow, m_longitude, eqTime);
    double hourAngle = trueSolarTime / 4.0 - 180.0;
    // Thanks to Louis Schwarzmayr for finding our error,
    // and providing the following 4 lines to fix it:
    if (hourAngle < -180.0) {
        hourAngle += 360.0;
    }

    double haRad = degToRad(hourAngle);

    double csz = sin(degToRad(latitude)) * sin(degToRad(solarDec)) +
                 cos(degToRad(latitude)) * cos(degToRad(solarDec)) * cos(haRad);
    if (csz > 1.0) {
        csz = 1.0;
    } else if (csz < -1.0) {
        csz = -1.0;
    }
    *zenith = radToDeg(acos(csz));
    double azDenom = (cos(degToRad(latitude)) * sin(degToRad(*zenith)));

    if (fabs(azDenom) > 0.001) {
        double azRad = ((sin(degToRad(latitude)) * cos(degToRad(*zenith))) -
                         sin(degToRad(solarDec))) / azDenom;
        if (fabs(azRad) > 1.0) {
            if (azRad < 0) {
                azRad = -1.0;
            } else {
                azRad = 1.0;
            }
        }

        *azimuth = 180.0 - radToDeg(acos(azRad));

        if (hourAngle > 0.0) {
            *azimuth = -*azimuth;
        }
    } else {
        if (latitude > 0.0) {
            *azimuth = 180.0;
        } else {
            *azimuth = 0.0;
        }
    }
    if (*azimuth < 0.0) {
        *azimuth += 360.0;
    }
}

double NOAASolarCalc::calcElevation(double zenith)
{
    double exoatmElevation = 90.0 - zenith;
    double refractionCorrection;
    if (exoatmElevation > 85.0) {
        refractionCorrection = 0.0;
    } else {
        double te = tan(degToRad(exoatmElevation));
        if (exoatmElevation > 5.0) {
            refractionCorrection = 58.1 / te - 0.07 / (te * te * te) +
                                   0.000086 / (te * te * te * te * te);
        } else if (exoatmElevation > -0.575) {
            refractionCorrection = 1735.0 + exoatmElevation *
                (-518.2 + exoatmElevation * (103.4 + exoatmElevation *
                (-12.79 + exoatmElevation * 0.711) ) );
        } else {
            refractionCorrection = -20.774 / te;
        }
        refractionCorrection = refractionCorrection / 3600.0;
    }
    double solarZen = zenith - refractionCorrection;
    return 90.0 - solarZen;
}

