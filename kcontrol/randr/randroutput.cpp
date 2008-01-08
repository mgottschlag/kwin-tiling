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

#include "randroutput.h"
#include "randrscreen.h"
#include "randrcrtc.h"
#include "randrmode.h"

#include <KConfig>
#include <KConfigGroup>
#include <QX11Info>
#include <QAction>

RandROutput::RandROutput(RandRScreen *parent, RROutput id)
: QObject(parent)
{
	m_screen = parent;
	Q_ASSERT(m_screen);

	m_id = id;
	
	// initialize members
	m_rotations = 0;
	m_connected = false;
	m_currentCrtc = None;
	m_relatedOutput = this;
	m_relation = SameAs;
	
	loadSettings();
}

RandROutput::~RandROutput()
{
}

RROutput RandROutput::id() const
{
	return m_id;
}

RandRScreen *RandROutput::screen() const
{
	return m_screen;
}

void RandROutput::loadSettings(bool notify)
{
	int changes = 0;
	XRROutputInfo *info = XRRGetOutputInfo(QX11Info::display(), m_screen->resources(), m_id);
	Q_ASSERT(info);


	if (RandR::timestamp != info->timestamp)
		RandR::timestamp = info->timestamp;

	// this information shouldn't change, so
	m_name = info->name;

	m_possibleCrtcs.clear();
	for (int i = 0; i < info->ncrtc; ++i)
		m_possibleCrtcs.append(info->crtcs[i]);

	//check if the crtc changed
	if (info->crtc != m_currentCrtc)
	{
		kDebug() << "CRTC changed for " << m_name << ": " << info->crtc;
		setCrtc(info->crtc);
		changes |= RandR::ChangeCrtc;
	}

			

	bool connected = (info->connection == RR_Connected);
	if (connected != m_connected)
	{
		m_connected = connected;
		changes |= RandR::ChangeConnection;
	}

	//CHECK: is it worth notifying changes on mode list changing?
	//get modes
	m_modes.clear();
	for (int i = 0; i < info->nmode; ++i)
		m_modes.append(info->modes[i]);

	//get all possible rotations
	m_rotations = 0;
	for (int i = 0; i < m_possibleCrtcs.count(); ++i)
	{
		RandRCrtc *crtc = m_screen->crtc(m_possibleCrtcs.at(i));
		Q_ASSERT(crtc);
		m_rotations |= crtc->rotations();
	}

	// free the info
	XRRFreeOutputInfo(info);

	if (changes && notify)
		emit outputChanged(m_id, changes);
}

void RandROutput::handleEvent(XRROutputChangeNotifyEvent *event)
{
	int changed = 0;

	kDebug() << "[OUTPUT] Got event for " << m_name;
	kDebug() << "       crtc: " << event->crtc;
	kDebug() << "       mode: " << event->mode;
	kDebug() << "       rotation: " << event->rotation;
	kDebug() << "       connection: " << event->connection;
	
	if (event->crtc != m_currentCrtc)
	{
		changed |= RandR::ChangeCrtc;
		// update crtc settings
		if (m_currentCrtc != None)
			m_screen->crtc(m_currentCrtc)->loadSettings(true);
		setCrtc(event->crtc);
		if (m_currentCrtc != None)
			m_screen->crtc(m_currentCrtc)->loadSettings(true);
	}

	if (event->mode != mode())
		changed |= RandR::ChangeMode;
	
	if (event->rotation != rotation())
		changed |= RandR::ChangeRotation;
	
	if((event->connection == RR_Connected) != m_connected)
	{
		changed |= RandR::ChangeConnection;
		m_connected = (event->connection == RR_Connected);
		if (!m_connected && m_currentCrtc != None)
			setCrtc(None);
	}

	// check if we are still connected, if not, release the crtc connection
	if(!m_connected && m_currentCrtc != None)
		setCrtc(None);

	if(changed)
		emit outputChanged(m_id, changed);
}

void RandROutput::handlePropertyEvent(XRROutputPropertyNotifyEvent *event)
{
	// TODO: Do something with this!
	// By perusing thru some XOrg drivers, some of the properties that can
	// are configured through XRANDR are:
	// - LVDS Backlights
	// - TV output formats
	
	char *name = XGetAtomName(QX11Info::display(), event->property);
	kDebug() << "Got XRROutputPropertyNotifyEvent for property Atom " << name;
	XFree(name);	
}

QString RandROutput::name() const
{
	return m_name;
}

QString RandROutput::icon() const
{
	// FIXME: check what names we should use and what kind of outputs randr can 
	// report. It would also be interesting to be able to get the monitor name
	// using EDID or something like that, just don't know if it is even possible.
	if (m_name.contains("VGA"))
		return "video-display";
	else if (m_name.contains("LVDS"))
		return "video-display";
	
	// I doubt this is a good choice; can't find anything better in the spec.
	// video-x-generic might work, but that's a mimetype, which is inappropriate
	// for an output connection type.
	else if (m_name.contains("TV"))
		return "multimedia-player";

	return "video-display";
}

CrtcList RandROutput::possibleCrtcs() const
{
	return m_possibleCrtcs;
}

RRCrtc RandROutput::crtc() const
{
	return m_currentCrtc;
}

ModeList RandROutput::modes() const
{
	return m_modes;
}
/*
RandRMode RandROutput::mode() const
{
	if(!isConnected())
		return RandRMode();
	
	return m_screen->mode(this->mode());
}*/

RRMode RandROutput::mode() const
{
	if (!isConnected())
		return None;

	RandRCrtc *crtc = m_screen->crtc(m_currentCrtc);
	if (!crtc)
		return None;

	return crtc->mode();
}

SizeList RandROutput::sizes() const
{
	SizeList sizeList;

	foreach(RRMode m, m_modes)
	{
		RandRMode mode = m_screen->mode(m);
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
		return QRect(0, 0, 0, 0);

	return m_screen->crtc(m_currentCrtc)->rect();
}

RateList RandROutput::refreshRates(const QSize &s) const
{
	RateList list;
	QSize size = s;
	if (!size.isValid())
		size = rect().size();

	foreach(RRMode m, m_modes)
	{
		RandRMode mode = m_screen->mode(m);
		if (!mode.isValid())
			continue;
		if (mode.size() == size)
			list.append(mode.refreshRate());
	}
	return list;
}

float RandROutput::refreshRate() const
{
	return m_screen->mode( mode() ).refreshRate();
}

int RandROutput::rotations() const
{
	return m_rotations;
}

int RandROutput::rotation() const
{
	if (!isActive())
		return RandR::Rotate0;

	RandRCrtc *crtc = m_screen->crtc(m_currentCrtc);
	Q_ASSERT(crtc);

	return crtc->rotation();
}

bool RandROutput::isConnected() const
{
	return m_connected;
}

bool RandROutput::isActive() const
{
	return (m_connected && m_currentCrtc != None);
}

void RandROutput::proposeOriginal()
{
	if (m_currentCrtc != None)
		m_screen->crtc(m_currentCrtc)->proposeOriginal();
}

void RandROutput::load(KConfig &config)
{
	if (!m_connected)
		return;

	KConfigGroup cg = config.group("Screen_" + QString::number(m_screen->index()) + "_Output_" + m_name);
	bool active = cg.readEntry("Active", true);

	if (!active && !m_screen->outputsUnified())
	{
		setCrtc(None);
		return;
	}
		
	RandRCrtc *crtc = 0;
	
	// use the current crtc if any, or try to find an empty one
	if (m_currentCrtc != None)
		crtc = m_screen->crtc(m_currentCrtc);
	else
		crtc = findEmptyCrtc();

	// if there is no crtc we can use, stop processing
	if (!crtc)
		return;

	setCrtc(crtc->id());

	// if the outputs are unified, the screen will handle size changing
	if (!m_screen->outputsUnified() || m_screen->connectedCount() <=1)
	{
		m_proposedRect = cg.readEntry("Rect", QRect());
		m_proposedRotation = cg.readEntry("Rotation", (int) RandR::Rotate0);
	}
	m_proposedRate = cg.readEntry("RefreshRate", 0);
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

	// if the outputs are unified, do not save size and rotation
	// this allow us to set back the size and rotation being used
	// when the outputs are not unified.
	if (!m_screen->outputsUnified() || m_screen->connectedCount() <=1)
	{
		cg.writeEntry("Rect", crtc->rect());
		cg.writeEntry("Rotation", crtc->rotation());
	}
	cg.writeEntry("RefreshRate", (double) crtc->refreshRate());
}

void RandROutput::proposeRect(const QRect &r)
{
	m_originalRect = rect();
	m_proposedRect = r;
}

void RandROutput::proposePosition(const QPoint &p)
{
	proposeRect(QRect(p, rect().size()));
}

void RandROutput::proposeRotation(int r)
{
	m_originalRotation = rotation();
	m_proposedRotation = r;
}

void RandROutput::setRelation(RandROutput *output, Relation relation)
{
	if(!output)
		return;
	
	m_relatedOutput = output;
	m_relation = relation;
}

RandROutput *RandROutput::relation(Relation *rel) const
{
	Q_ASSERT(m_relatedOutput);
	
	if(rel)
		*rel = m_relation;
	
	return m_relatedOutput;
}

void RandROutput::slotChangeSize(QAction *action)
{
	QSize size = action->data().toSize();
	m_proposedRect.setSize(size);
	applyProposed(RandR::ChangeRect, true);
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
	setCrtc(None);
}

void RandROutput::slotEnable()
{
	if(!m_connected)
		return;
	
	kDebug() << "Attempting to enable " << m_name;
	RandRCrtc *crtc = findEmptyCrtc();
	
	if(crtc)
		setCrtc(crtc->id());
}

RandRCrtc *RandROutput::findEmptyCrtc()
{
	RandRCrtc *crtc = 0;

	foreach(RRCrtc c, m_possibleCrtcs)
	{
		crtc = m_screen->crtc(c);
		if (crtc->connectedOutputs().count() == 0)
			return crtc;
	}

	return 0;
}

bool RandROutput::tryCrtc(RandRCrtc *crtc, int changes)
{
	RRCrtc oldCrtc = m_currentCrtc;

	// if we are not yet using this crtc, switch to use it
	if (crtc->id() != m_currentCrtc)
		setCrtc(crtc->id());

	crtc->setOriginal();

	if (changes & RandR::ChangeRect)
	{
		crtc->proposeSize(m_proposedRect.size());
		crtc->proposePosition(m_proposedRect.topLeft());
	}
	if (changes & RandR::ChangeRotation)
		crtc->proposeRotation(m_proposedRotation);
	if (changes & RandR::ChangeRate)
		crtc->proposeRefreshRate(m_proposedRate);

	if (crtc->applyProposed())
		return true;

	// revert changes if we didn't succeed
	crtc->proposeOriginal();
	crtc->applyProposed();

	// switch back to the old crtc
	setCrtc(oldCrtc);
	return false;
}

bool RandROutput::applyProposed(int changes, bool confirm)
{
	KConfig cfg("krandrrc");
	RandRCrtc *crtc;

	QRect r;

	if (changes & RandR::ChangeRect)
		r = m_proposedRect;


	// first try to apply to the already attached crtc if any
	if (m_currentCrtc != None)
	{
		crtc = m_screen->crtc(m_currentCrtc);
		if (tryCrtc(crtc, changes))
		{
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
		}
		return false;
	}

	//then try an empty crtc
	crtc = findEmptyCrtc();

	// TODO: check if we can add this output to a CRTC which already has an output 
	// connection
	if (!crtc)
		return false;
		
	// try the crtc, and if no confirmation is needed or the user confirm, save the new settings
	if (tryCrtc(crtc, changes)) 
	{
		if (!confirm || confirm && RandR::confirm(crtc->rect()))
		{
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

	return false;
}

void RandROutput::setCrtc(RRCrtc c)
{
	if (c == m_currentCrtc)
		return;

	RandRCrtc *crtc;
	if (m_currentCrtc != None)
	{
		crtc = m_screen->crtc(m_currentCrtc);
		disconnect(crtc, SIGNAL(crtcChanged(RRCrtc, int)), 
			   this, SLOT(slotCrtcChanged(RRCrtc, int)));
		crtc->removeOutput(m_id);
		crtc->applyProposed();
	}
	m_currentCrtc = c;
	if (c == None)
		return;

	crtc = m_screen->crtc(c);
	crtc->addOutput(m_id);
	connect(crtc, SIGNAL(crtcChanged(RRCrtc, int)),
		this, SLOT(slotCrtcChanged(RRCrtc, int)));
}

void RandROutput::slotCrtcChanged(RRCrtc c, int changes)
{
	Q_UNUSED(c);

	//FIXME select which changes we should notify
	emit outputChanged(m_id, changes);
}

#include "randroutput.moc"


