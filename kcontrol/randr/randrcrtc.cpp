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
#include "randrcrtc.h"
#include "randrscreen.h"

#ifdef HAS_RANDR_1_2
RandRCrtc::RandRCrtc(RandRScreen *parent, RRCrtc id)
	: QObject(parent), m_info(0L)
{
	m_id = id;
	loadSettings();
}

RandRCrtc::~RandRCrtc()
{
	// do nothing for now
}

int RandRCrtc::rotations() const
{
	return m_rotations;
}

int RandRCrtc::currentRotation() const
{
	return m_currentRotation;
}

void RandRCrtc::loadSettings()
{
	if (m_info)
		XRRFreeCrtcInfo(m_info);

	RandRScreen *screen = dynamic_cast<RandRScreen*>(parent());
	Q_ASSERT(screen);

	m_info = XRRGetCrtcInfo(QX11Info::display(), screen->resources(), m_id);
	Q_ASSERT(m_info);

	m_pos = QPoint(m_info->x, m_info->y);
	m_size = QSize(m_info->width, m_info->height);

	// get all connected outputs 
	m_connectedOutputs.clear();
	for (int i = 0; i < m_info->noutput; ++i)
		m_connectedOutputs.append(m_info->outputs[i]);

	// get all outputs this crtc can be connected to
	m_possibleOutputs.clear();
	for (int i = 0; i < m_info->npossible; ++i)
		m_possibleOutputs.append(m_info->possible[i]);

	// get all rotations
	m_rotations = m_info->rotations;
	m_currentRotation = m_info->rotation;
}

#include "randrcrtc.moc"

#endif

