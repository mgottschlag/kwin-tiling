/* This file is part of the KDE libraries

    Copyright (c) 2001  Martin R. Jones <mjones@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/
#include "kscreensaver.h"
#include "kscreensaver_vroot.h"

#include <config.h>

#include <stdlib.h>
#include <stdio.h>
#include <signal.h>

#include <QDialog>
#include <QEvent>
#include <QKeyEvent>

#include <klocale.h>
#include <kglobal.h>
#include <kdebug.h>
#include <kcmdlineargs.h>
#include <kapplication.h>
#include <kcrash.h>
#include <kaboutdata.h>

#include <QtGui/QX11Info>

static const KCmdLineOptions options[] =
{
  { "setup", I18N_NOOP("Setup screen saver"), 0 },
  { "window-id wid", I18N_NOOP("Run in the specified XWindow"), 0 },
  { "root", I18N_NOOP("Run in the root XWindow"), 0 },
  { "demo", I18N_NOOP("Start screen saver in demo mode"), "default"},
  KCmdLineLastOption
};

static void crashHandler( int  )
{
#ifdef SIGABRT
    signal (SIGABRT, SIG_DFL);
#endif
    abort();
}

//----------------------------------------------------------------------------

class DemoWindow : public QWidget
{
public:
    DemoWindow() : QWidget()
    {
	setFixedSize(600, 420);
    }

protected:
    virtual void keyPressEvent(QKeyEvent *e)
    {
        if (e->text() == QLatin1String("q"))
        {
            qApp->quit();
        }
    }

    virtual void closeEvent( QCloseEvent * )
    {
        qApp->quit();
    }
};


//----------------------------------------------------------------------------
#if defined(Q_WS_QWS) || defined(Q_WS_MACX)
typedef WId Window;
#endif

int kScreenSaverMain( int argc, char** argv, KScreenSaverInterface& screenSaverInterface )
{
    KLocale::setMainCatalog("libkscreensaver");
    KCmdLineArgs::init(argc, argv, screenSaverInterface.aboutData());

    KCmdLineArgs::addCmdLineOptions(options);

    KApplication app;

    KCrash::setCrashHandler( crashHandler );
    KGlobal::locale()->insertCatalog("klock");
    KGlobal::locale()->insertCatalog("kscreensaver");

    DemoWindow *demoWidget = 0;
    Window saveWin = 0;
    KScreenSaver *target;

    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

    if (args->isSet("setup"))
    {
       QDialog *dlg = screenSaverInterface.setup();
       dlg->exec();
       delete dlg;
       exit(0);
    }

    if (args->isSet("window-id"))
    {
        saveWin = atol(args->getOption("window-id"));
    }

#ifdef Q_WS_X11 //FIXME
    if (args->isSet("root"))
    {
		QX11Info inf;
        saveWin = RootWindow(QX11Info::display(), inf.screen());
    }
#endif

    if (args->isSet("demo"))
    {
        saveWin = 0;
    }

    if (saveWin == 0)
    {
        demoWidget = new DemoWindow();
        demoWidget->setAttribute(Qt::WA_OpaquePaintEvent);
        saveWin = demoWidget->winId();
        app.processEvents();
    }

    target = screenSaverInterface.create( saveWin );

    if ( demoWidget )
    {
        demoWidget->setFixedSize( 600, 420 );
        demoWidget->show();
    }

    app.exec();

    delete target;
    delete demoWidget;

    return 0;
}

