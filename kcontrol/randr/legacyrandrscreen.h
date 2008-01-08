/*
 * Copyright (c) 2007      Gustavo Pichorim Boiko <gustavo.boiko@kdemail.net>
 * Copyright (c) 2002,2003 Hamish Rodda <rodda@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef __LEGACYRANDRSCREEN_H__
#define __LEGACYRANDRSCREEN_H__

#include <QObject>
//Added by qt3to4:
#include <QPixmap>

#include <kcmodule.h>
#include <kconfig.h>
#include "randr.h"

class KTimerDialog;

class LegacyRandRScreen : public QObject
{
	Q_OBJECT
public:
	LegacyRandRScreen(int screenIndex);
	~LegacyRandRScreen();

	void loadSettings();
	void setOriginal();

	bool applyProposed();

	/**
	 * @returns false if the user did not confirm in time, or canceled, or the change failed
	 */
	bool applyProposedAndConfirm();

	Window rootWindow() const;

public:
	QString changedMessage() const;

	bool changedFromOriginal() const;
	void proposeOriginal();

	bool proposedChanged() const;

	QString currentRotationDescription() const;

	int rotationIndexToDegree(int rotation) const;
	int rotationDegreeToIndex(int degree) const;

	/**
	 * Refresh rate functions.
	 */
	RateList refreshRates(int size) const;

	QString refreshRateDirectDescription(int rate) const;
	QString refreshRateIndirectDescription(int size, int index) const;
	QString refreshRateDescription(int size, int index) const;

	int refreshRate() const;
	QString currentRefreshRateDescription() const;

	// Refresh rate hz <==> index conversion
	int refreshRateHzToIndex(int size, int hz) const;
	int refreshRateIndexToHz(int size, int index) const;

	/**
	 * Screen size functions.
	 */
	int numSizes() const;
	const QSize& pixelSize(int index) const;
	const QSize& mmSize(int index) const;
	int pixelCount(int index) const;

	SizeList pixelSizes() const;

	/**
	 * Retrieve the index of a screen size with a specified pixel size.
	 *
	 * @param pixelSize dimensions of the screen in pixels
	 * @returns the index of the requested screen size
	 */
	int sizeIndex(const QSize &pixelSize) const;

	int rotations() const;

	/**
	 * Current setting functions.
	 */
	const QSize& currentPixelSize() const;

	int rotation() const;
	int size() const;

	/**
	 * Proposed setting functions.
	 */
	int proposedSize() const;
	bool proposeSize(int newSize);

	int proposedRotation() const;
	void proposeRotation(int newRotation);

	int proposedRefreshRate() const;
	/**
	 * Propose a refresh rate.
	 * Please note that you must propose the target size first for this to work.
	 *
	 * @param index the index of the refresh rate (not a refresh rate in hz!)
	 * @returns true if successful, false otherwise.
	 */
	bool proposeRefreshRate(int index);

	/**
	 * Configuration functions.
	 */
	void load(KConfig& config);
	void save(KConfig& config) const;

private:
	XRRScreenConfiguration*	m_config;

	int m_screen;

	SizeList m_pixelSizes;
	SizeList m_mmSizes;
	int m_rotations;

	int m_originalRotation;
	int m_originalSize;
	int m_originalRefreshRate;

	int m_currentRotation;
	int m_currentSize;
	int m_currentRefreshRate;

	int m_proposedRotation;
	int m_proposedSize;
	int m_proposedRefreshRate;

	KTimerDialog* m_shownDialog;

};

#endif
