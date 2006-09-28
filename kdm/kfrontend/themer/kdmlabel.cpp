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
#include <kdebug.h>

#include <QDateTime>
#include <QPainter>
#include <QFontMetrics>
#include <QTimer>
#include <QMap>
#include <QHash>
#include <QAction>

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
	label.active.color.setRgb( 0xFFFFFF );
	label.active.present = false;
	label.prelight.present = false;

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

		if (tagName == "normal") {
			parseColor( el.attribute( "color", "#ffffff" ), QString(), label.normal.color );
			parseFont( el.attribute( "font", "Sans 14" ), label.normal.font );
		} else if (tagName == "active") {
			label.active.present = true;
			parseColor( el.attribute( "color", "#ffffff" ), QString(), label.active.color );
			parseFont( el.attribute( "font", "Sans 14" ), label.active.font );
		} else if (tagName == "prelight") {
			label.prelight.present = true;
			parseColor( el.attribute( "color", "#ffffff" ), QString(), label.prelight.color );
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
	label.isTimer = label.text.indexOf( "%c" ) >= 0;
	if (label.isTimer) {
		timer = new QTimer( this );
		timer->start( 1000 );
		connect( timer, SIGNAL(timeout()), SLOT(update()) );
	}
	setCText( lookupText( label.text ) );
}

void
KdmLabel::setText( const QString &txt )
{
	label.text = txt;
	update();
}

void
KdmLabel::setCText( const QString &txt )
{
	cText = txt;
	delete action;
	action = 0;
	cAccelOff = txt.find( '_' );
	if (cAccelOff >= 0) {
		action = new QAction( this );
		action->setShortcut( Qt::ALT + txt[cAccelOff + 1].unicode() );
		connect( action, SIGNAL(triggered( bool )), SLOT(activate()) );
		themer()->widget()->addAction( action );
	}
}

void
KdmLabel::activate()
{
	KdmItem *cp = this;
	do {
		if (cp->isButton) {
			emit activated( cp->id );
			return;
		}
		cp = qobject_cast<KdmItem *>(cp->parent());
	} while (cp);
	if (!buddy.isEmpty())
		activateBuddy();
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
	return QFontMetrics( l->font ).size( Qt::AlignLeft | Qt::TextSingleLine, cText );
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
	if (cAccelOff != -1) {
		QRect tarea( area );
		QFontMetrics fm( l->font );
		QString left = cText.left( cAccelOff );
		p->drawText( area, Qt::AlignLeft | Qt::SingleLine, left );
		tarea.rLeft() += fm.width( left );
		QFont f( l->font );
		f.setUnderline( true );
		p->setFont( f );
		QString acc( cText[cAccelOff + 1] );
		p->drawText( tarea, Qt::AlignLeft | Qt::SingleLine, acc );
		tarea.rLeft() += fm.width( acc );
		p->setFont( l->font );
		p->drawText( tarea, Qt::AlignLeft | Qt::SingleLine, cText.mid( cAccelOff + 2 ) );
	} else
		p->drawText( area, Qt::AlignLeft | Qt::TextSingleLine, cText );
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
