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
#include "randrscreen.h"
#include "randrcrtc.h"
#include "randroutput.h"
#include "randrmode.h"

#ifdef HAS_RANDR_1_2
RandRScreen::RandRScreen(int screenIndex)
	: m_resources(0L)
{
	m_index = screenIndex;
	m_rect = QRect(0, 0, XDisplayWidth(QX11Info::display(), m_index), XDisplayHeight(QX11Info::display(), m_index));

	loadSettings();

	// select for input events
	int mask = RRScreenChangeNotifyMask | 
		   RRCrtcChangeNotifyMask | 
		   RROutputChangeNotifyMask | 
		   RROutputPropertyNotifyMask;
	XRRSelectInput(QX11Info::display(), rootWindow(), 0);
	XRRSelectInput(QX11Info::display(), rootWindow(), mask); 
}

RandRScreen::~RandRScreen()
{
	if (m_resources)
		XRRFreeScreenResources(m_resources);
}

XRRScreenResources* RandRScreen::resources() const
{
	return m_resources;
}

Window RandRScreen::rootWindow() const
{
	return RootWindow(QX11Info::display(), m_index);
}

void RandRScreen::loadSettings()
{
	int minW, minH, maxW, maxH;

	Status status = XRRGetScreenSizeRange(QX11Info::display(), rootWindow(),
					 &minW, &minH, &maxW, &maxH);
	//FIXME: we should check the status here
	Q_UNUSED(status);
	m_minSize = QSize(minW, minH);
	m_maxSize = QSize(maxW, maxH);

	if (m_resources)
		XRRFreeScreenResources(m_resources);

	m_resources = XRRGetScreenResources(QX11Info::display(), rootWindow());
	Q_ASSERT(m_resources);

	if (RandR::timestamp != m_resources->timestamp)
		RandR::timestamp = m_resources->timestamp;

	//get all crtcs
	for (int i = 0; i < m_resources->ncrtc; ++i)
	{
		if (m_crtcs.contains(m_resources->crtcs[i]))
			m_crtcs[m_resources->crtcs[i]]->loadSettings();
		else
		{
			RandRCrtc *c = new RandRCrtc(this, m_resources->crtcs[i]);
			connect(c, SIGNAL(crtcChanged(RRCrtc, int)), this, SIGNAL(configChanged()));
			m_crtcs[m_resources->crtcs[i]] = c;
		}

	}

	//get all outputs
	for (int i = 0; i < m_resources->noutput; ++i)
	{
		if (m_outputs.contains(m_resources->outputs[i]))
			m_outputs[m_resources->outputs[i]]->loadSettings();
		else
		{
			RandROutput *o = new RandROutput(this, m_resources->outputs[i]);
			connect(o, SIGNAL(outputChanged(RROutput, int)), this, SIGNAL(configChanged()));
			m_outputs[m_resources->outputs[i]] = o;
		}
	}

	// get all modes
	for (int i = 0; i < m_resources->nmode; ++i)
	{
		if (!m_modes.contains(m_resources->modes[i].id))
			m_modes[m_resources->modes[i].id] = RandRMode(&m_resources->modes[i]);

	}
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

	return NULL;
}

OutputMap RandRScreen::outputs() const
{
	return m_outputs;
}

RandROutput* RandRScreen::output(RROutput id) const
{
	if (m_outputs.contains(id))
		return m_outputs[id];

	return NULL;
}

ModeMap RandRScreen::modes() const
{
	return m_modes;
}

RandRMode RandRScreen::mode(RRMode id) const
{
	if (m_modes.contains(id))
		return m_modes[id];

	return RandRMode();
}

bool RandRScreen::adjustSize(QRect minimumSize)
{
	//try to find a size in which all outputs fit
	
	//start with a 0x0 rect located at (0,0)
	QRect rect = minimumSize;

	OutputMap::const_iterator it;
	for (it = m_outputs.constBegin(); it != m_outputs.constEnd(); ++it)
	{
		// outputs that are not active should not be taken into account
		// when calculating the screen size
		RandROutput *o = (*it);
		if (!o->isActive())
			continue;
		rect = rect.united(o->rect());
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

bool RandRScreen::setSize(QSize s)
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

QRect RandRScreen::rect() const
{
	return m_rect;
}

#include "randrscreen.moc"

#endif

