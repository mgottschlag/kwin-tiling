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
#include <KConfig>
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
		// update crtc settings
		if (m_currentCrtc != None)
			m_screen->crtc(m_currentCrtc)->loadSettings();
		m_currentCrtc = event->crtc;
		if (m_currentCrtc != None)
			m_screen->crtc(m_currentCrtc)->loadSettings();
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

	// check if we are still connected, if not, release the crtc connection
	if (!m_connected && m_currentCrtc != None)
	{
		RandRCrtc *crtc = m_screen->crtc(m_currentCrtc);
		crtc->removeOutput(m_id);
		crtc->applyProposed();
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

	return m_screen->crtc(m_currentCrtc)->rect();
}

RateList RandROutput::refreshRates(QSize s) const
{
	RateList list;
	if (!s.isValid())
		s = rect().size();

	for (int i = 0; i < m_modes.count(); ++i)
	{
		RandRMode mode = m_screen->mode(m_modes.at(i));
		if (!mode.isValid())
			continue;
		if (mode.size() == s)
			list.append(mode.refreshRate());
	}
	return list;
}

float RandROutput::refreshRate() const
{
	return m_screen->mode( currentMode() ).refreshRate();
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

bool RandROutput::proposedChanged()
{
	if (!m_connected)
		return false;

	//TODO: add a config option for activating/deactivating outputs
	return true;
}

bool RandROutput::applyProposed()
{
	if (m_currentCrtc == None)
		return true;

	return m_screen->crtc(m_currentCrtc)->applyProposed();
}

void RandROutput::proposeOriginal()
{
	if (m_originalCrtc != m_currentCrtc)
	{
		if (m_currentCrtc != None)
			m_screen->crtc(m_currentCrtc)->removeOutput(m_id);

		if (m_originalCrtc != None)
			m_screen->crtc(m_originalCrtc)->addOutput(m_id);
	}

	m_currentCrtc = m_originalCrtc;
	
	if (m_currentCrtc != None)
		m_screen->crtc(m_currentCrtc)->proposeOriginal();
}

void RandROutput::load(KConfig &config)
{
	m_originalCrtc = m_currentCrtc;

	if (!m_connected)
		return;

	KConfigGroup cg = config.group("Screen_" + QString::number(m_screen->index()) + "_Output_" + m_name);
	bool active = cg.readEntry("Active", true);

	if (!active && m_currentCrtc != None)
	{
		RandRCrtc *crtc = m_screen->crtc(m_currentCrtc);
		crtc->removeOutput(m_id);
	}
	else if (active && m_currentCrtc == None)
	{
		//FIXME: not handling the case where there is no empty CRTC to use
		RandRCrtc *crtc = findEmptyCrtc();
		if (crtc->addOutput(m_id))
			m_currentCrtc = crtc->id();

	}

	// if the output is not going to be active, stop loading the config
	if (!active)
		return;

	RandRCrtc *crtc = m_screen->crtc(m_currentCrtc);
	QRect r = cg.readEntry("Rect", QRect());
	if (r.isValid())
	{
		crtc->proposePosition(r.topLeft());
		crtc->proposeSize(r.size());
	}
	crtc->proposeRotation(cg.readEntry("Rotation", (int) RandR::Rotate0));
	crtc->proposeRefreshRate(cg.readEntry("Rate", 0));
}

void RandROutput::save(KConfig &config)
{
	KConfigGroup cg = config.group("Screen_" + QString::number(m_screen->index()) + "_Output_" + m_name);
	if (!m_connected)
		return;

	if (m_currentCrtc == None)
	{
		cg.writeEntry("Active", false);
	       	return;	
	}

	RandRCrtc *crtc = m_screen->crtc(m_currentCrtc);
	cg.writeEntry("Active", true);
	cg.writeEntry("Rect", crtc->rect());
	kDebug() << "[OUTPUT] Saving rect " << crtc->rect() << endl;
	cg.writeEntry("Rotation", crtc->currentRotation());
	cg.writeEntry("RefreshRate", (double) crtc->currentRefreshRate());
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
		if (applyAndConfirm(crtc))
			return;

		crtc->proposeOriginal();
		crtc->applyProposed();
		return;
	}

	RandRCrtc *crtc = findEmptyCrtc();
	if (crtc)
	{
		crtc->setOriginal();
		crtc->proposeRotation(action->data().toInt());
		if (applyAndConfirm(crtc))
			return;
			
		crtc->proposeOriginal();
		crtc->applyProposed();
	}
	else
	{
		// try to add this output to a crtc
		for (int i = 0; i < m_possibleCrtcs.count(); ++i)
		{
			crtc = m_screen->crtc(m_possibleCrtcs.at(i));
			if (crtc->addOutput(m_id))
			{
				crtc->setOriginal();
				crtc->proposeRotation(action->data().toInt());
				if (applyAndConfirm(crtc))
					return;

				crtc->proposeOriginal();
				crtc->applyProposed();
			}
		}
	}
}

void RandROutput::slotChangeRefreshRate(QAction *action)
{
	float rate = action->data().toDouble();

	if (m_currentCrtc != None)
	{
		RandRCrtc *crtc = m_screen->crtc(m_currentCrtc);
		crtc->setOriginal();
		crtc->proposeRefreshRate(rate);
		if (applyAndConfirm(crtc))
			return;

		crtc->proposeOriginal();
		crtc->applyProposed();
		return;
	}

	RandRCrtc *crtc = findEmptyCrtc();
	if (crtc)
	{
		crtc->setOriginal();
		crtc->proposeRefreshRate(rate);
		if (applyAndConfirm(crtc))
			return;

		crtc->proposeOriginal();
		crtc->applyProposed();
		return;

	}
	else
	{
		// try to add this output to a crtc
		for (int i = 0; i < m_possibleCrtcs.count(); ++i)
		{
			crtc = m_screen->crtc(m_possibleCrtcs.at(i));
			if (crtc->addOutput(m_id))
			{
				crtc->setOriginal();
				crtc->proposeRefreshRate(rate);
				if (applyAndConfirm(crtc))
					return;
					
				crtc->proposeOriginal();
				crtc->applyProposed();
				return;
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
	crtc->applyProposed();
	// FIXME: check when there is only one output active, and ask the user for
	// confirmation before disabling: otherwise the user won't have any output 
	// active
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
		if (applyAndConfirm(crtc))
			return;

		crtc->proposeOriginal();
		crtc->applyProposed();
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
}

RandRCrtc *RandROutput::findEmptyCrtc()
{
	RandRCrtc *crtc = 0;

	for (int i = 0; i < m_possibleCrtcs.count(); ++i)
	{
		crtc = m_screen->crtc(m_possibleCrtcs.at(i));
		if (crtc->connectedOutputs().count() == 0)
			return crtc;
	}

	return 0;
}

bool RandROutput::applyAndConfirm(RandRCrtc *crtc)
{
	KConfig cfg("krandrrc");
	if (!crtc)
		return false;

	if (crtc->applyProposed())
	{
		kDebug() << "[OUTPUT] Saving rect " << crtc->rect() << endl;
		if (RandR::confirm(crtc->rect()))
		{
			save(cfg);
			return true;
		}
	}

	return false;	
}

#include "randroutput.moc"

#endif

