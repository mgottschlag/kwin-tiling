/*
 * Copyright (c) 2007      Gustavo Pichorim Boiko <gustavo.boiko@kdemail.net>
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

#include <KDebug>
#include <QX11Info>
#include "randrscreen.h"

#ifdef HAS_RANDR_1_2
RandRScreen::RandRScreen(int screenIndex)
	: m_resources(0L)
{
	m_index = screenIndex;
	loadSettings();
}

RandRScreen::~RandRScreen()
{
	// do nothing for now
}

void RandRScreen::loadSettings()
{
	int minW, minH, maxW, maxH;

	Status status = XRRGetScreenSizeRange(QX11Info::display(), RootWindow(QX11Info::display(), m_index),
					 &minW, &minH, &maxW, &maxH);
	//FIXME: we should check the status here
	Q_UNUSED(status);
	m_minSize = QSize(minW, minH);
	m_maxSize = QSize(maxW, maxH);

	if (m_resources)
		XRRFreeScreenResources(m_resources);

	m_resources = XRRGetScreenResources(QX11Info::display(), RootWindow(QX11Info::display(), m_index));
	Q_ASSERT(m_resources);
}

QSize RandRScreen::minSize() const
{
	return m_minSize;
}

QSize RandRScreen::maxSize() const
{
	return m_maxSize;
}

#include "randrscreen.moc"

#endif

