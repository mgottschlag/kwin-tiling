/*
 * Copyright (c) 2002 Hamish Rodda <meddie@yoyo.its.monash.edu.au>
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
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef KRANDRMODULE_H
#define KRANDRMODULE_H

#include "randr.h"

class QButtonGroup;
class KComboBox;

class KRandRModule : public KCModule, public RandRDisplay
{
	Q_OBJECT

public:
	KRandRModule(QWidget *parent, const char *name, const QStringList& _args);

	virtual void load();
	virtual void save();
	virtual void defaults();

protected slots:
	void slotScreenChanged(int screen);
	void slotRotationChanged();
	void slotSizeChanged(int index);
	void slotRefreshChanged(int index);
	void setChanged();

protected:
	void apply();
	void update();

	void addRotationButton(int thisRotation, bool checkbox);
	void populateRefreshRates();

	KComboBox*		m_screenSelector;
	KComboBox*		m_sizeCombo;
	QButtonGroup*	m_rotationGroup;
	KComboBox*		m_refreshRates;
	QCheckBox*		m_applyOnStartup;
	bool			m_oldApply;

	bool			m_changed;
};

#endif
