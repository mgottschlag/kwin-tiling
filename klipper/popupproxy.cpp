/* -------------------------------------------------------------

popupproxy.cpp (part of Klipper - Cut & paste history for KDE)

(C) Esben Mose Hansen <kde@mosehansen.dk>

Generated with the KDE Application Generator

Licensed under the GNU GPL Version 2

------------------------------------------------------------- */
#include <qregexp.h>

#include <kstringhandler.h>
#include <klocale.h>
#include <kdebug.h>

#include "historyitem.h"
#include "popupproxy.h"
#include "history.h"
#include "klipperpopup.h"


PopupProxy::PopupProxy( KlipperPopup* parent, const char* name, int itemsPerMenu )
    : QObject( parent, name ),
      proxy_for_menu( parent ),
      spillPointer( parent->history()->youngest() ),
      m_itemsPerMenu( itemsPerMenu ),
      nextItemNumber( 0 )
{
    connect( parent->history(), SIGNAL( changed() ), SLOT( slotHistoryChanged() ) );
}

void PopupProxy::slotHistoryChanged() {
    deleteMoreMenus();

}

void PopupProxy::deleteMoreMenus() {
    const KPopupMenu* myParent = parent();
    if ( myParent != proxy_for_menu ) {
        const KPopupMenu* delme = proxy_for_menu;;
        proxy_for_menu = static_cast<KPopupMenu*>( proxy_for_menu->parent() );
        while ( proxy_for_menu != myParent ) {
            delme = proxy_for_menu;
            proxy_for_menu = static_cast<KPopupMenu*>( proxy_for_menu->parent() );
        }
        delete delme;
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

int PopupProxy::insertFromSpill( int index ) {

    // This menu is going to be filled, so we don't need the aboutToShow()
    // signal anymore
    disconnect( proxy_for_menu, 0, this, 0 );

    // Insert history items into the current proxy_for_menu,
    // discarding any that doesn't match the current filter.
    // stop when the total number of items equal m_itemsPerMenu;
    int count = 0;
    const HistoryItem* item = spillPointer.current();
    int maxItems = m_itemsPerMenu - proxy_for_menu->count();
    for ( History* history = parent()->history();
          count < maxItems && item;
          nextItemNumber++, item = ++spillPointer )
    {
        if ( m_filter.search( item->text() ) == -1) {
            continue;
        }
        count++;
        int id = proxy_for_menu->insertItem( KStringHandler::cEmSqueeze(item->text().simplifyWhiteSpace(),
                                                                        proxy_for_menu->fontMetrics(),
                                                                        25).replace( "&", "&&" ),
                                             -1,
                                             index++);
        proxy_for_menu->connectItem(  id,
                                      history,
                                      SLOT( slotMoveToTop( int ) ) );
        proxy_for_menu->setItemParameter(  id,  nextItemNumber );
    }

    // If there is more items in the history, insert a new "More..." menu and
    // make *this a proxy for that menu ('s content).
    if ( spillPointer.current() ) {
        KPopupMenu* moreMenu = new KPopupMenu( proxy_for_menu, "a more menu" );
        proxy_for_menu->insertItem( i18n( "&More" ),  moreMenu, -1, index );
        connect( moreMenu, SIGNAL( aboutToShow() ), SLOT( slotAboutToShow() ) );
        proxy_for_menu = moreMenu;
    }

    // Return the number of items inserted.
    return count;

}
#include "popupproxy.moc"
