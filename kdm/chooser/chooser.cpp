/***************************************************************************
                         chooser.cpp  -  description
                             -------------------
    begin                : Tue Nov 9 1999
    copyright            : (C) 1999 by Harald Hoyer
    email                : Harald.Hoyer@RedHat.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "DXdmcp.h"
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <klocale.h>

#include <X11/Xlib.h>
#include <X11/keysym.h>

#include <stdlib.h>
#include <time.h>
#include <sys/stat.h>

static const char *description = I18N_NOOP("Login chooser for Xdmcp");

static const char *version = "v0.1";

static ChooserDlg *kchooser = 0;


class MyApp:public KApplication {

  public:
    virtual bool x11EventFilter(XEvent *);
};


bool MyApp::x11EventFilter(XEvent * ev)
{
    // Hack to tell dialogs to take focus
    if (ev->type == ConfigureNotify) {
	QWidget *target = QWidget::find(((XConfigureEvent *) ev)->window);
	if (target) {
	    target = target->topLevelWidget();
	    if (target->isVisible() && !target->isPopup())
		XSetInputFocus(qt_xdisplay(), target->winId(),
			       RevertToParent, CurrentTime);
	}
    }

    return FALSE;
}

static KCmdLineOptions options[] = {
    {"xdmaddress <addr>", I18N_NOOP("Specify the chooser socket (in hex)"), 0},
    {"clientaddress <addr>", I18N_NOOP("Specify the client ip (in hex)"), 0},
    {"connectionType <type>", I18N_NOOP("Specify the connection type (in dec)"), 0},
    {"+[host]", I18N_NOOP("Specify the hosts to list or use BROADCAST"), 0},
    KCmdLineLastOption
};

extern char *savhome;

int main(int argc, char **argv)
{
    /* for QSettings */
    srand( time( 0 ) );
    char qtrc[32];
    for (int i = 0; i < 10000; i++) {
	sprintf( qtrc, "/tmp/%010d", rand() );
	if (!mkdir( qtrc, 0700 ))
	    goto okay;
    }
    Die( EX_UNMANAGE_DPY, "Cannot create $HOME\n" );
  okay:
    if (setenv( "HOME", qtrc, 1 ))
	Die( EX_UNMANAGE_DPY, "Cannot set $HOME\n" );
    if (!(savhome = strdup (qtrc)))
	Die( EX_UNMANAGE_DPY, "Cannot save $HOME\n" );

    KApplication::disableAutoDcopRegistration();

    KCmdLineArgs::init(argc, argv, "chooser", description, version);
    KCmdLineArgs::addCmdLineOptions(options);

    MyApp app;

    CXdmcp *cxdmcp = new CXdmcp();

    kchooser = new ChooserDlg(cxdmcp);

    app.setMainWidget(kchooser);

    kchooser->show();
    kchooser->ping();
    app.exec();

    Exit(EX_NORMAL);
}
