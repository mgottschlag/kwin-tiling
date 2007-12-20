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

#include "randrmode.h"


RandRMode::RandRMode(XRRModeInfo *info)
{
	m_valid = false;
	m_rate = 0;
	
	if (info)
		m_valid = true;
	else
		return;

	m_name = info->name;
	m_id = info->id;

	m_size.setWidth(info->width);
	m_size.setHeight(info->height);
	
	// calculate the refresh rate
	if (info->hTotal && info->vTotal)
		m_rate = ((float) info->dotClock / ((float) info->hTotal * (float) info->vTotal));
	else
		m_rate = 0;

}

RandRMode::~RandRMode()
{
	// nothing to do for now
}

RRMode RandRMode::id() const
{
	if (!m_valid)
		return None;

	return m_id;
}

QString RandRMode::name() const
{
	return m_name;
}

QSize RandRMode::size() const
{
	return m_size;
}

float RandRMode::refreshRate() const
{
	return m_rate;
}

bool RandRMode::isValid() const
{
	return m_valid;
}

