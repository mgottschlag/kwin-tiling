/* This file is proposed to be part of the KDE libraries.
 * Copyright (C) 2003 Laur Ivan <laurivan@eircom.net>
 *
 * Many thanks to:
 *  - Bernardo Hung <deciare@gta.igs.net> for the enhanced shadow
 *    algorithm (currently used)
 *  - Tim Jansen <tim@tjansen.de> for the API updates and fixes.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License version 2 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "kshadowengine.h"
#include <qcolor.h>
#include "kshadowsettings.h"

KShadowEngine::KShadowEngine() :
	m_shadowSettings( new KShadowSettings )
{
}

KShadowEngine::~KShadowEngine()
{
  delete m_shadowSettings;
}

KShadowEngine::KShadowEngine(KShadowSettings *fx) :
	m_shadowSettings(0L)
{
  setShadowSettings(fx);
}


void KShadowEngine::setShadowSettings(KShadowSettings *fx)
{
  delete m_shadowSettings;

  m_shadowSettings = fx;
}

KShadowSettings *KShadowEngine::shadowSettings()
{
  return m_shadowSettings;
}

QImage KShadowEngine::makeShadow(const QPixmap& textPixmap, const QColor &bgColor)
{
  QImage result;

  // create a new image for for the shaddow
  int w = textPixmap.width();
  int h = textPixmap.height();

  // avoid calling these methods for every pixel
  int bgRed = bgColor.red();
  int bgGreen = bgColor.green();
  int bgBlue = bgColor.blue();

  int thick = m_shadowSettings->thickness() >> 1;

  double alphaShadow;

  /*
   *	This is the source pixmap
   */
  QImage img = textPixmap.convertToImage().convertDepth(32);

  /*
   *	Resize the image if necessary
   */
  if ((result.width() != w) || (result.height() != h))
  {
    result.create(w, h, 32);
  }

  result.fill(0); // all black
  result.setAlphaBuffer(true);

  for (int i = thick; i < w - thick; i++)
  {
    for (int j = thick; j < h - thick; j++)
    {
      switch (m_shadowSettings->algorithm())
      {
      case KShadowSettings::DoubleLinearDecay:
	alphaShadow = doubleLinearDecay(img, i, j);
	break;
      case KShadowSettings::RadialDecay:
	alphaShadow = radialDecay(img, i, j);
	break;
      case KShadowSettings::NoDecay:
	alphaShadow = noDecay(img, i, j);
	break;
      case KShadowSettings::DefaultDecay:
      default:
	alphaShadow = defaultDecay(img, i, j);
      }

      alphaShadow = (alphaShadow > m_shadowSettings->maxOpacity()) ? m_shadowSettings->maxOpacity() : alphaShadow;

      // update the shadow's i,j pixel.
      result.setPixel(i,j, qRgba(bgRed, bgGreen , bgBlue, (int) alphaShadow));
    }
  }
  return result;
}

// Multiplication factor for pixels directly above, under, or next to the text
#define AXIS_FACTOR 2.0
// Multiplication factor for pixels diagonal to the text
#define DIAGONAL_FACTOR 1.0

double KShadowEngine::defaultDecay(QImage& source, int i, int j)
{
  if ((i < 1) || (j < 1) || (i > source.width() - 2) || (j > source.height() - 2))
    return 0;

  double alphaShadow;
  alphaShadow =(qGray(source.pixel(i-1,j-1)) * DIAGONAL_FACTOR +
		qGray(source.pixel(i-1,j  )) * AXIS_FACTOR +
		qGray(source.pixel(i-1,j+1)) * DIAGONAL_FACTOR +
		qGray(source.pixel(i  ,j-1)) * AXIS_FACTOR +
		0                         +
		qGray(source.pixel(i  ,j+1)) * AXIS_FACTOR +
		qGray(source.pixel(i+1,j-1)) * DIAGONAL_FACTOR +
		qGray(source.pixel(i+1,j  )) * AXIS_FACTOR +
		qGray(source.pixel(i+1,j+1)) * DIAGONAL_FACTOR) / m_shadowSettings->multiplicationFactor();

  return alphaShadow;
}

double KShadowEngine::doubleLinearDecay(QImage& source, int i, int j)
{
  //printf("img: %p, %d %d\n", (char *) &source, i, j);
  return defaultDecay( source, i, j ); // for now
}

double KShadowEngine::radialDecay(QImage& source, int i, int j)
{
  //printf("img: %p, %d %d\n", (char *) &source, i, j);
  return defaultDecay( source, i, j ); // for now
}

double KShadowEngine::noDecay(QImage& source, int i, int j)
{
  // create a new image for for the shaddow
  int w = source.width();
  int h = source.height();
  int sx, sy;
  //int thick = m_shadowSettings->thickness() >> 1;

  double alphaShadow = 0;
  double opacity = 0;
  for (int k = 1; k <= m_shadowSettings->thickness(); k++) {
    /* Generate a shadow THICKNESS pixels thicker
     * on either side than the text image. Ensure
     * that i +/- k and j +/- k are within the
     * bounds of the text pixmap.
     */
    opacity = 0;
    for (int l = -k; l <= k; l++) {
      if (i < k)
	sx = 0;
      else if (i >= w - k)
	sx = w - 1;
      else
	sx = i + l;

      for (int m = -k; m <= k; m++) {
	if (j < k)
	  sy = 0;
	else if (j >= h - k)
	  sy = h - 1;
	else
	  sy = j + m;

	opacity += qGray(source.pixel(sx, sy));
      }
    }
    alphaShadow += opacity / m_shadowSettings->multiplicationFactor();
  }
  return alphaShadow;
}
