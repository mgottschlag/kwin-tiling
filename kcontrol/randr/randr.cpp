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

#include <qtimer.h>

#include <kdebug.h>
#include <klocale.h>
#include <kglobal.h>
#include <kapplication.h>
#include <kiconloader.h>
#include <dcopclient.h>
#include <kipc.h>

#include "randr.h"

#include "ktimerdialog.h"

RandRScreen::RandRScreen(int screenIndex)
	: m_config(0L)
	, m_screen(screenIndex)
	, m_shownDialog(NULL)
{
	m_root = RootWindow(qt_xdisplay(), m_screen);

	loadSettings();
	setOriginal();
}

RandRScreen::~RandRScreen()
{
	if (m_config)
		XRRFreeScreenConfigInfo(m_config);
}

void RandRScreen::loadSettings()
{
	m_config = XRRGetScreenInfo(qt_xdisplay(), m_root);
	Q_ASSERT(m_config);

	Rotation rotation;
	m_currentSize = m_proposedSize = XRRConfigCurrentConfiguration(m_config, &rotation);
	m_currentRotation = m_proposedRotation = rotation;

	int numSizes;
	XRRScreenSize* firstSize = XRRSizes(qt_xdisplay(), m_screen, &numSizes);
	m_sizes.duplicate(firstSize, numSizes);

	m_rotations = XRRRotations(qt_xdisplay(), m_screen, &rotation);

	m_currentRefreshRate = m_proposedRefreshRate = refreshRateHzToIndex(m_currentSize, XRRConfigCurrentRate(m_config));
}

void RandRScreen::setOriginal()
{
	m_originalSize = m_currentSize;
	m_originalRotation = m_currentRotation;
	m_originalRefreshRate = m_currentRefreshRate;
}

Status RandRScreen::applyProposed()
{
	//kdDebug() << k_funcinfo << " size " << proposedSize << ", rotation " << proposedRotation << ", refresh " << refreshRateIndexToHz(m_proposedSize, proposedRefreshRate) << endl;

	Status status = XRRSetScreenConfigAndRate(qt_xdisplay(), m_config, DefaultRootWindow (qt_xdisplay()), m_proposedSize, m_proposedRotation, refreshRateIndexToHz(m_proposedSize, m_proposedRefreshRate), CurrentTime);

	//kdDebug() << "New size: " << WidthOfScreen(ScreenOfDisplay(QPaintDevice::x11AppDisplay(), screen)) << ", " << HeightOfScreen(ScreenOfDisplay(QPaintDevice::x11AppDisplay(), screen)) << endl;

	if (status == RRSetConfigSuccess) {
		m_currentSize = m_proposedSize;
		m_currentRotation = m_proposedRotation;
		m_currentRefreshRate = m_proposedRefreshRate;
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

	// FIXME remember to put the dialog on the right screen

	KTimerDialog *acceptDialog = new KTimerDialog(
											15000,
											KTimerDialog::CountDown,
											KApplication::kApplication()->mainWidget(),
											"mainKTimerDialog",
											true,
											i18n("Confirm Display Setting Change"),
											KTimerDialog::Ok|KTimerDialog::Cancel,
											KTimerDialog::Cancel);

	acceptDialog->setButtonOKText(i18n("Accept Configuration"));
	acceptDialog->setButtonCancelText(i18n("Return to Previous Configuration"));

	KActiveLabel *label = new KActiveLabel(i18n("Your screen orientation, size and refresh rate have been changed to the requested settings. Please indicate whether you wish to keep this configuration. In 15 seconds your configuration will revert to normal."), acceptDialog, "userSpecifiedLabel");

	acceptDialog->setMainWidget(label);

	KDialog::centerOnScreen(acceptDialog, m_screen);

	m_shownDialog = acceptDialog;
	connect( m_shownDialog, SIGNAL( destroyed()), this, SLOT( shownDialogDestroyed()));
	connect( kapp->desktop(), SIGNAL( resized()), this, SLOT( desktopResized()));

    return acceptDialog->exec();
}

void RandRScreen::shownDialogDestroyed()
{
    m_shownDialog = NULL;
    disconnect( kapp->desktop(), SIGNAL( resized()), this, SLOT( desktopResized()));
}

void RandRScreen::desktopResized()
{
    if( m_shownDialog != NULL )
	KDialog::centerOnScreen(m_shownDialog, m_screen);
}

QString RandRScreen::changedMessage() const
{
	return i18n("New configuration:\nResolution: %1 x %1\nOrientation: %1\nRefresh rate: %1").arg(currentWidth()).arg(currentHeight()).arg(currentRotationDescription()).arg(currentRefreshRateDescription());
}

bool RandRScreen::changedFromOriginal() const
{
	return m_currentSize != m_originalSize || m_currentRotation != m_originalRotation || m_currentRefreshRate != m_originalRefreshRate;
}

void RandRScreen::proposeOriginal()
{
	m_proposedSize = m_originalSize;
	m_proposedRotation = m_originalRotation;
	m_proposedRefreshRate = m_originalRefreshRate;
}

bool RandRScreen::proposedChanged() const
{
	return m_currentSize != m_proposedSize || m_currentRotation != m_proposedRotation || m_currentRefreshRate != m_proposedRefreshRate;
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
			return i18n("Mirror horizontally");
		case RR_Reflect_Y:
			return i18n("Mirror vertically");
		default:
			return i18n("Unknown orientation");
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

QPixmap RandRScreen::rotationIcon(int rotation) const
{
	// Adjust icons for current screen orientation
	// FIXME untested, this might even be working the wrong way
	if (!(m_currentRotation & RR_Rotate_0) && rotation & (RR_Rotate_0 | RR_Rotate_90 | RR_Rotate_180 | RR_Rotate_270)) {
		int currentAngle = m_currentRotation & (RR_Rotate_90 | RR_Rotate_180 | RR_Rotate_270);
		switch (currentAngle) {
			case RR_Rotate_90:
				rotation <<= 1;
				break;
			case RR_Rotate_180:
				rotation <<= 2;
				break;
			case RR_Rotate_270:
				rotation <<= 3;
				break;
		}

		// Fix overflow
		if (rotation > RR_Rotate_270) {
			rotation >>= 4;
		}
	}

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

QString RandRScreen::currentRotationDescription() const
{
	QString ret = rotationName(m_currentRotation & 15);

	if (m_currentRotation != m_currentRotation & 15)
		if (m_currentRotation & RR_Rotate_0)
			ret = rotationName(m_currentRotation & (RR_Reflect_X + RR_Reflect_X), true, true);
		else
			ret += ", " + rotationName(m_currentRotation & (RR_Reflect_X + RR_Reflect_X), true, false);

	return ret;
}

int RandRScreen::currentWidth() const
{
	return m_sizes[m_currentSize].width;
}

int RandRScreen::currentHeight() const
{
	return m_sizes[m_currentSize].height;
}

QStringList RandRScreen::refreshRates(SizeID size) const
{
	int nrates;
	short* rates = XRRRates(qt_xdisplay(), m_screen, size, &nrates);

	QStringList ret;
	for (int i = 0; i < nrates; i++)
		ret << refreshRateDirectDescription(rates[i]);

	return ret;
}

QString RandRScreen::refreshRateDirectDescription(int rate) const
{
	return i18n("Refresh rate in Hertz (Hz)", "%1 Hz").arg(rate);
}

QString RandRScreen::refreshRateDescription(int index) const
{
	return refreshRates(m_proposedSize)[index];
}

void RandRScreen::proposeRefreshRate(int index)
{
	m_proposedRefreshRate = index;
}

int RandRScreen::currentRefreshRate() const
{
	return m_currentRefreshRate;
}

QString RandRScreen::currentRefreshRateDescription() const
{
	return refreshRateDirectDescription(m_currentRefreshRate);
}

int RandRScreen::proposedRefreshRate() const
{
	return m_proposedRefreshRate;
}

int RandRScreen::refreshRateHzToIndex(SizeID size, int hz) const
{
	int nrates;
	short* rates = XRRRates(qt_xdisplay(), m_screen, size, &nrates);

	for (int i = 0; i < nrates; i++)
		if (hz == rates[i])
			return i;

	// Wrong input Hz!
	Q_ASSERT(false);
	return 0;
}

int RandRScreen::refreshRateIndexToHz(SizeID size, int index) const
{
	int nrates;
	short* rates = XRRRates(qt_xdisplay(), m_screen, size, &nrates);

	// Wrong input Hz!
	Q_ASSERT(index < nrates);

	return rates[index];
}

int RandRScreen::numSizes() const
{
	return m_sizes.count();
}

const XRRScreenSize& RandRScreen::size(int index) const
{
	return m_sizes[index];
}

const Rotation RandRScreen::rotations() const
{
	return m_rotations;
}

Rotation RandRScreen::currentRotation() const
{
	return m_currentRotation;
}

SizeID RandRScreen::currentSize() const
{
	return m_currentSize;
}

Rotation RandRScreen::proposedRotation() const
{
	return m_proposedRotation;
}

void RandRScreen::proposeRotation(Rotation newRotation)
{
	m_proposedRotation = newRotation;
}

SizeID RandRScreen::proposedSize() const
{
	return m_proposedSize;
}

void RandRScreen::proposeSize(SizeID newSize)
{
	m_proposedSize = newSize;
}

RandRDisplay::RandRDisplay()
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
		m_screens.append(new RandRScreen(i));
	}

	setCurrentScreen(QApplication::desktop()->primaryScreen());
}

bool RandRDisplay::isValid() const
{
	return m_valid;
}

const QString& RandRDisplay::errorCode() const
{
	return m_errorCode;
}

int RandRDisplay::eventBase() const
{
	return m_eventBase;
}

int RandRDisplay::errorBase() const
{
	return m_errorBase;
}

const QString& RandRDisplay::version() const
{
	return m_version;
}

void RandRDisplay::setCurrentScreen(int index)
{
	m_currentScreenIndex = index;
	m_currentScreen = m_screens.at(m_currentScreenIndex);
	Q_ASSERT(m_currentScreen);
}

int RandRDisplay::screenIndexOfWidget(QWidget* widget)
{
	return QApplication::desktop()->screenNumber(widget);
}

int RandRDisplay::currentScreenIndex() const
{
	return m_currentScreenIndex;
}

void RandRDisplay::refresh()
{
	for (RandRScreen* s = m_screens.first(); s; s = m_screens.next())
		s->loadSettings();
}

int RandRDisplay::numScreens() const
{
	return m_numScreens;
}

RandRScreen* RandRDisplay::screen(int index)
{
	return m_screens.at(index);
}

RandRScreen* RandRDisplay::currentScreen()
{
	return m_currentScreen;
}

#include "randr.moc"
