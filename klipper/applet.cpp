/* -------------------------------------------------------------

   toplevel.cpp (part of Klipper - Cut & paste history for KDE)

   (C) by Andrew Stanley-Jones
   (C) 2000 by Carsten Pfeiffer <pfeiffer@kde.org>

   Generated with the KDE Application Generator

 ------------------------------------------------------------- */

#include "applet.h"

#include <kglobal.h>
#include <klocale.h>

#include "toplevel.h"

extern "C"
{
    KPanelApplet* init(QWidget *parent, const QString& configFile)
    {
        KGlobal::locale()->insertCatalogue("klipper");
        return new KlipperApplet(configFile, KPanelApplet::Normal, 0, parent, "klipper");
    }
}

KlipperApplet::KlipperApplet(const QString& configFile, Type t, int actions,
                         QWidget *parent, const char *name)
    : KPanelApplet(configFile, t, actions, parent, name)
{
    setBackgroundMode(QWidget::X11ParentRelative);
    toplevel = new TopLevel( true );
    // no longer toplevel >;)
    toplevel->reparent( this, QPoint( 0, 0 ));
    toplevel->move(( width() - toplevel->width()) / 2, ( height() - toplevel->height()) / 2 );
    toplevel->show();
}

// the klipper icon has harcoded size 20x20
int KlipperApplet::widthForHeight(int) const
{
    return 20;
}

int KlipperApplet::heightForWidth(int) const
{
    return 20;
}

void KlipperApplet::resizeEvent( QResizeEvent* ev )
{
    KPanelApplet::resizeEvent( ev );
    toplevel->move(( width() - toplevel->width()) / 2, ( height() - toplevel->height()) / 2 );
}

#include "applet.moc"
