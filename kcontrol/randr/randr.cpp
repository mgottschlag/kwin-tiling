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

#include <kactivelabel.h>

#include <kdebug.h>
#include <klocale.h>
#include <kglobal.h>
#include <kapplication.h>
#include <kiconloader.h>
#include <dcopclient.h>
#include <kipc.h>

#include "randr.h"

#include "ktimerdialog.h"

RandRScreen::RandRScreen(int screenIndex, bool requestScreenChangeEvents)
	: config(0L)
	, screen(screenIndex)
{
	root = RootWindow(qt_xdisplay(), screen);
	if (requestScreenChangeEvents)
		XRRSelectInput(qt_xdisplay(), root, True);
	
	loadSettings();
	setOriginal();
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
	currentSize = proposedSize = XRRConfigCurrentConfiguration(config, &rotation);
	currentRotation = proposedRotation = rotation;

	int numSizes;
	XRRScreenSize* firstSize = XRRSizes(qt_xdisplay(), screen, &numSizes);
	sizes.duplicate(firstSize, numSizes);

	rotations = XRRRotations(qt_xdisplay(), screen, &rotation);
	
	currentRefreshRate = proposedRefreshRate = XRRConfigCurrentRate(config);
}

void RandRScreen::setOriginal()
{
	originalSize = currentSize;
	originalRotation = currentRotation;
	originalRefreshRate = currentRefreshRate;
}

Status RandRScreen::applyProposed()
{
	//kdDebug() << k_funcinfo << " size " << proposedSize << ", rotation " << proposedRotation << ", refresh " << proposedRefreshRate << endl;
	
	Status status = XRRSetScreenConfigAndRate(qt_xdisplay(), config, DefaultRootWindow (qt_xdisplay()), proposedSize, proposedRotation, proposedRefreshRate, CurrentTime);
	
	// Restart kicker if its size should have changed
	if (proposedSize != currentSize || (proposedRotation & 15) != (currentRotation & 15)) {
		DCOPClient* dcop = KApplication::kApplication()->dcopClient();
		if (dcop->isApplicationRegistered("kicker"))
			dcop->send("kicker", "kicker", "restart()", "");
	}
	
	if (status == RRSetConfigSuccess) {
		currentSize = proposedSize;
		currentRotation = proposedRotation;
		currentRefreshRate = proposedRefreshRate;
	}
	
	return status;
}

Status RandRScreen::applyProposedAndConfirm()
{
	Status ret = RRSetConfigSuccess;
	if (proposedChanged()) {
		setOriginal();
		
		ret = applyProposed();
		switch (ret) {
			case RRSetConfigSuccess:
				if (!confirm()) {
					proposeOriginal();
					applyProposed();
					ret = -1;
				}
				break;
			case RRSetConfigInvalidConfigTime:
			case RRSetConfigInvalidTime:
			case RRSetConfigFailed:
				break;
		}
	}
	return ret;
}

bool RandRScreen::confirm()
{
	// uncomment the line below and edit out the KTimerDialog stuff to get
	// a version which works on today's kdelibs (no accept dialog is presented)

	// REMEMBER to put the dialog on the right screen

	KTimerDialog *acceptDialog = new KTimerDialog(
											15000,
											KTimerDialog::CountDown,
											KApplication::kApplication()->mainWidget(),
											"mainKTimerDialog",
											true,
											i18n("Confirm display setting change"),
											KTimerDialog::Ok|KTimerDialog::Cancel,
											KTimerDialog::Cancel);

	acceptDialog->setButtonOKText(i18n("Accept configuration"));
	acceptDialog->setButtonCancelText(i18n("Return to previous configuration"));
	
	KDialog::centerOnScreen(acceptDialog, screen);
	
	KActiveLabel *label = new KActiveLabel(i18n("Your screen orientation, size and refresh rate have been changed to the requested settings. Please indicate whether you wish to keep this configuration. In 15 seconds your configuration will revert to normal."), acceptDialog, "userSpecifiedLabel");

	acceptDialog->setMainWidget(label);

	return acceptDialog->exec();
}

QString RandRScreen::changedMessage()
{
	return i18n("New configuration:\nResolution: %1 x %1\nOrientation: %1\nRefresh Rate: %1").arg(currentWidth()).arg(currentHeight()).arg(currentRotationDescription()).arg(currentRefreshRateDescription());
}

bool RandRScreen::changedFromOriginal()
{
	return currentSize != originalSize || currentRotation != originalRotation || currentRefreshRate != originalRefreshRate;
}

void RandRScreen::proposeOriginal()
{
	proposedSize = originalSize;
	proposedRotation = originalRotation;
	proposedRefreshRate = originalRefreshRate;
}

bool RandRScreen::proposedChanged()
{
	return currentSize != proposedSize || currentRotation != proposedRotation || currentRefreshRate != proposedRefreshRate;
}

QString RandRScreen::rotationName(int rotation, bool pastTense, bool capitalised)
{
	if (!pastTense)
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
			return i18n("unknown orientation");
	}
	
	switch (rotation) {
		case RR_Rotate_0:
			return i18n("Normal");
		case RR_Rotate_90:
			return i18n("Rotated 90 degrees anticlockwise");
		case RR_Rotate_180:
			return i18n("Rotated 180 degrees anticlockwise");
		case RR_Rotate_270:
			return i18n("Rotated 270 degrees anticlockwise");
		default:
			if (rotation & RR_Reflect_X)
				if (rotation & RR_Reflect_Y)
					if (capitalised)
						return i18n("Mirrored horizontally and vertically");
					else
						return i18n("mirrored horizontally and vertically");
				else
					if (capitalised)
						return i18n("Mirrored horizontally");
					else
						return i18n("Mirrored horizontally");
			else if (rotation & RR_Reflect_Y)
				if (capitalised)
					return i18n("Mirrored vertically");
				else
					return i18n("mirrored vertically");
			else
				return i18n("unknown orientation");
	}
}

QPixmap RandRScreen::rotationIcon(int rotation)
{
	switch (rotation) {
		case RR_Rotate_0:
			return SmallIcon("up");
		case RR_Rotate_90:
			return SmallIcon("forward");
		case RR_Rotate_180:
			return SmallIcon("down");
		case RR_Rotate_270:
			return SmallIcon("back");
		case RR_Reflect_X:
		case RR_Reflect_Y:
		default:
			return SmallIcon("stop");
	}
}

QString RandRScreen::currentRotationDescription()
{
	QString ret = rotationName(currentRotation & 15);
	
	if (currentRotation != currentRotation & 15)
		if (currentRotation & RR_Rotate_0)
			ret = rotationName(currentRotation & (RR_Reflect_X + RR_Reflect_X), true, true);
		else
			ret += ", " + rotationName(currentRotation & (RR_Reflect_X + RR_Reflect_X), true, false);
	
	return ret;
}

int RandRScreen::currentWidth()
{
	return sizes[currentSize].width;
}

int RandRScreen::currentHeight()
{
	return sizes[currentSize].height;
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

QString RandRScreen::currentRefreshRateDescription()
{
	QStringList rr = refreshRates(currentSize);
	return rr[proposedRefreshRateIndex];
}

void RandRScreen::proposeRefreshRate(QString rateString)
{
	QStringList temp = QStringList::split(' ', rateString);
	proposedRefreshRate = temp.first().toShort();
}

RandRDisplay::RandRDisplay(bool requestScreenChangeEvents)
	: m_valid(true)
{
	// Check extension
	Status s = XRRQueryExtension(qt_xdisplay(), &m_eventBase, &m_errorBase);
	if (!s) {
		m_errorCode = QString("%1, base %1").arg(s).arg(m_errorBase);
		m_valid = false;
		return;
	}
	
	int major_version, minor_version;
	XRRQueryVersion(qt_xdisplay(), &major_version, &minor_version);
	
	m_version = QString("X Resize and Rotate extension version %1.%1").arg(major_version).arg(minor_version);

	m_numScreens = QApplication::desktop()->numScreens();
	// Haven't tested this assumption, it's here to remind me to test it
	Q_ASSERT(QApplication::desktop()->numScreens() == ScreenCount(qt_xdisplay()));
	
	m_screens.setAutoDelete(true);
	for (int i = 0; i < m_numScreens; i++) {
		m_screens.append(new RandRScreen(i, requestScreenChangeEvents));
	}
	
	setScreen(QApplication::desktop()->primaryScreen());
}

void RandRDisplay::setScreen(int index)
{
	m_currentScreenIndex = index;
	m_currentScreen = m_screens.at(m_currentScreenIndex);
	Q_ASSERT(m_currentScreen);
}

int RandRDisplay::widgetScreen(QWidget* widget)
{
	return QApplication::desktop()->screenNumber(widget);
}

void RandRDisplay::refresh()
{
	for (RandRScreen* s = m_screens.first(); s; s = m_screens.next())
		s->loadSettings();
}
