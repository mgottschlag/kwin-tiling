/* kasitem.cpp
**
** Copyright (C) 2001-2004 Richard Moore <rich@kde.org>
** Contributor: Mosfet
**     All rights reserved.
**
** KasBar is dual-licensed: you can choose the GPL or the BSD license.
** Short forms of both licenses are included below.
*/

/*
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program in a file called COPYING; if not, write to
** the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA 02110-1301, USA.
*/

/*
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions
** are met:
** 1. Redistributions of source code must retain the above copyright
**    notice, this list of conditions and the following disclaimer.
** 2. Redistributions in binary form must reproduce the above copyright
**    notice, this list of conditions and the following disclaimer in the
**    documentation and/or other materials provided with the distribution.
**
** THIS SOFTWARE IS PROVIDED BY THE AUTHORS AND CONTRIBUTORS ``AS IS'' AND
** ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
** IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
** ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
** FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
** DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
** OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
** HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
** LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
** OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
** SUCH DAMAGE.
*/

/*
** Bug reports and questions can be sent to kde-devel@kde.org
*/
#include <QCursor>
#include <QPainter>
#include <qdrawutil.h>
#include <QRegExp>
#include <QTimer>
#include <QApplication>
//Added by qt3to4:
#include <QMouseEvent>
#include <QPixmap>

#include <kdebug.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <kiconloader.h>
#include <klocale.h>

#include "kasitem.h"

#include "kaspopup.h"
#include "kasitem.moc"

/* XPM */
static const char *tiny_arrow[]={
"5 9 2 1",
". c None",
"# c #ffffff",
"....#",
"...##",
"..###",
".####",
"#####",
".####",
"..###",
"...##",
"....#"};

static const int KASITEM_CHECK_POPUP_DELAY = 500;

KasItem::KasItem( KasBar *parent )
   : QObject( parent ),
     kas( parent ), popupTimer( 0 ), dragTimer( 0 ),
     title( i18n( "Kasbar" ) ),
     mouseOver( false ), activated( false ),
     customPopup( false ), lockPopup(false), groupItem( false ),
     frame(true), modified(false), attention_(false), prog( -1 ),
     anim(), aniFrame( 0 ), drawAnim( false )
{
    connect( parent, SIGNAL( dragStarted() ), SLOT( hidePopup() ) );
    connect( this, SIGNAL( middleButtonClicked(QMouseEvent *) ), parent, SLOT( toggleOrientation() ) );
}

KasItem::~KasItem()
{
    delete (KasPopup *) pop;
}

void KasItem::setActive( bool yesno )
{
    if ( activated == yesno )
	return;

    activated = yesno;
    update();
}

void KasItem::setText( const QString &text )
{
    if ( title == text )
	return;

    title = text;
    update();
}

void KasItem::setIcon( const QPixmap &p )
{
    pix = p;
    update();
}

void KasItem::setProgress( int percent )
{
    if ( percent == prog )
	return;

    prog = percent;
    update();
}

void KasItem::setShowFrame( bool yes )
{
    if ( frame == yes )
	return;

    frame = yes;
    update();
}

void KasItem::setModified( bool yes )
{
    if ( modified == yes )
	return;

    modified = yes;
    update();
}

void KasItem::setAttention( bool yes )
{
    if ( attention_ == yes )
	return;

    attention_ = yes;
    update();
}

void KasItem::mouseEnter()
{
   static const int POPUP_DELAY = 300;

   if ( (!customPopup) && (popupTimer == 0) ) {
      popupTimer = new QTimer( this, "popupTimer" );
      connect( popupTimer, SIGNAL( timeout() ), SLOT( showPopup() ) );
      popupTimer->start( POPUP_DELAY, true );
   }

   mouseOver = true;
   update();
}

void KasItem::mouseReleaseEvent( QMouseEvent *ev )
{
    if ( ev->button() == Qt::LeftButton )
	emit leftButtonClicked( ev );
    else if ( ev->button() == Qt::RightButton )
	emit rightButtonClicked( ev );
    else if ( ev->button() == Qt::MidButton )
	emit middleButtonClicked( ev );
}

// Check periodically if the popup can be hidden (hack)
void KasItem::checkPopup()
{
    if ( pop.isNull() )
	return;
    if ( !pop->isVisible() )
	return;

    QWidget *w = QApplication::topLevelAt( QCursor::pos() );
    if ( !w ) {
	if ( popupTimer ) {
	    delete popupTimer;
	    popupTimer = 0;
	}
	if ( (!customPopup) && (!lockPopup) )
	    hidePopup();
    }
    else {
	QTimer::singleShot( KASITEM_CHECK_POPUP_DELAY, this, SLOT( checkPopup() ) );
    }
}

void KasItem::dragEnter()
{
   static const int DRAG_SWITCH_DELAY = 1000;

   if ( dragTimer == 0 ) {
      dragTimer = new QTimer( this, "dragTimer" );
      connect( dragTimer, SIGNAL( timeout() ), SLOT( dragOverAction() ) );
      dragTimer->start( DRAG_SWITCH_DELAY, true );
   }

   mouseOver = true;
   update();
}

void KasItem::mouseLeave()
{
   if ( popupTimer ) {
      delete popupTimer;
      popupTimer = 0;
   }

   mouseOver = false;
   update();
}

void KasItem::dragLeave()
{
   if ( dragTimer ) {
      delete dragTimer;
      dragTimer = 0;
   }

   mouseOver = false;
   update();
}

bool KasItem::isShowingPopup() const
{
    if ( pop.isNull() )
	return false;
    return pop->isVisible();
}

KasPopup *KasItem::createPopup()
{
    return 0;
}

void KasItem::showPopup()
{
   if ( pop.isNull() )
       pop = createPopup();

   if ( pop.isNull() )
       return;

   pop->show();
   update();

   QTimer::singleShot( KASITEM_CHECK_POPUP_DELAY, this, SLOT( checkPopup() ) );
}

void KasItem::hidePopup()
{
    if ( pop.isNull() )
	return;

    pop->hide();
    activated = false;
    update();
}

void KasItem::togglePopup()
{
   if ( activated )
      hidePopup();
   else
      showPopup();
}

void KasItem::setPopup( KasPopup *popup )
{
    if ( pop )
	pop->deleteLater();
    pop = popup;
}

void KasItem::paintFrame( QPainter *p )
{
   if ( !frame )
       return;

   qDrawShadePanel(p, 0, 0, extent(), extent(), colorGroup(), false, 2);

   QPen pen;
   
   if ( mouseOver ) {
       if ( attention_ ) {
	   pen = QPen( resources()->attentionColor(), 2 );
	   p->setPen( pen );
	   p->drawRect( 0, 0, extent(), extent());
       }
       else {
	   pen = QPen( Qt::white );
	   p->setPen( pen );
	   p->drawRect(0, 0, extent(), extent());
       }
   }
   else if ( kas->paintInactiveFrames() ) {
       p->setPen( attention_ ? resources()->attentionColor() : Qt::black );
       p->drawRect(0, 0, extent(), extent());
   }
}

void KasItem::paintLabel( QPainter *p )
{
    QString text = title;

    if ( !groupItem ) {
	p->fillRect( 2, 2, extent()-4, 13, QBrush( resources()->labelBgColor() ) );

	if ( isProgressItem() ) {
	    QRegExp reg( "(1?[0-9]?[0-9])%" );
	    if ( -1 != reg.search( text ) ) {
		prog = reg.cap(1).toInt();
		paintProgress( p, prog );
	    }
	    else {
		prog = 0;
	    }
	}

	p->setFont( KGlobalSettings::taskbarFont() );
	p->setPen( resources()->labelPenColor() );

	if ( fontMetrics().width( text ) > extent()-4 )
	    p->drawText( 2, 2, extent()-4, 12, Qt::AlignLeft | Qt::AlignVCenter, text );
	else
	    p->drawText( 2, 2, extent()-4, 12, Qt::AlignCenter, text );

	return;
    }
    else {
	QPixmap arrow( tiny_arrow );

	QPoint popupPos = KasPopup::calcPosition( this, 10, 10 );
	QPoint iPos = kas->mapToGlobal( kas->itemPos( this ) );
	QMatrix turn;

	if ( popupPos.x() < iPos.x() ) {
	    paintArrowLabel( p, arrow.width(), true );
	    p->drawPixmap( 3, 4, arrow );
	}
	else if ( popupPos.x() == iPos.x() ) {
	    if ( popupPos.y() < iPos.y() ) {
		turn.rotate( 90.0 );
		arrow = arrow.transformed( turn );
		paintArrowLabel( p, arrow.width(), true );
		p->drawPixmap( 3, 6, arrow );
	    }
	    else {
		turn.rotate( 270.0 );
		arrow = arrow.transformed( turn );
		paintArrowLabel( p, arrow.width(), false );
		p->drawPixmap( extent()-12, 6, arrow );
	    }
	}
	else {
	    turn.rotate( 180.0 );
	    arrow = arrow.transformed( turn );
	    paintArrowLabel( p, arrow.width(), false );
	    p->drawPixmap( extent()-8, 4, arrow );
	}
    }
}

void KasItem::paintArrowLabel( QPainter *p, int arrowSize, bool arrowOnLeft )
{
    QString text = title;
    int lx = 2;
    int ly = 2;
    int w = extent()-4;
    int h = 13;
    arrowSize+=2; // Add a space

    p->fillRect( lx, ly, w, h, QBrush( resources()->labelBgColor() ) );

    // Adjust for arrow
    if ( arrowOnLeft ) {
	lx += arrowSize;
	w -= arrowSize;
    }
    else {
	w -= arrowSize;
    }

    p->setFont( KGlobalSettings::taskbarFont() );
    p->setPen( resources()->labelPenColor() );
    if ( fontMetrics().width( text ) > w )
	p->drawText( lx, ly, w, h-1, Qt::AlignLeft | Qt::AlignVCenter, text );
    else
	p->drawText( lx, ly, w, h-1, Qt::AlignCenter, text );
}

void KasItem::paintModified( QPainter *p )
{
    if ( modified )
	p->drawPixmap(extent()-12, extent()-22, resources()->modifiedIcon() );
}

void KasItem::paintBackground( QPainter *p )
{
    if ( activated )
	p->drawPixmap( 0, 0, resources()->activeBg() );
    else if ( kas->isTransparent() )
	;
    else
	p->drawPixmap( 0, 0, resources()->inactiveBg() );
}

void KasItem::paintProgress( QPainter *p, int percent )
{
    double amt = (extent()-4) * (percent / 100.0L);
    p->fillRect( 2, 13, (int) amt, 2, QBrush( resources()->progressColor() ) );
}

void KasItem::paintStateIcon( QPainter *p, uint state )
{
    if ( kas->itemSize() != KasBar::Small ) {
	switch(state) {
	    case StateIcon:
		p->drawPixmap(extent()-11, extent()-11, resources()->minIcon() );
		break;
	    case StateShaded:
		p->drawPixmap(extent()-11, extent()-11, resources()->shadeIcon() );
		break;
	    case StateNormal:
		p->drawPixmap(extent()-11, extent()-11, resources()->maxIcon() );
		break;
	    default:
		break;
	}
    }
    else {
	switch(state) {
	    case StateIcon:
		p->drawPixmap(extent()-9, extent()-9, resources()->microMinIcon() );
		break;
	    case StateShaded:
		p->drawPixmap(extent()-9, extent()-9, resources()->microShadeIcon() );
		break;
	    case StateNormal:
		p->drawPixmap(extent()-9, extent()-9, resources()->microMaxIcon() );
		break;
	    default:
		break;
	}
    }
}

void KasItem::paintAttention( QPainter *p )
{
    p->setPen( resources()->attentionColor() );
    p->drawPixmap( 3, extent()-11, resources()->attentionIcon() );
}

void KasItem::setAnimation( const PixmapList &frames )
{
    anim = frames;
    aniFrame = 0;
}

void KasItem::advanceAnimation()
{
    aniFrame++;

    if ( aniFrame >= anim.count() )
        aniFrame = 0;

    update();
}

void KasItem::setShowAnimation( bool yes )
{
    if ( yes == drawAnim )
	return;

    drawAnim = yes;
    update();
}

void KasItem::paintAnimation( QPainter *p )
{
    if ( (aniFrame+1) > anim.count() )
	return;

    QPixmap pix = anim[ aniFrame ];
    if ( pix.isNull() )
	return;

    if ( kas->itemSize() == KasBar::Small )
	p->drawPixmap( 4, 16, pix );
    else
	p->drawPixmap( extent()-18, 16, pix );
}

void KasItem::paintIcon( QPainter *p )
{
    if ( pix.isNull() )
	return;

    int x = (extent() - 4 - pix.width()) / 2;
    int y = (extent() - 15 - pix.height()) / 2;
    p->drawPixmap( x-4, y+15, pix );
}

void KasItem::paint( QPainter *p )
{
   paintBackground( p );
   paintFrame( p );
   paintLabel( p );
   paintIcon( p );

   if ( drawAnim )
       paintAnimation( p );

   if ( attention_ )
       paintAttention( p );
}

void KasItem::paint( QPainter *p, int x, int y )
{
    p->save();
    p->translate( x, y );
    paint( p );
    p->restore();
}

void KasItem::repaint()
{
   repaint( true );
}

void KasItem::repaint( bool erase )
{
   if ( kas->isVisible() )
       kas->repaintItem( this, erase );
}

void KasItem::update()
{
   if ( kas->isVisible() )
       kas->updateItem( this );
}

