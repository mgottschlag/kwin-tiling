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

#ifndef __RANDRCRTC_H__
#define __RANDRCRTC_H__

#include <QObject>
#include "randr.h"

#ifdef HAS_RANDR_1_2
class RandRScreen;
typedef QList<RROutput> OutputList;

class RandRCrtc : public QObject
{
	Q_OBJECT

public:
	RandRCrtc(RandRScreen *parent, RRCrtc id);
	~RandRCrtc();

	void loadSettings();
	
private:
	RRCrtc m_id;
	XRRCrtcInfo* m_info;

	QSize m_size;
	QPoint m_pos;
	OutputList m_connectedOutputs;
	OutputList m_possibleOutputs;
};
#endif

#endif
