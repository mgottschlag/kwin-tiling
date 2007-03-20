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

#include "randr.h"

#include <QTimer>
//Added by qt3to4:
#include <QPixmap>

#include <kdebug.h>
#include <klocale.h>
#include <kglobal.h>
#include <kapplication.h>
#include <kiconloader.h>
#include <qlabel.h>
#include <QDesktopWidget>
#include "ktimerdialog.h"

#include <X11/Xlib.h>
#define INT8 _X11INT8
#define INT32 _X11INT32
#include <X11/Xproto.h>
#undef INT8
#undef INT32
#include <X11/extensions/Xrandr.h>
#include <QX11Info>

class RandRScreenPrivate
{
public:
	RandRScreenPrivate() : config(0L) {}
	~RandRScreenPrivate()
	{
		if (config)
			XRRFreeScreenConfigInfo(config);
	}

	XRRScreenConfiguration* config;
};

RandRScreen::RandRScreen(int screenIndex)
	: d(new RandRScreenPrivate())
	, m_screen(screenIndex)
	, m_shownDialog(NULL)
{
	loadSettings();
	setOriginal();
}

RandRScreen::~RandRScreen()
{
	delete d;
}

void RandRScreen::loadSettings()
{
	if (d->config)
		XRRFreeScreenConfigInfo(d->config);

	d->config = XRRGetScreenInfo(QX11Info::display(), RootWindow(QX11Info::display(), m_screen));
	Q_ASSERT(d->config);

	Rotation rotation;
	m_currentSize = m_proposedSize = XRRConfigCurrentConfiguration(d->config, &rotation);
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

	m_currentRefreshRate = m_proposedRefreshRate = refreshRateHzToIndex(m_currentSize, XRRConfigCurrentRate(d->config));
}

void RandRScreen::setOriginal()
{
	m_originalSize = m_currentSize;
	m_originalRotation = m_currentRotation;
	m_originalRefreshRate = m_currentRefreshRate;
}

bool RandRScreen::applyProposed()
{
	//kDebug() << k_funcinfo << " size " << (SizeID)proposedSize() << ", rotation " << proposedRotation() << ", refresh " << refreshRateIndexToHz(proposedSize(), proposedRefreshRate()) << endl;

	Status status;

	if (proposedRefreshRate() < 0)
		status = XRRSetScreenConfig(QX11Info::display(), d->config, DefaultRootWindow(QX11Info::display()), (SizeID)proposedSize(), (Rotation)proposedRotation(), CurrentTime);
	else {
		if( refreshRateIndexToHz(proposedSize(), proposedRefreshRate()) <= 0 ) {
			m_proposedRefreshRate = 0;
		}
		status = XRRSetScreenConfigAndRate(QX11Info::display(), d->config, DefaultRootWindow(QX11Info::display()), (SizeID)proposedSize(), (Rotation)proposedRotation(), refreshRateIndexToHz(proposedSize(), proposedRefreshRate()), CurrentTime);
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

bool RandRScreen::applyProposedAndConfirm()
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

bool RandRScreen::confirm()
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

void RandRScreen::shownDialogDestroyed()
{
    m_shownDialog = NULL;
    disconnect( kapp->desktop(), SIGNAL( resized(int)), this, SLOT( desktopResized()));
}

void RandRScreen::desktopResized()
{
	if( m_shownDialog != NULL )
		KDialog::centerOnScreen(m_shownDialog, m_screen);
}

QString RandRScreen::changedMessage() const
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
			return i18n("Rotated 90 degrees counterclockwise");
		case RR_Rotate_180:
			return i18n("Rotated 180 degrees counterclockwise");
		case RR_Rotate_270:
			return i18n("Rotated 270 degrees counterclockwise");
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
						return i18n("mirrored horizontally");
			else if (rotation & RR_Reflect_Y)
				if (capitalised)
					return i18n("Mirrored vertically");
				else
					return i18n("mirrored vertically");
			else
				if (capitalised)
					return i18n("Unknown orientation");
				else
					return i18n("unknown orientation");
	}
}

QPixmap RandRScreen::rotationIcon(int rotation) const
{
	// Adjust icons for current screen orientation
	if (!(m_currentRotation & RR_Rotate_0) && rotation & (RR_Rotate_0 | RR_Rotate_90 | RR_Rotate_180 | RR_Rotate_270)) {
		int currentAngle = m_currentRotation & (RR_Rotate_90 | RR_Rotate_180 | RR_Rotate_270);
		switch (currentAngle) {
			case RR_Rotate_90:
				rotation <<= 3;
				break;
			case RR_Rotate_180:
				rotation <<= 2;
				break;
			case RR_Rotate_270:
				rotation <<= 1;
				break;
		}

		// Fix overflow
		if (rotation > RR_Rotate_270) {
			rotation >>= 4;
		}
	}

	switch (rotation) {
		case RR_Rotate_0:
			return SmallIcon("go-up");
		case RR_Rotate_90:
			return SmallIcon("go-previous");
		case RR_Rotate_180:
			return SmallIcon("go-down");
		case RR_Rotate_270:
			return SmallIcon("go-next");
		case RR_Reflect_X:
		case RR_Reflect_Y:
		default:
			return SmallIcon("process-stop");
	}
}

QString RandRScreen::currentRotationDescription() const
{
	QString ret = rotationName(m_currentRotation & RotateMask);

	if (m_currentRotation != (m_currentRotation & RotateMask))
		if (m_currentRotation & RR_Rotate_0)
			ret = rotationName(m_currentRotation & (RR_Reflect_X + RR_Reflect_X), true, true);
		else
			ret += ", " + rotationName(m_currentRotation & (RR_Reflect_X + RR_Reflect_X), true, false);

	return ret;
}

int RandRScreen::rotationIndexToDegree(int rotation) const
{
	switch (rotation & RotateMask) {
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

int RandRScreen::rotationDegreeToIndex(int degree) const
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

int RandRScreen::currentPixelWidth() const
{
	return m_pixelSizes[m_currentSize].width();
}

int RandRScreen::currentPixelHeight() const
{
	return m_pixelSizes[m_currentSize].height();
}

int RandRScreen::currentMMWidth() const
{
	return m_pixelSizes[m_currentSize].width();
}

int RandRScreen::currentMMHeight() const
{
	return m_pixelSizes[m_currentSize].height();
}

QStringList RandRScreen::refreshRates(int size) const
{
	int nrates;
	short* rates = XRRRates(QX11Info::display(), m_screen, (SizeID)size, &nrates);

	QStringList ret;
	for (int i = 0; i < nrates; i++)
		ret << refreshRateDirectDescription(rates[i]);

	return ret;
}

QString RandRScreen::refreshRateDirectDescription(int rate) const
{
	return i18nc("Refresh rate in Hertz (Hz)", "%1 Hz", rate);
}

QString RandRScreen::refreshRateIndirectDescription(int size, int index) const
{
	return i18nc("Refresh rate in Hertz (Hz)", "%1 Hz", refreshRateIndexToHz(size, index));
}

QString RandRScreen::refreshRateDescription(int size, int index) const
{
	return refreshRates(size)[index];
}

bool RandRScreen::proposeRefreshRate(int index)
{
	if (index >= 0 && (int)refreshRates(proposedSize()).count() > index) {
		m_proposedRefreshRate = index;
		return true;
	}

	return false;
}

int RandRScreen::currentRefreshRate() const
{
	return m_currentRefreshRate;
}

QString RandRScreen::currentRefreshRateDescription() const
{
	return refreshRateIndirectDescription(m_currentSize, m_currentRefreshRate);
}

int RandRScreen::proposedRefreshRate() const
{
	return m_proposedRefreshRate;
}

int RandRScreen::refreshRateHzToIndex(int size, int hz) const
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

int RandRScreen::refreshRateIndexToHz(int size, int index) const
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

int RandRScreen::numSizes() const
{
	return m_pixelSizes.count();
}

const QSize& RandRScreen::pixelSize(int index) const
{
	return m_pixelSizes[index];
}

const QSize& RandRScreen::mmSize(int index) const
{
	return m_mmSizes[index];
}

int RandRScreen::sizeIndex(QSize pixelSize) const
{
	for (int i = 0; i < m_pixelSizes.count(); i++)
		if (m_pixelSizes[i] == pixelSize)
			return i;

	return -1;
}

int RandRScreen::rotations() const
{
	return m_rotations;
}

int RandRScreen::currentRotation() const
{
	return m_currentRotation;
}

int RandRScreen::currentSize() const
{
	return m_currentSize;
}

int RandRScreen::proposedRotation() const
{
	return m_proposedRotation;
}

void RandRScreen::proposeRotation(int newRotation)
{
	m_proposedRotation = newRotation & OrientationMask;
}

int RandRScreen::proposedSize() const
{
	return m_proposedSize;
}

bool RandRScreen::proposeSize(int newSize)
{
	if ((int)m_pixelSizes.count() > newSize) {
		m_proposedSize = newSize;
		return true;
	}

	return false;
}

void RandRScreen::load(KConfig& config)
{
	config.setGroup(QString("Screen%1").arg(m_screen));

	if (proposeSize(sizeIndex(QSize(config.readEntry("width", currentPixelWidth()), config.readEntry("height", currentPixelHeight())))))
		proposeRefreshRate(refreshRateHzToIndex(proposedSize(), config.readEntry("refresh", currentRefreshRate())));

	proposeRotation(rotationDegreeToIndex(config.readEntry("rotation", 0)) + (config.readEntry("reflectX", false) ? ReflectX : 0) + (config.readEntry("reflectY",false) ? ReflectY : 0));
}

void RandRScreen::save(KConfig& config) const
{
	config.setGroup(QString("Screen%1").arg(m_screen));
	config.writeEntry("width", currentPixelWidth());
	config.writeEntry("height", currentPixelHeight());
	config.writeEntry("refresh", refreshRateIndexToHz(currentSize(), currentRefreshRate()));
	config.writeEntry("rotation", rotationIndexToDegree(currentRotation()));
	config.writeEntry("reflectX", (bool)(currentRotation() & ReflectMask) == ReflectX);
	config.writeEntry("reflectY", (bool)(currentRotation() & ReflectMask) == ReflectY);
}

RandRDisplay::RandRDisplay()
	: m_valid(true)
{
	// Check extension
	Status s = XRRQueryExtension(QX11Info::display(), &m_eventBase, &m_errorBase);
	if (!s) {
		m_errorCode = QString("%1, base %1").arg(s).arg(m_errorBase);
		m_valid = false;
		return;
	}

	int major_version, minor_version;
	XRRQueryVersion(QX11Info::display(), &major_version, &minor_version);

	m_version = i18n("X Resize and Rotate extension version %1.%2",major_version,minor_version);

	m_numScreens = ScreenCount(QX11Info::display());

	// This assumption is WRONG with Xinerama
	// Q_ASSERT(QApplication::desktop()->numScreens() == ScreenCount(QX11Info::display()));

	for (int i = 0; i < m_numScreens; i++) {
		m_screens.append(new RandRScreen(i));
	}

	setCurrentScreen(QApplication::desktop()->primaryScreen());
}

RandRDisplay::~RandRDisplay()
{
		qDeleteAll(m_screens);
		m_screens.clear();
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

int RandRDisplay::screenChangeNotifyEvent() const
{
	return m_eventBase + RRScreenChangeNotify;
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
	int ret = QApplication::desktop()->screenNumber(widget);
	return ret != -1 ? ret : QApplication::desktop()->primaryScreen();
}

int RandRDisplay::currentScreenIndex() const
{
	return m_currentScreenIndex;
}

void RandRDisplay::refresh()
{
	for (int i = 0; i < m_screens.size(); ++i) {
		RandRScreen* s = m_screens.at(i);
		s->loadSettings();
	}
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

bool RandRDisplay::loadDisplay(KConfig& config, bool loadScreens)
{
	if (loadScreens)
	{
    	for (int i = 0; i < m_screens.size(); ++i) {
        	RandRScreen* s = m_screens.at(i);
        	s->load(config);
		}
    }
	return applyOnStartup(config);
}

bool RandRDisplay::applyOnStartup(KConfig& config)
{
	config.setGroup("Display");
	return config.readEntry("ApplyOnStartup", false);
}

bool RandRDisplay::syncTrayApp(KConfig& config)
{
	config.setGroup("Display");
	return config.readEntry("SyncTrayApp", false);
}

void RandRDisplay::saveDisplay(KConfig& config, bool applyOnStartup, bool syncTrayApp)
{
	config.setGroup("Display");
	config.writeEntry("ApplyOnStartup", applyOnStartup);
	config.writeEntry("SyncTrayApp", syncTrayApp);

	for (int i = 0; i < m_screens.size(); ++i) {
		RandRScreen* s = m_screens.at(i);
        s->save(config);
	}
}

void RandRDisplay::applyProposed(bool confirm)
{
	for (int screenIndex = 0; screenIndex < numScreens(); screenIndex++) {
		if (screen(screenIndex)->proposedChanged()) {
			if (confirm)
					screen(screenIndex)->applyProposedAndConfirm();
			else
					screen(screenIndex)->applyProposed();
		}
	}
}

int RandRScreen::pixelCount( int index ) const
{
	QSize sz = pixelSize(index);
	return sz.width() * sz.height();
}

#include "randr.moc"
