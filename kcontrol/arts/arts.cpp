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
#include <qwhatsthis.h>

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
    setButtons(Cancel|Apply|Ok);

    QVBoxLayout *layout = new QVBoxLayout(this, 10);
    
    // options
    startServer = new QCheckBox(this);
    startServer->setText(i18n("&Start aRts soundserver on KDE startup"));
    layout->addWidget(startServer);
    connect(startServer,SIGNAL(clicked()),this,SLOT(slotChanged()));

    QWhatsThis::add(startServer, i18n("If this option is enabled, the arts soundserver will be started on KDE startup. Recommended if you want sound."));

    // dependant options: only useful when soundserver will be started
    QFrame *hLine = new QFrame(this);
    hLine->setFrameStyle(QFrame::Sunken|QFrame::HLine);
    layout->addWidget(hLine);
    
    networkTransparent = new QCheckBox(this);
    networkTransparent->setText(i18n("Enable &network transparency"));
    layout->addWidget(networkTransparent);
    connect(networkTransparent,SIGNAL(clicked()),this,SLOT(slotChanged()));

    QWhatsThis::add(networkTransparent, i18n("This option allows sound requests coming in from over the network to be accepted, instead of just limiting the server to the local computer."));

    x11Comm = new QCheckBox(this);
    x11Comm->setText(i18n("Exchange security and reference info over the &X11 server"));
    layout->addWidget(x11Comm);
    connect(x11Comm,SIGNAL(clicked()),this,SLOT(slotChanged()));

    QWhatsThis::add(x11Comm, i18n("If you want network transparency or if you use the soundserver only when you use X11, enable this option."));

    startRealtime = new QCheckBox(this);
    startRealtime->setText(i18n("Run soundserver with &realtime priority"));
    layout->addWidget(startRealtime);
    connect(startRealtime,SIGNAL(clicked()),this,SLOT(slotChanged()));
    
    QWhatsThis::add(startRealtime, i18n("On systems which support realtime scheduling, if you have sufficient permissions, this option will enable a very high priority for processing sound requests."));
    
    responseGroup = new QButtonGroup(i18n("Response time"), this);
    QWhatsThis::add(responseGroup, i18n("If you increase the response time, the aRts soundserver will be able to keep up with playing incoming requests more easily, but CPU load will increase."));

    QVBoxLayout *vbox = new QVBoxLayout(responseGroup,10);
    vbox->addSpacing(responseGroup->fontMetrics().height());
    
    responseButton[0] = new QRadioButton( i18n("&Fast (10ms)"), responseGroup );
    responseButton[1] = new QRadioButton( i18n("&Standard (50ms)"), responseGroup );
    responseButton[2] = new QRadioButton( i18n("&Confortable (250ms)"), responseGroup);
    //connect(style_group, SIGNAL(clicked(int)), SLOT(style_clicked(int)));
    
    for (int i = 0; i < 3; i++)
	vbox->addWidget(responseButton[i]);
    layout->addWidget(responseGroup);
    
    // options end
    
    QLabel *restartHint = new QLabel(this);
    restartHint->setText(i18n("<qt>The aRts soundserver cannot be "
			      "initialized except at login time. "
			      "If you modify any settings, you will "
			      "have to restart your session in order for "
			      "those changes to take effect.</qt>"));
    layout->addWidget(restartHint);
    layout->addStretch();
    
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
	for(int i=0;i<3;i++)
	{
		if(config->readNumEntry("ResponseTime",1) == i)
			responseButton[i]->setChecked(true);
	}
	updateWidgets();
}

void KArtsModule::saveParams( void )
{
	config->setGroup("Arts");
	config->writeEntry("StartServer",startServer->isChecked());
	config->writeEntry("StartRealtime",startRealtime->isChecked());
	config->writeEntry("NetworkTransparent",networkTransparent->isChecked());
	config->writeEntry("X11GlobalComm",x11Comm->isChecked());
	for(int i=0;i<3;i++)
	{
		if(responseButton[i]->isChecked())
			config->writeEntry("ResponseTime",i);
	}
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
	responseButton[1]->setChecked(true);
}

void KArtsModule::updateWidgets()
{
	startRealtime->setEnabled(startServer->isChecked());
	networkTransparent->setEnabled(startServer->isChecked());
	x11Comm->setEnabled(startServer->isChecked());
	responseGroup->setEnabled(startServer->isChecked());
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
		int responseTime = config->readNumEntry("ResponseTime",1);

		/* put the value of x11Comm into .mcoprc */
		KConfig *X11CommConfig = new KConfig(QDir::homeDirPath()+"/.mcoprc");

		if(x11Comm)
			X11CommConfig->writeEntry("GlobalComm","Arts::X11GlobalComm");
		else
			X11CommConfig->writeEntry("GlobalComm","Arts::TmpGlobalComm");

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

			switch(responseTime)
			{
				/* 8.7 ms */
				case 0: cmdline += " -F 3 -S 512";
					break;
				/* 40 ms */
				case 1: cmdline += " -F 7 -S 1024";
					break;
				/* 255 ms */
				case 2: cmdline += " -F 11 -S 4096";
					break;
			}
			system(cmdline);
		}
	}
}

