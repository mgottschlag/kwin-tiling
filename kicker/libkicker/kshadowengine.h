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

#ifndef __FX_SHADOW
#define __FX_SHADOW

#include <kdemacros.h>

class KShadowSettings;
class QPixmap;
class QColor;
class QImage;

/**
 * This class implements the shadow algorithm(s). It uses a FxData
 * object for its parameters. Note that the shadow algorithm is using the
 * luminosity of the original pixmap for the shadow one.
 * @see KShadowSettings
 * @author laur.ivan@corvil.com
 * @since 3.2
 */
class KDE_EXPORT KShadowEngine
{
	public:
		/// Creates a new shadow engine.
		KShadowEngine();

		~KShadowEngine();

		/**
		 * Creates a new shadow engine.
		 * @param fx the shadow settings object with the configuration. The Shadow
		 *        Engine will own this object and also delete it. Must
		 *        be heap-allocated
		 */
		KShadowEngine(KShadowSettings *fx);

		/**
		 * Set the KShadowSettings object.
		 * @param fx the shadow settings object with the configuration. The Shadow
		 *        Engine will own this object and also delete it. Must
		 *        be heap-allocated.
		 */
		void setShadowSettings(KShadowSettings *fx);

		/**
		 * Get the current KShadowSettings.
		 * @param the current shadow settings
		 */
		KShadowSettings *shadowSettings();

		/**
		 * Make shadow!
		 *
		 * textPixmap is the original pixmap where a (white) text is drawn.
		 * bgColor is the color used for the shadow.
		 * @param textPixmap the pixmap of the text
		 * @param bgColor the background color
		 * @return the resulting image
		 */
		QImage makeShadow(const QPixmap& textPixmap, const QColor &bgColor);

	private:
		/*
		 * a simple algorithm with 3 pixels thickness
		 */
		double defaultDecay(QImage& source, int x, int y);

		/*
		 * a slower algorithm where the influence of a pixel
		 * is  qGray(px)/(abs(dx) + abs(dy) +1).
		 */
		double doubleLinearDecay(QImage& source, int x, int y);

		/*
		 * a very slow algorithm where the influence of a pixel
		 * is  qGray(px)/(sqrt(sqr(dx) + sqr(dy)) +1).
		 */
		double radialDecay(QImage& source, int x, int y);

		/*
		 * a nice/fast algorithm proposed by Bernardo Hung
		 */
		double noDecay(QImage& source, int x, int y);

        class Private;
		Private *d;
};

#endif
