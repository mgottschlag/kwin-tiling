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
#include "oldrandrscreen.h"

OldRandRScreen::OldRandRScreen(int screenIndex)
	: m_config(0L)
	, m_screen(screenIndex)
	, m_shownDialog(NULL)
{
	loadSettings();
	setOriginal();
}

OldRandRScreen::~OldRandRScreen()
{
	if (m_config)
		XRRFreeScreenConfigInfo(m_config);
}

void OldRandRScreen::loadSettings()
{
	if (m_config)
		XRRFreeScreenConfigInfo(m_config);

	m_config = XRRGetScreenInfo(QX11Info::display(), RootWindow(QX11Info::display(), m_screen));
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

void OldRandRScreen::setOriginal()
{
	m_originalSize = m_currentSize;
	m_originalRotation = m_currentRotation;
	m_originalRefreshRate = m_currentRefreshRate;
}

bool OldRandRScreen::applyProposed()
{
	//kDebug() << k_funcinfo << " size " << (SizeID)proposedSize() << ", rotation " << proposedRotation() << ", refresh " << refreshRateIndexToHz(proposedSize(), proposedRefreshRate()) << endl;

	Status status;

	if (proposedRefreshRate() < 0)
		status = XRRSetScreenConfig(QX11Info::display(), m_config, DefaultRootWindow(QX11Info::display()), (SizeID)proposedSize(), (Rotation)proposedRotation(), CurrentTime);
	else {
		if( refreshRateIndexToHz(proposedSize(), proposedRefreshRate()) <= 0 ) {
			m_proposedRefreshRate = 0;
		}
		status = XRRSetScreenConfigAndRate(QX11Info::display(), m_config, DefaultRootWindow(QX11Info::display()), (SizeID)proposedSize(), (Rotation)proposedRotation(), refreshRateIndexToHz(proposedSize(), proposedRefreshRate()), CurrentTime);
	}

	//kDebug() << "New size: " << WidthOfScreen(ScreenOfDisplay(QPaintDevice::x11AppDisplay(), screen)) << ", " << HeightOfScreen(ScreenOfDisplay(QPaintDevice::x11AppDisplay(), screen)) << endl;

	if (status == RRSetConfigSuccess) {
		m_currentSize = m_proposedSize;
		m_currentRotation = m_proposedRotation;
		m_currentRefreshRate = m_proposedRefreshRate;
		return true;
	}

	return false;
}

bool OldRandRScreen::applyProposedAndConfirm()
{
	if (proposedChanged()) {
		setOriginal();

		if (applyProposed()) {
			if (!confirm()) {
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

bool OldRandRScreen::confirm()
{
	// uncomment the line below and edit out the KTimerDialog stuff to get
	// a version which works on today's kdelibs (no accept dialog is presented)

	// FIXME remember to put the dialog on the right screen

	KTimerDialog acceptDialog(
											15000,
											KTimerDialog::CountDown,
											KApplication::kApplication()->mainWidget(),
											"mainKTimerDialog",
											true,
											i18n("Confirm Display Setting Change"),
											KTimerDialog::Ok|KTimerDialog::Cancel,
											KTimerDialog::Cancel);

	acceptDialog.setButtonGuiItem(KDialog::Ok, KGuiItem(i18n("&Accept Configuration"), "dialog-ok"));
	acceptDialog.setButtonGuiItem(KDialog::Cancel, KGuiItem(i18n("&Return to Previous Configuration"), "dialog-cancel"));

	QLabel *label = new QLabel(i18n("Your screen orientation, size and refresh rate have been "
                    "changed to the requested settings. Please indicate whether you wish to keep "
                    "this configuration. In 15 seconds the display will revert to your previous "
                    "settings."), &acceptDialog);
        acceptDialog.setMainWidget(label);

	KDialog::centerOnScreen(&acceptDialog, m_screen);

	m_shownDialog = &acceptDialog;
	connect( m_shownDialog, SIGNAL( destroyed()), this, SLOT( shownDialogDestroyed()));
	connect( kapp->desktop(), SIGNAL( resized(int)), this, SLOT( desktopResized()));

    return acceptDialog.exec();
}

void OldRandRScreen::shownDialogDestroyed()
{
    m_shownDialog = NULL;
    disconnect( kapp->desktop(), SIGNAL( resized(int)), this, SLOT( desktopResized()));
}

void OldRandRScreen::desktopResized()
{
	if( m_shownDialog != NULL )
		KDialog::centerOnScreen(m_shownDialog, m_screen);
}

QString OldRandRScreen::changedMessage() const
{
	if (currentRefreshRate() == -1)
		return i18n("New configuration:\nResolution: %1 x %2\nOrientation: %3",
			 currentPixelWidth(),
			 currentPixelHeight(),
			 currentRotationDescription());
	else
		return i18n("New configuration:\nResolution: %1 x %2\nOrientation: %3\nRefresh rate: %4",
			 currentPixelWidth(),
			 currentPixelHeight(),
			 currentRotationDescription(),
			 currentRefreshRateDescription());
}

bool OldRandRScreen::changedFromOriginal() const
{
	return m_currentSize != m_originalSize || m_currentRotation != m_originalRotation || m_currentRefreshRate != m_originalRefreshRate;
}

void OldRandRScreen::proposeOriginal()
{
	m_proposedSize = m_originalSize;
	m_proposedRotation = m_originalRotation;
	m_proposedRefreshRate = m_originalRefreshRate;
}

bool OldRandRScreen::proposedChanged() const
{
	return m_currentSize != m_proposedSize || m_currentRotation != m_proposedRotation || m_currentRefreshRate != m_proposedRefreshRate;
}

QString OldRandRScreen::currentRotationDescription() const
{
	QString ret = RandR::rotationName(m_currentRotation & RandR::RotateMask);

	if (m_currentRotation != (m_currentRotation & RandR::RotateMask))
		if (m_currentRotation & RR_Rotate_0)
			ret = RandR::rotationName(m_currentRotation & (RR_Reflect_X + RR_Reflect_X), true, true);
		else
			ret += ", " + RandR::rotationName(m_currentRotation & (RR_Reflect_X + RR_Reflect_X), true, false);

	return ret;
}

int OldRandRScreen::rotationIndexToDegree(int rotation) const
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

int OldRandRScreen::rotationDegreeToIndex(int degree) const
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

int OldRandRScreen::currentPixelWidth() const
{
	return m_pixelSizes[m_currentSize].width();
}

int OldRandRScreen::currentPixelHeight() const
{
	return m_pixelSizes[m_currentSize].height();
}

int OldRandRScreen::currentMMWidth() const
{
	return m_pixelSizes[m_currentSize].width();
}

int OldRandRScreen::currentMMHeight() const
{
	return m_pixelSizes[m_currentSize].height();
}

QStringList OldRandRScreen::refreshRates(int size) const
{
	int nrates;
	short* rates = XRRRates(QX11Info::display(), m_screen, (SizeID)size, &nrates);

	QStringList ret;
	for (int i = 0; i < nrates; i++)
		ret << refreshRateDirectDescription(rates[i]);

	return ret;
}

QString OldRandRScreen::refreshRateDirectDescription(int rate) const
{
	return i18nc("Refresh rate in Hertz (Hz)", "%1 Hz", rate);
}

QString OldRandRScreen::refreshRateIndirectDescription(int size, int index) const
{
	return i18nc("Refresh rate in Hertz (Hz)", "%1 Hz", refreshRateIndexToHz(size, index));
}

QString OldRandRScreen::refreshRateDescription(int size, int index) const
{
	return refreshRates(size)[index];
}

bool OldRandRScreen::proposeRefreshRate(int index)
{
	if (index >= 0 && (int)refreshRates(proposedSize()).count() > index) {
		m_proposedRefreshRate = index;
		return true;
	}

	return false;
}

int OldRandRScreen::currentRefreshRate() const
{
	return m_currentRefreshRate;
}

QString OldRandRScreen::currentRefreshRateDescription() const
{
	return refreshRateIndirectDescription(m_currentSize, m_currentRefreshRate);
}

int OldRandRScreen::proposedRefreshRate() const
{
	return m_proposedRefreshRate;
}

int OldRandRScreen::refreshRateHzToIndex(int size, int hz) const
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

int OldRandRScreen::refreshRateIndexToHz(int size, int index) const
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

int OldRandRScreen::numSizes() const
{
	return m_pixelSizes.count();
}

const QSize& OldRandRScreen::pixelSize(int index) const
{
	return m_pixelSizes[index];
}

const QSize& OldRandRScreen::mmSize(int index) const
{
	return m_mmSizes[index];
}

int OldRandRScreen::sizeIndex(QSize pixelSize) const
{
	for (int i = 0; i < m_pixelSizes.count(); i++)
		if (m_pixelSizes[i] == pixelSize)
			return i;

	return -1;
}

int OldRandRScreen::rotations() const
{
	return m_rotations;
}

int OldRandRScreen::currentRotation() const
{
	return m_currentRotation;
}

int OldRandRScreen::currentSize() const
{
	return m_currentSize;
}

int OldRandRScreen::proposedRotation() const
{
	return m_proposedRotation;
}

void OldRandRScreen::proposeRotation(int newRotation)
{
	m_proposedRotation = newRotation & RandR::OrientationMask;
}

int OldRandRScreen::proposedSize() const
{
	return m_proposedSize;
}

bool OldRandRScreen::proposeSize(int newSize)
{
	if ((int)m_pixelSizes.count() > newSize) {
		m_proposedSize = newSize;
		return true;
	}

	return false;
}

void OldRandRScreen::load(KConfig& config)
{
	KConfigGroup group = config.group(QString("Screen%1").arg(m_screen));

	if (proposeSize(sizeIndex(QSize(group.readEntry("width", currentPixelWidth()), group.readEntry("height", currentPixelHeight())))))
		proposeRefreshRate(refreshRateHzToIndex(proposedSize(), group.readEntry("refresh", currentRefreshRate())));

	proposeRotation(rotationDegreeToIndex(	group.readEntry("rotation", 0)) + 
						(group.readEntry("reflectX", false) ? RandR::ReflectX : 0) + 
						(group.readEntry("reflectY",false) ? RandR::ReflectY : 0));
}

void OldRandRScreen::save(KConfig& config) const
{
	KConfigGroup group = config.group(QString("Screen%1").arg(m_screen));
	group.writeEntry("width", currentPixelWidth());
	group.writeEntry("height", currentPixelHeight());
	group.writeEntry("refresh", refreshRateIndexToHz(currentSize(), currentRefreshRate()));
	group.writeEntry("rotation", rotationIndexToDegree(currentRotation()));
	group.writeEntry("reflectX", (bool)(currentRotation() & RandR::ReflectMask) == RandR::ReflectX);
	group.writeEntry("reflectY", (bool)(currentRotation() & RandR::ReflectMask) == RandR::ReflectY);
}

int OldRandRScreen::pixelCount( int index ) const
{
	QSize sz = pixelSize(index);
	return sz.width() * sz.height();
}

#include "oldrandrscreen.moc"
