/* -------------------------------------------------------------

   applet.cpp (part of Klipper - Cut & paste history for KDE)

   (C) by Andrew Stanley-Jones
   (C) 2000 by Carsten Pfeiffer <pfeiffer@kde.org>

   Generated with the KDE Application Generator

   Licensed under the Artistic License

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
    : KlipperWidget( parent, new KConfig( "klipperrc" ))
{
    // if there's klipper process running, quit it
    QByteArray arg1, arg2;
    QCString str;
    // call() - wait for finishing
    kapp->dcopClient()->call("klipper", "klipper", "quitProcess()", arg1, str, arg2 );
    // register ourselves, so if klipper process is started,
    // it will quit immediately (KUniqueApplication)
    m_dcop = new DCOPClient;
    m_dcop->registerAs( "klipper", false );
}

KlipperAppletWidget::~KlipperAppletWidget()
{
    delete m_dcop;
}

// this is just to make klipper process think we're KUniqueApplication
// (AKA ugly hack)
int KlipperAppletWidget::newInstance()
{
    return 0;
}


#include "applet.moc"
