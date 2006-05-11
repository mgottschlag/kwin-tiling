/* kasbarextension.cpp
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
#include <QLayout>
//Added by qt3to4:
#include <QShowEvent>

#include <dcopclient.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kconfig.h>

#include <kmessagebox.h>

#include "kastasker.h"
#include "kasprefsdlg.h"
#include "kasaboutdlg.h"

#include "version.h"

#include "kasbarextension.h"
#include "kasbarextension.moc"

extern "C"
{
   KDE_EXPORT KPanelExtension *init( QWidget *parent, const QString& configFile )
   {
      KGlobal::locale()->insertCatalog("kasbarextension");
      return new KasBarExtension( configFile,
				  KPanelExtension::Normal,
				  KPanelExtension::About | KPanelExtension::Preferences,
				  parent, "kasbarextension");
   }
}

KasBarExtension::KasBarExtension( const QString& configFile,
                                  Type type,
                                  int actions,
                                  QWidget *parent, const char *name )
    : KPanelExtension( configFile, type, actions, parent, name ),
      detached_( false )
{
    kDebug(1345) << "KasBarExtension: Created '" << name << "', '" << configFile << "'" << endl;
//    KApplication::kApplication()->dcopClient()->registerAs( "kasbar" );

//    setBackgroundMode( NoBackground );
    kasbar = new KasTasker( orientation(), this, name );

    connect( kasbar, SIGNAL( layoutChanged() ), this, SIGNAL( updateLayout() ) );
    connect( kasbar, SIGNAL( detachedChanged(bool) ), this, SLOT( setDetached(bool) ) );

    kasbar->setConfig( config() );
    kasbar->readConfig();
    kasbar->refreshAll();
}

KasBarExtension::~KasBarExtension()
{
    if ( detached_ && (!kasbar.isNull()) )
	kasbar->deleteLater();
    KGlobal::locale()->removeCatalog("kasbarextension");
}

void KasBarExtension::setDetached( bool detach )
{
    if ( detach == detached_ )
	return;

    detached_ = detach;

    if ( detach ) {

	Qt::WFlags wflags = Qt::WStyle_Customize | Qt::WX11BypassWM | Qt::WStyle_DialogBorder | Qt::WStyle_StaysOnTop;
	kasbar->reparent( 0, wflags, kasbar->detachedPosition(), true );
	updateGeometry();
	resize( detachedSize() );
    }
    else {
	kasbar->reparent( this, QPoint(0,0), true );
	kasbar->setOrientation( orientation() );

	updateGeometry();
	resize( kasbar->size() );
    }

    emit updateLayout();
}

void KasBarExtension::showEvent( QShowEvent */*se*/ )
{
    updateGeometry();
    resize( kasbar->size() );
    repaint( true );
}

QSize KasBarExtension::detachedSize()
{
    if ( orientation() == Qt::Vertical )
	return QSize( kasbar->itemExtent()/2, 0 );
    else
	return QSize( 0, kasbar->itemExtent()/2 );

}

QSize KasBarExtension::sizeHint(Position p, QSize maxSize ) const
{
   Qt::Orientation o = Qt::Horizontal;

   if ( p == Left || p == Right )
      o = Qt::Vertical;

   if ( detached_ ) {
       if ( o == Qt::Vertical )
	   return QSize( kasbar->itemExtent()/2, 0 );
       else
	   return QSize( 0, kasbar->itemExtent()/2 );
   }

   return kasbar->sizeHint( o, maxSize );
}

void KasBarExtension::positionChange( Position /* position */)
{
   kasbar->setOrientation( orientation() );
   kasbar->updateLayout();
   kasbar->refreshIconGeometry();
}

void KasBarExtension::about()
{
    kasbar->showAbout();
}

void KasBarExtension::preferences()
{
    kasbar->showPreferences();
}

