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

#include "kdmlist.h"
#include "kdmthemer.h"

#include <QListWidget>

KdmList::KdmList( QObject *parent, const QDomNode &node )
	: KdmItem( parent, node )
{
	itemType = "list";

	// A list can have no properties (defaults to parent ones)
	if (node.isNull())
		return;

	// Read RECT TAGS
	QDomNodeList childList = node.childNodes();
	for (int nod = 0; nod < childList.count(); nod++) {
		QDomNode child = childList.item( nod );
		QDomElement el = child.toElement();
		QString tagName = el.tagName();

		if (tagName == "color") {
			//parseColor( el.attribute( "iconcolor", QString() ), QString(), list.iconBg );
			parseColor( el.attribute( "labelcolor", QString() ), QString(), list.labelBg );
			parseColor( el.attribute( "altlabelcolor", QString() ), QString(), list.altLabelBg );
		}
	}
}

void
KdmList::drawContents( QPainter *, const QRect & )
{
}

void
KdmList::setWidget( QWidget *widget )
{
	KdmItem::setWidget( widget );
	if (QListWidget *lw = qobject_cast<QListWidget *>( widget )) {
		if (list.labelBg.isValid()) {
			QPalette pal;
			pal.setColor( QPalette::Base, list.labelBg );
			if (list.altLabelBg.isValid()) {
				pal.setColor( QPalette::AlternateBase, list.altLabelBg );
				lw->setAlternatingRowColors( true );
			}
			lw->setPalette( pal );
		} else
			lw->viewport()->setAutoFillBackground( false );
	}
}

#include "kdmlist.moc"
