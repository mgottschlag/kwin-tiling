    /*

    Copyright (C) 2000 Stefan Westerfeld
                       stefan@space.twc.de

    $Id$

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

#include <sys/types.h>
#include <sys/stat.h>

#include <stdlib.h>

#include <qfileinfo.h>
#include <qstring.h>
#include <qlayout.h>
#include <qwidget.h>
#include <qvbuttongroup.h>
#include <qdir.h>
#include <qwhatsthis.h>

#include <kconfig.h>
#include <ksimpleconfig.h>
#include <klocale.h>
#include <kglobal.h>
#include <kmessagebox.h>

#include "arts.h"

/* check if starting realtime would be possible */

bool artswrapper_check()
{
    if(system("artswrapper check") == 0)
        return true;
    return false;
}

extern "C" {

  QString
createArgs(bool netTrans, bool duplex, int responseTime)
{
    QString args;

    switch (responseTime) {
        case 0: args += " -F 3 -S 512";     break;
        case 1: args += " -F 7 -S 1024";    break;
        case 2: args += " -F 5 -S 8192";    break;
        case 3: args += " -F 128 -S 4096";  break;
    }

    if (duplex)
      args += " -d";

    if (netTrans)
      args += " -n";

    return args;
}
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

    fullDuplex = new QCheckBox(this);
    fullDuplex->setText(i18n("Enable full &duplex operation"));
    layout->addWidget(fullDuplex);
    connect(fullDuplex,SIGNAL(clicked()),this,SLOT(slotChanged()));

    QWhatsThis::add(fullDuplex, i18n("This enables the soundserver to record and play sound at the same time. If you use applications like internet telephony, voice recognition or similar, you probably want this."));

    responseGroup = new QVButtonGroup(i18n("Response time"), this);
    QWhatsThis::add(responseGroup, i18n("If you increase the response time, the aRts soundserver will be able to keep up with playing incoming requests more easily, but CPU load will increase."));

    responseButton[0] = new QRadioButton( i18n("&Fast (10ms)"), responseGroup );
    responseButton[1] = new QRadioButton( i18n("&Standard (50ms)"), responseGroup );
    responseButton[2] = new QRadioButton( i18n("&Comfortable (250ms)"), responseGroup);
    responseButton[3] = new QRadioButton( i18n("&Don't care (large!)"), responseGroup);
    connect(responseGroup, SIGNAL(clicked(int)), SLOT(slotChanged()));

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

    setMinimumSize(sizeHint());

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
    fullDuplex->setChecked(config->readBoolEntry("FullDuplex",false));
    int responseTime=config->readNumEntry("ResponseTime",2);
    responseButton[ (responseTime<4) ? responseTime : 2 ]->setChecked(true);

    updateWidgets();
}

void KArtsModule::saveParams( void )
{
    config->setGroup("Arts");
    config->writeEntry("StartServer",startServer->isChecked());
    config->writeEntry("StartRealtime",startRealtime->isChecked());
    config->writeEntry("NetworkTransparent",networkTransparent->isChecked());
    config->writeEntry("X11GlobalComm",x11Comm->isChecked());
    config->writeEntry("FullDuplex",fullDuplex->isChecked());

    int t;

    for (int i = 0; i < 4; i++) {

        if (responseButton[i]->isChecked()) {
          t = i;
          config->writeEntry("ResponseTime", t);
          break;
        }
    }

   // Save arguments string in case any other process wants to restart artsd.

   config->writeEntry("Arguments",
     createArgs(networkTransparent->isChecked(), fullDuplex->isChecked(), t));

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

        KMessageBox::error( 0,
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
    fullDuplex->setChecked(false);
    responseButton[1]->setChecked(true);
}

QString KArtsModule::quickHelp() const
{
        return i18n("<h1>The aRts sound server</h1> Here you can configure aRts, KDE's sound server."
          " This program not only allows you to hear your system sounds while simultaneously"
          " listening to some MP3 file or playing a game with a background music. It also allows you"
          " to apply different effects to your system sounds and provides programmers with"
          " an easy way to achieve sound support.");
}

void KArtsModule::updateWidgets()
{
    startRealtime->setEnabled(startServer->isChecked());
    networkTransparent->setEnabled(startServer->isChecked());
    x11Comm->setEnabled(startServer->isChecked());
    fullDuplex->setEnabled(startServer->isChecked());
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
        KConfig *config = new KConfig("kcmartsrc", true, false);
        QCString cmdline;

        config->setGroup("Arts");
        bool startServer = config->readBoolEntry("StartServer",true);
        bool startRealtime = config->readBoolEntry("StartRealtime",false);
        bool networkTransparent = config->readBoolEntry("NetworkTransparent",false);
        bool x11Comm = config->readBoolEntry("X11GlobalComm",false);
        bool fullDuplex = config->readBoolEntry("FullDuplex",false);
        int responseTime = config->readNumEntry("ResponseTime",2);

        delete config;

        /* put the value of x11Comm into .mcoprc */
        KConfig *X11CommConfig = new KSimpleConfig(QDir::homeDirPath()+"/.mcoprc");

        if(x11Comm)
            X11CommConfig->writeEntry("GlobalComm","Arts::X11GlobalComm");
        else
            X11CommConfig->writeEntry("GlobalComm","Arts::TmpGlobalComm");

        X11CommConfig->sync();
        delete X11CommConfig;

        if(startServer)
        {
            cmdline = "kdeinit_wrapper ";
            if(startRealtime)
                cmdline += "artswrapper";
            else
                cmdline += "artsd";

            QString args =
              createArgs(networkTransparent, fullDuplex, responseTime);

            cmdline += args.utf8();

            system(cmdline);
        }
    }
}

#include "arts.moc"
