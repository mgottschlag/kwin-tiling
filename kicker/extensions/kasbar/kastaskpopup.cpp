/* kastaskpopup.cpp
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
#include <qapplication.h>
//Added by qt3to4:
#include <QPixmap>
#include <QPaintEvent>

#include <kdebug.h>
#include <kglobalsettings.h>
#include <kpixmapeffect.h>

#include <taskmanager.h>

#include "kastaskitem.h"
#include "kastasker.h"

#include "kastaskpopup.h"
#include "kastaskpopup.moc"

static const int TITLE_HEIGHT = 13;

KasTaskPopup::KasTaskPopup( KasTaskItem *item, const char *name )
    : KasPopup( item, name )
{
    this->item = item;

    setFont(KGlobalSettings::generalFont());
    setMouseTracking( true );

    QString text = item->task()->visibleIconicName();
    if ( item->kasbar()->thumbnailsEnabled() && item->task()->hasThumbnail() ) {
        titleBg.resize( width(), TITLE_HEIGHT );

        setFixedSize( item->task()->thumbnail().width() + 2, 
		      TITLE_HEIGHT + item->task()->thumbnail().height() + 2 );
    }
    else {
        int w = fontMetrics().width( text ) + 4;
        int h = TITLE_HEIGHT + 1;
        titleBg.resize( w, h );
        setFixedSize( w, h );
    }

    KPixmapEffect::gradient( titleBg, 
                             Qt::black, colorGroup().mid(),
                             KPixmapEffect::DiagonalGradient );

    connect( item->task(), SIGNAL( thumbnailChanged() ), SLOT( refresh() ) );
}

KasTaskPopup::~KasTaskPopup()
{
}

void KasTaskPopup::refresh()
{
    QString text = item->task()->visibleIconicName();
    if (  item->kasbar()->thumbnailsEnabled() && item->task()->hasThumbnail() ) {
        resize( item->task()->thumbnail().width() + 2,
		TITLE_HEIGHT + item->task()->thumbnail().height() + 2 );
        titleBg.resize( width(), TITLE_HEIGHT );
    }
    update();
}

void KasTaskPopup::paintEvent( QPaintEvent * )
{
    QPainter p( this );
    p.drawPixmap( 0, 0, titleBg );

    QString text = item->task()->visibleIconicName();

    p.setPen( Qt::white );
    if ( fontMetrics().width( text ) > width() - 4 )
        p.drawText( 1, 1, width() - 4, TITLE_HEIGHT - 1, Qt::AlignLeft | Qt::AlignVCenter,
                     text );
    else
        p.drawText( 1, 1, width() - 4, TITLE_HEIGHT - 1, Qt::AlignCenter, text );

    QPixmap thumb = item->task()->thumbnail();
    if ( !thumb.isNull() )
        p.drawPixmap( 1, TITLE_HEIGHT, thumb );

    //
    // Draw border
    //
    p.setPen( Qt::black );
    p.drawRect( 0, 0, width(), height() );
}

