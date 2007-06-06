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

	m_rect = QRect(m_info->x, m_info->y, m_info->width, m_info->height);

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

	m_currentMode = m_info->mode;
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
	if (event->x != m_rect.x() || event->y != m_rect.y())
	{
		changed |= ChangePosition;
		m_rect.translate(event->x, event->y);
	}

	if ((int) event->width != m_rect.width() || (int)event->height != m_rect.height())
	{
		changed |= ChangeSize;
		m_rect.setWidth(event->width);
		m_rect.setHeight(event->height);
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
	return m_rect;
}

bool RandRCrtc::setMode(RRMode mode)
{
	if (!m_connectedOutputs.count())
		mode = None;

	for (int i = 0; i < m_connectedOutputs.count(); ++i)
	{
		// all connected outputs should support the mode
		// FIXME: this can probably be done in a better way
		RandROutput *o = m_screen->output(m_connectedOutputs.at(i));
		if (o->modes().indexOf(mode) == -1)
			return false;
	}

	RROutput *outputs = new RROutput[m_connectedOutputs.count()];
	for (int i = 0; i < m_connectedOutputs.count(); ++i)
		outputs[i] = m_connectedOutputs.at(i);


	RandRMode newMode = m_screen->mode(mode);
	if (newMode.isValid())
	{
		QRect r(m_rect.topLeft(), newMode.size());
		r = QRect(0,0,0,0).united(r);
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

	Status s = XRRSetCrtcConfig(QX11Info::display(), m_screen->resources(), m_id, 
				    RandR::timestamp, m_rect.x(), m_rect.y(), mode,
				    m_currentRotation, outputs, m_connectedOutputs.count()); 
	
	bool ret;
	if (s == RRSetConfigSuccess)
	{
		m_currentMode = mode;
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

bool RandRCrtc::rotate(int rotation)
{
	// check if this crtc supports the asked rotation
	if (!rotation & m_rotations)
		return false;

	QRect r(m_rect.topLeft(), QSize(m_rect.height(), m_rect.width()));
	if (!m_screen->rect().contains(r))
	{
		// check if the rotated rect is smaller than the max screen size
		r = m_screen->rect().united(r);
		if (r.width() > m_screen->maxSize().width() || r.height() > m_screen->maxSize().height())
			return false;
		
		// adjust the screen size
		r = r.united(m_rect);
		if (!m_screen->adjustSize(r))
			return false;
	}

	m_currentRotation = rotation;
	setMode(m_currentMode);
	m_screen->adjustSize();

	return true;
}

bool RandRCrtc::addOutput(RROutput output, RRMode mode)
{
	// if no mode was given, use the current one
	if (!mode)
		mode = m_currentMode;

	// check if this output is not already on this crtc
	// if not, add it
	if (m_connectedOutputs.indexOf(output) == -1)
	{
		// the given output is not possible
		if (m_possibleOutputs.indexOf(output) == -1)
			return false;

		m_connectedOutputs.append(output);
	}

	return setMode(mode);
}

bool RandRCrtc::removeOutput(RROutput output)
{
	int index = m_connectedOutputs.indexOf(output);
	if (index == -1)
		return false;

	m_connectedOutputs.removeAt(index);
	return setMode(m_currentMode);
}	

#include "randrcrtc.moc"

#endif

