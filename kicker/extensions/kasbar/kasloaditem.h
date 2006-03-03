// -*- c++ -*-


#ifndef KASLOADITEM_H
#define KASLOADITEM_H

#include "kasitem.h"

#include <kdemacros.h>
//Added by qt3to4:
#include <QMouseEvent>
#include <Q3ValueList>

/**
 * An item that displays the system load.
 */
class KDE_EXPORT KasLoadItem : public KasItem
{
    Q_OBJECT

public:
    KasLoadItem( KasBar *parent );
    virtual ~KasLoadItem();

    void paint( QPainter *p );

public Q_SLOTS:
    void updateDisplay();
    void showMenuAt( QMouseEvent *ev );
    void showMenuAt( QPoint p );

private:
    Q3ValueList<double> valuesOne;
    Q3ValueList<double> valuesFive;
    Q3ValueList<double> valuesFifteen;
};

#endif // KASLOADITEM_H

