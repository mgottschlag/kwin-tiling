/*
 * Copyright (c) 2008      Harry Bock <hbock@providence.edu>
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
#include <KConfigGroup>
#include <KDebug>
#include <QApplication>
#include <QDesktopWidget>
#include <QX11Info>

#include "randrdisplay.h"
#ifdef HAS_RANDR_1_2 
#include "randrscreen.h"
#endif
#include "legacyrandrscreen.h"
#include <config-randr.h>

RandRDisplay::RandRDisplay()
	: m_valid(true)
{
	m_dpy = QX11Info::display();
	
	// Check extension
	if(XRRQueryExtension(m_dpy, &m_eventBase, &m_errorBase) == False) {
		m_valid = false;
		return;
	}

	int major_version, minor_version;
	XRRQueryVersion(m_dpy, &major_version, &minor_version);

	m_version = i18n("X Resize and Rotate extension version %1.%2",
	                 major_version,minor_version);

	// check if we have the new version of the XRandR extension
	RandR::has_1_2 = (major_version > 1 || (major_version == 1 && minor_version >= 2));
	
	if(RandR::has_1_2)
		kDebug() << "Using XRANDR extension 1.2 or greater.";
	else kDebug() << "Using legacy XRANDR extension (1.1 or earlier).";
	
	kDebug() << "XRANDR error base: " << m_errorBase;
	m_numScreens = ScreenCount(m_dpy);

	// set the timestamp to 0
	RandR::timestamp = 0;

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

#if 0
//#ifdef HAS_RANDR_1_2
	// check if we have more than one output, if no, revert to the legacy behavior
	if (RandR::has_1_2)
	{
		int count = 0;
		foreach(RandRScreen *screen, m_screens)
			count += screen->outputs().count();

		if (count < 2)
		{
			RandR::has_1_2 = false;
			for (int i = 0; i < m_numScreens; ++i)
			{
				delete m_screens[i];
				m_legacyScreens.append(new LegacyRandRScreen(i));
			}
			m_screens.clear();
		}
	}
#endif
	setCurrentScreen(DefaultScreen(QX11Info::display()));
}

RandRDisplay::~RandRDisplay()
{
		qDeleteAll(m_legacyScreens);
#ifdef HAS_RANDR_1_2
		qDeleteAll(m_screens);
#endif
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
	Q_ASSERT(index < ScreenCount(m_dpy));
	m_currentScreenIndex = index;
}

int RandRDisplay::screenIndexOfWidget(QWidget* widget)
{
	//int ret = QApplication::desktop()->screenNumber(widget);
	//return ret != -1 ? ret : QApplication::desktop()->primaryScreen();
	
	// get info from Qt's X11 info directly; QDesktopWidget seems to use
	// Xinerama by default, which doesn't work properly with randr.
	// It will return more screens than exist for the display, causing
	// a crash in the screen/currentScreen methods.
	return widget->x11Info().screen();
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
			XRRScreenChangeNotifyEvent *event = (XRRScreenChangeNotifyEvent*)(e);
			for (int i=0; i < m_screens.count(); ++i) {
				RandRScreen *screen = m_screens.at(i);
				if (screen->rootWindow() == event->root)
					screen->handleEvent(event);
			}
			
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
	Q_ASSERT(ScreenCount(QX11Info::display()) == m_numScreens);
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
#ifdef HAS_RANDR_1_2
		if (RandR::has_1_2)
		{
			foreach(RandRScreen *s, m_screens)
				s->load(config);

		}
		else
#endif
		{
			foreach(LegacyRandRScreen* s, m_legacyScreens)
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

#ifdef HAS_RANDR_1_2
	if (RandR::has_1_2)
	{
		foreach(RandRScreen *s, m_screens)
			s->save(config);
	}
	else
#endif
	{
		foreach(LegacyRandRScreen *s, m_legacyScreens)
			s->save(config);
	}
}

void RandRDisplay::applyProposed(bool confirm)
{

#ifdef HAS_RANDR_1_2
	if (RandR::has_1_2)
		foreach(RandRScreen *s, m_screens)
			s->applyProposed(confirm);
	else
#endif
	{
		foreach(LegacyRandRScreen *s, m_legacyScreens)
		{
			if (s->proposedChanged()) {
				if (confirm)
						s->applyProposedAndConfirm();
				else
						s->applyProposed();
			}
		}
	}
}

