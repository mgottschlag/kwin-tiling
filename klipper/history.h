/* -------------------------------------------------------------

history.h (part of Klipper - Cut & paste history for KDE)

(C) Esben Mose Hansen <kde@mosehansen.dk>
(C) by Andrew Stanley-Jones

Generated with the KDE Application Generator

Licensed under the GNU GPL Version 2

------------------------------------------------------------- */


#ifndef _HISTORY_H_
#define _HISTORY_H_

#include <qobject.h>
#include <qptrlist.h>
#include "historyitem.h"

class KlipperPopup;
class QPopupMenu;
class QWidget;
class QptrListIterator;

class History : public QObject
{
    Q_OBJECT
public:
    History( QWidget* parent, const char* name );
    ~History();
    /**
     * Iterator for history
     */
    typedef QPtrListIterator<HistoryItem> iterator;

    /**
     * Return (toplevel) popup menu (or default view, of you like)
     */
    KlipperPopup* popup();

    /**
     * Inserts item into clipboard history
     * if duplicate entry exist, the older dublicate is deleted.
     * @param pos the position of the new item, 0 is top
     */
    void insert( const HistoryItem* item, unsigned pos );

    /**
     * Inserts item into clipboard history top
     * if duplicate entry exist, the older duplicate is deleted.
     */
    void insert( const HistoryItem* item );

    /**
     * Inserts item into clipboard without any checks
     * Used when restoring a saved history.
     */
    void forceInsert( const HistoryItem* item );

    /**
     * Remove (first) history item equal to item from history
     */
    void remove( const HistoryItem* item  );

    /**
     * Traversal: Get first item
     */
    const HistoryItem* first();

    /**
     * Traversal: Get current item
     */
    const HistoryItem* next();

    /**
     * Get an iterator pointing to the first (most recent) item
     * This iterator should probably be a constant iterator, but
     * the QTL doesn't support this easily.
     *
     * Anyway, if you modify the items via. the iterator, call changed()
     * when you're done. Calling changed() multiple times doesn't hurt.
     *
     * iterator could be made into a proxy class that did the right thing.
     */
    iterator youngest();

    /**
     * True if no history items
     */
    bool empty() const { return itemList.isEmpty(); }

    /**
     * Set maximum history size
     */
    void max_size( unsigned max_size );

    /**
     * Get the maximum history size
     */
    unsigned max_size() const { return m_max_size; }

public slots:
    /**
     * move the history in position pos to top
     */
    void slotMoveToTop(int pos );

    /**
     * Clear history
     */
    void slotClear();

signals:
    void changed();

private:

    /**
     * The history
     */
    QPtrList<HistoryItem> itemList;

    /**
     * ensure that the number of items does not exceed max_size()
     * Deletes items from the end as neccessary.
     */
    void trim();

private:
    /**
     * "Default view" --- a popupmenu containing the clipboard history.
     */
    KlipperPopup* m_popup;


    /**
     * The number of clipboard items stored.
     */
    unsigned m_max_size;

};

inline const HistoryItem* History::first() { return itemList.first(); }

inline const HistoryItem* History::next() { return itemList.next();  }

#endif
