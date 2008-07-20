// -*- Mode: C++; c-basic-offset: 4; indent-tabs-mode: nil; tab-width: 8; -*-
/* This file is part of the KDE project
   Copyright (C) 2004  Esben Mose Hansen <kde@mosehansen.dk>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#include <QRegExp>
#include <QStyle>
#include <QPixmap>
#include <QImage>
#include <QStyleOption>

#include <kstringhandler.h>
#include <klocale.h>
#include <kdebug.h>

#include "historyitem.h"
#include "popupproxy.h"
#include "history.h"
#include "klipperpopup.h"


PopupProxy::PopupProxy( KlipperPopup* parent, int menu_height, int menu_width )
    : QObject( parent ),
      proxy_for_menu( parent ),
      spillPointer( parent->history()->youngest() ),
      m_menu_height( menu_height ),
      m_menu_width( menu_width ),
      nextItemNumber( 0 )
{
    connect( parent->history(), SIGNAL( changed() ), SLOT( slotHistoryChanged() ) );
    connect(proxy_for_menu, SIGNAL(triggered(QAction*)), parent->history(), SLOT(slotMoveToTop(QAction*)));
}

void PopupProxy::slotHistoryChanged() {
    deleteMoreMenus();

}

void PopupProxy::deleteMoreMenus() {
    const KMenu* myParent = parent();
    if ( myParent != proxy_for_menu ) {
        KMenu* delme = proxy_for_menu;
        proxy_for_menu = static_cast<KMenu*>( proxy_for_menu->parent() );
        while ( proxy_for_menu != myParent ) {
            delme = proxy_for_menu;
            proxy_for_menu = static_cast<KMenu*>( proxy_for_menu->parent() );
        }
        // We are called probably from within the menus event-handler (triggered=>slotMoveToTop=>changed=>slotHistoryChanged=>deleteMoreMenus)
        // what can result in a crash if we just delete the menu here (#155196 and #165154) So, delay the delete.
        delme->deleteLater();
    }
}

int PopupProxy::buildParent( int index, const QRegExp& filter ) {
    deleteMoreMenus();
    // Start from top of  history (again)
    spillPointer = parent()->history()->youngest();
    nextItemNumber = 0;
    if ( filter.isValid() ) {
        m_filter = filter;
    }

    return insertFromSpill( index );

}

KlipperPopup* PopupProxy::parent() {
    return static_cast<KlipperPopup*>( QObject::parent() );
}

void PopupProxy::slotAboutToShow() {
    insertFromSpill();
}

void PopupProxy::tryInsertItem( HistoryItem const * const item,
                                int& remainingHeight,
                                const int index )
{
    QAction *action = new QAction(proxy_for_menu);
    QPixmap image( item->image() );
    if ( image.isNull() ) {
        // Squeeze text strings so that do not take up the entire screen (or more)
        QString text = proxy_for_menu->fontMetrics().elidedText( item->text().simplified(), Qt::ElideMiddle, m_menu_width );
        text.replace( "&", "&&" );
        action->setText(text);
    } else {
        const QSize max_size( m_menu_width,m_menu_height/4 );
        if ( image.height() > max_size.height() || image.width() > max_size.width() ) {
            image = image.scaled( max_size, Qt::KeepAspectRatio, Qt::SmoothTransformation );
        }
        action->setIcon(QIcon(image));
    }

    action->setData(nextItemNumber);

    // if the proxy_for_menu is a submenu (aka a "More" menu) then it may the case, that there is no other action in that menu yet.
    QAction *before = index < proxy_for_menu->actions().count() ? proxy_for_menu->actions().at(index) : 0;
    // insert the new action to the proxy_for_menu
    proxy_for_menu->insertAction(before, action);

    // Determine height of a menu item.
    int itemheight = QFontMetrics(proxy_for_menu->fontMetrics()).height();

//TODO Use old-style QStyle and QStyleOption API
#if 0
    Q_ASSERT( id != -1 ); // Be sure that the item was inserted.
    QMenuItem* mi = proxy_for_menu->findItem( id );


    int itemheight = proxy_for_menu->style().sizeFromContents(QStyle::CT_PopupMenuItem,
                                                              proxy_for_menu,
                                                              QSize( 0, fontheight ),
                                                              QStyleOption(mi,10,0) ).height();
#endif
    // Test if there was enough space
    remainingHeight -= itemheight;
}

int PopupProxy::insertFromSpill( int index ) {

    // This menu is going to be filled, so we don't need the aboutToShow()
    // signal anymore
    disconnect( proxy_for_menu, 0, this, 0 );

    // Insert history items into the current proxy_for_menu,
    // discarding any that doesn't match the current filter.
    // stop when the total number of items equal m_itemsPerMenu;
    int count = 0;
    int remainingHeight = m_menu_height - proxy_for_menu->sizeHint().height();
    // Force at least one item to be inserted.
    remainingHeight = qMax( remainingHeight, 0 );

    while (spillPointer.hasNext() && remainingHeight >= 0) {
        const HistoryItem *item = spillPointer.next();
        if ( m_filter.indexIn( item->text() ) == -1) {
            nextItemNumber++; // also count hidden items
            continue;
        }
        tryInsertItem( item, remainingHeight, index++ );
        count++;
        nextItemNumber++;
    }

    // If there is more items in the history, insert a new "More..." menu and
    // make *this a proxy for that menu ('s content).
    if (spillPointer.hasNext()) {
        KMenu* moreMenu = new KMenu(i18n("&More"), proxy_for_menu);
        connect(moreMenu, SIGNAL(aboutToShow()), SLOT(slotAboutToShow()));
        QAction *before = index < proxy_for_menu->actions().count() ? proxy_for_menu->actions().at(index) : 0;
        proxy_for_menu->insertMenu(before, moreMenu);
        proxy_for_menu = moreMenu;
    }

    // Return the number of items inserted.
    return count;

}
#include "popupproxy.moc"
