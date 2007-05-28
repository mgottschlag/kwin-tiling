/*
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

#ifndef __RANDRSCREEN_H__
#define __RANDRSCREEN_H__

#include <QObject>
#include <QStringList>
//Added by qt3to4:
#include <QPixmap>

#include <kcmodule.h>
#include <kconfig.h>

class KTimerDialog;
class RandRScreenPrivate;

class RandRScreen : public QObject
{
	Q_OBJECT

public:
	enum orientations {
		Rotate0			= 0x1,
		Rotate90		= 0x2,
		Rotate180		= 0x4,
		Rotate270		= 0x8,
		RotateMask		= 15,
		RotationCount	= 4,
		ReflectX		= 0x10,
		ReflectY		= 0x20,
		ReflectMask		= 48,
		OrientationMask	= 63,
		OrientationCount = 6
	};

	RandRScreen(int screenIndex);
	~RandRScreen();

	void		loadSettings();
	void		setOriginal();

	bool		applyProposed();

	/**
	 * @returns false if the user did not confirm in time, or canceled, or the change failed
	 */
	bool		applyProposedAndConfirm();

public Q_SLOTS:
	bool		confirm();

public:
	QString		changedMessage() const;

	bool		changedFromOriginal() const;
	void		proposeOriginal();

	bool		proposedChanged() const;

	static QString	rotationName(int rotation, bool pastTense = false, bool capitalised = true);
	QPixmap	        rotationIcon(int rotation) const;
	QString			currentRotationDescription() const;

	int				rotationIndexToDegree(int rotation) const;
	int				rotationDegreeToIndex(int degree) const;

	/**
	 * Refresh rate functions.
	 */
	QStringList refreshRates(int size) const;

	QString		refreshRateDirectDescription(int rate) const;
	QString		refreshRateIndirectDescription(int size, int index) const;
	QString		refreshRateDescription(int size, int index) const;

	int			currentRefreshRate() const;
	QString		currentRefreshRateDescription() const;

	// Refresh rate hz <==> index conversion
	int			refreshRateHzToIndex(int size, int hz) const;
	int			refreshRateIndexToHz(int size, int index) const;

	/**
	 * Screen size functions.
	 */
	int				numSizes() const;
	const QSize&	pixelSize(int index) const;
	const QSize&	mmSize(int index) const;
	int				pixelCount(int index) const;

	/**
	 * Retrieve the index of a screen size with a specified pixel size.
	 *
	 * @param pixelSize dimensions of the screen in pixels
	 * @returns the index of the requested screen size
	 */
	int				sizeIndex(QSize pixelSize) const;

	int			rotations() const;

	/**
	 * Current setting functions.
	 */
	int			currentPixelWidth() const;
	int			currentPixelHeight() const;
	int			currentMMWidth() const;
	int			currentMMHeight() const;

	int			currentRotation() const;
	int			currentSize() const;

	/**
	 * Proposed setting functions.
	 */
	int			proposedSize() const;
	bool		proposeSize(int newSize);

	int			proposedRotation() const;
	void		proposeRotation(int newRotation);

	int			proposedRefreshRate() const;
	/**
	 * Propose a refresh rate.
	 * Please note that you must propose the target size first for this to work.
	 *
	 * @param index the index of the refresh rate (not a refresh rate in hz!)
	 * @returns true if successful, false otherwise.
	 */
	bool		proposeRefreshRate(int index);

	/**
	 * Configuration functions.
	 */
	void		load(KConfig& config);
	void		save(KConfig& config) const;

private:
	RandRScreenPrivate*	d;

	int			m_screen;

	QList<QSize>		m_pixelSizes;
	QList<QSize>		m_mmSizes;
	int			m_rotations;

	int			m_originalRotation;
	int			m_originalSize;
	int			m_originalRefreshRate;

	int			m_currentRotation;
	int			m_currentSize;
	int			m_currentRefreshRate;

	int			m_proposedRotation;
	int			m_proposedSize;
	int			m_proposedRefreshRate;

	KTimerDialog*	m_shownDialog;

private Q_SLOTS:
	void		desktopResized();
	void		shownDialogDestroyed();
};

#endif
