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

#include <config.h>
#include <config-kdm.h>

#include "kdmlabel.h"
#include "kdmthemer.h"

#include <kglobal.h>
#include <klocale.h>
#include <kmacroexpander.h>

#include <QAction>
#include <QDateTime>
#include <QFontMetrics>
#include <QHash>
#include <QPainter>
#include <QTimer>

#include <unistd.h>
#include <sys/utsname.h>
#if !defined(HAVE_GETDOMAINNAME) && defined(HAVE_SYSINFO)
# include <sys/systeminfo.h>
#endif

KdmLabel::KdmLabel( QObject *parent, const QDomNode &node )
	: KdmItem( parent, node )
	, action( 0 )
{
	itemType = "label";

	// Set default values for label (note: strings are already Null)
	label.normal.font = label.active.font = label.prelight.font = style.font;
	label.normal.color = label.active.color = label.prelight.color =
		style.palette.isBrushSet( QPalette::Normal, QPalette::WindowText ) ?
			style.palette.color( QPalette::Normal, QPalette::WindowText ) :
			QColor( Qt::white );
	label.active.present = false;
	label.prelight.present = false;

	const QString locale = KGlobal::locale()->language();

	// Read LABEL TAGS
	QDomNodeList childList = node.childNodes();
	bool stockUsed = false;
	for (int nod = 0; nod < childList.count(); nod++) {
		QDomNode child = childList.item( nod );
		QDomElement el = child.toElement();
		QString tagName = el.tagName();

		if (tagName == "normal") {
			parseColor( el, label.normal.color );
			parseFont( el, label.normal.font );
		} else if (tagName == "active") {
			label.active.present = true;
			parseColor( el, label.active.color );
			parseFont( el, label.active.font );
		} else if (tagName == "prelight") {
			label.prelight.present = true;
			parseColor( el, label.prelight.color );
			parseFont( el, label.prelight.font );
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
	label.isTimer = label.text.indexOf( "%c" ) >= 0;
	if (label.isTimer) {
		timer = new QTimer( this );
		timer->start( 1000 );
		connect( timer, SIGNAL(timeout()), SLOT(update()) );
	}
	label.text.replace( '\n', ' ' );
	setCText( lookupText( label.text ) );
}

void
KdmLabel::setText( const QString &txt )
{
	label.text = txt;
	label.text.replace( '\n', ' ' );
	update();
}

void
KdmLabel::setCText( const QString &txt )
{
	delete action;
	action = 0;
	pText = cText = txt;
	pAccelOff = txt.indexOf( '_' );
	if (pAccelOff >= 0) {
		action = new QAction( this );
		action->setShortcut( Qt::ALT + txt[pAccelOff + 1].unicode() );
		connect( action, SIGNAL(triggered( bool )), SLOT(activate()) );
		emit needPlugging();
		pText.remove( pAccelOff, 1 );
	}
}

void
KdmLabel::activate()
{
	KdmItem *cp = this;
	do {
		if (cp->isButton) {
			emit activated( cp->objectName() );
			return;
		}
		cp = qobject_cast<KdmItem *>(cp->parent());
	} while (cp);
	if (!buddy.isEmpty())
		activateBuddy();
}

void
KdmLabel::doPlugActions( bool plug )
{
	if (action) {
		QWidget *w = themer()->widget();
		if (plug)
			w->addAction( action );
		else
			w->removeAction( action );
	}
}

QSize
KdmLabel::sizeHint()
{
	return QFontMetrics( label.normal.font.font ).size( 0, pText );
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
	p->setFont( l->font.font );
	p->setPen( l->color );
	if (pAccelOff != -1) {
		QRect tarea( area );
		QFontMetrics fm( l->font.font );
		QString left = pText.left( pAccelOff );
		p->drawText( area, 0, left );
		tarea.setLeft( tarea.left() + fm.width( left ) );
		QFont f( l->font.font );
		f.setUnderline( true );
		p->setFont( f );
		QString acc( pText[pAccelOff] );
		p->drawText( tarea, 0, acc );
		tarea.setLeft( tarea.left() + fm.width( acc ) );
		p->setFont( l->font.font );
		p->drawText( tarea, 0, pText.mid( pAccelOff + 1 ) );
	} else
		p->drawText( area, 0, cText );
}

void
KdmLabel::statusChanged( bool descend )
{
	KdmItem::statusChanged( descend );
	if (!label.active.present && !label.prelight.present)
		return;
	if ((state == Sprelight && !label.prelight.present) ||
	    (state == Sactive && !label.active.present))
		return;
	needUpdate();
}

void
KdmLabel::update()
{
	KdmItem::update();
	QString text = lookupText( label.text );
	if (text != cText) {
		setCText( text );
		needUpdate();
		// emit needPlacement() as well?
	}
}

static const struct {
	const char *type, *text;
} stocks[] = {
	{ "language",          I18N_NOOP("_Language") },
	{ "session",           I18N_NOOP("Session _Type") },
	{ "system",            I18N_NOOP("_Menu") },	// i18n("Actions");
	{ "disconnect",        I18N_NOOP("Disconn_ect") },
	{ "quit",              I18N_NOOP("_Quit") },
	{ "halt",              I18N_NOOP("Power o_ff") },
	{ "suspend",           I18N_NOOP("_Suspend") },
	{ "reboot",            I18N_NOOP("Re_boot") },
	{ "chooser",           I18N_NOOP("XDMCP Choose_r") },
	{ "config",            I18N_NOOP("Confi_gure") },
	{ "caps-lock-warning", I18N_NOOP("Caps Lock is enabled") },
	{ "timed-label",       I18N_NOOP("User %s will log in in %d seconds") },
	{ "welcome-label",     I18N_NOOP("Welcome to %h") },	// _greetString
	{ "domain-label",      I18N_NOOP("_Domain:") },
	{ "username-label",    I18N_NOOP("_Username:") },
	{ "password-label",    I18N_NOOP("_Password:") },
	{ "login",             I18N_NOOP("_Login") }
};

QString
KdmLabel::lookupStock( const QString &stock )
{
	QString type( stock.toLower() );

	for (uint i = 0; i < sizeof(stocks)/sizeof(stocks[0]); i++)
		if (type == stocks[i].type)
			return i18n(stocks[i].text);

	kDebug() << "Invalid <stock> element '" << stock << "'. Check your theme!" << endl;
	return stock;
}

QString KdmLabel::timedUser = QString();
int KdmLabel::timedDelay = -1;

QString
KdmLabel::lookupText( const QString &t )
{
	QString text = t;

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
	m['d'] = QString::number( timedDelay );
	m['s'] = timedUser;
	// xgettext:no-c-format
	KGlobal::locale()->setDateFormat( i18nc("date format", "%a %d %B") );
	m['c'] = KGlobal::locale()->formatDateTime( QDateTime::currentDateTime(), false, false );

	return KMacroExpander::expandMacros( text, m );
}

#include "kdmlabel.moc"
