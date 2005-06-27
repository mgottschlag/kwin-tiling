#ifndef KSSMONITOR_H
#define KSSMONITOR_H

#include "kswidget.h"

class KSSMonitor : public KSWidget
{
public:
    KSSMonitor( QWidget *parent ) : KSWidget( parent ) {}

    // we don't want no steenking palette change
    virtual void setPalette( const QPalette & ) {}
};

#endif
