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
#include <qstring.h>
#include <qregexp.h>

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
     * Called when rebuilding the menu
     * Deletes any More menus.. and start (re)inserting into the toplevel menu.
     * @param index Items are inserted at index.
     * @param filter If non-empty, only insert items that match filter as a regex
     * @return number of items inserted.
     */
    int buildParent( int index, const QRegExp& filter = QRegExp() );

public slots:
    void slotAboutToShow();
    void slotHistoryChanged();
private:
    /**
     * Insert up to m_itemsPerMenu items from spill and a new
     * more-menu if neccessary.
     * @param index Items are inserted at index
     * @return number of items inserted.
     */
    int insertFromSpill( int index = 0 );

    /**
     * Delete all "More..." menus current created.
     */
    void deleteMoreMenus();

private:
    KPopupMenu* proxy_for_menu;
    History::iterator spillPointer;
    QRegExp m_filter;
    int m_itemsPerMenu;
    int nextItemNumber;

};

#endif
