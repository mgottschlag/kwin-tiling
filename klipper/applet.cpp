/* -------------------------------------------------------------

   applet.cpp (part of Klipper - Cut & paste history for KDE)

   (C) by Andrew Stanley-Jones
   (C) 2000 by Carsten Pfeiffer <pfeiffer@kde.org>

   Generated with the KDE Application Generator

   Licensed under the GNU GPL Version 2

 ------------------------------------------------------------- */

#include "applet.h"

#include <kaboutapplication.h>
#include <kglobal.h>
#include <klocale.h>
#include <dcopclient.h>
#include <kconfig.h>

#include "toplevel.h"

extern "C"
{
    KPanelApplet* init(QWidget *parent, const QString& configFile)
    {
        KGlobal::locale()->insertCatalogue("klipper");
        int actions = KPanelApplet::Preferences | KPanelApplet::About | KPanelApplet::Help;
        return new KlipperApplet(configFile, KPanelApplet::Normal, actions, parent, "klipper");
    }
}

KlipperApplet::KlipperApplet(const QString& configFile, Type t, int actions,
                         QWidget *parent, const char *name)
    : KPanelApplet(configFile, t, actions, parent, name)
{
    KlipperWidget::createAboutData();
    move( 0, 0 );
    setBackgroundMode(QWidget::X11ParentRelative);
    widget = new KlipperAppletWidget( this );
    setCustomMenu(widget->popup());
    centerWidget();
    widget->show();
}

KlipperApplet::~KlipperApplet()
{
    widget->saveSession();
    delete widget;
    KlipperWidget::destroyAboutData();
}

int KlipperApplet::widthForHeight(int) const
{
    return widget->width();
}

int KlipperApplet::heightForWidth(int) const
{
    return widget->height();
}

void KlipperApplet::resizeEvent( QResizeEvent* ev )
{
    widget->adjustSize();
    KPanelApplet::resizeEvent( ev );
    centerWidget();
}

void KlipperApplet::centerWidget()
{
    int x = (width() - widget->width())/2;
    int y = (height() - widget->height())/2;
    widget->move( x, y );
}

void KlipperApplet::preferences()
{
    widget->slotConfigure();
}

void KlipperApplet::help()
{
    kapp->invokeHelp(QString::null, QString::fromLatin1("klipper"));
}

void KlipperApplet::about()
{
    KAboutApplication about(this, 0);
    about.exec();
}

KlipperAppletWidget::KlipperAppletWidget( QWidget* parent )
// init() is called first, before KlipperWidget is called with ( parent, kconfig )
    : KlipperWidget( ( init(), parent ), new KConfig( "klipperrc" ))
{
}

// this needs to be called before KlipperWidget ctor, because it performs already some
// operations with the clipboard, and the other running instance could notice that
// and request data while this instance is waiting in the DCOP call
void KlipperAppletWidget::init()
{
    // if there's klipper process running, quit it
    QByteArray arg1, arg2;
    QCString str;
    // call() - wait for finishing
    kapp->dcopClient()->call("klipper", "klipper", "quitProcess()", arg1, str, arg2 );
    // register ourselves, so if klipper process is started,
    // it will quit immediately (KUniqueApplication)
    s_dcop = new DCOPClient;
    s_dcop->registerAs( "klipper", false );
}

KlipperAppletWidget::~KlipperAppletWidget()
{
    delete s_dcop;
    s_dcop = 0;
}

DCOPClient* KlipperAppletWidget::s_dcop = 0;

// this is just to make klipper process think we're KUniqueApplication
// (AKA ugly hack)
int KlipperAppletWidget::newInstance()
{
    return 0;
}


#include "applet.moc"
