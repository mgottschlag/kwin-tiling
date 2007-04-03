/* kasstartupitem.cpp
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
#include <QPainter>
#include <QBitmap>
#include <qdrawutil.h>
#include <QTimer>
//Added by qt3to4:
#include <QPixmap>

#include <kdebug.h>
#include <kglobal.h>
#include <kwm.h>
#include <kiconloader.h>
#include <QPixmap>
#include <kpixmapeffect.h>
#include <klocale.h>
#include <taskmanager.h>

#include "kaspopup.h"

#include "kasstartupitem.h"
#include "kasstartupitem.moc"

KasStartupItem::KasStartupItem( KasBar *parent, Startup::StartupPtr startup )
    : KasItem( parent ),
      startup_(startup), frame(0)
{
    setText( startup_->text() );
    setIcon( icon() );
    setShowFrame( false );
    setAnimation( resources()->startupAnimation() );

    aniTimer = new QTimer( this );
    connect( aniTimer, SIGNAL( timeout() ), SLOT( aniTimerFired() ) );
    aniTimer->start( 100 );
}

KasStartupItem::~KasStartupItem()
{
}

QPixmap KasStartupItem::icon() const
{
   /**
    * This icon stuff should all be handled by the task manager api, but isn't yet.
    */
   QPixmap pixmap;

   switch( kasbar()->itemSize() ) {
   case KasBar::Small:
     /* ***** NOP ******
	pixmap = KGlobal::iconLoader()->loadIcon( startup_->icon(),
						  K3Icon::NoGroup,
						  K3Icon::SizeSmall );
     */
      break;
   case KasBar::Medium:
	pixmap = KGlobal::iconLoader()->loadIcon( startup_->icon(),
						  K3Icon::NoGroup,
						  K3Icon::SizeMedium );
      break;
   case KasBar::Large:
	pixmap = KGlobal::iconLoader()->loadIcon( startup_->icon(),
						  K3Icon::NoGroup,
						  K3Icon::SizeLarge );
      break;
   case KasBar::Huge:
	pixmap = KGlobal::iconLoader()->loadIcon( startup_->icon(),
						  K3Icon::NoGroup,
						  K3Icon::SizeHuge );
      break;
   case KasBar::Enormous:
	pixmap = KGlobal::iconLoader()->loadIcon( startup_->icon(),
						  K3Icon::NoGroup,
						  K3Icon::SizeEnormous );
      break;
   default:
	pixmap = KGlobal::iconLoader()->loadIcon( "error",
						  K3Icon::NoGroup,
						  K3Icon::SizeSmall );
   }

   return pixmap;
}

void KasStartupItem::aniTimerFired()
{

    if ( frame == 40 )
	frame = 0;
    else
	frame++;

    advanceAnimation();
}

void KasStartupItem::paint( QPainter *p )
{
    p->save();

    p->setClipRect( 0, 0, extent(), extent() );
    p->translate( extent()/2, extent()/2 );
    p->rotate( 9.0L * frame );
    p->scale( 0.7L, 0.7L );
    p->translate( -extent()/2, -extent()/2 );

    KasItem::paint( p );

    p->restore();
    paintAnimation( p );
}

