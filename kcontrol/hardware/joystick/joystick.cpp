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
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.             *
 ***************************************************************************/

#include <kaboutdata.h>
#include <kgenericfactory.h>
#include <kglobal.h>
#include <klocale.h>

#include "joystick.h"
#include "joywidget.h"
#include "joydevice.h"

#include <stdio.h>

//---------------------------------------------------------------------------------------------

typedef KGenericFactory<Joystick, QWidget> JoystickFactory;
K_EXPORT_COMPONENT_FACTORY(kcm_joystick, JoystickFactory("joystick"))

extern "C"
{
  KDE_EXPORT bool test_joystick()
  { /* Code stolen from JoyWidget::init() */
    int i;
    char dev[30];

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

      return true; /* We have at least one joystick and should hence be shown */
    }
    return false;
  }
}

//---------------------------------------------------------------------------------------------

Joystick::Joystick(QWidget *parent, const char *, const QStringList &)
  : KCModule(JoystickFactory::instance(), parent)
{
  setAboutData(new KAboutData("kcmjoystick", I18N_NOOP("KDE Joystick Control Module"), "1.0",
                               I18N_NOOP("KDE Control Center Module to test Joysticks"),
                               KAboutData::License_GPL, "(c) 2004, Martin Koller",
                               0, "m.koller@surfeu.at"));

  setQuickHelp( i18n("<h1>Joystick</h1>"
              "This module helps to check if your joystick is working correctly.<br>"
              "If it delivers wrong values for the axes, you can try to solve this with "
              "the calibration.<br>"
              "This module tries to find all available joystick devices "
              "by checking /dev/js[0-4] and /dev/input/js[0-4]<br>"
              "If you have another device file, enter it in the combobox.<br>"
              "The Buttons list shows the state of the buttons on your joystick, the Axes list "
              "shows the current value for all axes.<br>"
              "NOTE: the current Linux device driver (Kernel 2.4, 2.6) can only autodetect"
              "<ul>"
              "<li>2-axis, 4-button joystick</li>"
              "<li>3-axis, 4-button joystick</li>"
              "<li>4-axis, 4-button joystick</li>"
              "<li>Saitek Cyborg 'digital' joysticks</li>"
              "</ul>"
              "(For details you can check your Linux source/Documentation/input/joystick.txt)"
              ));

  joyWidget = new JoyWidget(this);

  setMinimumSize(joyWidget->minimumSize());

  setButtons(KCModule::Default);
}

//---------------------------------------------------------------------------------------------

void Joystick::load()
{
  joyWidget->init();
}

//---------------------------------------------------------------------------------------------

void Joystick::defaults()
{
  joyWidget->resetCalibration();

  emit changed(true);
}

//---------------------------------------------------------------------------------------------

#include "joystick.moc"
