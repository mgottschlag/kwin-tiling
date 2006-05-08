/* kasgroupitem.cpp
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
#include <qpainter.h>
#include <qbitmap.h>
#include <qtimer.h>
#include <qmatrix.h>
//Added by qt3to4:
#include <QMouseEvent>
#include <QPixmap>

#include <kdebug.h>
#include <kglobal.h>
#include <kwin.h>
#include <kiconloader.h>
#include <QPixmap>
#include <kpixmapeffect.h>
#include <kmenu.h>
#include <klocale.h>

#include <taskmanager.h>
#include <taskrmbmenu.h>

#include "kastasker.h"
#include "kaspopup.h"

#include "kasgroupitem.h"
#include "kasgroupitem.moc"


KasGroupItem::KasGroupItem( KasTasker *parent )
    : KasItem( parent ), items(), groupType_( GroupRelated )
     
{
    setCustomPopup( true );
    setGroupItem( true );
    setText( i18n("Group") );

    connect( parent, SIGNAL( layoutChanged() ), this, SLOT( hidePopup() ) );
    connect( parent, SIGNAL( layoutChanged() ), this, SLOT( update() ) );
    connect( this, SIGNAL(leftButtonClicked(QMouseEvent *)), SLOT(togglePopup()) );
    connect( this, SIGNAL(rightButtonClicked(QMouseEvent *)), SLOT(showGroupMenuAt(QMouseEvent *) ) );
}

KasGroupItem::~KasGroupItem()
{
}

KasTasker *KasGroupItem::kasbar() const
{
    return static_cast<KasTasker *> (KasItem::kasbar());
}

void KasGroupItem::addTask( Task::TaskPtr t )
{
    if (!t)
	return;

    items.append( t );
    if ( items.count() == 1 ) {
	setText( t->visibleName() );
	updateIcon();
    }

    connect( t, SIGNAL( changed() ), this, SLOT( update() ) );
    update();
}

void KasGroupItem::removeTask( Task::TaskPtr t )
{
    if ( !t )
	return;

    hidePopup();

    for (Task::List::iterator it = items.begin(); it != items.end();)
    {
        if ((*it) == t)
        {
            it = items.erase(it);
        }
        else
        {
            ++it;
        }
    }

    updateIcon();

    if ( items.count() == 1 )
	kasbar()->moveToMain( this, items.first() );
}

void KasGroupItem::updateIcon()
{
    QPixmap p;
    bool usedIconLoader = false;
    Task::TaskPtr t = items.first();
    if (!t)
	p = KGlobal::iconLoader()->loadIcon( "kicker",
					     K3Icon::NoGroup,
					     K3Icon::SizeSmall );

    int sizes[] = { K3Icon::SizeEnormous,
		    K3Icon::SizeHuge,
		    K3Icon::SizeLarge,
		    K3Icon::SizeMedium,
		    K3Icon::SizeSmall };

    p = t->bestIcon( sizes[kasbar()->itemSize()], usedIconLoader );

    if ( p.isNull() )
	p = KGlobal::iconLoader()->loadIcon( "error", K3Icon::NoGroup, K3Icon::SizeSmall );

    setIcon( p );
}

void KasGroupItem::paint( QPainter *p )
{
    KasItem::paint( p );

    //
    // Item summary info
    //
    int modCount = 0;
    for ( Task::List::iterator it = items.begin(); it != items.end() ; ++it ) {
	if ( (*it)->isModified() )
	    modCount++;
    }

    KasResources *res = resources();

    p->setPen( isShowingPopup() ? res->activePenColor() : res->inactivePenColor() );

    if ( modCount ) {
	QString modCountStr;
	modCountStr.setNum( modCount );
	p->drawText( extent()-fontMetrics().width( modCountStr )-3,
		     15+fontMetrics().ascent(),
		     modCountStr );

	p->drawPixmap( extent()-12, 29, res->modifiedIcon() );
    }

    int microsPerCol;
    switch( kasbar()->itemSize() ) {
	default:
	case KasBar::Small:
	    microsPerCol = 2;
	    break;
	case KasBar::Medium:
	    microsPerCol = 4;
	    break;
	case KasBar::Large:
	    microsPerCol = 7;
	    break;
	case KasBar::Huge:
	    microsPerCol = 9;
	    break;
	case KasBar::Enormous:
	    microsPerCol = 16;
	    break;
    }

    int xpos = 3;
    int ypos = 16;

    for ( int i = 0; ( i < (int) items.count() ) && ( i < microsPerCol ); i++ ) {
	Task::TaskPtr t = items.at( i );

	if( t->isIconified() )
	    p->drawPixmap( xpos, ypos, res->microMinIcon() );
	else if ( t->isShaded() )
	    p->drawPixmap( xpos, ypos, res->microShadeIcon() );
	else
	    p->drawPixmap( xpos, ypos, res->microMaxIcon() );

	ypos += 7;
    }

    if ( ((int) items.count() > microsPerCol) && ( kasbar()->itemSize() != KasBar::Small ) ) {
	QString countStr;
	countStr.setNum( items.count() );
	p->drawText( extent()-fontMetrics().width( countStr )-3,
		     extent()+fontMetrics().ascent()-16,
		     countStr );
    }
}

void KasGroupItem::updatePopup()
{
    if ( bar ) {
	bar->rereadMaster();

	bar->clear();
	if ( items.count() ) {
	    for ( Task::List::iterator t = items.begin(); t != items.end(); ++t ) {
		bar->addTask( *t );
	    }
	}

	bar->updateLayout();
	if ( popup() )
	    popup()->resize( bar->size() );
    }
}

KasPopup *KasGroupItem::createPopup()
{
    KasPopup *pop = new KasPopup( this );
    bar = kasbar()->createChildBar( ( kasbar()->orientation() == Qt::Horizontal ) ? Qt::Vertical : Qt::Horizontal, pop );

    connect( pop, SIGNAL(shown()), SLOT(updatePopup()) );

    return pop;

//     // Test code
//     //
//     // This generates cool looking fractal-like patterns if you keep unfolding the
//     // groups!
//     int pos = (int) this; 
//     if ( pos % 2 )
//        bar->append( new KasItem( bar ) );
//     if ( pos % 5 )
//        bar->append( new KasItem( bar ) );
//     bar->append( new KasGroupItem( bar ) );
//     if ( pos % 3 )
//        bar->append( new KasItem( bar ) );
//     if ( pos % 7 )
//        bar->append( new KasItem( bar ) );
//     ////////////
}

void KasGroupItem::ungroup()
{
    kasbar()->moveToMain( this );
}

void KasGroupItem::showGroupMenuAt( QMouseEvent *ev )
{
    showGroupMenuAt( ev->globalPos() );
}

void KasGroupItem::showGroupMenuAt( const QPoint &p )
{
    TaskRMBMenu *tm = new TaskRMBMenu(items, true, kasbar());
    tm->insertItem( i18n("&Ungroup" ), this, SLOT( ungroup() ) );
    tm->insertSeparator();
    tm->insertItem( i18n("&Kasbar"), kasbar()->contextMenu() );

    setLockPopup( true );
    tm->exec( p );
    delete tm;
    setLockPopup( false );
}



