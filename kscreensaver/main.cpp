//-----------------------------------------------------------------------------
//
// Screen savers for KDE
//
// Copyright (c)  Martin R. Jones 1999
//

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <qcolor.h>
#include <klocale.h>
#include <kconfig.h>
#include <kstddirs.h>
#include <kapp.h>
#include "demowin.h"
#include "saver.h"

KLocale *glocale = 0;

void usage( char *name );

//----------------------------------------------------------------------------

int main(int argc, char *argv[])
{
    KApplication app(argc, argv);

    // For the kdebase package we use a single strings file.
    glocale = new KLocale("klock");

    DemoWindow *demoWidget = 0;
	Window saveWin = 0;

	enum parameter_code { 
	    Setup, Window_id, Root, Descr, Help, Unknown
	} parameter;
	
	const char *strings[] = { 
	    "-setup", "-window-id", "-root", "-help", 0
	};

	int i = 1;
	while (i < argc)
    {
	    parameter = Unknown;

	    for ( int p = 0 ; strings[p]; p++)
        {
            if (!strcmp(argv[i], strings[p]))
            {
                parameter = static_cast<parameter_code>(p);
                break;
            }
        }

	    switch (parameter) 
        {
            case Setup:
                setupScreenSaver();
                exit(0);
                break;
            case Window_id:
                saveWin = atol(argv[++i]);
                break;
            case Root:
                saveWin = kapp->desktop()->winId();
                break;
            case Help:
                usage(argv[0]);
                break;
            default: // unknown
                debug("Unknown parameter: %s", argv[i]);
                usage(argv[0]);
                break;
        }
	    i++;
	}

    if (saveWin == 0)
    {
        demoWidget = new DemoWindow();
        demoWidget->setBackgroundMode(QWidget::NoBackground);
//        demoWidget->setBackgroundColor(Qt::black);
        demoWidget->show();
        saveWin = demoWidget->winId();
        app.setMainWidget(demoWidget);
    }

    startScreenSaver(saveWin);
    app.exec();
    stopScreenSaver();

    if (demoWidget)
    {
        delete demoWidget;
    }

	return 0;
}

//----------------------------------------------------------------------------
void usage(char *name)
{
	printf(glocale->translate(
	   "Usage: %1 [-setup|-window-id wid|-root|-help]\n").arg(name)); 
	printf(glocale->translate(
    "Without any parameters, a demo of the screensaver is displayed\n"\
	"  -setup            Setup screen saver\n"\
	"  -window-id wid    Run in the specified XWindow\n"\
	"  -root             Run in the root window\n"\
    "  -help             This help\n"));
	exit(1);
}

