/* -------------------------------------------------------------

historyitem.h (part of Klipper - Cut & paste history for KDE)

(C) Esben Mose Hansen <kde@mosehansen.dk>

Generated with the KDE Application Generator

Licensed under the GNU GPL Version 2

------------------------------------------------------------- */


#ifndef _HISTORYSTRINGITEM_H_
#define _HISTORYSTRINGITEM_H_

#include <qstring.h>

#include "historyitem.h"

/**
 * A string entry in the clipboard history.
 */
class HistoryStringItem : public HistoryItem
{
public:
    HistoryStringItem( const QString& data );
    const QString& text() const;
    bool operator==( const HistoryItem& rhs) const {
        if ( const HistoryStringItem* casted_rhs = dynamic_cast<const HistoryStringItem*>( &rhs ) ) {
            casted_rhs->m_data == m_data;
        }
        return false;
    }
private:
    QString m_data;
};

inline const QString& HistoryStringItem::text() const { return m_data; }


#endif
