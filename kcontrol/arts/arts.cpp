    /*

    Copyright (C) 2000 Stefan Westerfeld
                       stefan@space.twc.de

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    Permission is also granted to link this program with the Qt
    library, treating Qt like a library that normally accompanies the
    operating system kernel, whether or not that is in fact the case.

    */

#include <iostream.h>
#include <stdio.h> 
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
 
#include <qfileinfo.h> 
#include <qstring.h>
#include <qmessagebox.h> 
#include <qlayout.h>
#include <qwidget.h>
#include <qtabwidget.h>
#include <qtabbar.h>
#include <qdir.h>

#include <kconfig.h>
#include <klocale.h>
#include <kglobal.h>

#include "arts.h"

/* check if starting realtime would be possible */

bool artswrapper_check()
{
	if(system("artswrapper check") == 0)
		return true;
	return false;
}

KArtsModule::KArtsModule(QWidget *parent, const char *name)
  : KCModule(parent, name)
{
  QVBoxLayout *layout = new QVBoxLayout(this, 10);

  // options
  startServer = new QCheckBox(this);
  startServer->setText(i18n("&Start aRts soundserver on KDE startup"));
  layout->addWidget(startServer);
  connect(startServer,SIGNAL(clicked()),this,SLOT(slotChanged()));

  // dependant options: only useful when soundserver will be started
  QFrame *hLine1 = new QFrame(this);
  hLine1->setFrameStyle(QFrame::Sunken|QFrame::HLine);
  layout->addWidget(hLine1);

  startRealtime = new QCheckBox(this);
  startRealtime->setText(i18n("Run soundserver with &realtime priority"));
  layout->addWidget(startRealtime);
  connect(startRealtime,SIGNAL(clicked()),this,SLOT(slotChanged()));

  networkTransparent = new QCheckBox(this);
  networkTransparent->setText(i18n("Enable &network transparency"));
  layout->addWidget(networkTransparent);
  connect(networkTransparent,SIGNAL(clicked()),this,SLOT(slotChanged()));

  x11Comm = new QCheckBox(this);
  x11Comm->setText(i18n("Exchange security and reference info over the &X11 server"));
  layout->addWidget(x11Comm);
  connect(x11Comm,SIGNAL(clicked()),this,SLOT(slotChanged()));

  QLabel *x11CommHint = new QLabel(this);
  x11CommHint->setText(i18n("    If you want network transparency or if you use the soundserver\n    only when you use X11, enable this."));
  layout->addWidget(x11CommHint);

  QFrame *hLine = new QFrame(this);
  hLine->setFrameStyle(QFrame::Sunken|QFrame::HLine);
 

  // options end

  layout->addWidget(hLine);
  QLabel *restartHint = new QLabel(this);
  restartHint->setText(i18n("As the aRts soundserver will be started when KDE is started,\nthe changes you make in this dialog will only take effect then.\nSo logout and login again after having changed something in this dialog."));
  restartHint->setTextFormat(RichText);
  layout->addWidget(restartHint);
  layout->addStretch(5);
  layout->activate();

  config = new KConfig("kcmartsrc");

  GetSettings();
}


void KArtsModule::GetSettings( void )
{
	config->setGroup("Arts");
	startServer->setChecked(config->readBoolEntry("StartServer",true));
	startRealtime->setChecked(config->readBoolEntry("StartRealtime",false));
	networkTransparent->setChecked(config->readBoolEntry("NetworkTransparent",false));
	x11Comm->setChecked(config->readBoolEntry("X11GlobalComm",false));
	updateWidgets();
}

void KArtsModule::saveParams( void )
{
	config->setGroup("Arts");
	config->writeEntry("StartServer",startServer->isChecked());
	config->writeEntry("StartRealtime",startRealtime->isChecked());
	config->writeEntry("NetworkTransparent",networkTransparent->isChecked());
	config->writeEntry("X11GlobalComm",x11Comm->isChecked());
	config->sync();
}

void KArtsModule::load()
{
	GetSettings();
}

void KArtsModule::save()
{
	if(startRealtime->isChecked() && !artswrapper_check())
	{
		FILE *why = popen("artswrapper check 2>&1","r");
		char reason[1024];
		QString thereason;
		while(fgets(reason,1024,why)) thereason += reason;
		fclose(why);

		QMessageBox::warning( 0, "kcmarts",
				i18n("There is an installation problem which doesn't allow "
					"starting the aRts server with realtime priority. \n"
					"The following problem occured:\n")+thereason);
	}
	saveParams();
}

void KArtsModule::defaults()
{
	startServer->setChecked(true);
	startRealtime->setChecked(false);
	networkTransparent->setChecked(false);
	x11Comm->setChecked(false);
}

void KArtsModule::updateWidgets()
{
	startRealtime->setEnabled(startServer->isChecked());
	networkTransparent->setEnabled(startServer->isChecked());
	x11Comm->setEnabled(startServer->isChecked());
}

void KArtsModule::slotChanged()
{
	updateWidgets();
	emit changed(true);
}


extern "C"
{
	KCModule *create_arts(QWidget *parent, const char *name) 
	{ 
		KGlobal::locale()->insertCatalogue("kcmarts");
		return new KArtsModule(parent, name);
	}

	void init_arts()
	{
		KConfig *config = new KConfig("kcmartsrc");
		QCString cmdline;

		config->setGroup("Arts");
		bool startServer = config->readBoolEntry("StartServer",true);
		bool startRealtime = config->readBoolEntry("StartRealtime",false);
		bool networkTransparent = config->readBoolEntry("NetworkTransparent",false);
		bool x11Comm = config->readBoolEntry("X11GlobalComm",false);

		/* put the value of x11Comm into .mcoprc */
		KConfig *X11CommConfig = new KConfig(QDir::homeDirPath()+"/.mcoprc");

		if(x11Comm)
			X11CommConfig->writeEntry("GlobalComm","X11GlobalComm");
		else
			X11CommConfig->writeEntry("GlobalComm","TmpGlobalComm");

		X11CommConfig->sync();
		delete X11CommConfig;

		if(startServer)
		{
			cmdline = "kdeinit_wrapper ";
			if(startRealtime && artswrapper_check())
				cmdline += "artswrapper";
			else
				cmdline += "artsd";

			if(networkTransparent)
				cmdline += " -n";

			system(cmdline);
		}
	}
}

