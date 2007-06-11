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
#include <QAction>
#include "randroutput.h"
#include "randrscreen.h"
#include "randrcrtc.h"
#include "randrmode.h"

#ifdef HAS_RANDR_1_2
RandROutput::RandROutput(RandRScreen *parent, RROutput id)
	: QObject(parent), m_info(0L)
{
	m_screen = parent;
	Q_ASSERT(m_screen);

	m_id = id;
	
	// initialize members
	m_rotations = 0;
	m_connected = false;
	
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

	m_info = XRRGetOutputInfo(QX11Info::display(), m_screen->resources(), m_id);
	Q_ASSERT(m_info);


	if (RandR::timestamp != m_info->timestamp)
		RandR::timestamp = m_info->timestamp;

	m_name = m_info->name;

	m_possibleCrtcs.clear();
	for (int i = 0; i < m_info->ncrtc; ++i)
		m_possibleCrtcs.append(m_info->crtcs[i]);

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
		RandRCrtc *crtc = m_screen->crtc(m_possibleCrtcs.at(i));
		Q_ASSERT(crtc);
		m_rotations |= crtc->rotations();
	}
}

void RandROutput::handleEvent(XRROutputChangeNotifyEvent *event)
{
	int changed = 0;

	if (event->crtc != m_currentCrtc)
	{
		changed |= ChangeCrtc;
		m_currentCrtc = event->crtc;
	}

	if (event->mode != currentMode())
	{
		changed |= ChangeMode;

	}
	if (event->rotation != currentRotation())
	{
		changed |= ChangeRotation;
	}
	if ((event->connection == RR_Connected) != m_connected)
	{
		changed |= ChangeConnection;
		m_connected = !m_connected;
	}

	if (changed)
		emit outputChanged(m_id, changed);
}

void RandROutput::handlePropertyEvent(XRROutputPropertyNotifyEvent *event)
{
	Q_UNUSED(event);
	//TODO: implement
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
	if (!isConnected())
		return None;

	RandRCrtc *crtc = m_screen->crtc(m_currentCrtc);
	if (!crtc)
		return None;

	return crtc->currentMode();
}

SizeList RandROutput::sizes() const
{
	SizeList sizeList;

	for (int i = 0; i < m_modes.count(); ++i)
	{
		RandRMode mode = m_screen->mode(m_modes.at(i));
		if (!mode.isValid())
			continue;
		if (sizeList.indexOf(mode.size()) == -1)
			sizeList.append(mode.size());
	}
	return sizeList;
}

QRect RandROutput::rect() const
{
	if (m_currentCrtc == None)
		return QRect(0,0,0,0);

	QRect r = m_screen->crtc(m_currentCrtc)->rect();

	// if the current rotation is 90 or 270, invert the rect
	if (currentRotation() & (RandR::Rotate90 | RandR::Rotate270))
	{
		kDebug() << "Current Rotation: " << currentRotation() << endl;
		QRect inverted;
		inverted.setWidth(r.height());
		inverted.setHeight(r.width());
		return inverted;
	}

	return r;
}

int RandROutput::rotations() const
{
	return m_rotations;
}

int RandROutput::currentRotation() const
{
	if (!isActive())
		return RandR::Rotate0;

	RandRCrtc *crtc = m_screen->crtc(m_currentCrtc);
	Q_ASSERT(crtc);

	return crtc->currentRotation();
}

bool RandROutput::isConnected() const
{
	return m_connected;
}

bool RandROutput::isActive() const
{
	return (m_connected && m_currentCrtc != None);
}

void RandROutput::slotChangeSize(QAction *action)
{
	QSize size = action->data().toSize();

	setSize(size);
}

void RandROutput::slotChangeRotation(QAction *action)
{
	if (m_currentCrtc != None)
	{
		RandRCrtc *crtc = m_screen->crtc(m_currentCrtc);
		crtc->setOriginal();
		crtc->proposeRotation(action->data().toInt());
		if (!applyAndConfirm(crtc))
		{
			crtc->proposeOriginal();
			crtc->applyProposed();
		}
	}
	else
	{
		// try to add this output to a crtc
		for (int i = 0; i < m_possibleCrtcs.count(); ++i)
		{
			RandRCrtc *crtc = m_screen->crtc(m_possibleCrtcs.at(i));
			if (crtc->addOutput(m_id))
			{
				crtc->setOriginal();
				crtc->proposeRotation(action->data().toInt());
				if (!applyAndConfirm(crtc))
				{
					crtc->proposeOriginal();
					crtc->applyProposed();
				}
				break;
			}
		}
	}
}

void RandROutput::slotDisable()
{
	if (m_currentCrtc == None)
		return;

	RandRCrtc *crtc = m_screen->crtc(m_currentCrtc);
	crtc->removeOutput(m_id);
}

void RandROutput::setSize(QSize s)
{
	if (s == rect().size())
	       return;	


	// try to set the mode in the current crtc
	if (m_currentCrtc != None)
	{
		RandRCrtc *crtc = m_screen->crtc(m_currentCrtc);
		crtc->setOriginal();
		crtc->proposeSize(s);
		if (!applyAndConfirm(crtc))
		{
			crtc->proposeOriginal();
			crtc->applyProposed();
		}
		return;
	}

	// try to add this output to an empty crtc
	RandRCrtc *crtc = findEmptyCrtc();

	if (crtc)
	{
		crtc->setOriginal();
		crtc->addOutput(m_id, s);
		if (!applyAndConfirm(crtc))
		{
			crtc->removeOutput(m_id);
			crtc->proposeOriginal();
			crtc->applyProposed();
		}
	}
	else
	{
		// use any crtc available
		for (int i = 0; i < m_possibleCrtcs.count(); ++i)
		{
			RandRCrtc *crtc = m_screen->crtc(m_possibleCrtcs.at(i));
			crtc->setOriginal();
			crtc->addOutput(m_id, s);
			if (!applyAndConfirm(crtc))
			{
				crtc->removeOutput(m_id);
				crtc->proposeOriginal();
				crtc->applyProposed();
			}

		}
	}

	loadSettings();
	emit outputChanged(m_id, ChangeMode);
}

RandRCrtc *RandROutput::findEmptyCrtc()
{
	RandRCrtc *crtc = 0;

	for (int i = 0; i < m_possibleCrtcs.count(); ++i)
	{
		crtc = m_screen->crtc(m_possibleCrtcs.at(i));
		if (crtc->connectedOutputs().count() == 0)
			break;
	}

	return crtc;
}

bool RandROutput::applyAndConfirm(RandRCrtc *crtc)
{
	if (!crtc)
		return false;

	if (crtc->applyProposed())
	{
		if (RandR::confirm(crtc->rect()))
			return true;
	}

	return false;	
}

#include "randroutput.moc"

#endif

