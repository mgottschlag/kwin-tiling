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

#include <KLocale>
#include <QApplication>
#include <QDesktopWidget>
#include <QX11Info>

#include "randrdisplay.h"
#include "randrscreen.h"
#include "legacyrandrscreen.h"

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
#ifdef HAS_RANDR_1_2
		if (RandR::has_1_2)
			m_screens.append(new RandRScreen(i));
		else
#endif
			m_legacyScreens.append(new LegacyRandRScreen(i));
	}

	setCurrentScreen(QApplication::desktop()->primaryScreen());
}

RandRDisplay::~RandRDisplay()
{
		qDeleteAll(m_legacyScreens);
		m_legacyScreens.clear();
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
#ifdef HAS_RANDR_1_2
	if (RandR::has_1_2) {
		for (int i = 0; i < m_screens.count(); ++i) {
			RandRScreen* s = m_screens.at(i);
			s->loadSettings();
		}
	}
	else
#endif
	{
		for (int i = 0; i < m_legacyScreens.size(); ++i) {
			LegacyRandRScreen* s = m_legacyScreens.at(i);
			s->loadSettings();
		}
	}
}

bool RandRDisplay::canHandle(const XEvent *e) const 
{
	if (e->type == m_eventBase + RRScreenChangeNotify)
		return true;
#ifdef HAS_RANDR_1_2
	else if (e->type == m_eventBase + RRNotify)
		return true;
#endif
	return false;
}


void RandRDisplay::handleEvent(XEvent *e)
{
	if (e->type == m_eventBase + RRScreenChangeNotify) {
#ifdef HAS_RANDR_1_2
		if (RandR::has_1_2) {
		}
		else
#endif
		{
			// handle the event
		}
	}
#ifdef HAS_RANDR_1_2
	else if (e->type == m_eventBase + RRNotify) {
		//forward the event to the right screen
		XRRNotifyEvent *event = (XRRNotifyEvent*)e;
		for (int i=0; i < m_screens.count(); ++i) {
			RandRScreen *screen = m_screens.at(i);
			// FIXME: check which screen should receive the event
			//        this needs a dual-head setup
			screen->handleRandREvent(event);
		}
	}
#endif
}

int RandRDisplay::numScreens() const
{
	return m_numScreens;
}

LegacyRandRScreen* RandRDisplay::legacyScreen(int index)
{
	return m_legacyScreens.at(index);
}

LegacyRandRScreen* RandRDisplay::currentLegacyScreen()
{
	return m_legacyScreens.at(m_currentScreenIndex);
}

#ifdef HAS_RANDR_1_2
RandRScreen* RandRDisplay::screen(int index)
{
	return m_screens.at(index);
}

RandRScreen* RandRDisplay::currentScreen()
{
	return m_screens.at(m_currentScreenIndex);
}
#endif

bool RandRDisplay::loadDisplay(KConfig& config, bool loadScreens)
{
	if (loadScreens)
	{
		for (int i = 0; i < m_legacyScreens.size(); ++i) {
#ifdef HAS_RANDR_1_2
			if (RandR::has_1_2)
				//TODO: load screen settings here
				currentScreenIndex();
			else
#endif
			{
        			LegacyRandRScreen* s = m_legacyScreens.at(i);
        			s->load(config);
			}
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

	for (int i = 0; i < m_legacyScreens.size(); ++i) {
#ifdef HAS_RANDR_1_2
		if (RandR::has_1_2)
			//TODO: save screen settings here
			currentScreenIndex();
		else
#endif
		{
			LegacyRandRScreen* s = m_legacyScreens.at(i);
        		s->save(config);
		}
	}
}

void RandRDisplay::applyProposed(bool confirm)
{

#ifdef HAS_RANDR_1_2
	if (RandR::has_1_2)
		//TODO: apply display settings here
		currentScreenIndex();
	else
#endif
	{
		for (int screenIndex = 0; screenIndex < numScreens(); screenIndex++) {
			if (legacyScreen(screenIndex)->proposedChanged()) {
				if (confirm)
						legacyScreen(screenIndex)->applyProposedAndConfirm();
				else
						legacyScreen(screenIndex)->applyProposed();
			}
		}
	}
}

