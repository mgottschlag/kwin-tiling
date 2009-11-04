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



#include <QTimer>
//Added by qt3to4:
#include <QPixmap>
#include "ktimerdialog.h"

#include <kdebug.h>
#include <klocale.h>
#include <kglobal.h>
#include <kapplication.h>
#include <kiconloader.h>
#include <QtGui/QLabel>
#include <QDesktopWidget>

#include <QX11Info>
#include "legacyrandrscreen.h"

LegacyRandRScreen::LegacyRandRScreen(int screenIndex)
	: m_config(0L)
	, m_screen(screenIndex)
	, m_shownDialog(NULL)
{
	loadSettings();
	setOriginal();
}

LegacyRandRScreen::~LegacyRandRScreen()
{
	if (m_config)
		XRRFreeScreenConfigInfo(m_config);
}

void LegacyRandRScreen::loadSettings()
{
	if (m_config)
		XRRFreeScreenConfigInfo(m_config);

	m_config = XRRGetScreenInfo(QX11Info::display(), rootWindow());
	Q_ASSERT(m_config);

	Rotation rotation;
	m_currentSize = m_proposedSize = XRRConfigCurrentConfiguration(m_config, &rotation);
	m_currentRotation = m_proposedRotation = rotation;

	m_pixelSizes.clear();
	m_mmSizes.clear();
	int numSizes;
	XRRScreenSize* sizes = XRRSizes(QX11Info::display(), m_screen, &numSizes);
	for (int i = 0; i < numSizes; i++) {
		m_pixelSizes.append(QSize(sizes[i].width, sizes[i].height));
		m_mmSizes.append(QSize(sizes[i].mwidth, sizes[i].mheight));
	}

	m_rotations = XRRRotations(QX11Info::display(), m_screen, &rotation);

	m_currentRefreshRate = m_proposedRefreshRate = refreshRateHzToIndex(m_currentSize, XRRConfigCurrentRate(m_config));
}

void LegacyRandRScreen::setOriginal()
{
	m_originalSize = m_currentSize;
	m_originalRotation = m_currentRotation;
	m_originalRefreshRate = m_currentRefreshRate;
}

bool LegacyRandRScreen::applyProposed()
{
	//kDebug() << " size " << (SizeID)proposedSize() << ", rotation " << proposedRotation() << ", refresh " << refreshRateIndexToHz(proposedSize(), proposedRefreshRate());

	Status status;

	if (proposedRefreshRate() < 0)
		status = XRRSetScreenConfig(QX11Info::display(), m_config, rootWindow(), (SizeID)proposedSize(), (Rotation)proposedRotation(), CurrentTime);
	else {
		if( refreshRateIndexToHz(proposedSize(), proposedRefreshRate()) <= 0 ) {
			m_proposedRefreshRate = 0;
		}
		status = XRRSetScreenConfigAndRate(QX11Info::display(), m_config, rootWindow(), (SizeID)proposedSize(), (Rotation)proposedRotation(), refreshRateIndexToHz(proposedSize(), proposedRefreshRate()), CurrentTime);
	}

	//kDebug() << "New size: " << WidthOfScreen(ScreenOfDisplay(QPaintDevice::x11AppDisplay(), screen)) << ", " << HeightOfScreen(ScreenOfDisplay(QPaintDevice::x11AppDisplay(), screen));

	if (status == RRSetConfigSuccess) {
		m_currentSize = m_proposedSize;
		m_currentRotation = m_proposedRotation;
		m_currentRefreshRate = m_proposedRefreshRate;
		return true;
	}

	return false;
}

bool LegacyRandRScreen::applyProposedAndConfirm()
{
	if (proposedChanged()) {
		setOriginal();

		if (applyProposed()) {
			if (!RandR::confirm()) {
				proposeOriginal();
				applyProposed();
				return false;
			}
		} else {
			return false;
		}
	}

	return true;
}

Window LegacyRandRScreen::rootWindow() const
{
	return RootWindow(QX11Info::display(), m_screen);
}

QString LegacyRandRScreen::changedMessage() const
{
	if (refreshRate() == -1)
		return i18n("New configuration:\nResolution: %1 x %2\nOrientation: %3",
			 currentPixelSize().width(),
			 currentPixelSize().height(),
			 currentRotationDescription());
	else
		return i18n("New configuration:\nResolution: %1 x %2\nOrientation: %3\nRefresh rate: %4",
			 currentPixelSize().width(),
			 currentPixelSize().height(),
			 currentRotationDescription(),
			 currentRefreshRateDescription());
}

bool LegacyRandRScreen::changedFromOriginal() const
{
	return m_currentSize != m_originalSize || m_currentRotation != m_originalRotation || m_currentRefreshRate != m_originalRefreshRate;
}

void LegacyRandRScreen::proposeOriginal()
{
	m_proposedSize = m_originalSize;
	m_proposedRotation = m_originalRotation;
	m_proposedRefreshRate = m_originalRefreshRate;
}

bool LegacyRandRScreen::proposedChanged() const
{
	return m_currentSize != m_proposedSize || m_currentRotation != m_proposedRotation || m_currentRefreshRate != m_proposedRefreshRate;
}

QString LegacyRandRScreen::currentRotationDescription() const
{
	QString ret = RandR::rotationName(m_currentRotation & RandR::RotateMask);

	if (m_currentRotation != (m_currentRotation & RandR::RotateMask)) {
		if (m_currentRotation & RR_Rotate_0)
			ret = RandR::rotationName(m_currentRotation & (RR_Reflect_X + RR_Reflect_X), true, true);
		else
			ret += ", " + RandR::rotationName(m_currentRotation & (RR_Reflect_X + RR_Reflect_X), true, false);
	}
	
	return ret;
}

int LegacyRandRScreen::rotationIndexToDegree(int rotation) const
{
	switch (rotation & RandR::RotateMask) {
		case RR_Rotate_90:
			return 90;

		case RR_Rotate_180:
			return 180;

		case RR_Rotate_270:
			return 270;

		default:
			return 0;
	}
}

int LegacyRandRScreen::rotationDegreeToIndex(int degree) const
{
	switch (degree) {
		case 90:
			return RR_Rotate_90;

		case 180:
			return RR_Rotate_180;

		case 270:
			return RR_Rotate_270;

		default:
			return RR_Rotate_0;
	}
}

const QSize& LegacyRandRScreen::currentPixelSize() const
{
	return m_pixelSizes[m_currentSize];
}

RateList LegacyRandRScreen::refreshRates(int size) const
{
	int nrates;
	short* rrates = XRRRates(QX11Info::display(), m_screen, (SizeID)size, &nrates);

	RateList rateList;
	for (int i = 0; i < nrates; i++)
		rateList.append(rrates[i]);

	return rateList;
}

QString LegacyRandRScreen::refreshRateDirectDescription(int rate) const
{
	return i18nc("Refresh rate in Hertz (Hz)", "%1 Hz", rate);
}

QString LegacyRandRScreen::refreshRateIndirectDescription(int size, int index) const
{
	return i18nc("Refresh rate in Hertz (Hz)", "%1 Hz", refreshRateIndexToHz(size, index));
}

QString LegacyRandRScreen::refreshRateDescription(int size, int index) const
{
	return ki18n("%1 Hz").subs(refreshRates(size)[index], 0, 'f', 1).toString();
}

bool LegacyRandRScreen::proposeRefreshRate(int index)
{
	if (index >= 0 && (int)refreshRates(proposedSize()).count() > index) {
		m_proposedRefreshRate = index;
		return true;
	}

	return false;
}

int LegacyRandRScreen::refreshRate() const
{
	return m_currentRefreshRate;
}

QString LegacyRandRScreen::currentRefreshRateDescription() const
{
	return refreshRateIndirectDescription(m_currentSize, m_currentRefreshRate);
}

int LegacyRandRScreen::proposedRefreshRate() const
{
	return m_proposedRefreshRate;
}

int LegacyRandRScreen::refreshRateHzToIndex(int size, int hz) const
{
	int nrates;
	short* rates = XRRRates(QX11Info::display(), m_screen, (SizeID)size, &nrates);

	for (int i = 0; i < nrates; i++)
		if (hz == rates[i])
			return i;

	if (nrates != 0)
		// Wrong input Hz!
		Q_ASSERT(false);

	return -1;
}

int LegacyRandRScreen::refreshRateIndexToHz(int size, int index) const
{
	int nrates;
	short* rates = XRRRates(QX11Info::display(), m_screen, (SizeID)size, &nrates);

	if (nrates == 0 || index < 0)
		return 0;

	// Wrong input Hz!
	if(index >= nrates)
		return 0;

	return rates[index];
}

int LegacyRandRScreen::numSizes() const
{
	return m_pixelSizes.count();
}

const QSize& LegacyRandRScreen::pixelSize(int index) const
{
	return m_pixelSizes[index];
}

const QSize& LegacyRandRScreen::mmSize(int index) const
{
	return m_mmSizes[index];
}

int LegacyRandRScreen::sizeIndex(const QSize &pixelSize) const
{
	for (int i = 0; i < m_pixelSizes.count(); i++)
		if (m_pixelSizes[i] == pixelSize)
			return i;

	return -1;
}

int LegacyRandRScreen::rotations() const
{
	return m_rotations;
}

int LegacyRandRScreen::rotation() const
{
	return m_currentRotation;
}

int LegacyRandRScreen::size() const
{
	return m_currentSize;
}

int LegacyRandRScreen::proposedRotation() const
{
	return m_proposedRotation;
}

void LegacyRandRScreen::proposeRotation(int newRotation)
{
	m_proposedRotation = newRotation & RandR::OrientationMask;
}

int LegacyRandRScreen::proposedSize() const
{
	return m_proposedSize;
}

bool LegacyRandRScreen::proposeSize(int newSize)
{
	if ((int)m_pixelSizes.count() > newSize) {
		m_proposedSize = newSize;
		return true;
	}

	return false;
}

void LegacyRandRScreen::load(KConfig& config)
{
	KConfigGroup group = config.group(QString("Screen%1").arg(m_screen));

	if (proposeSize(sizeIndex(group.readEntry("size", currentPixelSize()))))
		proposeRefreshRate(refreshRateHzToIndex(proposedSize(), group.readEntry("refresh", refreshRate())));

	proposeRotation(rotationDegreeToIndex(	group.readEntry("rotation", 0)) + 
						(group.readEntry("reflectX", false) ? RandR::ReflectX : 0) + 
						(group.readEntry("reflectY",false) ? RandR::ReflectY : 0));
}

void LegacyRandRScreen::save(KConfig& config) const
{
	KConfigGroup group = config.group(QString("Screen%1").arg(m_screen));
	group.writeEntry("size", currentPixelSize());
	group.writeEntry("refresh", refreshRateIndexToHz(size(), refreshRate()));
	group.writeEntry("rotation", rotationIndexToDegree(rotation()));
	group.writeEntry("reflectX", (bool)(rotation() & RandR::ReflectMask) == RandR::ReflectX);
	group.writeEntry("reflectY", (bool)(rotation() & RandR::ReflectMask) == RandR::ReflectY);
}

int LegacyRandRScreen::pixelCount( int index ) const
{
	QSize sz = pixelSize(index);
	return sz.width() * sz.height();
}

SizeList LegacyRandRScreen::pixelSizes() const
{
	return m_pixelSizes;
}

#include "legacyrandrscreen.moc"
