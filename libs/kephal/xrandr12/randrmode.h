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

#ifndef __RANDRMODE_H__
#define __RANDRMODE_H__

#include "randr.h"


class RandRMode
{
public:
	RandRMode(XRRModeInfo *info = 0);
	~RandRMode();

	RRMode id() const;
	QString name() const;
	bool isValid() const;
	QSize size() const;
	float refreshRate() const;
private:
	bool m_valid;
	QString m_name;
	QSize m_size;
	float m_rate;
	RRMode m_id;
};


#endif
