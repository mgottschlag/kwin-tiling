/* 
 * Copyright (c) 2002 Hamish Rodda <meddie@yoyo.its.monash.edu.au>
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
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <kdebug.h>
#include <klocale.h>
#include <kglobal.h>
#include <kapplication.h>
#include <dcopclient.h>

#include "randr.h"

RandRScreen::RandRScreen(int screenIndex)
	: config(0L)
	, screen(screenIndex)
{
	root = RootWindow(qt_xdisplay(), screen);

	loadSettings();
}

RandRScreen::~RandRScreen()
{
	if (config)
		XRRFreeScreenConfigInfo(config);
}

void RandRScreen::loadSettings()
{
	config = XRRGetScreenInfo(qt_xdisplay(), root);
	Q_ASSERT(config);

	Rotation rotation;
	originalSize = currentSize = proposedSize = XRRConfigCurrentConfiguration(config, &rotation);
	originalRotation = currentRotation = proposedRotation = rotation;

	int numSizes;
	XRRScreenSize* firstSize = XRRSizes(qt_xdisplay(), screen, &numSizes);
	sizes.duplicate(firstSize, numSizes);

	rotations = XRRRotations(qt_xdisplay(), screen, &rotation);
	
	originalRefreshRate = currentRefreshRate = proposedRefreshRate = XRRConfigCurrentRate(config);
}

Status RandRScreen::applyProposed()
{
	XRRSelectInput(qt_xdisplay(), root, True);

	//kdDebug() << k_funcinfo << " size " << proposedSize << ", rotation " << proposedRotation << ", refresh " << proposedRefreshRate << endl;
	
	Status status = XRRSetScreenConfigAndRate(qt_xdisplay(), config, DefaultRootWindow (qt_xdisplay()), proposedSize, proposedRotation, proposedRefreshRate, CurrentTime);
	
	// Restart kicker if its size should have changed
	if (proposedSize != currentSize || (proposedRotation & 15) != (currentRotation & 15)) {
		DCOPClient* dcop = KApplication::kApplication()->dcopClient();
		if (dcop->isApplicationRegistered("kicker"))
			dcop->send("kicker", "kicker", "restart()", "");
	}
	
	currentSize = proposedSize;
	currentRotation = proposedRotation;
	currentRefreshRate = proposedRefreshRate;
	
	return status;
}

bool RandRScreen::changedFromOriginal()
{
	return currentSize != originalSize || currentRotation != originalRotation || currentRefreshRate != originalRefreshRate;
}

void RandRScreen::proposeOriginal()
{
	currentSize = proposedSize;
	currentRotation = proposedRotation;
	currentRefreshRate = proposedRefreshRate;
}

bool RandRScreen::proposedChanged()
{
	return currentSize != proposedSize || currentRotation != proposedRotation || currentRefreshRate != proposedRefreshRate;
}

QString RandRScreen::rotationName(int rotation)
{
	switch (rotation) {
		case RR_Rotate_0:
			return i18n("Normal");
		case RR_Rotate_90:
			return i18n("Left (90 degrees)");
		case RR_Rotate_180:
			return i18n("Upside-down (180 degrees)");
		case RR_Rotate_270:
			return i18n("Right (270 degrees)");
		case RR_Reflect_X:
			return i18n("Mirror Horizontally");
		case RR_Reflect_Y:
			return i18n("Mirror Vertically");
		default:
			return i18n("Unknown rotation");
	}
}

QStringList RandRScreen::refreshRates(SizeID size)
{
	int nrates;
	short* rates = XRRRates(qt_xdisplay(), screen, size, &nrates);
	
	QStringList ret;
	for (int i = 0; i < nrates; i++) {
		ret << QString("%1 Hz").arg(rates[i]);
		if (rates[i] == proposedRefreshRate)
			proposedRefreshRateIndex = i;
	}
	
	return ret;
}

void RandRScreen::proposeRefreshRate(QString rateString)
{
	QStringList temp = QStringList::split(' ', rateString);
	proposedRefreshRate = temp.first().toShort();
}

RandRDisplay::RandRDisplay()
	: m_valid(true)
{
	// Check extension
	int event, error;
	if (!XRRQueryExtension(qt_xdisplay(), &event, &error)) {
		m_errorCode = QString("%1, %1").arg(event).arg(error);
		m_valid = false;
		return;
	}
	
	int major_version, minor_version;
	XRRQueryVersion(qt_xdisplay(), &major_version, &minor_version);
	
	m_version = QString("X Resize and Rotate extension version %1.%1").arg(major_version).arg(minor_version);

	m_numScreens = ScreenCount(qt_xdisplay());
	m_currentScreenIndex = DefaultScreen(qt_xdisplay());
	m_screens.setAutoDelete(true);
	for (int i = 0; i < m_numScreens; i++) {
		m_screens.append(new RandRScreen(i));
		if (i == m_currentScreenIndex)
			m_currentScreen = m_screens.last();
	}
}
