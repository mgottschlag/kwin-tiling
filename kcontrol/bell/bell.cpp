/*
  Copyright (c) 1997 Christian Czezatke (e9025461@student.tuwien.ac.at)
                1998 Bernd Wuebben <wuebben@kde.org>
                2000 Matthias Elter <elter@kde.org>

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
*/

#include <qpushbutton.h>
#include <qlayout.h>
#include <qwhatsthis.h>

#include <klocale.h>
#include <kconfig.h>
#include <kglobal.h>
#include <kapp.h>
#include <kdialog.h>
#include <knuminput.h>

#include "bell.h"
#include "bell.moc"

#include <X11/Xlib.h>

extern "C"
{
  KCModule *create_bell(QWidget *parent, const char *name)
  {
    KGlobal::locale()->insertCatalogue("kcmbell");
    return new KBellConfig(parent, name);
  }

  void init_bell()
  {
    XKeyboardState kbd;
    XKeyboardControl kbdc;

    XGetKeyboardControl(kapp->getDisplay(), &kbd);

    KConfig config("kcmbellrc");
    config.setGroup("General");

    kbdc.bell_percent = config.readNumEntry("Volume", kbd.bell_percent);
    kbdc.bell_pitch = config.readNumEntry("Pitch", kbd.bell_pitch);
    kbdc.bell_duration = config.readNumEntry("Duration", kbd.bell_duration);
    XChangeKeyboardControl(kapp->getDisplay(),
			   KBBellPercent | KBBellPitch | KBBellDuration,
			   &kbdc);
  }
}

KBellConfig::KBellConfig(QWidget *parent, const char *name):
    KCModule(parent, name)
{
  QBoxLayout *layout = new QVBoxLayout(this,
				       KDialog::marginHint(),
				       KDialog::spacingHint());

  m_volume = new KIntNumInput(50, this);
  m_volume->setLabel(i18n("Volume:"));
  m_volume->setRange(0, 100, 5);
  m_volume->setSuffix("%");
  m_volume->setSteps(5,25);
  layout->addWidget(m_volume);
  QWhatsThis::add( m_volume, i18n("Here you can customize the volume of the system bell. For further"
    " customization of the bell, see the \"Accessability\" control module.") );

  m_pitch = new KIntNumInput(m_volume, 800, this);
  m_pitch->setLabel(i18n("Pitch:"));
  m_pitch->setRange(20, 2000, 20);
  m_pitch->setSuffix(i18n("Hz"));
  m_pitch->setSteps(40,200);
  layout->addWidget(m_pitch);
  QWhatsThis::add( m_pitch, i18n("Here you can customize the pitch of the system bell. For further"
    " customization of the bell, see the \"Accessability\" control module.") );

  m_duration = new KIntNumInput(m_pitch, 100, this);
  m_duration->setLabel(i18n("Duration:"));
  m_duration->setRange(1, 1000, 50);
  m_duration->setSuffix(i18n("ms"));
  m_duration->setSteps(20,100);
  layout->addWidget(m_duration);
  QWhatsThis::add( m_duration, i18n("Here you can customize the duration of the system bell. For further"
    " customization of the bell, see the \"Accessability\" control module.") );

  QFrame *hLine = new QFrame(this);
  hLine->setFrameStyle(QFrame::Sunken|QFrame::HLine);

  layout->addWidget(hLine);

  QPushButton *test = new QPushButton(i18n("&Test"), this, "test");
  layout->addWidget(test, 0, AlignRight);
  connect( test, SIGNAL(clicked()), SLOT(ringBell()));
  QWhatsThis::add( test, i18n("Click \"Test\" to hear how the system bell will sound using your changed settings.") );

  layout->addStretch(1);

  // watch for changes
  connect(m_volume, SIGNAL(valueChanged(int)), this, SIGNAL(changed(bool)));
  connect(m_pitch, SIGNAL(valueChanged(int)), this, SIGNAL(changed(bool)));
  connect(m_duration, SIGNAL(valueChanged(int)), SIGNAL(changed(bool)));

  load();
}

void KBellConfig::load()
{
  XKeyboardState kbd;
  XGetKeyboardControl(kapp->getDisplay(), &kbd);

  m_volume->setValue(kbd.bell_percent);
  m_pitch->setValue(kbd.bell_pitch);
  m_duration->setValue(kbd.bell_duration);
}

void KBellConfig::save()
{
  XKeyboardControl kbd;

  int bellVolume = m_volume->value();
  int bellPitch = m_pitch->value();
  int bellDuration = m_duration->value();

  kbd.bell_percent = bellVolume;
  kbd.bell_pitch = bellPitch;
  kbd.bell_duration = bellDuration;
  XChangeKeyboardControl(kapp->getDisplay(),
			 KBBellPercent | KBBellPitch | KBBellDuration,
			 &kbd);

  KConfig config("kcmbellrc");
  config.setGroup("General");
  config.writeEntry("Volume",bellVolume);
  config.writeEntry("Pitch",bellPitch);
  config.writeEntry("Duration",bellDuration);

  config.sync();
}

void KBellConfig::ringBell()
{
  // store the old state
  XKeyboardState old_state;
  XGetKeyboardControl(kapp->getDisplay(), &old_state);

  // switch to the test state
  XKeyboardControl kbd;
  kbd.bell_percent = m_volume->value();
  kbd.bell_pitch = m_pitch->value();
  kbd.bell_duration = m_duration->value();
  XChangeKeyboardControl(kapp->getDisplay(),
			 KBBellPercent | KBBellPitch | KBBellDuration,
			 &kbd);
  // ring bell
  XBell(kapp->getDisplay(),100);

  // restore old state
  kbd.bell_percent = old_state.bell_percent;
  kbd.bell_pitch = old_state.bell_pitch;
  kbd.bell_duration = old_state.bell_duration;
  XChangeKeyboardControl(kapp->getDisplay(),
			 KBBellPercent | KBBellPitch | KBBellDuration,
			 &kbd);
}

void KBellConfig::defaults()
{
  m_volume->setValue(100);
  m_pitch->setValue(800);
  m_duration->setValue(100);
}

QString KBellConfig::quickHelp() const
{
  return i18n("<h1>System Bell</h1> Here you can customize the sound of the standard system bell,"
    " i.e. the \"beeep\" you always hear when there's something wrong. Note that you can further"
    " customize this sound using the \"Accessibility\" control module: for example you can choose"
    " a sound file to be played instead of the standard bell.");
}
