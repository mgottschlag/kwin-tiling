// -*- Mode: C++; c-basic-offset: 4; indent-tabs-mode: nil; tab-width: 8; -*-
/* This file is part of the KDE project
   Copyright (C) by Andrew Stanley-Jones
   Copyright (C) 2000 by Carsten Pfeiffer <pfeiffer@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#include "applet.h"

#include <kaboutapplication.h>
#include <kglobal.h>
#include <klocale.h>
#include <dcopclient.h>
#include <kconfig.h>

#include "toplevel.h"
#include "history.h"
#include "klipperpopup.h"
//Added by qt3to4:
#include <QResizeEvent>
#include <Q3CString>
#include <ktoolinvocation.h>

extern "C"
{
    KDE_EXPORT KPanelApplet* init(QWidget *parent, const QString& configFile)
    {
        KGlobal::locale()->insertCatalog("klipper");
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
    setCustomMenu(widget->history()->popup());
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
    KToolInvocation::invokeHelp(QString::null, QString::fromLatin1("klipper"));
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
    DCOPCString str;
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
