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
    move( 0, 0 );
    setBackgroundMode(QWidget::X11ParentRelative);
    toplevel = new TopLevel( this, true );
    centerWidget();
    toplevel->show();
}

KlipperApplet::~KlipperApplet()
{
    toplevel->saveSession();
    delete toplevel;
}

int KlipperApplet::widthForHeight(int) const
{
    return toplevel->width();
}

int KlipperApplet::heightForWidth(int) const
{
    return toplevel->height();
}

void KlipperApplet::resizeEvent( QResizeEvent* ev )
{
    toplevel->adjustSize();
    KPanelApplet::resizeEvent( ev );
    centerWidget();
}

void KlipperApplet::centerWidget()
{
    int x = (width() - toplevel->width())/2;
    int y = (height() - toplevel->height())/2;
    toplevel->move( x, y );
}

#include "applet.moc"
