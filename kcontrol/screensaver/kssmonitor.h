#ifndef KSSMONITOR_H
#define KSSMONITOR_H

#include <qxembed.h>

class KSSMonitor : public QXEmbed
{
public:
    KSSMonitor( QWidget *parent ) : QXEmbed( parent ) {}

    // we don't want no steenking palette change
    virtual void setPalette( const QPalette & ) {}
};

#endif
