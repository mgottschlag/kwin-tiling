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


#include <stdio.h>
#include <stdlib.h>
#include <iostream.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <qobject.h>
#include <qlabel.h>
#include <qtabwidget.h>
#include <qlistbox.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qdir.h>
#include <qdragobject.h>
#include <qfileinfo.h>
#include <qmessagebox.h>

#include <klocale.h>
#include <ksimpleconfig.h>
#include <kglobal.h>
#include <kdebug.h>
#include <kstddirs.h>
#include <kapp.h>
#include <kwm.h>
#include <kmessagebox.h>
#include <kdialog.h>

#include "syssound.h"
#include "syssound.moc"

#include <X11/Xlib.h>
#include <config.h>

#define EVENT_COUNT 29

#define SOUND_DEBUG 1

template class QList<QString>;

const char *eventNames[] = 
{
    "GeneralBeep",
    "Desktop1",
    "Desktop2",
    "Desktop3",
    "Desktop4",
    "Desktop5",

    "Desktop6",
    "Desktop7",
    "Desktop8",
    "WindowActivate",
    "WindowOpen",
    "WindowClose",
    "Startup",
    "WindowShadeUp",
    "WindowShadeDown",
    "WindowIconify",
    "WindowDeIconify",
    "WindowMaximize",
    "WindowUnMaximize",
    "WindowSticky",
    "WindowUnSticky",
    "WindowTransNew",
    "WindowTransDelete",
    "Logout",
    "LogoutMessage",
    "WindowMoveStart",
    "WindowMoveEnd",
    "WindowResizeStart",
    "WindowResizeEnd",
};


KSoundWidget::KSoundWidget(QWidget *parent, const char *name):
    KCModule(parent, name), selected_event(0)
{
    
    QTabWidget *tabs;
    QFrame *mainFrame, *bellFrame;

    QBoxLayout *layout, *status_layout;
    QGridLayout *grid_layout;
    
    QString path;
    QDir dir;
    QStringList list;
    QLabel *eventlabel, *soundlabel, *statustext;

    tabs = new QTabWidget(this);
    layout = new QVBoxLayout(this);
    layout->addWidget(tabs);

    mainFrame = new QFrame(this);
    bellFrame = new QFrame(this);

    tabs->addTab(mainFrame, i18n("&Main"));
    tabs->addTab(bellFrame, i18n("&Bell"));

    //
    // CC: Set up the List of known System Events
    //

    eventlist = new QListBox(mainFrame);
    eventlist->insertItem(i18n("General Beep"));
    eventlist->insertItem(i18n("Change to Desktop 1"));
    eventlist->insertItem(i18n("Change to Desktop 2"));
    eventlist->insertItem(i18n("Change to Desktop 3"));
    eventlist->insertItem(i18n("Change to Desktop 4"));
    eventlist->insertItem(i18n("Change to Desktop 5"));
    eventlist->insertItem(i18n("Change to Desktop 6"));
    eventlist->insertItem(i18n("Change to Desktop 7"));
    eventlist->insertItem(i18n("Change to Desktop 8"));
    
    eventlist->insertItem(i18n("Activate Window"));
    eventlist->insertItem(i18n("Open new window"));
    eventlist->insertItem(i18n("Close Window"));
    eventlist->insertItem(i18n("Startup"));

    eventlist->insertItem(i18n("Window Shade Up"));
    eventlist->insertItem(i18n("Window Shade Down"));
    eventlist->insertItem(i18n("Window Iconify"));
    eventlist->insertItem(i18n("Window DeIconify"));
    eventlist->insertItem(i18n("Window Maximize"));
    eventlist->insertItem(i18n("Window UnMaximize"));
    eventlist->insertItem(i18n("Window Sticky"));
    eventlist->insertItem(i18n("Window UnSticky"));
    eventlist->insertItem(i18n("Window Trans New"));
    eventlist->insertItem(i18n("Window Trans Delete"));
    eventlist->insertItem(i18n("Logout"));
    eventlist->insertItem(i18n("Logout Message"));
    eventlist->insertItem(i18n("Window Move Start"));
    eventlist->insertItem(i18n("Window Move End"));
    eventlist->insertItem(i18n("Window Resize Start"));
    eventlist->insertItem(i18n("Window Resize End"));

    eventlist->setMinimumSize(eventlist->sizeHint());
    //
    // CC: Now set up the list of known WAV Files
    //
    
    soundlist = new QListBox(mainFrame);
    soundlist->setAcceptDrops(true);
    setAcceptDrops(true);
    soundlist->installEventFilter(mainFrame);

    soundlist->insertItem(i18n("(none)"));
    
    list = KGlobal::dirs()->findAllResources("sound");
    
    soundlist->insertStringList(list);
    soundlist->setMinimumSize(soundlist->sizeHint());

    sounds_enabled = new QCheckBox(mainFrame);
    sounds_enabled->setText(i18n("E&nable system sounds"));
    connect(sounds_enabled, SIGNAL(clicked()), this, SLOT(changed()));
    
    btn_test = new QPushButton(mainFrame);
    btn_test->setText(i18n("&Test"));
    
    eventlabel = new QLabel(eventlist, i18n("&Events:"), mainFrame);
    soundlabel = new QLabel(soundlist, i18n("Available &Sounds:"), mainFrame);
    statustext = new QLabel(i18n(
	"Additional WAV files can be dropped onto the sound list."
	),mainFrame);
    eventlabel->setAlignment(QLabel::AlignLeft);
    soundlabel->setAlignment(QLabel::AlignLeft);
    statustext->setAlignment(QLabel::AlignLeft);
    
    grid_layout = new QGridLayout(2, 2);
    grid_layout->addWidget(eventlabel, 0, 0);
    grid_layout->addWidget(eventlist, 1, 0);
    grid_layout->addWidget(soundlabel, 0, 1);
    grid_layout->addWidget(soundlist, 1, 1);
    grid_layout->setRowStretch(1, 1);
    grid_layout->setColStretch(0, 30);
    grid_layout->setColStretch(1, 70);
    
    layout = new QVBoxLayout(mainFrame, KDialog::marginHint(),
				 KDialog::spacingHint());
    layout->addWidget(sounds_enabled,0,AlignLeft);
    layout->addLayout(grid_layout, 1);
    
    status_layout = new QHBoxLayout();
    status_layout->addWidget(statustext, 1);
    status_layout->addWidget(btn_test);
    layout->addLayout(status_layout);

    QLabel *label = new QLabel(bellFrame);
    QString setting = sounds_enabled->isChecked() ? 
	i18n("enabled") : i18n("disabled");
    label->setText(i18n("The following settings are only relevant to the X bell or beep.\nIf you are using system sounds, the \"General Beep\" sound will override\nthese settings, but they will still apply to non-KDE programs.  Otherwise,\nthe values you specify here will be used.\n\nSystem sounds are currently %1.").arg(setting));
    // set up bell tab
    layout = new QVBoxLayout(bellFrame, KDialog::marginHint(),
			     KDialog::spacingHint());
    layout->addWidget(label);

    // args: label, min, max, step, initial, units
    volume = new KIntNumInput(50, bellFrame);
    volume->setLabel(i18n("Volume:"));
    volume->setRange(0, 100, 5);
    volume->setSuffix("%");
    volume->setSteps(5,25);
    layout->addWidget(volume);

    pitch = new KIntNumInput(volume, 800, bellFrame);
    pitch->setLabel(i18n("Pitch:"));
    pitch->setRange(0, 2000, 20);
    pitch->setSuffix(i18n("Hz"));
    pitch->setSteps(40,200);
    layout->addWidget(pitch);

    duration = new KIntNumInput(pitch, 100, bellFrame);
    duration->setLabel(i18n("Duration:"));
    duration->setRange(0, 1000, 50);
    duration->setSuffix(i18n("ms"));
    duration->setSteps(20,100);
    layout->addWidget(duration);
    
    QFrame *hLine = new QFrame(bellFrame);
    hLine->setFrameStyle(QFrame::Sunken|QFrame::HLine);
    
    layout->addWidget(hLine);
    
    test = new QPushButton(i18n("&Test"), bellFrame, "test");
    layout->addWidget(test, 0, AlignRight);
    connect( test, SIGNAL(clicked()), SLOT(ringBell()));
  
    layout->addStretch(1);

    setUpdatesEnabled(TRUE);
    
    load();
    
    connect(eventlist, SIGNAL(highlighted(int)), this, SLOT(eventSelected(int)));
    connect(soundlist, SIGNAL(highlighted(const QString &)),
	    this, SLOT(soundSelected(const QString &)));
    connect(btn_test, SIGNAL(clicked()), this, SLOT(playCurrentSound()));

    // watch for changes
    connect(volume, SIGNAL(valueChanged(int)), 
	    this, SLOT(changed()));
    connect(pitch, SIGNAL(valueChanged(int)), 
	    this, SLOT(changed()));
    connect(duration, SIGNAL(valueChanged(int)), 
	    this, SLOT(changed()));

};

KSoundWidget::~KSoundWidget()
{
}

void KSoundWidget::load()
{
  QString hlp;
  KSimpleConfig config("kwmsoundrc");
  int lf;

  // CC: we need to read/write the config file of "kwmsound" and not
  // our own (that would be called syssoundrc)

  config.setGroup("SoundConfiguration");

  soundnames.clear();
  for( lf = 0; lf < EVENT_COUNT; lf++) {

    QString str;
    str = config.readEntry(eventNames[lf],"(none)");

    if (str.at(0) == '/') {
      // CC: a file that is not in the default
      // sound directory-> add it to the soundlist too

      addToSoundList(str);
    }

    soundnames.append(str);

  }

  config.setGroup("GlobalConfiguration");

  hlp = config.readEntry("EnableSounds","No");

  if (!stricmp(hlp.ascii(),"Yes"))
    sounds_enabled->setChecked(True);
  else
    sounds_enabled->setChecked(False);

  eventlist->setCurrentItem(0);
  soundlist->setCurrentItem(0);

  // bell
  XKeyboardState kbd;
  XGetKeyboardControl(kapp->getDisplay(), &kbd);
  
  config.setGroup("Bell");
  bellVolume = config.readNumEntry("Volume", kbd.bell_percent);
  bellPitch = config.readNumEntry("Pitch", kbd.bell_pitch);
  bellDuration = config.readNumEntry("Duration", kbd.bell_duration);

  // the GUI should reflect the real values
  setBellVolume(kbd.bell_percent);
  setBellPitch(kbd.bell_pitch);
  setBellDuration(kbd.bell_duration);
}


void KSoundWidget::eventSelected(int index){


  int i;
  uint listlen;
  char found;
  QString sname, hlp;

  selected_event = index;

  if (0 == index)
    soundlist->setCurrentItem(0);
  else {
    // CC: at first, get the name of the sound file we want to select
    sname = soundnames[index-1];
    kDebugInfo(0, "event %d wants sound %s", index, debugString(sname));

    i = 1;
    listlen = soundlist->count();
    found = 0;
    while ( (!found) && (i < (int)listlen) ) {
      hlp = soundlist->text(i);
      if (hlp == sname)
	found = 1;
      else
	i++;
    }

    if (found)
      soundlist->setCurrentItem(i);
    else
      soundlist->setCurrentItem(0);
      // CC: By default, select "no sound"

  }

  soundlist->centerCurrentItem();

}

void KSoundWidget::soundSelected(const QString &filename)
{
  QString snd;
  QStringList::Iterator it;

  if (selected_event > 0) {
      it = soundnames.at(selected_event - 1);
      soundnames.remove(it);
      soundnames.insert(it, filename);

      emit KCModule::changed(true);
  }
}

// set the slider and the LCD to 'val'
void KSoundWidget::setBellVolume(int val)
{
    volume->setValue(val);
}

void KSoundWidget::setBellPitch(int val)
{
    pitch->setValue(val);
}

void KSoundWidget::setBellDuration(int val)
{
    duration->setValue(val);
}

// return the current LCD setting
int  KSoundWidget::getBellVolume()
{
    return volume->value();
}

int  KSoundWidget::getBellPitch()
{
    return pitch->value();
}

int  KSoundWidget::getBellDuration()
{
    return duration->value();
}


void KSoundWidget::save(){

  KSimpleConfig config("kwmsoundrc");
  QString sname, helper;
  int lf;
  KWM kwm;

  config.setGroup("SoundConfiguration");

  for( lf = 0; lf < EVENT_COUNT; lf++) {
    sname = soundnames[lf];

    if (sname.isEmpty())
      config.writeEntry(eventNames[lf],"(none)");
    else {
      // keep configuration files language--independent

      if (i18n("(none)") == sname)
	config.writeEntry(eventNames[lf], "(none)");
      else
	config.writeEntry(eventNames[lf], sname);

    }

  }

  config.setGroup("GlobalConfiguration");

  if (sounds_enabled->isChecked())
    config.writeEntry("EnableSounds", "Yes");
  else
    config.writeEntry("EnableSounds", "No");

  // bell
  XKeyboardControl kbd;
  
  bellVolume = getBellVolume();
  bellPitch = getBellPitch();
  bellDuration = getBellDuration();
  
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
  kwm.sendKWMCommand("syssnd_restart");
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
  QString hlp, sname;
  int soundno;

  audio.stop();
  soundno = soundlist->currentItem();
  if (soundno > 0) {
      sname = locate("sound", soundlist->text(soundno));
      audio.play(sname);
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
      if ( stricmp(".WAV",fname.right(4).latin1()) ) {
        msg = i18n("Sorry, but \n%1\ndoes not seem "\
			 "to be a WAV--file.").arg(fname);

        KMessageBox::sorry(this, msg);

      } else {

	// CC: Hurra! Finally we've got a WAV file to add to the list
	if (!addToSoundList(fname)) {

	  // CC: did not add file because it is already in the list
	  msg = i18n("The file\n"
		           "%1\n"
			   "is already in the list").arg(fname);

	  KMessageBox::information(this, msg);
	
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


bool KSoundWidget::addToSoundList(QString sound){

  // Add "sound" to the sound list, but only if it is not already there

  char found = 0;
  int i, len;

  i = 0;
  len = soundnames.count();

  while ((!found) && (i < len)) {

    found = (sound == soundnames[i]);
    i++;
  }

 if (!found) {

   // CC: Fine, the sound is not already in the sound list!

   QString *tmp = new QString(sound); // CC: take a copy...
   soundlist->insertItem(*tmp);
   soundlist->setTopItem(soundlist->count()-1);

 }

 return !found;

}


void KSoundWidget::defaults()
{
  QString none("none");

  soundnames.clear();
  for (int lf = 0; lf < EVENT_COUNT; lf++)
    soundnames.append(none);

  sounds_enabled->setChecked(False);

  eventlist->setCurrentItem(0);
  soundlist->setCurrentItem(0);

  setBellVolume(100);
  setBellPitch(800);
  setBellDuration(100);

}


void KSoundWidget::changed()
{
  emit KCModule::changed(true);
}
