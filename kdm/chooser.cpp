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
#include <kapp.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>

static ChooserDlg *kchooser = 0;

class MyApp:public KApplication {
public:
     MyApp( int &argc, char **argv );
     virtual ~MyApp();
     virtual bool x11EventFilter( XEvent * );
};

MyApp::MyApp(int &argc, char **argv ) : KApplication(argc, argv, "chooser")
{}

MyApp::~MyApp()
{}

bool
MyApp::x11EventFilter( XEvent * ev){
/*
	if( ev->type == KeyPress && kchooser){
	  // This should go away
	  KeySym ks = XLookupKeysym(&(ev->xkey),0);
	  if (ks == XK_Return ||
	      ks == XK_KP_Enter)
	       kchooser->ReturnPressed();
  }
*/

  // Hack to tell dialogs to take focus
  if( ev->type == ConfigureNotify) {
	  QWidget* target = QWidget::find( (( XConfigureEvent *) ev)->window);
	  target = target->topLevelWidget();
	 	if( target->isVisible() && !target->isPopup())
		 	XSetInputFocus( qt_xdisplay(), target->winId(),
		RevertToParent, CurrentTime);
	}

  return FALSE;
}

main ( int argc, char **argv)
{
	CXdmcp *cxdmcp = new CXdmcp(argc, argv);

  MyApp app( argc, argv);

	kchooser = new ChooserDlg(cxdmcp);

	app.setMainWidget(kchooser);

	kchooser->show();
	kchooser->ping();
  app.exec();

  exit(0);
}
