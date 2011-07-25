/*
 * Copyright (c) 2007 Gustavo Pichorim Boiko <gustavo.boiko@kdemail.net>
 * Copyright (c) 2007 Harry Bock <hbock@providence.edu>
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

#ifndef __RANDRCONFIG_H__
#define __RANDRCONFIG_H__

#include "ui_randrconfigbase.h"
#include "randr.h"
#include "outputconfig.h"

#include <QWidget>
#include <QTimer>

class QGraphicsScene;
class SettingsContainer;
class CollapsibleWidget;
class RandRDisplay;
class OutputGraphicsItem;
class LayoutManager;

class RandRConfig : public QWidget, public Ui::RandRConfigBase
{
	Q_OBJECT
public:
	RandRConfig(QWidget *parent, RandRDisplay *display);
	virtual ~RandRConfig();

	void load(void);
	void save(void);
	void defaults(void);

	void apply();
	void update();

	virtual bool x11Event(XEvent* e);

public slots:
	void slotUpdateView();
	void slotDelayedUpdateView();

protected slots:
	void slotChanged(void);
	void slotAdjustOutput(OutputGraphicsItem *o);
	void identifyOutputs();
	void clearIndicators();
	void saveStartup();
	void disableStartup();
	void unifiedOutputChanged(bool checked);
	void outputConnectedChanged(bool);

signals:
	void changed(bool change);

protected:
	virtual bool eventFilter(QObject *obj, QEvent *event);
	
private:
        void insufficientVirtualSize();
	RandRDisplay *m_display;
	bool m_changed;
	bool m_firstLoad;
	
	SettingsContainer *m_container;
	QList<CollapsibleWidget*> m_outputList;
	QGraphicsScene *m_scene;
	LayoutManager *m_layoutManager;
	QList<QWidget*> m_indicators;
	QTimer identifyTimer;
	OutputConfigList m_configs;
	QTimer compressUpdateViewTimer;
};


#endif
