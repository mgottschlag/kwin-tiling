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

#include <QString>
#include <qstringlist.h>
#include "kshadowsettings.h"

class KShadowSettings::Private
{
public:
    /*
     * The employed algorithm (see fxshadow.h)
     */
    Algorithm algorithm;
    /**
     * This is the multiplication factor for the resulted shadow
     */
    double multiplicationFactor;
    /**
     * The maximum permitted opacity for the shadow
     */
    double maxOpacity;
    /*
     * offsetX and offsetY are the x/y offsets of the shadow with
     * the mention that 0,0 is a centered shadow.
     */
    int offsetX;
    int offsetY;
    /*
     * The shadow thickness:
     * shadow is this many pixels thicker than the text.
     */
    int thickness;
    /*
     * If the value is InverseVideoOnSelection, then the fg/bg
     * colours are swapped when the element is selected.
     * Otherwise, the selected fg/bg colors are used for text
     * as well
     */
    SelectionType selectionType;
};

KShadowSettings::KShadowSettings()
{
    d = new Private;
	// init the components with some default values
	setDefaults();
}

KShadowSettings::~KShadowSettings()
{
    delete d;
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
	return d->algorithm;
}

/**
 * Returns a multiplication facor used to average the resulted data
 */
double KShadowSettings::multiplicationFactor() const
{
	return d->multiplicationFactor;
}

/**
 * Returns the max opacity allowed (0 = transparent, 255 = opaque)
 */
double KShadowSettings::maxOpacity() const
{
	return d->maxOpacity;
}

/**
 * Returns the Y offset (0 is centered on text)
 */
int KShadowSettings::offsetX() const
{
	return d->offsetX;
}

/**
 * Returns the Y offset (0 is centered on text)
 */
int KShadowSettings::offsetY() const
{
	return d->offsetY;
}

/**
 * Returns the thickness. Used by the KShadow algorithm
 */
int KShadowSettings::thickness() const
{
	return d->thickness;
}

/**
 * Returns the selection type
 */
KShadowSettings::SelectionType KShadowSettings::selectionType() const
{
	return d->selectionType;
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
	d->algorithm = val;
}

/**
 * Set the multiplication factor
 */
void KShadowSettings::setMultiplicationFactor(double val)
{
	d->multiplicationFactor = val;
}

/**
 * Set the max. opacity
 */
void KShadowSettings::setMaxOpacity(double val)
{
	d->maxOpacity = val;
}

/**
 * Set the X offset of the shadow
 */
void KShadowSettings::setOffsetX(int val)
{
	d->offsetX = val;
}

/**
 * Set the Y offset of the shadow
 */
void KShadowSettings::setOffsetY(int val)
{
	d->offsetY = val;
}

/**
 * Set the shadow thickness
 */
void KShadowSettings::setThickness(int val)
{
	d->thickness = val;
}

/**
 * Set the selection type
 */
void KShadowSettings::setSelectionType(SelectionType val)
{
	d->selectionType = val;
}
