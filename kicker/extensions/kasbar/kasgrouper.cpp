/* kasgrouper.cpp
**
** Copyright (C) 2001-2004 Richard Moore <rich@kde.org>
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

#include <taskmanager.h>

#include "kastasker.h"
#include "kasgroupitem.h"
#include "kastaskitem.h"

#include "kasgrouper.h"

KasGrouper::KasGrouper( KasTasker *bar )
    : kasbar( bar )
{
}

KasGrouper::~KasGrouper()
{
}

KasItem *KasGrouper::maybeGroup( Task::TaskPtr t )
{
    KasItem *item = 0;

    if ( kasbar->groupInactiveDesktops() )
	item = maybeAddToDesktopGroup( t );
    if ( item )
	return item;

    if ( kasbar->groupWindows() )
	item = maybeAddToGroup( t );
    if ( item )
	return item;

    return item;
}

KasItem *KasGrouper::maybeAddToDesktopGroup( Task::TaskPtr t )
{
   if ( t->isOnCurrentDesktop() )
       return 0;

   KasItem *item = 0;
   for ( uint i = 0; i < kasbar->itemCount(); i++ ) {
      KasItem *ei = kasbar->itemAt( i );

      if ( ei->inherits( "KasTaskItem" ) ) {
	 KasTaskItem *eti = static_cast<KasTaskItem *> (ei);
	 if ( eti->task()->desktop() == t->desktop() ) {
	    KasGroupItem *egi = kasbar->convertToGroup( eti->task() );
	    egi->setGroupType( KasGroupItem::GroupDesktop );
	    egi->addTask( t );
	    item = egi;
	 }
      }
      else if ( ei->inherits( "KasGroupItem" ) ) {
	  KasGroupItem *egi = static_cast<KasGroupItem *> (ei);
	  if ( egi->groupType() == KasGroupItem::GroupDesktop ) {
	      if ( egi->task(0)->desktop() == t->desktop() ) {
		  egi->addTask( t );
		  item = egi;
	      }
	  }
      }
   }

   return item;
}

KasItem *KasGrouper::maybeAddToGroup( Task::TaskPtr t )
{
   KasItem *item = 0;

   QString taskClass = t->className().lower();

   for ( uint i = 0; (!item) && (i < kasbar->itemCount()); i++ ) {
      KasItem *ei = kasbar->itemAt( i );

      if ( ei->inherits( "KasTaskItem" ) ) {

	 KasTaskItem *eti = static_cast<KasTaskItem *> (ei);

	 // NB This calls Task::className() not QObject::className()
	 QString currClass = eti->task()->className().lower();

	 if ( Task::idMatch( currClass, taskClass ) ) {
	    KasGroupItem *egi = kasbar->convertToGroup( eti->task() );
	    egi->addTask( t );
	    item = egi;
	    break;
	 }
      }
      else if ( ei->inherits( "KasGroupItem" ) ) {
	  KasGroupItem *egi = static_cast<KasGroupItem *> (ei);

	  for ( int i = 0; i < egi->taskCount(); i++ ) {

	      // NB This calls Task::className() not QObject::className()
	      QString currClass = egi->task( i )->className().lower();
	     
	      if ( Task::idMatch( currClass, taskClass ) ) {
		  egi->addTask( t );
		  item = egi;
		  break;
	      }
	  }
      }
   }

   return item;
}
