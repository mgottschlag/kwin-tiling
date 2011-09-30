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

#include <KConfig>
#include <KConfigGroup>
#include <QAction>

RandRScreen::RandRScreen(int screenIndex)
: m_originalPrimaryOutput(0),
  m_proposedPrimaryOutput(0),
  m_resources(0)
{
	m_index = screenIndex;
	m_rect = QRect(0, 0, XDisplayWidth(QX11Info::display(), m_index),
				 XDisplayHeight(QX11Info::display(), m_index));

	m_connectedCount = 0;
	m_activeCount = 0;

	loadSettings();
	KConfig cfg("krandrrc");
	load(cfg, true);

	m_originalPrimaryOutput = primaryOutput();

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
	kDebug() << "Creating CRTC object for XID 0 (\"None\")";
	RandRCrtc *c_none = new RandRCrtc(this, None);
	m_crtcs[None] = c_none;
	
	for (int i = 0; i < m_resources->ncrtc; ++i)
	{
		if (m_crtcs.contains(m_resources->crtcs[i]))
			m_crtcs[m_resources->crtcs[i]]->loadSettings(notify);
		else
		{
			kDebug() << "Creating CRTC object for XID" << m_resources->crtcs[i];
			RandRCrtc *c = new RandRCrtc(this, m_resources->crtcs[i]);
			connect(c, SIGNAL(crtcChanged(RRCrtc,int)), this, SIGNAL(configChanged()));
			connect(c, SIGNAL(crtcChanged(RRCrtc,int)), this, SLOT(save()));
			c->loadSettings(notify);
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
			kDebug() << "Creating output object for XID" << m_resources->outputs[i];
			RandROutput *o = new RandROutput(this, m_resources->outputs[i]);
			connect(o, SIGNAL(outputChanged(RROutput,int)), this,
				      SLOT(slotOutputChanged(RROutput,int)));
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

#ifdef HAS_RANDR_1_3
void RandRScreen::setPrimaryOutput(RandROutput* output)
{
	if (RandR::has_1_3)
	{
		RROutput id = None;
		if (output)
			id = output->id();
		XRRSetOutputPrimary(QX11Info::display(), rootWindow(), id);
	}
}

void RandRScreen::proposePrimaryOutput(RandROutput* output)
{
	m_proposedPrimaryOutput = output;
}

RandROutput* RandRScreen::primaryOutput()
{
	if (RandR::has_1_3)
	{
		return output(XRRGetOutputPrimary(QX11Info::display(), rootWindow()));
	}
	return 0;
}
#endif

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

bool RandRScreen::outputsUnified() const
{
	return m_outputsUnified;
}

void RandRScreen::setOutputsUnified(bool unified)
{
	m_outputsUnified = unified;
	
	// should this be called here?
	slotUnifyOutputs(unified);
}

int RandRScreen::unifiedRotations() const
{

	bool first = true;
	int rotations = RandR::Rotate0;

	foreach(RandRCrtc *crtc, m_crtcs)
	{
		if (!crtc->connectedOutputs().count())
			continue;

		if (first)
		{
			rotations = crtc->rotations();
			first = false;
		}
		else
			rotations &= crtc->rotations();
	}

	return rotations;
}

SizeList RandRScreen::unifiedSizes() const
{
	SizeList sizeList;
	bool first = true;

	foreach(RandROutput *output, m_outputs)
	{
		if (!output->isConnected())
			continue;

		if (first)
		{
			// we start using the list from the first output
			sizeList = output->sizes();
			first = false;
		}
		else
		{
			SizeList outputSizes = output->sizes();
			for (int i = sizeList.count() - 1; i >=0; --i)
			{
				// check if the current output has the i-th size of the sizeList
				// if not, remove from the list
				if (outputSizes.indexOf(sizeList[i]) == -1)
					sizeList.removeAt(i);
			}
		}
	}

	return sizeList;
}

QRect RandRScreen::rect() const
{
	return m_rect;
}

void RandRScreen::load(KConfig& config, bool skipOutputs)
{
	KConfigGroup group = config.group("Screen_" + QString::number(m_index));
	m_outputsUnified = group.readEntry("OutputsUnified", false);
	m_unifiedRect = (group.readEntry("UnifiedRect", "0,0,0,0") == "0,0,0,0")
		? QRect() // "0,0,0,0" (serialization for QRect()) does not convert to a QRect
		: group.readEntry("UnifiedRect", QRect());
	m_unifiedRotation = group.readEntry("UnifiedRotation", (int) RandR::Rotate0);

//	slotUnifyOutputs(m_outputsUnified);

	if (skipOutputs) {
		return;
	}

	foreach(RandROutput *output, m_outputs)
	{
		if (output->isConnected())
			output->load(config);
	}
}

void RandRScreen::save(KConfig &config)
{
	KConfigGroup group = config.group("Screen_" + QString::number(m_index));
	group.writeEntry("OutputsUnified", m_outputsUnified);
	group.writeEntry("UnifiedRect", m_unifiedRect);
	group.writeEntry("UnifiedRotation", m_unifiedRotation);

	foreach(RandROutput *output, m_outputs)
	{
		if (output->isConnected())
			output->save(config);
	}
}

void RandRScreen::save()
{
	KConfig cfg("krandrrc");
	save(cfg);
}

QStringList RandRScreen::startupCommands() const
{
	QStringList commands;
	foreach(RandROutput *output, m_outputs)
	{
		if (output->isConnected())
			commands += output->startupCommands();
	}
	return commands;
}

void RandRScreen::load()
{
	KConfig cfg("krandrrc");
	load(cfg);
}

bool RandRScreen::applyProposed(bool confirm)
{
	kDebug() << "Applying proposed changes for screen" << m_index << "...";
	
	bool succeed = true;
	QRect r;
	
	foreach(RandROutput *output, m_outputs) {
		/*
		r = output->rect();
		RandROutput::Relation outputRelation;
		RandROutput *related = output->relation(&outputRelation);
		
		if(!related) {
			r.setTopLeft(QPoint(0, 0));
		}
		QRect relativeRect = related->rect();
	
		switch(outputRelation) {
		case RandROutput::LeftOf:
			r.setTopLeft(QPoint(relativeRect.x() - r.x(),
			                    relativeRect.y()));
		case RandROutput::RightOf:
			r.setTopLeft(QPoint(relativeRect.x() + r.x(),
			                    relativeRect.y()));
		case RandROutput::Over:
			r.setTopLeft(QPoint(relativeRect.x(),
			                    relativeRect.y() - r.y()));
		case RandROutput::Under:
			r.setTopLeft(QPoint(relativeRect.x(),
			                    relativeRect.y() + r.y()));
							
		case RandROutput::SameAs:
			r.setTopLeft(related->rect().topLeft());
			break;
		}
		output->proposeRect(r);
		*/
		if(!output->applyProposed()) {
			succeed = false;
			break;
		}
	}
	/*
	foreach(RandROutput *output, m_outputs)
	{
		r = output->rect();
		if (!output->applyProposed())
		{
			succeed = false;
			break;
		}
	}*/
#ifdef HAS_RANDR_1_3
	if (succeed)
	{
		setPrimaryOutput(m_proposedPrimaryOutput);
	}
#endif //HAS_RANDR_1_3

	kDebug() << "Changes have been applied to all outputs.";

	// if we could apply the config clean, ask for confirmation
	if (succeed && confirm)
		succeed = RandR::confirm(r);

	// if we succeeded applying and the user confirmed the changes,
	// just return from here 
	if (succeed)
		return true;

	kDebug() << "Changes canceled, reverting to original setup.";

	//Revert changes if not succeed
	foreach(RandROutput *o, m_outputs)
	{
		if (o->isConnected())
		{
			o->proposeOriginal();
			o->applyProposed();
		}
	}

#ifdef HAS_RANDR_1_3
	m_proposedPrimaryOutput = m_originalPrimaryOutput;
	setPrimaryOutput(m_proposedPrimaryOutput);
#endif //HAS_RANDR_1_3
	return false;
}

void RandRScreen::unifyOutputs()
{
	KConfig cfg("krandrrc");
	SizeList sizes = unifiedSizes();

	//FIXME: better handle this
	if (!sizes.count())
		return;

	// if there is only one output connected, there is no way to unify it
	if (m_connectedCount <= 1)
		return;

	if (sizes.indexOf(m_unifiedRect.size()) == -1)
		m_unifiedRect.setSize(sizes.first());

	kDebug() << "Unifying outputs using rect " << m_unifiedRect;
	// iterate over all outputs and make sure all connected outputs get activated
	// and use the right size
	foreach(RandROutput *o, m_outputs)
	{
		// if the output is not connected we don't need to do anything
		if (!o->isConnected())
		       continue;

		// if the output is connected and already has the same rect and rotation
		// as the unified ones, continue
		if (o->isActive() && o->rect() == m_unifiedRect 
				  && o->rotation() == m_unifiedRotation)
			continue;

		// this is to get the refresh rate to use
		//o->load(cfg);
		o->proposeRect(m_unifiedRect);
		o->proposeRotation(m_unifiedRotation);
		o->applyProposed(RandR::ChangeRect | RandR::ChangeRotation, false);
	}

	// FIXME: if by any reason we were not able to unify the outputs, we should 
	// do something
	save();
	emit configChanged();
}

void RandRScreen::slotResizeUnified(QAction *action)
{
	m_unifiedRect.setSize(action->data().toSize()); 
	unifyOutputs();
}

void RandRScreen::slotUnifyOutputs(bool unified)
{
	m_outputsUnified = unified;
	KConfig cfg("krandrrc");

	if (!unified || m_connectedCount <= 1)
	{
		foreach(RandROutput *output, m_outputs)
			if (output->isConnected())
			{
				output->load(cfg);
				output->applyProposed();
			}
	}
	else
	{
		SizeList sizes = unifiedSizes();

		if (!sizes.count())
		{
			// FIXME: this should be better handle
			return;
		}

		m_unifiedRect.setTopLeft(QPoint(0,0));
		m_unifiedRect.setSize(sizes.first());
		unifyOutputs();
	}
}

void RandRScreen::slotRotateUnified(QAction *action)
{
	m_unifiedRotation = action->data().toInt(); 
	
	unifyOutputs();
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

	// if there is less than 2 outputs connected, there is no need to unify
	if (connected <= 1)
		return;
}

#include "randrscreen.moc"


// vim:noet:sts=8:sw=8:
