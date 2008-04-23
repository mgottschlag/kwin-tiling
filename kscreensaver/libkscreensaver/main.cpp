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

#include <config-workspace.h>

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>

#include <QDialog>
#include <QEvent>
#include <QKeyEvent>
#include <QSocketNotifier>

#include <klocale.h>
#include <kglobal.h>
#include <kdebug.h>
#include <kcmdlineargs.h>
#include <kapplication.h>
#include <kcrash.h>
#include <kaboutdata.h>

#include <QtGui/QX11Info>

static void crashHandler( int  )
{
#ifdef SIGABRT
    signal (SIGABRT, SIG_DFL);
#endif
    abort();
}

extern "C" {

static int termPipe[2];

static void termHandler( int )
{
    write( termPipe[1], "", 1 );
}

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
    virtual bool eventFilter( QObject *, QEvent *e )
    {
        if (e->type() == QEvent::KeyPress) {
            keyPressEvent( (QKeyEvent *)e );
            return true;
        } else if( e->type() == QEvent::Close ) {
            // In demo mode, screensaver's QWidget does create()
            // with winId of the DemoWidget, which results in two QWidget's
            // sharing the same winId and Qt delivering events only to one of them.
            qApp->quit();
        }
        return false;
    }

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


    KCmdLineOptions options;

    options.add("setup", ki18n("Setup screen saver"));

    options.add("window-id wid", ki18n("Run in the specified XWindow"));

    options.add("root", ki18n("Run in the root XWindow"));

    options.add("demo", ki18n("Start screen saver in demo mode"), "default");

    KCmdLineArgs::addCmdLineOptions(options);

    KApplication app;

    if (!pipe(termPipe))
    {
        struct sigaction sa;
        sa.sa_handler = termHandler;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = 0;
        sigaction(SIGTERM, &sa, 0);
        QSocketNotifier *sn = new QSocketNotifier(termPipe[0], QSocketNotifier::Read, &app);
        QObject::connect(sn, SIGNAL(activated(int)), &app, SLOT(quit()));
    }

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
       args->clear();
       dlg->exec();
       delete dlg;
       return 0;
    }

    if (args->isSet("window-id"))
    {
        saveWin = args->getOption("window-id").toInt();
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
        demoWidget->setAttribute(Qt::WA_NoSystemBackground);
        demoWidget->setAttribute(Qt::WA_PaintOnScreen);
        demoWidget->show();
        app.processEvents();
        saveWin = demoWidget->winId();
    }

    target = screenSaverInterface.create( saveWin );
    target->setAttribute(Qt::WA_NoSystemBackground);
    target->setAttribute(Qt::WA_PaintOnScreen);
    target->show();

    if (demoWidget)
    {
        target->installEventFilter( demoWidget );
    }

    args->clear();
    app.exec();

    delete target;
    delete demoWidget;

    return 0;
}

