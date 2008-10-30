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
      m_proxy_for_menu( parent ),
      m_spillPointer( parent->history()->youngest() ),
      m_menu_height( menu_height ),
      m_menu_width( menu_width ),
      m_nextItemNumber( 0 )
{
    connect( parent->history(), SIGNAL( changed() ), SLOT( slotHistoryChanged() ) );
    connect(m_proxy_for_menu, SIGNAL(triggered(QAction*)), parent->history(), SLOT(slotMoveToTop(QAction*)));
}

void PopupProxy::slotHistoryChanged() {
    deleteMoreMenus();

}

void PopupProxy::deleteMoreMenus() {
    const KMenu* myParent = parent();
    if ( myParent != m_proxy_for_menu ) {
        KMenu* delme = m_proxy_for_menu;
        m_proxy_for_menu = static_cast<KMenu*>( m_proxy_for_menu->parent() );
        while ( m_proxy_for_menu != myParent ) {
            delme = m_proxy_for_menu;
            m_proxy_for_menu = static_cast<KMenu*>( m_proxy_for_menu->parent() );
        }
        // We are called probably from within the menus event-handler (triggered=>slotMoveToTop=>changed=>slotHistoryChanged=>deleteMoreMenus)
        // what can result in a crash if we just delete the menu here (#155196 and #165154) So, delay the delete.
        delme->deleteLater();
    }
}

int PopupProxy::buildParent( int index, const QRegExp& filter ) {
    deleteMoreMenus();
    // Start from top of  history (again)
    m_spillPointer = parent()->history()->youngest();
    m_nextItemNumber = 0;
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
    QAction *action = new QAction(m_proxy_for_menu);
    QPixmap image( item->image() );
    if ( image.isNull() ) {
        // Squeeze text strings so that do not take up the entire screen (or more)
        QString text = m_proxy_for_menu->fontMetrics().elidedText( item->text().simplified(), Qt::ElideMiddle, m_menu_width );
        text.replace( "&", "&&" );
        action->setText(text);
    } else {
        const QSize max_size( m_menu_width,m_menu_height/4 );
        if ( image.height() > max_size.height() || image.width() > max_size.width() ) {
            image = image.scaled( max_size, Qt::KeepAspectRatio, Qt::SmoothTransformation );
        }
        action->setIcon(QIcon(image));
    }

    action->setData(m_nextItemNumber);

    // if the m_proxy_for_menu is a submenu (aka a "More" menu) then it may the case, that there is no other action in that menu yet.
    QAction *before = index < m_proxy_for_menu->actions().count() ? m_proxy_for_menu->actions().at(index) : 0;
    // insert the new action to the m_proxy_for_menu
    m_proxy_for_menu->insertAction(before, action);

    // Determine height of a menu item.
    int itemheight = QFontMetrics(m_proxy_for_menu->fontMetrics()).height();

//TODO Use old-style QStyle and QStyleOption API
#if 0
    Q_ASSERT( id != -1 ); // Be sure that the item was inserted.
    QMenuItem* mi = m_proxy_for_menu->findItem( id );


    int itemheight = m_proxy_for_menu->style().sizeFromContents(QStyle::CT_PopupMenuItem,
                                                              m_proxy_for_menu,
                                                              QSize( 0, fontheight ),
                                                              QStyleOption(mi,10,0) ).height();
#endif
    // Test if there was enough space
    remainingHeight -= itemheight;
}

int PopupProxy::insertFromSpill( int index ) {

    // This menu is going to be filled, so we don't need the aboutToShow()
    // signal anymore
    disconnect( m_proxy_for_menu, 0, this, 0 );

    // Insert history items into the current m_proxy_for_menu,
    // discarding any that doesn't match the current filter.
    // stop when the total number of items equal m_itemsPerMenu;
    int count = 0;
    int remainingHeight = m_menu_height - m_proxy_for_menu->sizeHint().height();
    // Force at least one item to be inserted.
    remainingHeight = qMax( remainingHeight, 0 );

    while (m_spillPointer.hasNext() && remainingHeight >= 0) {
        const HistoryItem *item = m_spillPointer.next();
        if ( m_filter.indexIn( item->text() ) == -1) {
            m_nextItemNumber++; // also count hidden items
            continue;
        }
        tryInsertItem( item, remainingHeight, index++ );
        count++;
        m_nextItemNumber++;
    }

    // If there is more items in the history, insert a new "More..." menu and
    // make *this a proxy for that menu ('s content).
    if (m_spillPointer.hasNext()) {
        KMenu* moreMenu = new KMenu(i18n("&More"), m_proxy_for_menu);
        connect(moreMenu, SIGNAL(aboutToShow()), SLOT(slotAboutToShow()));
        QAction *before = index < m_proxy_for_menu->actions().count() ? m_proxy_for_menu->actions().at(index) : 0;
        m_proxy_for_menu->insertMenu(before, moreMenu);
        m_proxy_for_menu = moreMenu;
    }

    // Return the number of items inserted.
    return count;

}
#include "popupproxy.moc"
