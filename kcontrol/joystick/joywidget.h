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
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifndef _JOYWIDGET_H_
#define _JOYWIDGET_H_

#include <qwidget.h>

class JoyDevice;

class PosWidget;
class QTable;
class QTimer;
class QComboBox;
class QPushButton;
class QCheckBox;

// the widget which displays all buttons, values, etc.
class JoyWidget : public QWidget
{
  Q_OBJECT
  
  public:
    JoyWidget(QWidget *parent = 0, const char *name = 0);

    ~JoyWidget();

    // initialize list of possible devices and open the first available
    void init();

  public slots:
    // reset calibration values to their value when this KCM was started
    void resetCalibration();

  private slots:
    void checkDevice();
    void deviceChanged(const QString &dev);
    void traceChanged(bool);
    void calibrateDevice();

  private:
    void showDeviceProps(JoyDevice *joy);  // fill widgets with given device parameters
    void restoreCurrDev(); // restores the content of the combobox to reflect the current open device

  private:
    QComboBox *device;
    PosWidget *xyPos;
    QTable *buttonTbl;
    QTable *axesTbl;
    QCheckBox *trace;
    QPushButton *calibrate;

    QTimer *idle;

    JoyDevice *joydev;
};

#endif
