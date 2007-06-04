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

	//get all crtcs
	for (int i = 0; i < m_resources->ncrtc; ++i)
	{
		if (m_crtcs.contains(m_resources->crtcs[i]))
			m_crtcs[m_resources->crtcs[i]]->loadSettings();
		else
		{
			RandRCrtc *c = new RandRCrtc(this, m_resources->crtcs[i]);
			connect(c, SIGNAL(crtcChanged(RRCrtc)), this, SIGNAL(configChanged()));
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
			connect(o, SIGNAL(outputChanged(RROutput)), this, SIGNAL(configChanged()));
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
#include "randrscreen.moc"

#endif

