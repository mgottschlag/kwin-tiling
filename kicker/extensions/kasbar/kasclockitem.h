// -*- c++ -*-


#ifndef KASCLOCKITEM_H
#define KASCLOCKITEM_H

#include "kasitem.h"
//Added by qt3to4:
#include <QMouseEvent>

/**
 * An item that displays a clock.
 */
class KDE_EXPORT KasClockItem : public KasItem
{
    Q_OBJECT

public:
    KasClockItem( KasBar *parent );
    virtual ~KasClockItem();

    void paint( QPainter *p );

public Q_SLOTS:
    void updateTime();

    void showMenuAt( QMouseEvent *ev );
    void showMenuAt( QPoint p );

protected:
    /** Reimplemented from KasItem to create a date picker. */
    virtual KasPopup *createPopup();

private:
    class LCD *lcd;
};

#endif // KASCLOCKITEM_H

