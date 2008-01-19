/*
 * Copyright (c) 2007, 2008 Harry Bock <hbock@providence.edu>
 * Copyright (c) 2007 Gustavo Pichorim Boiko <gustavo.boiko@kdemail.net>
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


#include "outputconfig.h"
#include "outputgraphicsitem.h"
#include "collapsiblewidget.h"
#include "layoutmanager.h"

#include "randrconfig.h"
#include "randroutput.h"
#include "randrdisplay.h"
#include "randrscreen.h"

RandRConfig::RandRConfig(QWidget *parent, RandRDisplay *display)
	: QWidget(parent), Ui::RandRConfigBase()
{
	m_display = display;
	Q_ASSERT(m_display);
	
	m_changed = false;
	m_firstLoad = true;

	if (!m_display->isValid()) {
		// FIXME: this needs much better handling of this error...
		return;
	}

	setupUi(this);

	// create the container for the settings widget
	QHBoxLayout *layout = new QHBoxLayout(outputList);
	layout->setSpacing(0);
	layout->setContentsMargins(0,0,0,0);
	m_container = new SettingsContainer(outputList);
	m_container->setSizePolicy(QSizePolicy::Minimum,
						  QSizePolicy::Minimum);
	layout->addWidget(m_container);

	// create the scene
	m_scene = new QGraphicsScene(m_display->currentScreen()->rect());	
	screenView->setScene(m_scene);

	m_layoutManager = new LayoutManager(m_display->currentScreen(), m_scene);
}

RandRConfig::~RandRConfig()
{
}

void RandRConfig::load(void)
{
	if (!m_display->isValid()) {
		kDebug() << "Invalid display! Aborting config load.";
		return;
	}
	
	if(!m_firstLoad) {
		qDeleteAll(m_outputList);
		m_outputList.clear();
		
		QList<QGraphicsItem*> items = m_scene->items();
		foreach(QGraphicsItem *i, items) {
			if(i->scene() == m_scene)
				m_scene->removeItem(i);
		}
	}
	
	m_firstLoad = false;
	
	OutputMap outputs = m_display->currentScreen()->outputs();

	// FIXME: adjust it to run on a multi screen system
	CollapsibleWidget *w;
	OutputGraphicsItem *o;
	foreach(RandROutput *output, outputs)
	{
		o = new OutputGraphicsItem(output);
		m_scene->addItem(o);
		
		connect(o,    SIGNAL(itemChanged(OutputGraphicsItem*)), 
		        this, SLOT(slotAdjustOutput(OutputGraphicsItem*)));

		OutputConfig *config = new OutputConfig(0, output, o);
		
		QString description = output->name() + 
		                     (output->isConnected() ? " (Connected)" : "");
		w = m_container->insertWidget(config, description);
		if(output->isConnected()) {
			w->setExpanded(true);
			kDebug() << "Output rect: " << output->rect();
		}
		m_outputList.append(w);
		
		connect(config, SIGNAL(updateView()), this, SLOT(slotUpdateView()));
		connect(config, SIGNAL(optionChanged()), this, SLOT(slotChanged()));
	}		    
	slotUpdateView();
}

void RandRConfig::save()
{
	if (!m_display->isValid())
		return;

	apply();
}

void RandRConfig::defaults()
{
	update();
}

void RandRConfig::apply()
{
	kDebug() << "Applying settings... ";
	foreach(CollapsibleWidget *w, m_outputList) {
		OutputConfig *config = static_cast<OutputConfig *>(w->innerWidget());
		RandROutput *output = config->output();
		
		if(!output->isConnected())
			continue;
		
		QSize res = config->resolution();
		QRect configuredRect(config->position(), res);
		
		if(!res.isNull()) {
			if(output->rect() == configuredRect) {
				kDebug() << "Ignoring identical config for " << output->name();
				continue;
			}
			
			kDebug() << "Output config: rect =" << configuredRect << ", rot = "
			         << config->rotation() << ", rate =" << config->refreshRate();
				    
			output->proposeRect(configuredRect);
			output->proposeRotation(config->rotation());
			output->proposeRefreshRate(config->refreshRate());
		} else { // user wants to disable this output
			kDebug() << "Disabling " << output->name();
			output->slotDisable();
		}
	}
	m_display->applyProposed();
	update();
}

void RandRConfig::slotChanged(void)
{
	m_changed = true;
	
	emit changed(true);
}

void RandRConfig::update()
{
	// TODO: implement
	m_changed = false;
	emit changed(false);
}

void RandRConfig::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
	slotUpdateView();
}

void RandRConfig::slotAdjustOutput(OutputGraphicsItem *o)
{
    Q_UNUSED(o);
	kDebug() << "Output graphics item changed: ";
	
	// TODO: Implement
}

void RandRConfig::slotUpdateView()
{
	QRect r;
	bool first = true;

	// updates the graphics view so that all outputs fit inside of it
	OutputMap outputs = m_display->currentScreen()->outputs();
	foreach(RandROutput *output, outputs)
	{		
		if (first)
		{
			first = false;
			r = output->rect();
		}
		else
			r = r.united(output->rect());
	}
	// scale the total bounding rectangle for all outputs to fit
	// 80% of the containing QGraphicsView
	float scaleX = (float)screenView->width() / r.width();
	float scaleY = (float)screenView->height() / r.height();
	float scale = (scaleX < scaleY) ? scaleX : scaleY;
	scale *= 0.80f;
	
	screenView->resetMatrix();
	screenView->scale(scale,scale);
	screenView->ensureVisible(r);
	screenView->setSceneRect(r);
}

#include "randrconfig.moc"

