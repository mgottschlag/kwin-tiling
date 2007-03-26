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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

    Permission is also granted to link this program with the Qt
    library, treating Qt like a library that normally accompanies the
    operating system kernel, whether or not that is in fact the case.

    */

#ifndef KARTSCONFIG_H
#define KARTSCONFIG_H

#include <kapplication.h>

#include <QButtonGroup>
#include <QCheckBox>
#include <QLabel>
#include <QLineEdit>
#include <QRadioButton>
#include <QTimer>

#include <kcmodule.h>
#include <knuminput.h>

#include "generaltab.h"
#include "hardwaretab.h"


class K3Process;
class DeviceManager;

class KArtsModule : public KCModule
{
  Q_OBJECT

public:

  KArtsModule(const KComponentData &inst, QWidget *parent);
  ~KArtsModule();
  void saveParams( void );

  void load();
  void save();
  void defaults();

  bool artsdIsRunning();

private Q_SLOTS:

  void slotChanged();
  void slotTestSound();
  void slotTestMIDI();
  void slotArtsdExited(K3Process* proc);
  void slotProcessArtsdOutput(K3Process* p, char* buf, int len);
  //void slotStartServerChanged();

private:

  void updateWidgets ();
  void calculateLatency();
  QString createArgs(bool netTrans,bool duplex, int fragmentCount,
                     int fragmentSize,
                     const QString &deviceName,
                     int rate, int bits, const QString &audioIO,
                     const QString &addOptions, bool autoSuspend,
                     int suspendTime);
  void GetSettings ();
  int userSavedChanges();

  QCheckBox *startServer, *startRealtime, *networkTransparent,
  			*fullDuplex, *customDevice, *customRate, *autoSuspend;
  QLineEdit *deviceName;
  QSpinBox *samplingRate;
  KIntNumInput *suspendTime;
  generalTab *general;
  hardwareTab *hardware;
  KConfig *config;
  DeviceManager *deviceManager;
  int latestProcessStatus;
  int fragmentCount;
  int fragmentSize;
  bool configChanged;
  bool realtimePossible;

  class AudioIOElement {
  public:
	  AudioIOElement(const QString &name, const QString &fullName)
		  : name(name), fullName(fullName) {;}
	  QString name;
	  QString fullName;
  };

  void initAudioIOList();
  Q3PtrList<AudioIOElement> audioIOList;

  void restartServer();
  bool realtimeIsPossible();
};


class KStartArtsProgressDialog : public KProgressDialog
{
   Q_OBJECT
public:
   KStartArtsProgressDialog(KArtsModule *parent, const char *name,
                          const QString &caption, const QString &text);
public Q_SLOTS:
   void slotProgress();
   void slotFinished();

private:
   QTimer m_timer;
   int m_timeStep;
   KArtsModule *m_module;
   bool m_shutdown;
};

#endif

