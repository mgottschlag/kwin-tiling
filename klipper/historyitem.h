#ifndef _HISTORYITEM_H_
#define _HISTORYITEM_H_
/* -------------------------------------------------------------

historyitem.h (part of Klipper - Cut & paste history for KDE)

(C) Esben Mose Hansen <kde@mosehansen.dk>

Generated with the KDE Application Generator

Licensed under the GNU GPL Version 2

------------------------------------------------------------- */



class QString;

/**
 * An entry in the clipboard history.
 */
class HistoryItem
{
public:
    HistoryItem( );
    virtual ~HistoryItem();
    virtual const QString& text() const = 0;
    virtual bool operator==(const HistoryItem& rhs) const = 0;
};


#endif
