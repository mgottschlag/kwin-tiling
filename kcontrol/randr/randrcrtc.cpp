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
#include "randroutput.h"
#include "randrmode.h"

RandRCrtc::RandRCrtc(RandRScreen *parent, RRCrtc id)
: QObject(parent)
{
	m_screen = parent;
	Q_ASSERT(m_screen);

	m_id = id;
	loadSettings();
}

RandRCrtc::~RandRCrtc()
{
	// do nothing for now
}

RRCrtc RandRCrtc::id() const
{
	return m_id;
}

int RandRCrtc::rotations() const
{
	return m_rotations;
}

int RandRCrtc::rotation() const
{
	return m_currentRotation;
}

void RandRCrtc::loadSettings(bool notify)
{
	int changes = 0;
	XRRCrtcInfo *info = XRRGetCrtcInfo(QX11Info::display(), m_screen->resources(), m_id);
	Q_ASSERT(info);

	if (RandR::timestamp != info->timestamp)
		RandR::timestamp = info->timestamp;

	QRect rect = QRect(info->x, info->y, info->width, info->height);
	if (rect != m_currentRect)
	{
		m_currentRect = rect;
		changes |= RandR::ChangeRect;
	}

	// get all connected outputs 
	// and create a list of modes that are available in all connected outputs
	OutputList outputs;

	for (int i = 0; i < info->noutput; ++i)
		outputs.append(info->outputs[i]);

	// check if the list changed from the original one
	if (outputs != m_connectedOutputs)
	{
		changes |= RandR::ChangeOutputs;
		m_connectedOutputs = outputs;	
	}
	
	// get all outputs this crtc can be connected to
	outputs.clear();
	for (int i = 0; i < info->npossible; ++i)
		outputs.append(info->possible[i]);

	if (outputs != m_possibleOutputs)
	{
		changes |= RandR::ChangeOutputs;
		m_possibleOutputs = outputs;
	}

	// get all rotations
	m_rotations = info->rotations;
	if (m_currentRotation != info->rotation)
	{
		m_currentRotation = info->rotation;
		changes |= RandR::ChangeRotation;
	}

	// check if the current mode has changed
	if (m_currentMode != info->mode)
	{
		m_currentMode = info->mode;
		changes |= RandR::ChangeMode;
	}

	RandRMode m = m_screen->mode(m_currentMode);
	if (m_currentRate != m.refreshRate())
	{
		m_currentRate = m.refreshRate();
		changes |= RandR::ChangeRate;
	}

	// just to make sure it gets initialized
	m_proposedRect = m_currentRect;
	m_proposedRotation = m_currentRotation;
	m_proposedRate = m_currentRate;
		
	// free the info
	XRRFreeCrtcInfo(info);

	if (changes && notify)
		emit crtcChanged(m_id, changes);
}

void RandRCrtc::handleEvent(XRRCrtcChangeNotifyEvent *event)
{
	kDebug() << "[CRTC] Event...";
	int changed = 0;

	if (event->mode != m_currentMode)
	{
		kDebug() << "   Changed mode";
		changed |= RandR::ChangeMode;
		m_currentMode = event->mode;
	}
	
	if (event->rotation != m_currentRotation)
	{
		kDebug() << "   Changed rotation: " << event->rotation;
		changed |= RandR::ChangeRotation;
		m_currentRotation = event->rotation;
	}
	if (event->x != m_currentRect.x() || event->y != m_currentRect.y())
	{
		kDebug() << "   Changed position: " << event->x << "," << event->y;
		changed |= RandR::ChangeRect;
		m_currentRect.moveTopLeft(QPoint(event->x, event->y));
	}

	RandRMode mode = m_screen->mode(m_currentMode);
	if (mode.size() != m_currentRect.size())
	{
		kDebug() << "   Changed size: " << mode.size();
		changed |= RandR::ChangeRect;
		m_currentRect.setSize(mode.size());
		//Do NOT use event->width and event->height here, as it is being returned wrongly
	}

	if (changed)
		emit crtcChanged(m_id, changed);
}

RRMode RandRCrtc::mode() const
{
	return m_currentMode;
}

QRect RandRCrtc::rect() const
{
	return m_currentRect;
}

float RandRCrtc::refreshRate() const
{
	return m_currentRate;
}

bool RandRCrtc::applyProposed()
{
#if 1
	kDebug() << "[CRTC] Going to apply (" << m_id << ") ....";
	kDebug() << "       Current Screen rect: " << m_screen->rect();
	kDebug() << "       Current CRTC Rect: " << m_currentRect;
	kDebug() << "       Current Rotation: " << m_currentRotation;
	kDebug() << "       Proposed rect: " << m_proposedRect;
	kDebug() << "       Proposed rotation: " << m_proposedRotation;
	kDebug() << "       Proposed refresh rate: " << m_proposedRate;
	kDebug() << "       Outputs: ";
	for (int i = 0; i < m_connectedOutputs.count(); ++i)
		kDebug() << "               - " << m_screen->output(m_connectedOutputs.at(i))->name();
#endif
	RandRMode mode;
	if (m_proposedRect.size() == m_currentRect.size() && m_proposedRate == m_currentRate)
	{
		mode = m_screen->mode(m_currentMode);
	}
	else
	{
		// find a mode that has the desired size and is supported
		// by all connected outputs
		ModeList modeList = modes();
		ModeList matchModes;

		foreach(RRMode m, modeList)
		{
			RandRMode mode = m_screen->mode(m);
			if (mode.size() == m_proposedRect.size())
				matchModes.append(m);
		}

		// if no matching modes were found, disable output
		// else set the mode to the first mode in the list. If no refresh rate was given
		// or no mode was found matching the given refresh rate, the first mode of the
		// list will be used
		if (!matchModes.count())
			mode = RandRMode();
		else
			mode = m_screen->mode(matchModes.first());

		foreach(RRMode m, matchModes)
		{
			RandRMode testMode = m_screen->mode(m);
			if (testMode.refreshRate() == m_proposedRate)
			{
				mode = testMode;
				break;
			}
		}
	}
	
	// if no output was connected, set the mode to None
	if (!m_connectedOutputs.count())
		mode = RandRMode();
	else if (!mode.isValid())
		return false;

	RROutput *outputs = new RROutput[m_connectedOutputs.count()];
	for (int i = 0; i < m_connectedOutputs.count(); ++i)
		outputs[i] = m_connectedOutputs.at(i);

	if (mode.isValid())
	{
		if (m_currentRotation == m_proposedRotation ||
		    m_currentRotation == RandR::Rotate0 && m_proposedRotation == RandR::Rotate180 ||
		    m_currentRotation == RandR::Rotate180 && m_proposedRotation == RandR::Rotate0 ||
		    m_currentRotation == RandR::Rotate90 && m_proposedRotation == RandR::Rotate270 ||
		    m_currentRotation == RandR::Rotate270 && m_proposedRotation == RandR::Rotate90)
		{
			QRect r = QRect(0,0,0,0).united(m_proposedRect);
			if (r.width() > m_screen->maxSize().width() || r.height() > m_screen->maxSize().height())
				return false;

			// if the desired mode is bigger than the current screen size, first change the 
			// screen size, and then the crtc size
			if (!m_screen->rect().contains(r))
			{
				// try to adjust the screen size
				if (!m_screen->adjustSize(r))
					return false;
			}

		}
		else
		{

			QRect r(m_proposedRect.topLeft(), QSize(m_proposedRect.height(), m_proposedRect.width()));
			if (!m_screen->rect().contains(r))
			{
				// check if the rotated rect is smaller than the max screen size
				r = m_screen->rect().united(r);
				if (r.width() > m_screen->maxSize().width() || r.height() > m_screen->maxSize().height())
					return false;
				
				// adjust the screen size
				r = r.united(m_currentRect);
				if (!m_screen->adjustSize(r))
					return false;
			}
		}
	}


	Status s = XRRSetCrtcConfig(QX11Info::display(), m_screen->resources(), m_id, 
				    RandR::timestamp, m_proposedRect.x(), m_proposedRect.y(), mode.id(),
				    m_proposedRotation, outputs, m_connectedOutputs.count()); 

	bool ret;
	if (s == RRSetConfigSuccess)
	{
		m_currentMode = mode.id();
		m_currentRotation = m_proposedRotation;
		m_currentRect = m_proposedRect;
		m_currentRate = mode.refreshRate();
		emit crtcChanged(m_id, RandR::ChangeMode);
		ret = true;
	}
	else
	{
		ret = false;
		loadSettings(true);
	}

	m_screen->adjustSize();
	return ret;
}

bool RandRCrtc::proposeSize(const QSize &s)
{
	m_proposedRect.setSize(s);
	m_proposedRate = 0;
	return true;
}

bool RandRCrtc::proposePosition(const QPoint &p)
{
	m_proposedRect.moveTopLeft(p);
	return true;
}

bool RandRCrtc::proposeRotation(int rotation)
{
	// check if this crtc supports the asked rotation
	if (!rotation & m_rotations)
		return false;

	m_proposedRotation = rotation;
	return true;

}

bool RandRCrtc::proposeRefreshRate(float rate)
{
	m_proposedRate = rate;
	return true;
}

void RandRCrtc::proposeOriginal()
{
	m_proposedRotation = m_originalRotation;
	m_proposedRect = m_originalRect;
	m_proposedRate = m_originalRate;
}

void RandRCrtc::setOriginal()
{
	m_originalRotation = m_currentRotation;
	m_originalRect = m_currentRect;
	m_originalRate = m_currentRate;
}

bool RandRCrtc::proposedChanged()
{
	return (m_proposedRotation != m_currentRotation ||
		m_proposedRect != m_currentRect ||
		m_proposedRate != m_currentRate);
}

bool RandRCrtc::addOutput(RROutput output, const QSize &s)
{
	QSize size = s;
	// if no mode was given, use the current one
	if (!size.isValid())
		size = m_currentRect.size();

	// check if this output is not already on this crtc
	// if not, add it
	if (m_connectedOutputs.indexOf(output) == -1)
	{
		// the given output is not possible
		if (m_possibleOutputs.indexOf(output) == -1)
			return false;

		m_connectedOutputs.append(output);
	}
	m_proposedRect = QRect(m_proposedRect.topLeft(), s);
	return true;
}

bool RandRCrtc::removeOutput(RROutput output)
{
	int index = m_connectedOutputs.indexOf(output);
	if (index == -1)
		return false;

	m_connectedOutputs.removeAt(index);
	return true;
}	

OutputList RandRCrtc::connectedOutputs() const
{
	return m_connectedOutputs;
}

ModeList RandRCrtc::modes() const
{	
	ModeList modeList;

	bool first = true;

	foreach(RROutput o, m_connectedOutputs)
	{
		RandROutput *output = m_screen->output(o);
		if (first)
		{
			modeList = output->modes();
			first = false;
		}
		else
		{
			foreach(RRMode m, modeList)
			{
				if (output->modes().indexOf(m) == -1)
					modeList.removeAll(m);
			}
		}
	}

	return modeList;
}

#include "randrcrtc.moc"


