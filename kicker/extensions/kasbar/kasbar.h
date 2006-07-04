// -*- c++ -*-

/* kasbar.h
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
// -*- c++ -*-


#ifndef __KASBAR_H
#define __KASBAR_H

#include <QWidget>
#include <QPoint>
#include <q3ptrlist.h>
#include <QLayout>
#include <qnamespace.h>
//Added by qt3to4:
#include <QPixmap>
#include <QPaintEvent>
#include <QDragMoveEvent>
#include <QBoxLayout>
#include <QResizeEvent>
#include <QMouseEvent>

#include "kasresources.h"

class KasItem;
class KasResources;

typedef Q3PtrList<KasItem> KasItemList;


/**
 * The main view for KasBar.
 */
class KDE_EXPORT KasBar : public QWidget
{
   Q_OBJECT
   Q_PROPERTY( int maxBoxes READ maxBoxes )
   Q_PROPERTY( uint boxesPerLine READ boxesPerLine )
   Q_PROPERTY( Direction direction READ direction )
   Q_PROPERTY( Qt::Orientation orientation READ orientation )
   Q_PROPERTY( bool masked READ isMasked )
   Q_ENUMS( Direction )

   friend class KasItem;
public:
   KasBar( Qt::Orientation o, QWidget *parent=0, const char *name=0, Qt::WFlags f=0 );
   KasBar( Qt::Orientation o, KasBar *master, 
	   QWidget* parent=0, const char* name=0, Qt::WFlags f=0 );

   virtual ~KasBar();

   typedef QBoxLayout::Direction Direction;

   /** Returns true iff this is a top-level bar. This is unrelated to it being a top-level widget. */
   bool isTopLevel() const { return !master_; }

   /** Returns the bar from which this bar inherits its settings (or 0 if this is the top-level bar). */
   KasBar *master() const { return master_; }

   /** Creates a child bar of the kasbar. The child will inherit the appearance options. */
   virtual KasBar *createChildBar( Qt::Orientation o, QWidget *parent, const char *name=0 );

   /** Factory method that returns the singleton resources object. */
   virtual KasResources *resources();

   /** Returns true iff we have a resources object. */
   bool hasResources() const { return (res ? true : false); }

   //
   // Item management
   //
   void append( KasItem *i );
   void insert( int index, KasItem *i );
   void remove( KasItem *i );
   void clear();
   KasItem *take( KasItem *i ) { return items.take( indexOf(i) ); }
   KasItem *itemAt( uint i ) { return items.at( i ); }
   int indexOf( KasItem *i ) { return items.find( i ); }

   KasItemList *itemList() { return &items; }

   //
   // Layout options.
   //

   /** The possible item sizes. */
   enum ItemSize { Enormous, Huge, Large, Medium, Small, Custom };

   int itemSize() const { return itemSize_; }
   int itemExtent() const { return itemExtent_; }

   /** The number of items in the bar. */
   unsigned int itemCount() const { return items.count(); }

   int maxBoxes() const { return maxBoxes_; }
   uint boxesPerLine() const { return boxesPerLine_; }

   void setOrientation( Qt::Orientation o );
   Qt::Orientation orientation() const { return orient; }

   void setDirection( Direction dir );
   Direction direction() const { return direction_; }

   bool isDetached() const { return detached; }
   QPoint detachedPosition() const { return detachedPos; }

   bool isDrag() const { return inDrag; }

   QSize sizeHint( Qt::Orientation,  QSize max );

   //
   // Look and feel options
   //

   bool isMasked() const { return useMask_; }

   /** Is transparency enabled? */
   bool isTransparent() const { return transparent_; }

   /** Is tinting enabled? */
   bool hasTint() const { return enableTint_; }

   /** Sets the amount and color of the tint. */
   void setTint( double amount, QColor color );

   /** Sets the amount of tinting. */
   void setTintAmount( double amount ) { setTint( amount, tintColour_ ); }

   /** Get the amount of tinting. */
   double tintAmount() const { return tintAmount_; }

   /** Get the color of the tint. */
   QColor tintColor() const { return tintColour_; }

   /** Returns true iff we will paint frames around inactive items. */
   bool paintInactiveFrames() const { return paintInactiveFrame_; }

   //
   // Utilities
   //

   void updateItem( KasItem *i );

   /** Redraws the specified item. */
   void repaintItem(KasItem *i, bool erase = true );

   /** Returns the item at p or 0. */
   KasItem* itemAt(const QPoint &p);

   /** Get the position of the specified item. */
    QPoint itemPos( KasItem *i );

   /** The item under the mouse pointer (or 0). */
   KasItem *itemUnderMouse() const { return itemUnderMouse_; }

public Q_SLOTS:
   //
   // Layout slots
   //
   void setMaxBoxes( int count );
   void setBoxesPerLine( int count );

   void setItemSize( int size );
   void setItemExtent( int size );
   void setDetachedPosition( const QPoint &pos );

   virtual void updateLayout();

   void updateMouseOver();
   void updateMouseOver( QPoint pos );

   /** Enable or disable tinting. */
   void setTint( bool enable );

   /** Enable or disable transparency. */
   void setTransparent( bool enable );

   /** Set the color of the tint. */
   void setTintColor( const QColor &c );

   /** Set the strength of the tint (as a percentage). */
   void setTintAmount( int percent );

   void setBackground( const QPixmap &pix );

   void setMasked( bool mask );

   void setPaintInactiveFrames( bool enable );

   void toggleOrientation();
   void toggleDetached();
   void setDetached( bool detach );

    /** Rereads the configuration of the master Kasbar. */
    virtual void rereadMaster();

    virtual void addTestItems();

Q_SIGNALS:

   void detachedChanged( bool );
   void detachedPositionChanged( const QPoint & );
   void dragStarted();

   void directionChanged();

   /** Emitted when kasbar wants to resize. This happens when a new window is added. */
   void layoutChanged();

   /** Emitted when the item size is changed. */
   void itemSizeChanged( int );

   void configChanged();

protected:
   /** Displays the popup menus, hides/shows windows. */
   void mousePressEvent(QMouseEvent *ev);

   /** Displays the popup menus, hides/shows windows. */
   void mouseReleaseEvent(QMouseEvent *ev);

   /** Overridden to implement the mouse-over highlight effect. */
   void mouseMoveEvent(QMouseEvent *ev);

   /** Overridden to implement the drag-over task switching. */
   void dragMoveEvent(QDragMoveEvent *ev);

   /** Paints the background of the item to the painter. */
   void paintBackground( QPainter *p, const QRect &r );

   /** Calls the paint methods for the items in the rectangle specified by the event. */
   void paintEvent(QPaintEvent *ev);

   /** Forces the widget to re-layout it's contents. */
   void resizeEvent(QResizeEvent *ev);

private:
   // Core data
   QPixmap offscreen;
   KasBar *master_;
   KasItemList items;
   Qt::Orientation orient;
   Direction direction_;
   KasItem *itemUnderMouse_;
   uint boxesPerLine_;
   QPoint pressPos;
   bool inDrag;
   bool detached;
   int maxBoxes_;
   int itemSize_;
   int itemExtent_;
   QPoint detachedPos;
   bool paintInactiveFrame_;

   // Implements pseudo-transparency
   bool transparent_;
   KPixmap bg;
   bool enableTint_;
   double tintAmount_;
   QColor tintColour_;
   bool useMask_;

   // Look and feel resources
   KasResources *res;
};



#endif
