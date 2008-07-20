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

#include "randrscreen.h"
#include "randrcrtc.h"
#include "randroutput.h"
#include "randrmode.h"


RandRScreen::RandRScreen(int screenIndex)
: m_resources(0L)
{
	m_index = screenIndex;
	m_rect = QRect(0, 0, XDisplayWidth(QX11Info::display(), m_index),
				 XDisplayHeight(QX11Info::display(), m_index));

	m_connectedCount = 0;
	m_activeCount = 0;

	loadSettings();

	// select for randr input events
	int mask = RRScreenChangeNotifyMask | 
		   RRCrtcChangeNotifyMask   | 
		   RROutputChangeNotifyMask | 
		   RROutputPropertyNotifyMask;
	
	XRRSelectInput(QX11Info::display(), rootWindow(), 0);
	XRRSelectInput(QX11Info::display(), rootWindow(), mask); 
}

RandRScreen::~RandRScreen()
{
	if (m_resources)
		XRRFreeScreenResources(m_resources);

	//qDeleteAll(m_crtcs);
	//qDeleteAll(m_outputs);
	//qDeleteAll(m_modes);
}

int RandRScreen::index() const
{
	return m_index;
}

XRRScreenResources* RandRScreen::resources() const
{
	return m_resources;
}

Window RandRScreen::rootWindow() const
{
	return RootWindow(QX11Info::display(), m_index);
}

void RandRScreen::loadSettings(bool notify)
{
	bool changed = false;
	int minW, minH, maxW, maxH;

	Status status = XRRGetScreenSizeRange(QX11Info::display(), rootWindow(),
					 &minW, &minH, &maxW, &maxH);
	//FIXME: we should check the status here
	Q_UNUSED(status);
	QSize minSize = QSize(minW, minH);
	QSize maxSize = QSize(maxW, maxH);

	if (minSize != m_minSize || maxSize != m_maxSize)
	{
		m_minSize = minSize;
		m_maxSize = maxSize;
		changed = true;
	}

	if (m_resources)
		XRRFreeScreenResources(m_resources);

	m_resources = XRRGetScreenResources(QX11Info::display(), rootWindow());
	Q_ASSERT(m_resources);

	RandR::timestamp = m_resources->timestamp;

	// get all modes
	for (int i = 0; i < m_resources->nmode; ++i)
	{
		if (!m_modes.contains(m_resources->modes[i].id))
		{
			m_modes[m_resources->modes[i].id] = RandRMode(&m_resources->modes[i]);
			changed = true;
		}
	}

	//get all crtcs
	RandRCrtc *c_none = new RandRCrtc(this, None);
	m_crtcs[None] = c_none;
	
	for (int i = 0; i < m_resources->ncrtc; ++i)
	{
		if (m_crtcs.contains(m_resources->crtcs[i]))
			m_crtcs[m_resources->crtcs[i]]->loadSettings(notify);
		else
		{
			RandRCrtc *c = new RandRCrtc(this, m_resources->crtcs[i]);
			connect(c, SIGNAL(crtcChanged(RRCrtc, int)), this, SIGNAL(configChanged()));
			m_crtcs[m_resources->crtcs[i]] = c;
			changed = true;
		}
	}

	//get all outputs
	for (int i = 0; i < m_resources->noutput; ++i)
	{
		if (m_outputs.contains(m_resources->outputs[i]))
			;//m_outputs[m_resources->outputs[i]]->loadSettings(notify);
		else
		{
			RandROutput *o = new RandROutput(this, m_resources->outputs[i]);
			connect(o, SIGNAL(outputChanged(RROutput, int)), this,
				      SLOT(slotOutputChanged(RROutput, int)));
			m_outputs[m_resources->outputs[i]] = o;
			if (o->isConnected())
				m_connectedCount++;
			if (o->isActive())
				m_activeCount++;

			changed = true;
		}
	}

	if (notify && changed)
		emit configChanged();

}

void RandRScreen::handleEvent(XRRScreenChangeNotifyEvent* event)
{
	m_rect.setWidth(event->width);
	m_rect.setHeight(event->height);

	emit configChanged();
}

void RandRScreen::handleRandREvent(XRRNotifyEvent* event)
{
	RandRCrtc *c;
	RandROutput *o;
	XRRCrtcChangeNotifyEvent *crtcEvent;
	XRROutputChangeNotifyEvent *outputEvent;
	XRROutputPropertyNotifyEvent *propertyEvent;

	// forward events to crtcs and outputs
	switch (event->subtype) {
		case RRNotify_CrtcChange:
			crtcEvent = (XRRCrtcChangeNotifyEvent*)event;
			c = crtc(crtcEvent->crtc);
			Q_ASSERT(c);
			c->handleEvent(crtcEvent);
			return;

		case RRNotify_OutputChange:
			outputEvent = (XRROutputChangeNotifyEvent*)event;
			o = output(outputEvent->output);
			Q_ASSERT(o);
			o->handleEvent(outputEvent);
			return;

		case RRNotify_OutputProperty:
			propertyEvent = (XRROutputPropertyNotifyEvent*)event;
			o = output(propertyEvent->output);
			Q_ASSERT(o);
			o->handlePropertyEvent(propertyEvent);
			return;
	}	
}

QSize RandRScreen::minSize() const
{
	return m_minSize;
}

QSize RandRScreen::maxSize() const
{
	return m_maxSize;
}

CrtcMap RandRScreen::crtcs() const
{
	return m_crtcs;
}

RandRCrtc* RandRScreen::crtc(RRCrtc id) const
{
	if (m_crtcs.contains(id))
		return m_crtcs[id];

	return 0;
}

OutputMap RandRScreen::outputs() const
{
	return m_outputs;
}

RandROutput* RandRScreen::output(RROutput id) const
{
	if (m_outputs.contains(id))
		return m_outputs[id];

	return 0;
}

ModeMap RandRScreen::modes() const
{
	return m_modes;
}

RandRMode RandRScreen::mode(RRMode id) const
{
	if (m_modes.contains(id))
		return m_modes[id];

	return RandRMode(0);
}

bool RandRScreen::adjustSize(const QRect &minimumSize)
{
	//try to find a size in which all outputs fit
	
	//start with a given minimum rect
	QRect rect = QRect(0, 0, 0, 0).united(minimumSize);

	foreach(RandROutput *output, m_outputs)
	{
		// outputs that are not active should not be taken into account
		// when calculating the screen size
		if (!output->isActive())
			continue;
		rect = rect.united(output->rect());
	}


	// check bounds
	if (rect.width() < m_minSize.width())
		rect.setWidth(m_minSize.width());
	if (rect.height() < m_minSize.height())
		rect.setHeight(m_minSize.height());

	if (rect.width() > m_maxSize.width())
		return false;
	if (rect.height() > m_maxSize.height())
		return false;

	return setSize(rect.size());
}

bool RandRScreen::setSize(const QSize &s)
{
	if (s == m_rect.size())
		return true;

	if (s.width() < m_minSize.width() || 
	    s.height() < m_minSize.height() ||
	    s.width() > m_maxSize.width() ||
	    s.height() > m_maxSize.height())
		return false;

	int widthMM, heightMM;
	float dpi;

	/* values taken from xrandr */
	dpi = (25.4 * DisplayHeight(QX11Info::display(), m_index)) / DisplayHeightMM(QX11Info::display(), m_index);
	widthMM =  (int) ((25.4 * s.width()) / dpi);
	heightMM = (int) ((25.4 * s.height()) / dpi);

	XRRSetScreenSize(QX11Info::display(), rootWindow(), s.width(), s.height(), widthMM, heightMM);
	m_rect.setSize(s);

	return true;
}

int RandRScreen::connectedCount() const
{
	return m_connectedCount;
}

int RandRScreen::activeCount() const
{
	return m_activeCount;
}

QRect RandRScreen::rect() const
{
	return m_rect;
}

void RandRScreen::slotOutputChanged(RROutput id, int changes)
{
	Q_UNUSED(id);
	Q_UNUSED(changes);

	int connected = 0, active = 0;
	foreach(RandROutput *output, m_outputs)
	{
		if (output->isConnected())
			connected++;
		if (output->isActive())
			active++;
	}

	m_connectedCount = connected;
	m_activeCount = active;
}


