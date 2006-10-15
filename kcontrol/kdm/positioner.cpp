/*
    Copyright (C) 2006 Oswald Buddenhagen <ossi@kde.org>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "positioner.h"

#include <klocale.h>
#include <kstandarddirs.h>

#include <QFrame>
#include <QLabel>
#include <QMouseEvent>
#include <QPainter>

#define TOTAL_WIDTH 200
#define TOTAL_HEIGHT 186
#define ACTIVE_X 23
#define ACTIVE_Y 14
#define ACTIVE_WIDTH 151
#define ACTIVE_HEIGHT 115
#define MARGIN 10

#define SNAP 10
#define STEP 5

static void
fit( int &p )
{
	if (p < SNAP)
		p = 0;
	else if (p > 100 - SNAP)
		p = 100;
	else if (p > 50 - SNAP / 2 && p < 50 + SNAP / 2)
		p = 50;
}

static void
step( int &p, int d )
{
	if (p < SNAP)
		p = 0 + (d + 1) * SNAP / 2;
	else if (p > 100 - SNAP)
		p = 100 + (d - 1) * SNAP / 2;
	else if (p > 50 - SNAP / 2 && p < 50 + SNAP / 2)
		p = 50 + d * SNAP / 2;
	else {
		p += d * STEP;
		fit( p );
	}
}

Positioner::Positioner( QWidget *parent )
	: QWidget( parent )
	, m_readOnly( false )
	, m_x( 50 )
	, m_y( 50 )
{
	m_monitor = QImage( KStandardDirs::locate( "data", "kcontrol/pics/monitor.png" ) );
	m_anchor = QPixmap( KStandardDirs::locate( "data", "kcontrol/pics/anchor.png" ) );
	setFocusPolicy( Qt::StrongFocus );
	const int fw = MARGIN * 2;
	setMinimumSize( TOTAL_WIDTH + fw, TOTAL_HEIGHT + fw );
	setMaximumWidth( 400 );
	QSizePolicy sp( QSizePolicy::Expanding, QSizePolicy::Expanding );
	sp.setHeightForWidth( true );
	setSizePolicy( sp );
	m_frame = new QFrame( this );
	m_screen = new QWidget( m_frame );
	m_screen->setAutoFillBackground( true );
	QPalette pal;
	pal.setColor( QPalette::Background, QColor( 255, 255, 255, 128 ) );
	m_screen->setPalette( pal );
	m_dlg = new QFrame( m_screen );
	m_dlg->setFrameStyle( QFrame::Panel | QFrame::Raised );
	m_dlg->setAutoFillBackground( true );
	QPalette pal2;
	pal2.setBrush( QPalette::Background, pal2.brush( QPalette::Normal, QPalette::Background ) );
	m_dlg->setPalette( pal2 );
	m_ptr = new QLabel( m_screen );
	m_ptr->setPixmap( m_anchor );
	QString wts( i18n("Drag the anchor to move the center of the dialog to the desired position. "
	                  "Keyboard control is possible as well: Use the arrow keys or Home to center. "
	                  "Note that the actual proportions of the dialog are probably different.") );
	m_frame->setWhatsThis( wts );
	m_screen->setWhatsThis( wts );
	m_ptr->setWhatsThis( wts );
}

void
Positioner::setPosition( int x, int y )
{
	m_x = x;
	m_y = y;
	updateHandle();
}

int
Positioner::heightForWidth( int w ) const
{
	const int fw = MARGIN * 2;
	return ((w - fw) * TOTAL_HEIGHT + (TOTAL_WIDTH / 2)) / TOTAL_WIDTH + fw;
}

void
Positioner::updateHandle()
{
	int px = m_screen->size().width() * m_x / 100;
	int py = m_screen->size().height() * m_y / 100;
	m_ptr->setGeometry( px - m_anchor.width() / 2, py - m_anchor.height() / 2,
	                    m_anchor.width(), m_anchor.height() );
	int sw = m_screen->width() * 2 / 5;
	int sh = m_screen->height() * 2 / 5;
	QRect grt( px - sw / 2, py - sh / 2, sw, sh );
	int di;
	if ((di = m_screen->size().width() - grt.right()) < 0)
		grt.translate( di, 0 );
	if ((di = - grt.left()) > 0)
		grt.translate( di, 0 );
	if ((di = m_screen->size().height() - grt.bottom()) < 0)
		grt.translate( 0, di );
	if ((di = - grt.top()) > 0)
		grt.translate( 0, di );
	m_dlg->setGeometry( grt );
}

void
Positioner::resizeEvent( QResizeEvent * )
{
	const int fw = MARGIN * 2;
	QSize rs( TOTAL_WIDTH, TOTAL_HEIGHT );
	rs.scale( size() - QSize( fw, fw ), Qt::KeepAspectRatio );
	m_scaledMonitor = QPixmap::fromImage(
		m_monitor.scaled( rs, Qt::KeepAspectRatio, Qt::SmoothTransformation ) );
	m_frame->setGeometry( 0, 0, rs.width() + fw, rs.height() + fw );
	m_screen->setGeometry(
		MARGIN + ACTIVE_X * m_scaledMonitor.width() / TOTAL_WIDTH,
		MARGIN + ACTIVE_Y * m_scaledMonitor.height() / TOTAL_HEIGHT,
		ACTIVE_WIDTH * m_scaledMonitor.width() / TOTAL_WIDTH,
		ACTIVE_HEIGHT * m_scaledMonitor.height() / TOTAL_HEIGHT );
	updateHandle();
}

void
Positioner::focusInEvent( QFocusEvent * )
{
	m_frame->setFrameStyle( QFrame::Panel | QFrame::Sunken );
}

void
Positioner::focusOutEvent( QFocusEvent * )
{
	m_frame->setFrameStyle( QFrame::NoFrame );
}

void
Positioner::paintEvent( QPaintEvent * )
{
	QPainter p( this );
	p.drawPixmap( MARGIN, MARGIN, m_scaledMonitor );
}

void
Positioner::mousePressEvent( QMouseEvent *event )
{
	QPoint cp = event->pos() - m_screen->pos();
	if (!m_readOnly && m_ptr->geometry().contains( cp ))
		m_delta = m_ptr->geometry().center() - cp;
	else
		m_delta.setX( -1 );
}

void
Positioner::mouseMoveEvent( QMouseEvent *event )
{
	if (m_delta.x() != -1) {
		QPoint cp = event->pos() - m_screen->pos() + m_delta;
		m_x = cp.x() * 100 / m_screen->size().width();
		m_y = cp.y() * 100 / m_screen->size().height();
		fit( m_x );
		fit( m_y );
		updateHandle();
		emit positionChanged();
	}
}

void
Positioner::keyPressEvent( QKeyEvent *event )
{
	switch (event->key()) {
	case Qt::Key_Home:
		m_x = m_y = 50;
		break;
	case Qt::Key_Left:
		step( m_x, -1 );
		break;
	case Qt::Key_Right:
		step( m_x, 1 );
		break;
	case Qt::Key_Up:
		step( m_y, -1 );
		break;
	case Qt::Key_Down:
		step( m_y, 1 );
		break;
	default:
		event->ignore();
		return;
	}
	updateHandle();
	emit positionChanged();
	event->accept();
}

#include "positioner.moc"
