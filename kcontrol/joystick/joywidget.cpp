/***************************************************************************
 *   Copyright (C) 2003 by Martin Koller                                   *
 *   m.koller@surfeu.at                                                    *
 *   This file is part of the KDE Control Center Module for Joysticks      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Steet, Fifth Floor, Boston, MA  02110-1301, USA.             *
 ***************************************************************************/
#include "joywidget.h"
#include "joydevice.h"
#include "poswidget.h"
#include "caldialog.h"

#include <q3hbox.h>
#include <q3vbox.h>
#include <q3table.h>
#include <qlabel.h>
#include <qcombobox.h>
#include <q3listbox.h>
#include <qcheckbox.h>
#include <qtimer.h>
#include <qfontmetrics.h>
#include <qpushbutton.h>

#include <klocale.h>
#include <kdialog.h>
#include <kdebug.h>
#include <kmessagebox.h>

#include <stdio.h>

//--------------------------------------------------------------
static QString PRESSED = I18N_NOOP("PRESSED");
//--------------------------------------------------------------

JoyWidget::JoyWidget(QWidget *parent, const char *name)
 : QWidget(parent, name), idle(0), joydev(0)
{
  Q3VBox *mainVbox = new Q3VBox(parent);
  mainVbox->setSpacing(KDialog::spacingHint());

  message = new QLabel(mainVbox);
  message->hide();

  Q3HBox *devHbox = new Q3HBox(mainVbox);
  new QLabel(i18n("Device:"), devHbox);
  device = new QComboBox(true, devHbox);
  device->setInsertionPolicy(QComboBox::NoInsertion);
  connect(device, SIGNAL(activated(const QString &)), this, SLOT(deviceChanged(const QString &)));
  devHbox->setStretchFactor(device, 3);

  Q3HBox *hbox = new Q3HBox(mainVbox);
  hbox->setSpacing(KDialog::spacingHint());

  Q3VBox *vboxLeft = new Q3VBox(hbox);
  vboxLeft->setSpacing(KDialog::spacingHint());

  new QLabel(i18n("Position:"), vboxLeft);
  xyPos = new PosWidget(vboxLeft);
  trace = new QCheckBox(i18n("Show trace"), mainVbox);
  connect(trace, SIGNAL(toggled(bool)), this, SLOT(traceChanged(bool)));

  Q3VBox *vboxMid = new Q3VBox(hbox);
  vboxMid->setSpacing(KDialog::spacingHint());

  Q3VBox *vboxRight = new Q3VBox(hbox);
  vboxRight->setSpacing(KDialog::spacingHint());

  // calculate the column width we need
  QFontMetrics fm(font());
  int colWidth = QMAX(fm.width(PRESSED), fm.width("-32767")) + 10;  // -32767 largest string

  new QLabel(i18n("Buttons:"), vboxMid);
  buttonTbl = new Q3Table(0, 1, vboxMid);
  buttonTbl->setReadOnly(true);
  buttonTbl->horizontalHeader()->setLabel(0, i18n("State"));
  buttonTbl->horizontalHeader()->setClickEnabled(false);
  buttonTbl->horizontalHeader()->setResizeEnabled(false);
  buttonTbl->verticalHeader()->setClickEnabled(false);
  buttonTbl->verticalHeader()->setResizeEnabled(false);
  buttonTbl->setColumnWidth(0, colWidth);

  new QLabel(i18n("Axes:"), vboxRight);
  axesTbl = new Q3Table(0, 1, vboxRight);
  axesTbl->setReadOnly(true);
  axesTbl->horizontalHeader()->setLabel(0, i18n("Value"));
  axesTbl->horizontalHeader()->setClickEnabled(false);
  axesTbl->horizontalHeader()->setResizeEnabled(false);
  axesTbl->verticalHeader()->setClickEnabled(false);
  axesTbl->verticalHeader()->setResizeEnabled(false);
  axesTbl->setColumnWidth(0, colWidth);

  // calibrate button
  calibrate = new QPushButton(i18n("Calibrate"), mainVbox);
  connect(calibrate, SIGNAL(clicked()), this, SLOT(calibrateDevice()));
  calibrate->setEnabled(false);

  // set up a timer for idle processing of joystick events
  idle = new QTimer(this);
  connect(idle, SIGNAL(timeout()), this, SLOT(checkDevice()));

  // check which devicefiles we have
  init();

  vboxLeft->adjustSize();
  vboxMid->adjustSize();
  vboxRight->adjustSize();
  hbox->adjustSize();
  mainVbox->adjustSize();

  setMinimumSize(mainVbox->size());
}

//--------------------------------------------------------------

JoyWidget::~JoyWidget()
{
  delete joydev;
}

//--------------------------------------------------------------

void JoyWidget::init()
{
  // check which devicefiles we have
  int i;
  bool first = true;
  char dev[30];

  device->clear();
  buttonTbl->setNumRows(0);
  axesTbl->setNumRows(0);

  for (i = 0; i < 5; i++)  // check the first 5 devices
  {
    sprintf(dev, "/dev/js%d", i);  // first look in /dev
    JoyDevice *joy = new JoyDevice(dev);

    if ( joy->open() != JoyDevice::SUCCESS )
    {
      delete joy;
      sprintf(dev, "/dev/input/js%d", i);  // then look in /dev/input
      joy = new JoyDevice(dev);

      if ( joy->open() != JoyDevice::SUCCESS )
      {
        delete joy;
        continue;    // try next number
      }
    }

    // we found one

    device->insertItem(QString("%1 (%2)").arg(joy->text()).arg(joy->device()));

    // display values for first device
    if ( first )
    {
      showDeviceProps(joy);  // this sets the joy object into this->joydev
      first = false;
    }
    else
      delete joy;
  }

  /* KDE 4: Remove this check(and i18n) when all KCM wrappers properly test modules */
  if ( device->count() == 0 )
  {
    message->show();
    message->setText(QString("<qt><b>%1</b></qt>").arg(
      i18n("No joystick device automatically found on this computer.\n"
           "Checks were done in /dev/js[0-4] and /dev/input/js[0-4]\n"
           "If you know that there is one attached, please enter the correct device file.")
      .replace("\n", "<br>")));  // hack because we're in string freeze
  }
}

//--------------------------------------------------------------

void JoyWidget::traceChanged(bool state)
{
  xyPos->showTrace(state);
}

//--------------------------------------------------------------

void JoyWidget::restoreCurrDev()
{
  if ( !joydev )  // no device open
  {
    device->setCurrentText("");
    calibrate->setEnabled(false);
  }
  else
  {
    // try to find the current open device in the combobox list
    int index = device->findText(joydev->device(), Qt::MatchContains);

    if ( index == -1 )  // the current open device is one the user entered (not in the list)
      device->setEditText(joydev->device());
    else
      device->setEditText(device->itemText(index));
  }
}

//--------------------------------------------------------------

void JoyWidget::deviceChanged(const QString &dev)
{
  // find "/dev" in given string
  int start, stop;
  QString devName;

  if ( (start = dev.find("/dev")) == -1 )
  {
    KMessageBox::sorry(this,
      i18n("The given device name is invalid (does not contain /dev).\n"
           "Please select a device from the list or\n"
           "enter a device file, like /dev/js0."), i18n("Unknown Device"));

    restoreCurrDev();
    return;
  }

  if ( (stop = dev.find(")", start)) != -1 )  // seems to be text selected from our list
    devName = dev.mid(start, stop - start);
  else
    devName = dev.mid(start);

  if ( joydev && (devName == joydev->device()) ) return;  // user selected the current device; ignore it

  JoyDevice *joy = new JoyDevice(devName);
  JoyDevice::ErrorCode ret = joy->open();

  if ( ret != JoyDevice::SUCCESS )
  {
    KMessageBox::error(this, joy->errText(ret), i18n("Device Error"));

    delete joy;
    restoreCurrDev();
    return;
  }

  showDeviceProps(joy);
}

//--------------------------------------------------------------

void JoyWidget::showDeviceProps(JoyDevice *joy)
{
  joydev = joy;

  buttonTbl->setNumRows(joydev->numButtons());

  axesTbl->setNumRows(joydev->numAxes());
  if ( joydev->numAxes() >= 2 )
  {
    axesTbl->verticalHeader()->setLabel(0, "1(x)");
    axesTbl->verticalHeader()->setLabel(1, "2(y)");
  }

  calibrate->setEnabled(true);
  idle->start(0);

  // make both tables use the same space for header; this looks nicer
  buttonTbl->setLeftMargin(QMAX(buttonTbl->verticalHeader()->width(),
                                  axesTbl->verticalHeader()->width()));
  axesTbl->setLeftMargin(buttonTbl->verticalHeader()->width());
}

//--------------------------------------------------------------

void JoyWidget::checkDevice()
{
  if ( !joydev ) return;  // no open device yet

  JoyDevice::EventType type;
  int number, value;

  if ( !joydev->getEvent(type, number, value) )
    return;

  if ( type == JoyDevice::BUTTON )
  {
    if ( value == 0 )  // button release
      buttonTbl->setText(number, 0, "-");
    else
      buttonTbl->setText(number, 0, PRESSED);
  }

  if ( type == JoyDevice::AXIS )
  {
    if ( number == 0 ) // x-axis
      xyPos->changeX(value);

    if ( number == 1 ) // y-axis
      xyPos->changeY(value);

    axesTbl->setText(number, 0, QString("%1").arg(int(value)));
  }
}

//--------------------------------------------------------------

void JoyWidget::calibrateDevice()
{
  if ( !joydev ) return;  // just to be save

  JoyDevice::ErrorCode ret = joydev->initCalibration();

  if ( ret != JoyDevice::SUCCESS )
  {
    KMessageBox::error(this, joydev->errText(ret), i18n("Communication Error"));
    return;
  }

  if ( KMessageBox::messageBox(this, KMessageBox::Information,
        i18n("<qt>Calibration is about to check the precision.<br><br>"
             "<b>Please move all axes to their center position and then "
             "do not touch the joystick anymore.</b><br><br>"
             "Click OK to start the calibration.</qt>"),
        i18n("Calibration"),
        KStdGuiItem::ok(), KStdGuiItem::cancel()) != KMessageBox::Ok )
    return;

  idle->stop();  // stop the joystick event getting; this must be done inside the calibrate dialog

  CalDialog dlg(this, joydev);
  dlg.calibrate();

  // user cancelled somewhere during calibration, therefore the device is in a bad state
  if ( dlg.result() == QDialog::Rejected )
    joydev->restoreCorr();

  idle->start(0);  // continue with event getting
}

//--------------------------------------------------------------

void JoyWidget::resetCalibration()
{
  if ( !joydev ) return;  // just to be save

  JoyDevice::ErrorCode ret = joydev->restoreCorr();

  if ( ret != JoyDevice::SUCCESS )
  {
    KMessageBox::error(this, joydev->errText(ret), i18n("Communication Error"));
  }
  else
  {
    KMessageBox::information(this,
      i18n("Restored all calibration values for joystick device %1.").arg(joydev->device()),
      i18n("Calibration Success"));
  }
}

//--------------------------------------------------------------

#include "joywidget.moc"
