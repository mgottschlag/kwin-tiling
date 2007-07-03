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
	m_currentCrtc = None;
	
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

	setCurrentCrtc(m_info->crtc);

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
		changed |= RandR::ChangeCrtc;
		// update crtc settings
		if (m_currentCrtc != None)
			m_screen->crtc(m_currentCrtc)->loadSettings();
		setCurrentCrtc(event->crtc);
		if (m_currentCrtc != None)
			m_screen->crtc(m_currentCrtc)->loadSettings();
	}

	if (event->mode != currentMode())
	{
		changed |= RandR::ChangeMode;

	}
	if (event->rotation != currentRotation())
	{
		changed |= RandR::ChangeRotation;
	}
	if ((event->connection == RR_Connected) != m_connected)
	{
		changed |= RandR::ChangeConnection;
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

void RandROutput::proposeOriginal()
{
	// this is just what we need to do here
	setCurrentCrtc(m_originalCrtc);
}

void RandROutput::load(KConfig &config)
{
	m_originalCrtc = m_currentCrtc;

	if (!m_connected)
		return;

	KConfigGroup cg = config.group("Screen_" + QString::number(m_screen->index()) + "_Output_" + m_name);
	bool active = cg.readEntry("Active", true);

	if (!active)
		m_proposedCrtc = None;
	else
	{
		RandRCrtc *crtc = findEmptyCrtc();
		if (crtc)
			m_proposedCrtc = crtc->id();
		else
			m_proposedCrtc = None;

	}

	// if the output is not going to be active, stop loading the config
	if (m_proposedCrtc == None)
		return;

	m_proposedRect = cg.readEntry("Rect", QRect());
	m_proposedRotation = cg.readEntry("Rotation", (int) RandR::Rotate0);
	m_proposedRate = cg.readEntry("Rate", 0);
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
	m_proposedRect.setSize(size);
	applyProposed(RandR::ChangeSize, true);
}

void RandROutput::slotChangeRotation(QAction *action)
{
	m_proposedRotation = action->data().toInt();
	applyProposed(RandR::ChangeRotation, true);
}

void RandROutput::slotChangeRefreshRate(QAction *action)
{
	float rate = action->data().toDouble();

	m_proposedRate = rate;
	applyProposed(RandR::ChangeRate, true);
}

void RandROutput::slotDisable()
{
	if (m_currentCrtc == None)
		return;

	RandRCrtc *crtc = m_screen->crtc(m_currentCrtc);
	crtc->removeOutput(m_id);
	crtc->applyProposed();
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

bool RandROutput::tryCrtc(RandRCrtc *crtc, int changes)
{
	crtc->setOriginal();

	if (changes & RandR::ChangeSize)
		crtc->proposeSize(m_proposedRect.size());
	if (changes & RandR::ChangePosition)
		crtc->proposePosition(m_proposedRect.topLeft());
	if (changes & RandR::ChangeRotation)
		crtc->proposeRotation(m_proposedRotation);
	if (changes & RandR::ChangeRate)
		crtc->proposeRefreshRate(m_proposedRate);

	if (crtc->applyProposed())
		return true;

	// revert changes if we didn't succeed
	crtc->proposeOriginal();
	crtc->applyProposed();
	return false;
}

bool RandROutput::applyProposed(int changes, bool confirm)
{
	KConfig cfg("krandrrc");
	RandRCrtc *crtc;

	QRect r;

	if (changes & RandR::ChangeSize)
		r.setSize(m_proposedRect.size());

	// check if we should apply changes to the crtc
	if (changes & RandR::ChangeCrtc)
	{
		if (m_proposedCrtc != m_currentCrtc)
		{
			// if we are currently attached to a crtc that is different from the proposed
			// one, we need to detach
			if (m_currentCrtc != None)
				m_screen->crtc(m_currentCrtc)->removeOutput(m_id);

			// if the proposed CRTC is None, then we should stop processing here
			if (m_proposedCrtc == None)
			{
				setCurrentCrtc(None);
				return true;
			}

			// if we were asked to attach to another crtc, try it
			if (m_screen->crtc(m_proposedCrtc)->addOutput(m_id, r.size()))
				setCurrentCrtc(m_proposedCrtc);
			else
			{
				setCurrentCrtc(None);
				return false;
			}
		}
	}

	// first try to apply to the already attached crtc if any
	if (m_currentCrtc != None)
	{
		crtc = m_screen->crtc(m_currentCrtc);
		if (tryCrtc(crtc, changes))
			if (!confirm || confirm && RandR::confirm(crtc->rect()))
			{
				save(cfg);
				return true;
			}
			else
			{
				crtc->proposeOriginal();
				crtc->applyProposed();
			}
		return false;
	}

	//then try an empty crtc
	crtc = findEmptyCrtc();
	if (crtc)
	{
		if (crtc->addOutput(m_id, r.size()))
		{
			// try the crtc, and if no confirmation is needed or the user confirm, save the new settings
			if (tryCrtc(crtc, changes)) 
			{
				if (!confirm || confirm && RandR::confirm(crtc->rect()))
				{
					setCurrentCrtc(crtc->id());
					save(cfg);
					return true;
				}
				else
				{
					crtc->proposeOriginal();
					crtc->applyProposed();
					return false;
				}
			}
			else
			{
				setCurrentCrtc(None);
				crtc->removeOutput(m_id);
				return false;
			}
		}
	}

	// TODO: check if we can add this output to a CRTC which already has an output 
	// connection
	return false;
}

void RandROutput::setCurrentCrtc(RRCrtc c)
{
	RandRCrtc *crtc;
	if (m_currentCrtc != None)
	{
		crtc = m_screen->crtc(m_currentCrtc);
		disconnect(crtc, SIGNAL(crtcChanged(RRCrtc, int)), 
			   this, SLOT(slotCrtcChanged(RRCrtc, int)));
	}
	if (c == None)
		return;

	crtc = m_screen->crtc(c);
	connect(crtc, SIGNAL(crtcChanged(RRCrtc, int)),
		this, SLOT(slotCrtcChanged(RRCrtc, int)));
	m_currentCrtc = c;
}

void RandROutput::slotCrtcChanged(RRCrtc c, int changes)
{
	kDebug() << "CRTC changed" << endl;
	//FIXME select which changes we should notify
	emit outputChanged(m_id, changes);
}

#include "randroutput.moc"

#endif

