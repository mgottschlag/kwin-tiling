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

#include <KLocale>
#include <QApplication>
#include <QDesktopWidget>
#include <QX11Info>

#include "randrdisplay.h"
#include "oldrandrscreen.h"

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

	// check if we have the new version of the XRandR extension
	if (major_version > 1 || (major_version == 1 && minor_version >= 2))
		RandR::has_1_2 = true;
	else
		RandR::has_1_2 = false;

	m_numScreens = ScreenCount(QX11Info::display());

	// This assumption is WRONG with Xinerama
	// Q_ASSERT(QApplication::desktop()->numScreens() == ScreenCount(QX11Info::display()));

	for (int i = 0; i < m_numScreens; i++) {
		m_screens.append(new OldRandRScreen(i));
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
		OldRandRScreen* s = m_screens.at(i);
		s->loadSettings();
	}
}

int RandRDisplay::numScreens() const
{
	return m_numScreens;
}

OldRandRScreen* RandRDisplay::screen(int index)
{
	return m_screens.at(index);
}

OldRandRScreen* RandRDisplay::currentScreen()
{
	return m_currentScreen;
}

bool RandRDisplay::loadDisplay(KConfig& config, bool loadScreens)
{
	if (loadScreens)
	{
    	for (int i = 0; i < m_screens.size(); ++i) {
        	OldRandRScreen* s = m_screens.at(i);
        	s->load(config);
		}
    }
	return applyOnStartup(config);
}

bool RandRDisplay::applyOnStartup(KConfig& config)
{
	return config.group("Display").readEntry("ApplyOnStartup", false);
}

bool RandRDisplay::syncTrayApp(KConfig& config)
{
	return config.group("Display").readEntry("SyncTrayApp", false);
}

void RandRDisplay::saveDisplay(KConfig& config, bool applyOnStartup, bool syncTrayApp)
{
	KConfigGroup group = config.group("Display");
	group.writeEntry("ApplyOnStartup", applyOnStartup);
	group.writeEntry("SyncTrayApp", syncTrayApp);

	for (int i = 0; i < m_screens.size(); ++i) {
		OldRandRScreen* s = m_screens.at(i);
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

