/*
 *  Copyright (C) 2003 by Unai Garro <ugarro@users.sourceforge.net>
 *  Copyright (C) 2007 by Oswald Buddenhagen <ossi@kde.org>
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

#include "kdmbutton.h"

#include "kdmthemer.h"
#include "kdmlabel.h"

#include <kglobal.h>
#include <klocale.h>

#include <QPushButton>

KdmButton::KdmButton( QObject *parent, const QDomNode &node )
	: KdmItem( parent, node )
{
	itemType = "button";
	if (!isVisible())
		return;

	const QString locale = KGlobal::locale()->language();

	// Read LABEL TAGS
	QDomNodeList childList = node.childNodes();
	bool stockUsed = false;
	for (int nod = 0; nod < childList.count(); nod++) {
		QDomNode child = childList.item( nod );
		QDomElement el = child.toElement();
		QString tagName = el.tagName();

		if (tagName == "text" && el.attributes().count() == 0 && !stockUsed) {
			text = el.text();
		} else if (tagName == "text" && !stockUsed) {
			QString lang = el.attribute( "xml:lang", "" );
			if (lang == locale)
				text = el.text();
		} else if (tagName == "stock") {
			text = KdmLabel::lookupStock( el.attribute( "type", "" ) );
			stockUsed = true;
		}
	}

	text.replace( '\n', ' ' ).replace( '_', '&' );
}

void
KdmButton::doPlugActions( bool )
{
	QWidget *w = themer()->widget();
	if (w) {
		if (!myWidget) {
			QPushButton *btn = new QPushButton( text, w );
			btn->setAutoDefault( false );
			myWidget = btn;
			myWidget->hide(); // yes, really
			setWidgetAttribs( myWidget );
			connect( myWidget, SIGNAL(destroyed()), SLOT(widgetGone()) );
			connect( myWidget, SIGNAL(clicked()), SLOT(activate()) );
			emit needPlacement();
		}
	} else {
		if (myWidget)
			delete myWidget;
	}
}

void
KdmButton::widgetGone()
{
	myWidget = 0;

	emit needPlacement();
}

void
KdmButton::activate()
{
	emit activated( objectName() );
}

void
KdmButton::drawContents( QPainter *, const QRect & )
{
}

#include "kdmbutton.moc"
