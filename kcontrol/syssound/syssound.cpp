/*

    $Id$

    Copyright (C) 1997 Christian Czezatke (e9025461@student.tuwien.ac.at)
                  1998 Bernd Wuebben <wuebben@kde.org>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.

*/


#include <qlabel.h>
#include <qtabwidget.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qdragobject.h>

#include <klocale.h>
#include <kconfig.h>
#include <kglobal.h>
#include <kdebug.h>
#include <kstddirs.h>
#include <kapp.h>
#include <kwin.h>
#include <kmessagebox.h>
#include <klistbox.h>
#include <kdialog.h>

#include "syssound.h"
#include "syssound.moc"

#include <X11/Xlib.h>
#include <config.h>

#define SOUND_DEBUG 1

struct soundEvent { 
   const char *name;
   const char *title;
};

const soundEvent soundEvents[] =
{
    { "GeneralBeep", I18N_NOOP("General Beep") },
    { "Desktop1", I18N_NOOP("Change to Desktop 1") },
    { "Desktop2", I18N_NOOP("Change to Desktop 2") },
    { "Desktop3", I18N_NOOP("Change to Desktop 3") },
    { "Desktop4", I18N_NOOP("Change to Desktop 4") },
    { "Desktop5", I18N_NOOP("Change to Desktop 5") },
    { "Desktop6", I18N_NOOP("Change to Desktop 6") },
    { "Desktop7", I18N_NOOP("Change to Desktop 7") },
    { "Desktop8", I18N_NOOP("Change to Desktop 8") },
    { "WindowActivate", I18N_NOOP("Activate Window") },
    { "WindowOpen", I18N_NOOP("Open New Window") },
    { "WindowClose", I18N_NOOP("Close Window") },
    { "Startup", I18N_NOOP("Startup") },
    { "WindowShadeUp", I18N_NOOP("Window Shade Up") },
    { "WindowShadeDown", I18N_NOOP("Window Shade Down") },
    { "WindowIconify", I18N_NOOP("Window Minimize") },
    { "WindowDeIconify", I18N_NOOP("Window Restore") },
    { "WindowMaximize", I18N_NOOP("Window Maximize") },
    { "WindowUnMaximize", I18N_NOOP("Window UnMaximize") },
    { "WindowSticky", I18N_NOOP("Window Sticky") },
    { "WindowUnSticky", I18N_NOOP("Window UnSticky") },
    { "WindowTransNew",I18N_NOOP("Window Trans New") },
    { "WindowTransDelete", I18N_NOOP("Window Trans Delete") },
    { "Logout", I18N_NOOP("Logout") },
    { "LogoutMessage", I18N_NOOP("Logout Message") },
    { "WindowMoveStart", I18N_NOOP("Window Move Start") },
    { "WindowMoveEnd", I18N_NOOP("Window Move End") },
    { "WindowResizeStart", I18N_NOOP("Window Resize Start") },
    { "WindowResizeEnd", I18N_NOOP("Window Resize End") },
    { 0, 0 }
};

KSoundWidget::KSoundWidget(QWidget *parent, const char *name):
    KCModule(parent, name), m_selectedEvent(0)
{
    m_none = QString::fromLatin1("(none)");
    m_i18nNone = i18n("(none)");
    QBoxLayout *layout = new QVBoxLayout(this);
    m_tabs = new QTabWidget(this);
    layout->addWidget(m_tabs);

    addMainTab();
    addBellTab();
    load();
}

KSoundWidget::~KSoundWidget()
{
}

void KSoundWidget::addMainTab()
{
    QFrame *mainFrame = new QFrame(this);
    m_tabs->addTab(mainFrame, i18n("&Main"));
    //
    // CC: Set up the List of known System Events
    //

    m_eventList = new KListBox(mainFrame);
    //
    // CC: Now set up the list of known WAV Files
    //

    m_soundList = new KListBox(mainFrame);
    m_soundList->setAcceptDrops(true);
    setAcceptDrops(true);
    m_soundList->installEventFilter(mainFrame);


    m_soundsEnabled = new QCheckBox(mainFrame);
    m_soundsEnabled->setText(i18n("E&nable system sounds"));
    connect(m_soundsEnabled, SIGNAL(clicked()), this, SLOT(changed()));

    m_testButton = new QPushButton(mainFrame);
    m_testButton->setText(i18n("&Test"));

    m_eventLabel = new QLabel(m_eventList, i18n("&Events:"), mainFrame);
    m_soundLabel = new QLabel(m_soundList, i18n("Available &Sounds:"), mainFrame);
    m_statusText = new QLabel(i18n(
	"Additional WAV files can be dropped onto the sound list."
	),mainFrame);
    m_eventLabel->setAlignment(QLabel::AlignLeft);
    m_soundLabel->setAlignment(QLabel::AlignLeft);
    m_statusText->setAlignment(QLabel::AlignLeft);

    QGridLayout *grid_layout = new QGridLayout(2, 2);
    grid_layout->addWidget(m_eventLabel, 0, 0);
    grid_layout->addWidget(m_eventList, 1, 0);
    grid_layout->addWidget(m_soundLabel, 0, 1);
    grid_layout->addWidget(m_soundList, 1, 1);
    grid_layout->setRowStretch(1, 1);
    grid_layout->setColStretch(0, 30);
    grid_layout->setColStretch(1, 70);

    QBoxLayout *layout = new QVBoxLayout(mainFrame, KDialog::marginHint(),
				 KDialog::spacingHint());
    layout->addWidget(m_soundsEnabled,0,AlignLeft);
    layout->addLayout(grid_layout, 1);

    QBoxLayout *status_layout = new QHBoxLayout();
    status_layout->addWidget(m_statusText, 1);
    status_layout->addWidget(m_testButton);
    layout->addLayout(status_layout);

    connect(m_eventList, SIGNAL(highlighted(int)), this, SLOT(eventSelected(int)));
    connect(m_soundList, SIGNAL(highlighted(int)), this, SLOT(soundSelected(int)));
    connect(m_testButton, SIGNAL(clicked()), this, SLOT(playCurrentSound()));
}

void KSoundWidget::addBellTab()
{
    QFrame *bellFrame = new QFrame(this);
    m_tabs->addTab(bellFrame, i18n("&Bell"));

    QLabel *label = new QLabel(bellFrame);
    QString setting = m_soundsEnabled->isChecked() ?
	i18n("enabled") : i18n("disabled");
    label->setText(i18n("The following settings are only relevant to the X bell or beep.\nIf you are using system sounds, the \"General Beep\" sound will override\nthese settings, but they will still apply to non-KDE programs.  Otherwise,\nthe values you specify here will be used.\n\nSystem sounds are currently %1.").arg(setting));
    // set up bell tab
    QBoxLayout *layout = new QVBoxLayout(bellFrame, KDialog::marginHint(),
			     KDialog::spacingHint());
    layout->addWidget(label);

    // args: label, min, max, step, initial, units
    m_volume = new KIntNumInput(50, bellFrame);
    m_volume->setLabel(i18n("Volume:"));
    m_volume->setRange(0, 100, 5);
    m_volume->setSuffix("%");
    m_volume->setSteps(5,25);
    layout->addWidget(m_volume);

    m_pitch = new KIntNumInput(m_volume, 800, bellFrame);
    m_pitch->setLabel(i18n("Pitch:"));
    m_pitch->setRange(0, 2000, 20);
    m_pitch->setSuffix(i18n("Hz"));
    m_pitch->setSteps(40,200);
    layout->addWidget(m_pitch);

    m_duration = new KIntNumInput(m_pitch, 100, bellFrame);
    m_duration->setLabel(i18n("Duration:"));
    m_duration->setRange(0, 1000, 50);
    m_duration->setSuffix(i18n("ms"));
    m_duration->setSteps(20,100);
    layout->addWidget(m_duration);

    QFrame *hLine = new QFrame(bellFrame);
    hLine->setFrameStyle(QFrame::Sunken|QFrame::HLine);

    layout->addWidget(hLine);

    QPushButton *test = new QPushButton(i18n("&Test"), bellFrame, "test");
    layout->addWidget(test, 0, AlignRight);
    connect( test, SIGNAL(clicked()), SLOT(ringBell()));

    layout->addStretch(1);

    // watch for changes
    connect(m_volume, SIGNAL(valueChanged(int)),
	    this, SLOT(changed()));
    connect(m_pitch, SIGNAL(valueChanged(int)),
	    this, SLOT(changed()));
    connect(m_duration, SIGNAL(valueChanged(int)),
	    this, SLOT(changed()));
}

void KSoundWidget::load()
{
  KConfig config("kwmsoundrc");

  // CC: we need to read/write the config file of "kwmsound" and not
  // our own (that would be called syssoundrc)

  config.setGroup("SoundConfiguration");

  m_allSounds = KGlobal::dirs()->findAllResources("sound");
  m_allSounds.prepend(m_i18nNone);

  m_soundSettings.clear();
  
  const soundEvent *event = soundEvents;
  while(event && event->name)
  {
    QString sound = config.readEntry(event->name, m_none);
   
    if (!sound.isEmpty() && (sound[0] == '/')) 
    {
      // CC: a file that is not in the default
      // sound directory-> add it to the soundlist too
      
      if (!m_allSounds.contains(sound))
         m_allSounds.append(sound);
    }

    m_soundSettings.append(sound);
    event++;
  }

  config.setGroup("GlobalConfiguration");

  bool enableSounds = config.readBoolEntry("EnableSounds",false);

  m_soundsEnabled->setChecked(enableSounds);

  m_soundList->setCurrentItem(0);

  // bell
  XKeyboardState kbd;
  XGetKeyboardControl(kapp->getDisplay(), &kbd);

#if 0
  config.setGroup("Bell");
  bellVolume = config.readNumEntry("Volume", kbd.bell_percent);
  bellPitch = config.readNumEntry("Pitch", kbd.bell_pitch);
  bellDuration = config.readNumEntry("Duration", kbd.bell_duration);
#endif

  // the GUI should reflect the real values
  setBellVolume(kbd.bell_percent);
  setBellPitch(kbd.bell_pitch);
  setBellDuration(kbd.bell_duration);

  initLists();
}

void KSoundWidget::initLists()
{
  m_eventList->clear();
  const soundEvent *event = soundEvents;
  while(event && event->name)
  {
    m_eventList->insertItem(i18n(event->title));
    event++;
  }
  m_eventList->setMinimumSize(m_eventList->sizeHint());

  m_soundList->clear();
  m_soundList->insertStringList(m_allSounds);
  m_soundList->setMinimumSize(m_soundList->sizeHint());
  updateStatus();
  m_eventList->setCurrentItem(0);
  eventSelected(0);
}


void KSoundWidget::eventSelected(int index)
{
  m_selectedEvent = index;

  QString sound = m_soundSettings[index];
  
  int soundIndex = m_allSounds.findIndex(sound);
  if (soundIndex > 0)
  {
     m_soundList->setCurrentItem(soundIndex);
  }
  else
  {
     m_soundList->setCurrentItem(0);
  }
  m_soundList->centerCurrentItem();
}

void KSoundWidget::soundSelected(int index)
{
  QString sound = m_none;
  if ((index > 0) && (index < (int) m_allSounds.count()))
  {
      sound = m_allSounds[index];
  }
  if (sound == m_soundSettings[m_selectedEvent]) 
     return;
  
  m_soundSettings[m_selectedEvent] = sound;
  emit KCModule::changed(true);
}

// set the slider and the LCD to 'val'
void KSoundWidget::setBellVolume(int val)
{
    m_volume->setValue(val);
}

void KSoundWidget::setBellPitch(int val)
{
    m_pitch->setValue(val);
}

void KSoundWidget::setBellDuration(int val)
{
    m_duration->setValue(val);
}

// return the current LCD setting
int  KSoundWidget::getBellVolume()
{
    return m_volume->value();
}

int  KSoundWidget::getBellPitch()
{
    return m_pitch->value();
}

int  KSoundWidget::getBellDuration()
{
    return m_duration->value();
}


void KSoundWidget::save(){

  KConfig config("kwmsoundrc");
  config.setGroup("SoundConfiguration");

  const soundEvent *event = soundEvents;
  int i = 0;
  while(event && event->name)
  {
    QString sound = m_soundSettings[i];

    if (sound.isEmpty())
      config.writeEntry(event->name, m_none);
    else {
      // keep configuration files language--independent
      if (sound == m_i18nNone)
	config.writeEntry(event->name, m_none);
      else
	config.writeEntry(event->name, sound);

    }
    event++; i++;
  }

  config.setGroup("GlobalConfiguration");

  config.writeEntry("EnableSounds", m_soundsEnabled->isChecked());

  // bell
  XKeyboardControl kbd;

  int bellVolume = getBellVolume();
  int bellPitch = getBellPitch();
  int bellDuration = getBellDuration();

  kbd.bell_percent = bellVolume;
  kbd.bell_pitch = bellPitch;
  kbd.bell_duration = bellDuration;
  XChangeKeyboardControl(kapp->getDisplay(),
			 KBBellPercent | KBBellPitch | KBBellDuration,
			 &kbd);

  config.setGroup("Bell");
  config.writeEntry("Volume",bellVolume);
  config.writeEntry("Pitch",bellPitch);
  config.writeEntry("Duration",bellDuration);

  config.sync();
#warning "use of sendKWMCommand(syssnd_start)"
  //kwin.sendKWMCommand("syssnd_restart");
}

void KSoundWidget::ringBell()
{
    // store the old state
    XKeyboardState old_state;
    XGetKeyboardControl(kapp->getDisplay(), &old_state);

    // switch to the test state
    XKeyboardControl kbd;
    kbd.bell_percent = getBellVolume();
    kbd.bell_pitch = getBellPitch();
    kbd.bell_duration = getBellDuration();
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


void KSoundWidget::playCurrentSound()
{
//audio.stop();
  int soundno = m_soundList->currentItem();
  if (soundno >= 0) {
     QString sound = locate("sound", m_allSounds[soundno]);
//   audio.play(sound);
  }
}


bool KSoundWidget::eventFilter(QObject */*o*/, QEvent *e)
{
  if (e->type() == QEvent::DragEnter) {
    soundlistDragEnterEvent((QDragEnterEvent *) e);
    return true;
  }

  if (e->type() == QEvent::Drop) {
    soundlistDropEvent((QDropEvent *) e);
    return true;
  }

  return false;
}

void KSoundWidget::soundlistDragEnterEvent(QDragEnterEvent *e)
{
  e->accept(QUriDrag::canDecode(e));
}

void KSoundWidget::soundlistDropEvent(QDropEvent *e)
{
  QStringList uris;

  if (QUriDrag::decodeLocalFiles(e, uris) && (uris.count() > 0)) {
    QString msg;

    QStringList::Iterator it(uris.begin());
    for (; it != uris.end(); ++it) {
      QString fname = *it;

      // check for the ending ".wav"
      if ( fname.right(4) != QString::fromLatin1(".WAV")) {
        msg = i18n("Sorry, but \n%1\ndoes not seem "\
			 "to be a WAV--file.").arg(fname);

        KMessageBox::sorry(this, msg);

      } else {

	// CC: Hurra! Finally we've got a WAV file to add to the list
	if (m_allSounds.contains(fname)) {

	  // CC: did not add file because it is already in the list
	  msg = i18n("The file\n"
		           "%1\n"
			   "is already in the list").arg(fname);

	  KMessageBox::information(this, msg);
	}
        else {
          m_allSounds.append(fname);
          m_soundList->insertItem(fname);
        }
      }
    }
  } else {
    // non-local file present
    KMessageBox::sorry(this,
                       i18n("At least one file that was dropped "
                            "was not a local file.  You may only "
                            "add local files."));
  }
}

void KSoundWidget::defaults()
{
  m_allSounds = KGlobal::dirs()->findAllResources("sound");
  m_allSounds.prepend(m_i18nNone);

  m_soundSettings.clear();
  
  const soundEvent *event = soundEvents;
  while(event && event->name)
  {
    m_soundSettings.append(m_none);
    event++;
  }

  m_soundsEnabled->setChecked(False);

  setBellVolume(100);
  setBellPitch(800);
  setBellDuration(100);

  initLists();
}

void KSoundWidget::updateStatus()
{
  bool b = m_soundsEnabled->isChecked();

  m_soundList->setEnabled(b);
  m_eventList->setEnabled(b);
  m_testButton->setEnabled(b);
  m_eventLabel->setEnabled(b);
  m_soundLabel->setEnabled(b);
  m_statusText->setEnabled(b);
}


void KSoundWidget::changed()
{
  updateStatus();

  emit KCModule::changed(true);
}
