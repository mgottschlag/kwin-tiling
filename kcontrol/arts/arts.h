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

#ifndef KARTSCONFIG_H
#define KARTSCONFIG_H

#include <kapplication.h>

#include <qlabel.h>
#include <qlineedit.h>
#include <qcheckbox.h>

#include <kcmodule.h>
#include <knuminput.h>

#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <artsconfig.h>


class KProcess;

class KArtsModule : public KCModule
{
  Q_OBJECT

public:

  KArtsModule(QWidget *parent=0, const char *name=0, const QStringList & = QStringList());
  ~KArtsModule();
  void saveParams( void );

  void load();
  void save();
  void defaults();

  QString quickHelp() const;
  const KAboutData* aboutData() const;

private slots:

  void slotChanged();
  /* void slotRestartServer(); */
  void slotTestSound();
  void slotArtsdExited(KProcess* proc);
  void slotProcessArtsdOutput(KProcess* p, char* buf, int len);

private:

  void updateWidgets ();
  void calculateLatency();
  QString createArgs(bool netTrans,bool duplex, int fragmentCount,
                     int fragmentSize,
                     const QString &deviceName,
                     int rate, int bits, const QString &audioIO,
                     const QString &addOptions, bool autoSuspend,
                     int suspendTime,
                     const QString &messageApplication, int loggingLevel);
  void GetSettings ();
  int userSavedChanges();
  bool artsdIsRunning();

  QCheckBox *startServer, *startRealtime, *networkTransparent, *x11Comm,
  			*fullDuplex, *customDevice, *customRate, *autoSuspend, *displayMessage;
  QLineEdit *deviceName;
  QLineEdit *samplingRate;
  QLineEdit *messageApplication;
  KIntNumInput *suspendTime;
  ArtsConfig *artsConfig;
  KConfig *config;
  int latestProcessStatus;
  int fragmentCount;
  int fragmentSize;
  bool configChanged;

  class AudioIOElement {
  public:
	  AudioIOElement(const QString &name, const QString &fullName)
		  : name(name), fullName(fullName) {;}
	  QString name;
	  QString fullName;
  };

  void initAudioIOList();
  QPtrList<AudioIOElement> audioIOList;

  void initServer();
  void stopServer();
  void restartServer();
  bool realtimeIsPossible();
};


#endif

