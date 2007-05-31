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

#include "randrconfig.h"
#include "randrdisplay.h"
#include "randr.h"

#ifdef HAS_RANDR_1_2

RandRConfig::RandRConfig(QWidget *parent, RandRDisplay *display)
: QWidget(parent), Ui::RandRConfigBase()
{
	m_display = display;
	Q_ASSERT(m_display);

	if (!m_display->isValid())
		return;

	setupUi(this);

	load();
}

RandRConfig::~RandRConfig()
{
}

void RandRConfig::load()
{
	if (!m_display->isValid())
		return;

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

#include "randrconfig.moc"

#endif
