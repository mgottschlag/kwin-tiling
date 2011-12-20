/*
 * Copyright (c) 2007      Gustavo Pichorim Boiko <gustavo.boiko@kdemail.net>
 * Copyright (c) 2007, 2008 Harry Bock <hbock@providence.edu>
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
#include <KShell>
#include <QX11Info>
#include <QAction>

RandROutput::RandROutput(RandRScreen *parent, RROutput id)
: QObject(parent)
{	
	m_screen = parent;
	Q_ASSERT(m_screen);

	m_id = id;
	m_crtc = 0;
	m_rotations = 0;
	
	queryOutputInfo();
	
	m_proposedRotation = m_originalRotation;
	m_proposedRate = m_originalRate;
	m_proposedRect = m_originalRect;
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

void RandROutput::queryOutputInfo(void)
{
	XRROutputInfo *info = XRRGetOutputInfo(QX11Info::display(), m_screen->resources(), m_id);
	Q_ASSERT(info);
	
	if (RandR::timestamp != info->timestamp)
		RandR::timestamp = info->timestamp;
	
	// Set up the output's connection status, name, and current
	// CRT controller.
	m_connected = (info->connection == RR_Connected);
	m_name = info->name;
	
	kDebug() << "XID" << m_id << "is output" << m_name <<
	            (isConnected() ? "(connected)" : "(disconnected)");
	
	setCrtc(m_screen->crtc(info->crtc));
	kDebug() << "Possible CRTCs for output" << m_name << ":";

	if (!info->ncrtc) {
		kDebug() << "   - none";
	}
	for(int i = 0; i < info->ncrtc; ++i) {
		kDebug() << "   - CRTC" << info->crtcs[i];
		m_possibleCrtcs.append(info->crtcs[i]);
	}
	
	//TODO: is it worth notifying changes on mode list changing?
	m_modes.clear();
	
	for (int i = 0; i < info->nmode; ++i) {
		if (i < info->npreferred) {
			m_preferredMode = m_screen->mode(info->modes[i]);
		}
		m_modes.append(info->modes[i]);
	}
	
	//get all possible rotations
	m_rotations = 0;
	for (int i = 0; i < m_possibleCrtcs.count(); ++i)
	{
		RandRCrtc *crtc = m_screen->crtc(m_possibleCrtcs.at(i));
		Q_ASSERT(crtc);
		m_rotations |= crtc->rotations();
	}
	m_originalRotation = m_crtc->rotation();
	m_originalRate     = m_crtc->refreshRate();
	m_originalRect     = m_crtc->rect();
	
	if(isConnected()) {
		kDebug() << "Current configuration for output" << m_name << ":";
		kDebug() << "   - Refresh rate:" << m_originalRate;
		kDebug() << "   - Rect:" << m_originalRect;
		kDebug() << "   - Rotation:" << m_originalRotation;
	}
	
	XRRFreeOutputInfo(info);
}

void RandROutput::loadSettings(bool notify)
{
	Q_UNUSED(notify);
	queryOutputInfo();
	
	kDebug() << "STUB: calling queryOutputInfo instead. Check if this has "
	            "any undesired effects. ";

	/*
	int changes = 0;
	
	XRROutputInfo *info = XRRGetOutputInfo(QX11Info::display(), m_screen->resources(), m_id);
	Q_ASSERT(info);
	
	if (RandR::timestamp != info->timestamp)
		RandR::timestamp = info->timestamp;
	
	m_possibleCrtcs.clear();
	for (int i = 0; i < info->ncrtc; ++i)
		m_possibleCrtcs.append(info->crtcs[i]);

	//check if the crtc changed
	if (info->crtc != m_crtc->id())
	{
		kDebug() << "CRTC changed for " << m_name << ": " << info->crtc;
		setCrtc(m_screen->crtc(info->crtc));
		changes |= RandR::ChangeCrtc;
	}

	bool connected = (info->connection == RR_Connected);
	if (connected != m_connected)
	{
		m_connected = connected;
		changes |= RandR::ChangeConnection;
	}

	// free the info
	XRRFreeOutputInfo(info);

	if (changes && notify)
		emit outputChanged(m_id, changes);
	*/
}

void RandROutput::handleEvent(XRROutputChangeNotifyEvent *event)
{
	int changed = 0;

	kDebug() << "[OUTPUT] Got event for " << m_name;
	kDebug() << "       crtc: " << event->crtc;
	kDebug() << "       mode: " << event->mode;
	kDebug() << "       rotation: " << event->rotation;
	kDebug() << "       connection: " << event->connection;

	//FIXME: handling these events incorrectly, causing an X11 I/O error...
	// Disable for now.
// 	kWarning() << "FIXME: Output event ignored!";
// 	return;
	
	RRCrtc currentCrtc = m_crtc->id();
	if (event->crtc != currentCrtc)
	{
		changed |= RandR::ChangeCrtc;
		// update crtc settings
		if (currentCrtc != None)
			m_crtc->loadSettings(true);
			//m_screen->crtc(m_currentCrtc)->loadSettings(true);
		setCrtc(m_screen->crtc(event->crtc), false);
		if (currentCrtc != None)
			m_crtc->loadSettings(true);
	}

	if (event->mode != mode().id())
		changed |= RandR::ChangeMode;
	
	if (event->rotation != rotation())
		changed |= RandR::ChangeRotation;
	
	if((event->connection == RR_Connected) != m_connected)
	{
		changed |= RandR::ChangeConnection;
		m_connected = (event->connection == RR_Connected);
		loadSettings(false);
		if (!m_connected && currentCrtc != None)
			setCrtc(None);
	}

	// check if we are still connected, if not, release the crtc connection
	if(!m_connected && m_crtc->isValid())
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
	// http://www.thinkwiki.org/wiki/Xorg_RandR_1.2#Output_port_names has a
	// list of possible output names, at least for the intel and radeon drivers
	// that support RandR 1.2. (nVidia drivers are not yet there at the time
	// of writing, 2008.)
	// FIXME: It would also be interesting to be able to get the monitor name
	// using EDID or something like that, just don't know if it is even possible.
	if (m_name.contains("VGA") || m_name.contains("DVI") || m_name.contains("TMDS"))
		return "video-display";
	else if (m_name.contains("LVDS"))
		return "video-display";
	else if (m_name.contains("TV") || m_name.contains("S-video"))
		return "video-television";

	return "video-display";
}

CrtcList RandROutput::possibleCrtcs() const
{
	return m_possibleCrtcs;
}

RandRCrtc *RandROutput::crtc() const
{
	return m_crtc;
}

ModeList RandROutput::modes() const
{
	return m_modes;
}

RandRMode RandROutput::mode() const
{
	if (!isConnected())
		return None;

	if (!m_crtc)
		return RandRMode();

	return m_crtc->mode();
}

RandRMode RandROutput::preferredMode(void) const
{
	return m_preferredMode;
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
	if (!m_crtc->isValid())
		return QRect(0, 0, 0, 0);

	return m_crtc->rect();
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
	if (!m_crtc->isValid())
		return 0;

	return m_crtc->mode().refreshRate();
}

int RandROutput::rotations() const
{
	return m_rotations;
}

int RandROutput::rotation() const
{
	if (!isActive())
		return RandR::Rotate0;

	Q_ASSERT(m_crtc);
	return m_crtc->rotation();
}

bool RandROutput::isConnected() const
{
	return m_connected;
}

bool RandROutput::isActive() const
{
	return (m_connected && m_crtc->id() != None);
}

void RandROutput::proposeOriginal()
{
	m_proposedRect = m_originalRect;
	m_proposedRate = m_originalRate;
	m_proposedRotation = m_originalRotation;

	if (m_crtc->id() != None)
		m_crtc->proposeOriginal();
}

void RandROutput::load(KConfig &config)
{
	if (!m_connected)
		return;

	KConfigGroup cg = config.group("Screen_" + QString::number(m_screen->index()) +
	                               "_Output_" + m_name);
	
	bool active = cg.readEntry("Active", true);

	if (!active && !m_screen->outputsUnified())
	{
		setCrtc(m_screen->crtc(None));
		return;
	}

	// use the current crtc if any, or try to find an empty one
	if (!m_crtc->isValid() && m_originalRect.isValid()) {
		kDebug() << "Finding empty CRTC for" << m_name;
		kDebug() << "  with rect = " << m_originalRect;
		
		m_crtc = findEmptyCrtc();
	}
	// if there is no crtc we can use, stop processing
	if (!m_crtc->isValid())
		return;

	setCrtc(m_crtc);

	// if the outputs are unified, the screen will handle size changing
	if (!m_screen->outputsUnified() || m_screen->connectedCount() <= 1)
	{
		m_proposedRect = (cg.readEntry("Rect", "0,0,0,0") == "0,0,0,0")
			? QRect() // "0,0,0,0" (serialization for QRect()) does not convert to a QRect
			: cg.readEntry("Rect", QRect());
		m_proposedRotation = cg.readEntry("Rotation", (int) RandR::Rotate0);
	}
	m_proposedRate = cg.readEntry("RefreshRate", 0);
}

void RandROutput::save(KConfig &config)
{
	KConfigGroup cg = config.group("Screen_" + QString::number(m_screen->index()) +
	                               "_Output_" + m_name);
	if (!m_connected)
		return;

	cg.writeEntry("Active", isActive());
	
	if (!isActive())
	     return;

	// if the outputs are unified, do not save size and rotation
	// this allow us to set back the size and rotation being used
	// when the outputs are not unified.
	if (!m_screen->outputsUnified() || m_screen->connectedCount() <=1)
	{
		cg.writeEntry("Rect", m_crtc->rect());
		cg.writeEntry("Rotation", m_crtc->rotation());
	}
	cg.writeEntry("RefreshRate", (double)m_crtc->refreshRate());
}

QStringList RandROutput::startupCommands() const
{
	if (!m_connected)
		return QStringList();
	if (!isActive())
		return QStringList() << QString( "xrandr --output %1 --off" ).arg( KShell::quoteArg( m_name ));
	if (m_crtc->id() == None)
	     return QStringList();	
	QString command = QString( "xrandr --output %1" ).arg( KShell::quoteArg( m_name ));
	// if the outputs are unified, do not save size and rotation
	// this allow us to set back the size and rotation being used
	// when the outputs are not unified.
	if (!m_screen->outputsUnified() || m_screen->connectedCount() <=1)
	{
		command += QString( " --pos %1x%2 --mode %3x%4" ).arg( m_crtc->rect().x())
		    .arg( m_crtc->rect().y()).arg( m_crtc->rect().width()).arg( m_crtc->rect().height());
		switch( m_crtc->rotation()) {
			case RandR::Rotate90:
				command += " --rotate left";
				break;
			case RandR::Rotate180:
				command += " --rotate inverted";
				break;
			case RandR::Rotate270:
				command += " --rotate right";
				break;
		}
	}
	command += QString(" --refresh %1").arg( m_crtc->refreshRate());
	return QStringList() << command;
}

void RandROutput::proposeRefreshRate(float rate)
{
	if (!m_crtc->isValid())
		slotEnable();
	
	m_originalRate = refreshRate();
	m_proposedRate = rate;
}

void RandROutput::proposeRect(const QRect &r)
{
	if (!m_crtc->isValid())
		slotEnable();
	
	m_originalRect = rect();
	m_proposedRect = r;
}

void RandROutput::proposeRotation(int r)
{
	if (!m_crtc->isValid())
		slotEnable();
	
	m_originalRotation = rotation();
	m_proposedRotation = r;
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
	m_originalRect = rect();
	m_proposedRect = QRect();
	m_originalRate = refreshRate();
	m_proposedRate = 0;
	setCrtc(m_screen->crtc(None));
}

void RandROutput::slotEnable()
{
	if(!m_connected)
		return;
	
	kDebug() << "Attempting to enable" << m_name;
	RandRCrtc *crtc = findEmptyCrtc();
	
	if(crtc)
		setCrtc(crtc);
}

void RandROutput::slotSetAsPrimary(bool primary)
{
	if (!primary)
	{
		if (m_screen->primaryOutput() == this)
		{
			kDebug() << "Removing" << m_name << "as primary output";
			m_screen->setPrimaryOutput(0);
		}
	}
	else if (m_connected)
	{
		kDebug() << "Setting" << m_name << "as primary output";
		m_screen->setPrimaryOutput(this);
	}
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
	kDebug() << "Trying to change output" << m_name << "to CRTC" << crtc->id() << "...";
	RandRCrtc *oldCrtc = m_crtc;

	// if we are not yet using this crtc, switch to use it
	if (crtc->id() != oldCrtc->id())
		setCrtc(crtc);

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

	if (crtc->applyProposed()) {
		kDebug() << "Changed output" << m_name << "to CRTC" << crtc->id();
		kDebug() << "   ( from old CRTC" << oldCrtc->id() << ")";
		return true;
	}

	// revert changes if we didn't succeed
	crtc->proposeOriginal();
	crtc->applyProposed();

	// switch back to the old crtc
	kDebug() << "Failed to change output" << m_name << "to CRTC" << crtc->id();
	kDebug() << "   Switching back to old CRTC" << oldCrtc->id();
	setCrtc(oldCrtc);
	return false;
}

bool RandROutput::applyProposed(int changes, bool confirm)
{
	// If disabled, save anyway to ensure it's saved
	if (!isConnected())
	{
		KConfig cfg("krandrrc");
		save(cfg);
		return true;
	}
	// Don't try to disable an already disabled output.
	if (!m_proposedRect.isValid() && !m_crtc->isValid()) {
		return true;
	}
	// Don't try to change an enabled output if there is nothing to change.
	if (m_crtc->isValid()
	    && (m_crtc->rect() == m_proposedRect || !(changes & RandR::ChangeRect))
	    && (m_crtc->rotation() == m_proposedRotation || !(changes & RandR::ChangeRotation))
	    && ((m_crtc->refreshRate() == m_proposedRate || !m_proposedRate || !(changes & RandR::ChangeRate))))
	{
		return true;
	}
	kDebug() << "Applying proposed changes for output" << m_name << "...";

	KConfig cfg("krandrrc");
	RandRCrtc *crtc;

	// first try to apply to the already attached crtc if any
	if (m_crtc->isValid())
	{
		crtc = m_crtc;
		if (tryCrtc(crtc, changes))
		{
			if ( !confirm || (confirm && RandR::confirm(crtc->rect())) )
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
		if ( !confirm || (confirm && RandR::confirm(crtc->rect())) )
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

bool RandROutput::setCrtc(RandRCrtc *crtc, bool applyNow)
{
	if( !crtc || (m_crtc && crtc->id() == m_crtc->id()) )
		return false;
	
	kDebug() << "Setting CRTC" << crtc->id()
	         << (crtc->isValid() ? "(enabled)" : "(disabled)")
	         << "on output" << m_name;

	if(m_crtc && m_crtc->isValid()) {
		disconnect(m_crtc, SIGNAL(crtcChanged(RRCrtc,int)), 
		           this, SLOT(slotCrtcChanged(RRCrtc,int)));
				 
		m_crtc->removeOutput(m_id);
		if( applyNow )
			m_crtc->applyProposed();
	}
	m_crtc = crtc;
	if (!m_crtc->isValid())
		return true;

	m_crtc->addOutput(m_id);
	connect(m_crtc, SIGNAL(crtcChanged(RRCrtc,int)),
	        this, SLOT(slotCrtcChanged(RRCrtc,int)));
		   
	return true;
}

void RandROutput::disconnectFromCrtc()
{ // but don't apply now
	setCrtc(m_screen->crtc(None), false);
}

void RandROutput::slotCrtcChanged(RRCrtc c, int changes)
{
	Q_UNUSED(c);

	//FIXME select which changes we should notify
	emit outputChanged(m_id, changes);
}

#include "randroutput.moc"


// vim:noet:sts=8:sw=8:
