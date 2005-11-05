/* This file is proposed to be part of the KDE libraries.
 * Copyright (C) 2003 Laur Ivan <laurivan@eircom.net>
 *
 * Many thanks to:
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

#include <qstring.h>
#include <qstringlist.h>
#include "kshadowsettings.h"

KShadowSettings::KShadowSettings()
{
	// init the components with some default values
	setDefaults();
}

// load/save methods
void KShadowSettings::fromString(const QString &val)
{
  setOffsetX(val.section(',', OFFSET_X, OFFSET_X).toInt());
  setOffsetY(val.section(',', OFFSET_Y, OFFSET_Y).toInt());
  setMultiplicationFactor(val.section(',', MULTIPLICATION_FACTOR, MULTIPLICATION_FACTOR).toDouble());
  setMaxOpacity(val.section(',', MAX_OPACITY, MAX_OPACITY).toDouble());
  setThickness(val.section(',', THICKNESS, THICKNESS).toInt());
  setAlgorithm((Algorithm) val.section(',', ALGORITHM, ALGORITHM).toInt());
  setSelectionType((SelectionType)val.section(',', SELECTION_TYPE, SELECTION_TYPE).toInt());
}

QString KShadowSettings::toString() const
{
  QString result;
  result.sprintf("%d,%d,%f,%f,%d,%d,%d",
		 offsetX(),
		 offsetY(),
		 multiplicationFactor(),
		 maxOpacity(),
		 thickness(),
		 (int)algorithm(),
		 (int)selectionType());
  return result;
}

//***********************************
//               get methods
//***********************************

/**
 * Returns the decay algorithm to be used (see the alg. enumeration in the .h)
 */
KShadowSettings::Algorithm KShadowSettings::algorithm() const
{
	return _algorithm;
}

/**
 * Returns a multiplication facor used to average the resulted data
 */
double KShadowSettings::multiplicationFactor() const
{
	return _multiplicationFactor;
}

/**
 * Returns the max opacity allowed (0 = transparent, 255 = opaque)
 */
double KShadowSettings::maxOpacity() const
{
	return _maxOpacity;
}

/**
 * Returns the Y offset (0 is centered on text)
 */
int KShadowSettings::offsetX() const
{
	return _offsetX;
}

/**
 * Returns the Y offset (0 is centered on text)
 */
int KShadowSettings::offsetY() const
{
	return _offsetY;
}

/**
 * Returns the thickness. Used by the KShadow algorithm
 */
int KShadowSettings::thickness() const
{
	return _thickness;
}

/**
 * Returns the selection type
 */
KShadowSettings::SelectionType KShadowSettings::selectionType() const
{
	return _selectionType;
}

// set methods
/**
 * set the default parameters
 */
void KShadowSettings::setDefaults()
{
	fromString(DEFAULT_SHADOW_CONFIGURATION);
}


/**
 * Set the algorithm
 */
void KShadowSettings::setAlgorithm(Algorithm val)
{
	_algorithm = val;
}

/**
 * Set the multiplication factor
 */
void KShadowSettings::setMultiplicationFactor(double val)
{
	_multiplicationFactor = val;
}

/**
 * Set the max. opacity
 */
void KShadowSettings::setMaxOpacity(double val)
{
	_maxOpacity = val;
}

/**
 * Set the X offset of the shadow
 */
void KShadowSettings::setOffsetX(int val)
{
	_offsetX = val;
}

/**
 * Set the Y offset of the shadow
 */
void KShadowSettings::setOffsetY(int val)
{
	_offsetY = val;
}

/**
 * Set the shadow thickness
 */
void KShadowSettings::setThickness(int val)
{
	_thickness = val;
}

/**
 * Set the selection type
 */
void KShadowSettings::setSelectionType(SelectionType val)
{
	_selectionType = val;
}
