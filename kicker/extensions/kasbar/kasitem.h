//Added by qt3to4:
#include <QMouseEvent>
#include <QPixmap>
/* kasitem.h
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

#ifndef KASITEM_H
#define KASITEM_H

class QPainter;
class QMouseEvent;
class KasPopup;

#include <qobject.h>
#include <qpointer.h>
#include <qpoint.h>


#include <kdemacros.h>

#include "kasbar.h"
#include "kaspopup.h"

/**
 * Abstract base class for items that can be in a KasBar.
 *
 * @author Richard Moore, rich@kde.org
 */
class KDE_EXPORT KasItem : public QObject
{
    Q_OBJECT

public:
    friend class KasBar;

    typedef Q3ValueVector<QPixmap> PixmapList;

    /** The states that a window can be in. */
    enum WindowState {
	StateIcon, StateShaded, StateNormal
    };

    KasItem( KasBar *parent );
    virtual ~KasItem();

    /** Returns the parent KasBar of this item. */
    KasBar *kasbar() const { return kas; }

    /** Returns the size of the item in pixels. */
    int extent() const { return kas->itemExtent(); }

    /** Returns the text that will be displayed in the title. */
    QString text() const { return title; }

    /** Returns the position of this item. */
    QPoint pos() const { return pos_; }
    void setPos( const QPoint &p ) { pos_ = p; }
    void setPos( int x, int y ) { pos_ = QPoint( x, y ); }

    /** Returns the progress so far. This will -1 if the item is not displaying progress info. */
    int progress() const { return prog; }

    /** Returns true iff this item is displaying progress info. */
    bool isProgressItem() const { return prog != -1; }

    /** Returns true iff this item will display the modified indicator. */
    bool isModified() const { return modified; }

    /**
     * Returns true if this is a group item. Group items display an arrow
     * showing where the popup containing their children will appear.
     */
    void setGroupItem( bool enable = true  ) { groupItem = enable; }

    //
    // Popup
    //

    /** Returns true iff this item is showing a popup. */
    bool isShowingPopup() const;

    /** Returns the active popup or 0. */
    KasPopup *popup() const { return pop; }

    /** Sets the popup to be used by this item. */
    void setPopup( KasPopup *popup );

    /**
     * Returns true iff this item uses a custom popup policy.  If this flag is
     * set, the default popup behaviour is disabled. This means you must call
     * show/hide/toggle yourself if you want the popup to be shown.
     */
    bool hasCustomPopup() const { return customPopup; }

    /** Enables or disables custom popup handling. */
    void setCustomPopup( bool enable = true ) { customPopup = enable; }

    //
    // Drawing Methods
    //

    /** Translates the QPainter then calls paintItem(). */
    void paint( QPainter *p, int x, int y );

    /**
     * Subclasses should reimplement this method to paint themselves. The painter is setup so
     * that the item is always at 0, 0.
     */
    virtual void paint( QPainter *p );

    /** Draw a standard frame for the item. */
    void paintFrame( QPainter *p );

    /** Paint the background. */
    void paintBackground( QPainter *p );

    /** Draw the label for the item. */
    void paintLabel( QPainter *p );

    void paintIcon( QPainter *p );

    void paintModified( QPainter *p );

public Q_SLOTS:
    void repaint();
    void repaint( bool erase );
    void update();

    void setActive( bool yes );
    void setText( const QString &title );
    void setIcon( const QPixmap &icon );
    void setProgress( int percent );
    void setShowFrame( bool yes );
    void setModified( bool yes );
    void setAttention( bool yes );
    void setAnimation( const PixmapList &frames );
    void setShowAnimation( bool yes );

    void advanceAnimation();

    void setLockPopup( bool yes ) { lockPopup = yes; }

    /** Shows the items popup. */
    void showPopup();

    /** Hides the items popup. */
    void hidePopup();

    /** Check if the popup should be visible. */
    void checkPopup();
    
    /** Hides or shows the popup. */
    void togglePopup();

    /**
     * Called when something being dragged is held over the item for a while.
     */
    virtual void dragOverAction() {}

Q_SIGNALS:
    void leftButtonClicked( QMouseEvent *ev );
    void middleButtonClicked( QMouseEvent *ev );
    void rightButtonClicked( QMouseEvent *ev );

protected:
    KasResources *resources() { return kas->resources(); }

    /** Gets the font metrics from the parent. */
    QFontMetrics fontMetrics() const { return kas->fontMetrics(); }

    /** Gets the color group from the parent. */
    const QColorGroup &colorGroup() const { return kas->colorGroup(); }

    /** Factory method that creates a popup widget for the item. */
    virtual KasPopup *createPopup();

    /** Draw a label with an arrow, the parameters specify the position and size of the arrow. */
    void paintArrowLabel( QPainter *p, int arrowSize, bool arrowOnLeft );

    /** Paints a progress graph. */
    void paintProgress( QPainter *p, int percent );

    void paintStateIcon( QPainter *p, uint state );

    void paintAttention( QPainter *p );

    void paintAnimation( QPainter *p );

    //
    // Event Handlers
    //

    /** Called when the item receives a mouse event. */
    virtual void mousePressEvent( QMouseEvent * ) {}

    /** Called when the item receives a mouse event. */
    virtual void mouseReleaseEvent( QMouseEvent * );

    /** Called when the mouse enters the item. */
    virtual void mouseEnter();

    /** Called when the mouse leaves the item. */
    virtual void mouseLeave();

    /** Called when a drag enters the item. */
    virtual void dragEnter();

    /** Called when a drag leaves the item. */
    virtual void dragLeave();

private:
    KasBar *kas;
    QPointer<KasPopup> pop;
    QTimer *popupTimer;
    QTimer *dragTimer;

    QPoint pos_;
    QString title;
    QPixmap pix;
    bool mouseOver;
    bool activated;
    bool customPopup;
    bool lockPopup;
    bool groupItem;
    bool frame;
    bool modified;
    bool attention_;
    int prog;

    PixmapList anim;
    uint aniFrame;
    bool drawAnim;
};

#endif // KASITEM_H

