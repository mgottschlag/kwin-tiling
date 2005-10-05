/*
 *  Copyright (C) 2003 by Unai Garro <ugarro@users.sourceforge.net>
 *  Copyright (C) 2004 by Enrico Ros <rosenric@dei.unipd.it>
 *  Copyright (C) 2004 by Stephan Kulow <coolo@kde.org>
 *  Copyright (C) 2004 by Oswald Buddenhagen <ossi@kde.org>
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

#include "kdmlabel.h"

#include <kglobal.h>
#include <klocale.h>
#include <kmacroexpander.h>
#include <kdebug.h>

#include <qdatetime.h>
#include <qpainter.h>
#include <qfontmetrics.h>
#include <qtimer.h>
#include <qmap.h>

#include <unistd.h>
#include <sys/utsname.h>
#if !defined(HAVE_GETDOMAINNAME) && defined(HAVE_SYSINFO)
# include <sys/systeminfo.h>
#endif

KdmLabel::KdmLabel( KdmItem *parent, const QDomNode &node, const char *name )
    : KdmItem( parent, node, name )
{
	itemType = "label";

	// Set default values for label (note: strings are already Null)
	label.active.color.setRgb( 0xFFFFFF );
	label.active.present = false;
	label.prelight.present = false;
	label.maximumWidth = -1;

	const QString locale = KGlobal::locale()->language();

	// Read LABEL ID
	QDomNode n = node;
	QDomElement elLab = n.toElement();
	// ID types: clock, pam-error, pam-message, pam-prompt,
	//  pam-warning, timed-label
	label.id = elLab.attribute( "id", "" );
	label.hasId = !(label.id).isEmpty();

	// Read LABEL TAGS
	QDomNodeList childList = node.childNodes();
	bool stockUsed = false;
	for (int nod = 0; nod < childList.count(); nod++) {
		QDomNode child = childList.item( nod );
		QDomElement el = child.toElement();
		QString tagName = el.tagName();

		if (tagName == "pos")
			label.maximumWidth = el.attribute( "max-width", "-1" ).toInt();
		else if (tagName == "normal") {
			parseColor( el.attribute( "color", "#ffffff" ), label.normal.color );
			parseFont( el.attribute( "font", "Sans 14" ), label.normal.font );
		} else if (tagName == "active") {
			label.active.present = true;
			parseColor( el.attribute( "color", "#ffffff" ), label.active.color );
			parseFont( el.attribute( "font", "Sans 14" ), label.active.font );
		} else if (tagName == "prelight") {
			label.prelight.present = true;
			parseColor( el.attribute( "color", "#ffffff" ), label.prelight.color );
			parseFont( el.attribute( "font", "Sans 14" ), label.prelight.font );
		} else if (tagName == "text" && el.attributes().count() == 0 && !stockUsed) {
			label.text = el.text();
		} else if (tagName == "text" && !stockUsed) {
			QString lang = el.attribute( "xml:lang", "" );
			if (lang == locale)
				label.text = el.text();
		} else if (tagName == "stock") {
			label.text = lookupStock( el.attribute( "type", "" ) );
			stockUsed = true;
		}
	}

	// Check if this is a timer label
	label.isTimer = label.text.find( "%c" ) >= 0;
	if (label.isTimer) {
		timer = new QTimer( this );
		timer->start( 1000 );
		connect( timer, SIGNAL(timeout()), SLOT(updateTimer()) );
	}
	cText = lookupText( label.text );
}

void
KdmLabel::setText( const QString &txt )
{
	label.text = txt;
	cText = lookupText( label.text );
}

QSize
KdmLabel::sizeHint()
{
	// choose the correct label class
	struct LabelStruct::LabelClass *l = &label.normal;
	if (state == Sactive && label.active.present)
		l = &label.active;
	else if (state == Sprelight && label.prelight.present)
		l = &label.prelight;
	// get the hint from font metrics
	QSize hint = QFontMetrics( l->font ).size( Qt::AlignLeft | Qt::SingleLine, cText );
	// clip the result using the max-width label(pos) parameter
	if (label.maximumWidth > 0 && hint.width() > label.maximumWidth)
		hint.setWidth( label.maximumWidth );
	return hint;
}

void
KdmLabel::drawContents( QPainter *p, const QRect &/*r*/  )
{
	// choose the correct label class
	struct LabelStruct::LabelClass *l = &label.normal;
	if (state == Sactive && label.active.present)
		l = &label.active;
	else if (state == Sprelight && label.prelight.present)
		l = &label.prelight;
	// draw the label
	p->setFont( l->font );
	p->setPen( l->color );
	//TODO paint clipped (tested but not working..)
	p->drawText( area, Qt::AlignLeft | Qt::TextSingleLine, cText );
}

void
KdmLabel::statusChanged()
{
	KdmItem::statusChanged();
	if (!label.active.present && !label.prelight.present)
		return;
	if ((state == Sprelight && !label.prelight.present) ||
	    (state == Sactive && !label.active.present))
		return;
	needUpdate( area.x(), area.y(), area.width(), area.height() );
}

void
KdmLabel::updateTimer()
{
	QString text = lookupText( label.text );
	if (text != cText) {
		cText = text;
		needUpdate( area.x(), area.y(), area.width(), area.height() );
	}
}

static const struct {
	const char *type, *text;
} stocks[] = {
	{ "language",           I18N_NOOP("Language") },
	{ "session",            I18N_NOOP("Session Type") },
	{ "system",             I18N_NOOP("Menu") },	// i18n("Actions");
	{ "disconnect",         I18N_NOOP("Disconnect") },
	{ "quit",               I18N_NOOP("Quit") },
	{ "halt",               I18N_NOOP("Power off") },
	{ "suspend",            I18N_NOOP("Suspend") },
	{ "reboot",             I18N_NOOP("Reboot") },
	{ "chooser",            I18N_NOOP("XDMCP Chooser") },
	{ "config",             I18N_NOOP("Configure") },
	{ "caps-lock-warning",  I18N_NOOP("You have got caps lock on.") },
	{ "timed-label",        I18N_NOOP("User %s will login in %d seconds") },
	{ "welcome-label",      I18N_NOOP("Welcome to %h") },	// _greetString
	{ "username-label",     I18N_NOOP("Username:") },
	{ "password-label",     I18N_NOOP("Password:") },
	{ "login",              I18N_NOOP("Login") }
};

QString
KdmLabel::lookupStock( const QString &stock )
{
	//FIXME add key accels!
	QString type( stock.lower() );

	for (uint i = 0; i < sizeof(stocks)/sizeof(stocks[0]); i++)
		if (type == stocks[i].type)
			return i18n(stocks[i].text);

	kdDebug() << "Invalid <stock> element. Check your theme!" << endl;
	return stock;
}

QString
KdmLabel::lookupText( const QString &t )
{
	QString text = t;

	text.replace( '_', '&' );
//	text.remove( '_' ); // FIXME add key accels, remove underscores for now

	QHash<QChar,QString> m;
	struct utsname uts;
	uname( &uts );
	m['n'] = QString::fromLocal8Bit( uts.nodename );
	char buf[256];
	buf[sizeof(buf) - 1] = '\0';
	m['h'] = gethostname( buf, sizeof(buf) - 1 ) ? "localhost" : QString::fromLocal8Bit( buf );
#ifdef HAVE_GETDOMAINNAME
	m['o'] = getdomainname( buf, sizeof(buf) - 1 ) ? "localdomain" : QString::fromLocal8Bit( buf );
#elif defined(HAVE_SYSINFO)
	m['o'] = (unsigned)sysinfo( SI_SRPC_DOMAIN, buf, sizeof(buf) ) > sizeof(buf) ? "localdomain" : QString::fromLocal8Bit( buf );
#endif
	//m['d'] = delay;	not implemented (yet)
	//m['s'] = delayuser;	not implemented (yet)
        // xgettext:no-c-format
	KGlobal::locale()->setDateFormat( i18n("date format", "%a %d %B") );
	m['c'] = KGlobal::locale()->formatDateTime( QDateTime::currentDateTime(), false, false );

	return KMacroExpander::expandMacros( text, m );
}

#include "kdmlabel.moc"
