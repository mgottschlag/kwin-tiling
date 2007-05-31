/*
 * Copyright (c) 2007 Gustavo Pichorim Boiko <gustavo.boiko@kdemail.net>
 * Copyright (c) 2002 Hamish Rodda <rodda@kde.org>
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

#ifndef __LEGACYRANDRCONFIG_H__
#define __LEGACYRANDRCONFIG_H__

#include <QButtonGroup>
#include <QWidget>
#include "ui_legacyrandrconfigbase.h"

class RandRDisplay;

class LegacyRandRConfig : public QWidget, public Ui::LegacyRandRConfigBase
{
	Q_OBJECT
public:
	LegacyRandRConfig(QWidget *parent, RandRDisplay *display);
	virtual ~LegacyRandRConfig();

	void load();
	void save();
	void defaults();

	void apply();
	void update();

protected Q_SLOTS:
	void slotScreenChanged(int screen);
	void slotRotationChanged();
	void slotSizeChanged(int index);
	void slotRefreshChanged(int index);
	void setChanged();

signals:
	void changed(bool c);

protected:
	void addRotationButton(int thisRotation, bool checkbox);
	void populateRefreshRates();

private:
	RandRDisplay *m_display;
	bool m_oldApply;
	bool m_oldSyncTrayApp;
	bool m_changed;
	QButtonGroup m_rotationGroup;
};

#endif
