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

#ifndef KRANDRMODULE_H
#define KRANDRMODULE_H

#include <QLabel>
#include <QLayout>
#include <KDialog>
#include <KCModule>

class RandRDisplay;
class LegacyRandRConfig;
class RandRConfig;

class KRandRModule : public KCModule
{
	Q_OBJECT

public:
	KRandRModule(QWidget *parent, const QVariantList& _args);
	~KRandRModule(void);

	virtual void load();
	virtual void save();
	virtual void defaults();
	virtual bool x11Event(XEvent* );

protected:
	void apply();
	void update();

private:
	LegacyRandRConfig *m_legacyConfig;
	RandRConfig *m_config;
	RandRDisplay *m_display;
};

#endif
