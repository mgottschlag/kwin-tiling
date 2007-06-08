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

#include <kdebug.h>

#include "krandrapp.h"
#include "krandrapp.moc"

#include "krandrtray.h"

#include "randrscreen.h"
#include "legacyrandrscreen.h"
#include <X11/Xlib.h>

KRandRApp::KRandRApp()
	: m_tray(new KRandRSystemTray(0L))
{
	m_tray->show();
	m_tray->setObjectName("RANDRTray");

}

bool KRandRApp::x11EventFilter(XEvent* e)
{

	if (m_tray->canHandle(e))
		m_tray->handleEvent(e);

	return KApplication::x11EventFilter( e );
}
