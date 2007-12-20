/*
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

#include "collapsiblewidget.h"
#include "outputconfig.h"
#include "outputgraphicsitem.h"
#include "randrconfig.h"
#include "randrdisplay.h"
#include "randrscreen.h"
#include "randroutput.h"
#include "layoutmanager.h"
#include "randr.h"
#include <kdebug.h>


RandRConfig::RandRConfig(QWidget *parent, RandRDisplay *display)
: QWidget(parent), Ui::RandRConfigBase()
{
	m_display = display;
	Q_ASSERT(m_display);
	m_changed = false;

	if (!m_display->isValid())
		return;

	setupUi(this);

	// create the container for the settings widget
	QHBoxLayout *l = new QHBoxLayout(outputList);
	l->setSpacing(0);
	l->setContentsMargins(0,0,0,0);
	m_container = new SettingsContainer(outputList);
	l->addWidget(m_container);


	// create the scene
	m_scene = new QGraphicsScene(m_display->currentScreen()->rect());	
	screenView->setScene(m_scene);

	m_layoutManager = new LayoutManager(m_display->currentScreen(), m_scene);
}

RandRConfig::~RandRConfig()
{
}

void RandRConfig::load()
{
	kDebug() << "LOAD";
	if (!m_display->isValid())
		return;

	qDeleteAll(m_outputList);
	m_outputList.clear();

	QList<QGraphicsItem*> items = m_scene->items();
	foreach(QGraphicsItem *i, items)
		m_scene->removeItem(i);

	OutputMap outputs = m_display->currentScreen()->outputs();
	OutputMap::iterator it;

	// FIXME: adjust it to run on a multi screen system
	CollapsibleWidget *w;
	OutputGraphicsItem *o;
	foreach(RandROutput *output, outputs)
	{
		o = new OutputGraphicsItem(output);
		m_scene->addItem(o);

		connect(o, SIGNAL(itemChanged(OutputGraphicsItem*)), 
				this, SLOT(slotAdjustOutput(OutputGraphicsItem*)));

		OutputConfig *c = new OutputConfig(0, output, o);
		w = m_container->insertWidget(c, output->name());
		m_outputList.append(w);
		kDebug() << "Rect: " << output->rect();
		connect(c, SIGNAL(updateView()), this, SLOT(slotUpdateView()));
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
	if (m_changed) {
		m_display->applyProposed();

		update();
	}
}

void RandRConfig::update()
{
	// TODO: implement
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
	float scaleX = (float)screenView->width() / r.width();
	float scaleY = (float)screenView->height() / r.height();
	float scale = (scaleX < scaleY)? scaleX : scaleY;
	kDebug() << "Scaling by " << scale;
	kDebug() << "ScreenView rect = " << screenView->rect() << " visible rect: " << r;
	screenView->resetMatrix();
	screenView->scale(scale,scale);
	screenView->ensureVisible(r);
	screenView->setSceneRect(r);
}

#include "randrconfig.moc"

