/* -------------------------------------------------------------

popupproxy.cpp (part of Klipper - Cut & paste history for KDE)

(C) Esben Mose Hansen <kde@mosehansen.dk>

Generated with the KDE Application Generator

Licensed under the GNU GPL Version 2

------------------------------------------------------------- */
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
    // Delete more menus
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

void PopupProxy::buildParent() {
    // Start from top of  history (again)
    spillPointer = parent()->history()->youngest();
    nextItemNumber = 0;

    insertFromSpill();
}

KlipperPopup* PopupProxy::parent() {
    return static_cast<KlipperPopup*>( QObject::parent() );
}

void PopupProxy::slotAboutToShow() {
    insertFromSpill();
}

void PopupProxy::insertFromSpill() {
    const HistoryItem* item = spillPointer.current();
    disconnect( proxy_for_menu, 0, this, 0 );
    int lastNumber = nextItemNumber + m_itemsPerMenu;
    for ( History* history = parent()->history();
          nextItemNumber < lastNumber && item;
          nextItemNumber++, item = ++spillPointer )
    {
        int id = proxy_for_menu->insertItem( KStringHandler::cEmSqueeze(item->text().simplifyWhiteSpace(), proxy_for_menu->fontMetrics(), 25).replace( "&", "&&" ));
        proxy_for_menu->connectItem(  id,
                                      history,
                                      SLOT( slotMoveToTop( int ) ) );
        proxy_for_menu->setItemParameter(  id,  nextItemNumber );
    }
    if ( spillPointer.current() ) {
        KPopupMenu* moreMenu = new KPopupMenu( proxy_for_menu, "a more menu" );
        proxy_for_menu->insertItem( i18n( "&More" ),  moreMenu );
        connect( moreMenu, SIGNAL( aboutToShow() ), SLOT( slotAboutToShow() ) );
        proxy_for_menu = moreMenu;
    }

}
