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

#ifdef HAS_RANDR_1_2
RandRCrtc::RandRCrtc(RandRScreen *parent, RRCrtc id)
	: QObject(parent), m_info(0L)
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

int RandRCrtc::currentRotation() const
{
	return m_currentRotation;
}

void RandRCrtc::loadSettings()
{
	if (m_info)
		XRRFreeCrtcInfo(m_info);

	m_info = XRRGetCrtcInfo(QX11Info::display(), m_screen->resources(), m_id);
	Q_ASSERT(m_info);

	if (RandR::timestamp != m_info->timestamp)
		RandR::timestamp = m_info->timestamp;

	m_currentRect = QRect(m_info->x, m_info->y, m_info->width, m_info->height);

	// get all connected outputs 
	// and create a list of modes that are available in all connected outputs
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

	m_currentMode = m_info->mode;
	RandRMode m = m_screen->mode(m_currentMode);
	m_currentRate = m.refreshRate();

	// just to make sure it gets initialized
	m_proposedRect = m_currentRect;
	m_proposedRotation = m_currentRotation;
	m_proposedRate = m_currentRate;
}

void RandRCrtc::handleEvent(XRRCrtcChangeNotifyEvent *event)
{
	int changed = 0;

	if (event->mode != m_currentMode)
	{
		changed |= ChangeMode;
		m_currentMode = event->mode;
	}
	
	if (event->rotation != m_currentRotation)
	{
		changed |= ChangeRotation;
		m_currentRotation = event->rotation;
	}
	if (event->x != m_currentRect.x() || event->y != m_currentRect.y())
	{
		changed |= ChangePosition;
		m_currentRect.translate(event->x, event->y);
	}

	if ((int) event->width != m_currentRect.width() || (int)event->height != m_currentRect.height())
	{
		changed |= ChangeSize;
		m_currentRect.setWidth(event->width);
		m_currentRect.setHeight(event->height);
	}

	if (changed)
		emit crtcChanged(m_id, changed);
}

RRMode RandRCrtc::currentMode() const
{
	return m_currentMode;
}

QRect RandRCrtc::rect() const
{
	return m_currentRect;
}

bool RandRCrtc::applyProposed()
{
#if 0
	kDebug() << "[CRTC] Going to apply...." << endl;
	kDebug() << "       Current Screen rect: " << m_screen->rect() << endl;
	kDebug() << "       Current CRTC Rect: " << m_currentRect << endl;
	kDebug() << "       Current Rotation: " << m_currentRotation << endl;
	kDebug() << "       Proposed rect: " << m_proposedRect << endl;
	kDebug() << "       Proposed rotation: " << m_proposedRotation << endl;
	kDebug() << "       Outputs: " << endl;
	for (int i = 0; i < m_connectedOutputs.count(); ++i)
		kDebug() << "               - " << m_screen->output(m_connectedOutputs.at(i))->name() << endl;
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
		ModeList::iterator it;
		for (it = modeList.begin(); it != modeList.end(); ++it)
		{
			RandRMode m = m_screen->mode(*it);
			if (m.size() == m_proposedRect.size() && (!m_proposedRate || m_proposedRate == m.refreshRate()))
			{
				mode = m;
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
		emit crtcChanged(m_id, ChangeMode);
		ret = true;
	}
	else
	{
		ret = false;
		loadSettings();
	}

	m_screen->adjustSize();
	return ret;
}

bool RandRCrtc::proposeSize(QSize s)
{
	m_proposedRect.setSize(s);
	m_proposedRate = 0;
	return true;
}

bool RandRCrtc::proposePosition(QPoint p)
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

bool RandRCrtc::addOutput(RROutput output, QSize s)
{
	// if no mode was given, use the current one
	if (!s.isValid())
		s = m_currentRect.size();

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
	ModeList m;

	bool first = true;
	OutputList::iterator out;

	for (int i = 0; i < m_connectedOutputs.count(); ++i)
	{
		RandROutput *output = m_screen->output(m_connectedOutputs.at(i));
		if (first)
		{
			m = output->modes();
			first = false;
		}
		else
		{
			ModeList::iterator it;
			for (it = m.begin(); it != m.end(); ++it)
			{
				if (output->modes().indexOf(*it) == -1)
					it = m.erase(it);
			}
		}
	}

	return m;
}

#include "randrcrtc.moc"

#endif

