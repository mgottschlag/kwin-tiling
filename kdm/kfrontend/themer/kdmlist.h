/*
 *  Copyright (C) 2003 by Unai Garro <ugarro@users.sourceforge.net>
 *  Copyright (C) 2004 by Enrico Ros <rosenric@dei.unipd.it>
 *  Copyright (C) 2004 by Stephan Kulow <coolo@kde.org>
 *  Copyright (C) 2004,2006 by Oswald Buddenhagen <ossi@kde.org>
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

#ifndef KDMLIST_H
#define KDMLIST_H

#include "kdmitem.h"

#include <QColor>

/*
 * KdmList: A themed (user) list
 */

class KdmList : public KdmItem {
	Q_OBJECT

public:
	KdmList( QObject *parent, const QDomNode &node );

protected:
	// no-op
	virtual void drawContents( QPainter *p, const QRect &r );

	struct ListStruct {
		//QColor iconBg;
		QColor labelBg, altLabelBg;
	} list;

	virtual void setWidget( QWidget *widget );
};

#endif
