/* -------------------------------------------------------------

popupproxy.h (part of Klipper - Cut & paste history for KDE)

(C) Esben Mose Hansen <kde@mosehansen.dk>

Generated with the KDE Application Generator

Licensed under the GNU GPL Version 2

------------------------------------------------------------- */


#ifndef _POPUPPROXY_H_
#define _POPUPPROXY_H_

#include <qptrlist.h>
#include <qobject.h>

#include <kpopupmenu.h>
#include <history.h>

class HistoryItem;
class KlipperPopup;

/**
 * Proxy helper for the "more" menu item
 *
 */
class PopupProxy : public QObject
{
    Q_OBJECT

public:
    /**
     * Inserts up to itemsPerMenu into parent from parent->youngest(),
     * and spills any remaining items into a more menu.
     */
    PopupProxy( KlipperPopup* parent, const char* name, int itemsPerMenu );

    KlipperPopup* parent();

    /**
     * Called when rebuilding the menu...
     */
    void buildParent();

public slots:
    void slotAboutToShow();
    void slotHistoryChanged();
private:
    /**
     * Insert up to m_itemsPerMenu items from spill and a new
     * more-menu if neccessary.
     */
    void insertFromSpill();

private:
    KPopupMenu* proxy_for_menu;
    History::iterator spillPointer;
    int m_itemsPerMenu;
    int nextItemNumber;

};

#endif
