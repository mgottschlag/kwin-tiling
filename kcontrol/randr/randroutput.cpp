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
#include "randroutput.h"
#include "randrscreen.h"
#include "randrcrtc.h"

#ifdef HAS_RANDR_1_2
RandROutput::RandROutput(RandRScreen *parent, RROutput id)
	: QObject(parent), m_info(0L)
{
	m_id = id;
	loadSettings();
}

RandROutput::~RandROutput()
{
	if (m_info)
		XRRFreeOutputInfo(m_info);
}

void RandROutput::loadSettings()
{
	if (m_info)
		XRRFreeOutputInfo(m_info);

	RandRScreen *screen = dynamic_cast<RandRScreen*>(parent());
	Q_ASSERT(screen);

	m_info = XRRGetOutputInfo(QX11Info::display(), screen->resources(), m_id);
	Q_ASSERT(m_info);

	m_name = m_info->name;

	m_possibleCrtcs.clear();
	for (int i = 0; i < m_info->ncrtc; ++i)
		m_possibleCrtcs.append(m_info->crtcs[i]);

	kDebug() << "Got " << m_possibleCrtcs.count() << " CRTCS for output " << m_name << endl;
	m_currentCrtc = m_info->crtc;

	m_connected = (m_info->connection == RR_Connected);

	//get modes
	m_modes.clear();
	for (int i = 0; i < m_info->nmode; ++i)
		m_modes.append(m_info->modes[i]);

	//get all possible rotations
	m_rotations = 0;
	for (int i = 0; i < m_possibleCrtcs.count(); ++i)
	{
		RandRCrtc *crtc = screen->crtc(m_possibleCrtcs.at(i));
		Q_ASSERT(crtc);
		m_rotations |= crtc->rotations();
	}
}

QString RandROutput::name() const
{
	return m_name;
}
QString RandROutput::icon() const
{
	//FIXME: check what names we should use
	if (m_name.contains("VGA"))
		return "screen";
	else if (m_name.contains("LVDS"))
		return "screen";
	else if (m_name.contains("TV"))
		return "video-television";

	return "screen";
}

CrtcList RandROutput::possibleCrtcs() const
{
	return m_possibleCrtcs;
}

RRCrtc RandROutput::currentCrtc() const
{
	return m_currentCrtc;
}

ModeList RandROutput::modes() const
{
	return m_modes;
}

RRMode RandROutput::currentMode() const
{
	return m_currentMode;
}

int RandROutput::rotations() const
{
	return m_rotations;
}

int RandROutput::currentRotation() const
{
	RandRScreen *screen = dynamic_cast<RandRScreen*>(parent());
	if (!screen)
		return RandR::Rotate0;

	if (!isConnected() || m_currentCrtc == None)
		return RandR::Rotate0;

	RandRCrtc *crtc = screen->crtc(m_currentCrtc);
	Q_ASSERT(crtc);

	return crtc->currentRotation();
}

bool RandROutput::isConnected() const
{
	return m_connected;
}

#include "randroutput.moc"

#endif

