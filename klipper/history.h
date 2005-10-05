// -*- Mode: C++; c-basic-offset: 4; indent-tabs-mode: nil; tab-width: 8; -*-
/* This file is part of the KDE project
   Copyright (C) 2004  Esben Mose Hansen <kde@mosehansen.dk>
   Copyright (C) Andrew Stanley-Jones

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
#ifndef _HISTORY_H_
#define _HISTORY_H_

#include <qobject.h>
#include <q3ptrlist.h>
//Added by qt3to4:
#include <Q3PopupMenu>
#include "historyitem.h"

class KlipperPopup;
class Q3PopupMenu;
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
    typedef Q3PtrListIterator<HistoryItem> iterator;

    /**
     * Return (toplevel) popup menu (or default view, of you like)
     */
    KlipperPopup* popup();

    /**
     * Inserts item into clipboard history top
     * if duplicate entry exist, the older duplicate is deleted.
     * The duplicate concept is "deep", so that two text string
     * are considerd duplicate if identical.
     */
    void insert( const HistoryItem* item );

    /**
     * Inserts item into clipboard without any checks
     * Used when restoring a saved history and internally.
     * Don't use this unless you're reasonable certain
     * that no duplicates are introduced
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

    /**
     * returns true if the user has selected the top item
     */
    bool topIsUserSelected() {
        return m_topIsUserSelected;
    }

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

    /**
     * Emitted when the first history item has changed.
     */
    void topChanged();

private:

    /**
     * The history
     */
    Q3PtrList<HistoryItem> itemList;

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

    /**
     * True if the top is selected by the user
     */
    bool m_topIsUserSelected;

};

inline const HistoryItem* History::first() { return itemList.first(); }

inline const HistoryItem* History::next() { return itemList.next();  }

#endif
