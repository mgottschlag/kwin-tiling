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

#include <klocale.h>
#include <kglobal.h>
#include <kaboutdata.h>
#include <kgenericfactory.h>

#include "joystick.h"
#include "joywidget.h"

//---------------------------------------------------------------------------------------------

typedef KGenericFactory<joystick, QWidget> JoystickFactory;
K_EXPORT_COMPONENT_FACTORY(kcm_joystick, JoystickFactory("kcmjoystick"))

//---------------------------------------------------------------------------------------------

joystick::joystick(QWidget *parent, const char *name, const QStringList &)
  : KCModule(JoystickFactory::instance(), parent, name)
{
  myAboutData = new KAboutData("kcmjoystick", I18N_NOOP("KDE Joystick Control Module"), "0.5",
                               I18N_NOOP("KDE Control Center Module to test Joysticks"),
                               KAboutData::License_GPL, "(c) 2004, Martin Koller",
                               0, "m.koller@surfeu.at");

  joyWidget = new JoyWidget(this);

  setMinimumSize(joyWidget->minimumSize());

  setButtons(KCModule::Default);
};

//---------------------------------------------------------------------------------------------

joystick::~joystick()
{
  delete myAboutData;
}

//---------------------------------------------------------------------------------------------

void joystick::load()
{
  joyWidget->init();
}

//---------------------------------------------------------------------------------------------

void joystick::defaults()
{
  joyWidget->resetCalibration();

  emit changed(true);
}

//---------------------------------------------------------------------------------------------

void joystick::save()
{
  // insert your saving code here...
  emit changed(true);
}

//---------------------------------------------------------------------------------------------

QString joystick::quickHelp() const
{
  return i18n("<h1>Joystick</h1>"
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
              );
}

//---------------------------------------------------------------------------------------------

#include "joystick.moc"
