#ifndef _HISTORYITEM_H_
#define _HISTORYITEM_H_
/* -------------------------------------------------------------

historyitem.h (part of Klipper - Cut & paste history for KDE)

(C) Esben Mose Hansen <kde@mosehansen.dk>

Generated with the KDE Application Generator

Licensed under the GNU GPL Version 2

------------------------------------------------------------- */

#include <qpixmap.h>

class QString;
class QMimeSource;
class QDataStream;

/**
 * An entry in the clipboard history.
 */
class HistoryItem
{
public:
    HistoryItem( );
    virtual ~HistoryItem();

    /**
     * Return the current item as text
     * An image would be returned as a descriptive
     * text, such as 32x43 image.
     */
    virtual const QString& text() const = 0;

    /**
     * Return the current item as text
     * A text would be returned as a null pixmap,
     * which is also the default implementation
     */
    inline virtual const QPixmap& image() const;

    /**
     * Write object on datastream
     */
    virtual void write( QDataStream& stream ) const = 0;

    /**
     * Equality.
     */
    virtual bool operator==(const HistoryItem& rhs) const = 0;

    /**
     * Create an HistoryItem from MimeSources (i.e., clipboard data)
     * returns null if create fails (e.g, unsupported mimetype)
     */
    static HistoryItem* create( const QMimeSource& aSource );

    /**
     * Create an HistoryItem from MimeSources (i.e., clipboard data)
     * returns null if creation fails. In this case, the datastream
     * is left in an undefined state.
     */
    static HistoryItem* create( QDataStream& aSource );
};

inline
const QPixmap& HistoryItem::image() const {
    static QPixmap nullPixmap;
    return nullPixmap;
}

inline
QDataStream& operator<<( QDataStream& lhs, HistoryItem const * const rhs ) {
    if ( rhs ) {
        rhs->write( lhs );
    }
    return lhs;

}

#endif
