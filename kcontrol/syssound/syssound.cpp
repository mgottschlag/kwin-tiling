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
#include <qobject.h>
#include <qlabel.h>

#include <qlistbox.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qdir.h>
#include <qdragobject.h>

#include <klocale.h>
#include <kconfig.h>
#include <kglobal.h>
#include <kdebug.h>
#include <kstddirs.h>
#include <kapp.h>
#include <kwm.h>
#include <kmessagebox.h>

#include "syssound.h"
#include "syssound.moc"

#include <config.h>

#define EVENT_COUNT 28

#define SOUND_DEBUG 1

template class QList<QString>;

const char *eventNames[2][29] = {

  {
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
  },
  {
    "Desktop1",
    "Desktop2",
    "Desktop3",
    "Desktop4",
    "Desktop5",
    "Desktop6",
    "Desktop7",
    "Desktop8",
    "Window Activate",
    "Window New",
    "Window Delete",
    "Startup",
    "Window Shade Up",
    "Window Shade Down",
    "Window Iconify",
    "Window DeIconify",
    "Window Maximize",
    "Window UnMaximize",
    "Window Sticky",
    "Window UnSticky",
    "Window Trans New",
    "Window Trans Delete",
    "Logout",
    "Logout Message",
    "Window Move Start",
    "Window Move End",
    "Window Resize Start",
    "Window Resize End",
  }
};


KSoundWidget::KSoundWidget(QWidget *parent, const char *name):
  KCModule(parent, name), selected_event(0){

  QBoxLayout *top_layout, *status_layout;
  QGridLayout *grid_layout;

  QString path;
  QDir dir;
  QStringList list;
  QLabel *eventlabel, *soundlabel, *statustext;

  //
  // CC: Set up the List of known System Events
  //

  eventlist = new QListBox(this);
  eventlist->insertItem(i18n("(none)"));
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

  eventlist->insertItem(i18n(    "Window Shade Up"));
  eventlist->insertItem(i18n(    "Window Shade Down"));
  eventlist->insertItem(i18n(    "Window Iconify"));
  eventlist->insertItem(i18n(    "Window DeIconify"));
  eventlist->insertItem(i18n(    "Window Maximize"));
  eventlist->insertItem(i18n(    "Window UnMaximize"));
  eventlist->insertItem(i18n(    "Window Sticky"));
  eventlist->insertItem(i18n(    "Window UnSticky"));
  eventlist->insertItem(i18n(    "Window Trans New"));
  eventlist->insertItem(i18n(    "Window Trans Delete"));
  eventlist->insertItem(i18n(    "Logout"));
  eventlist->insertItem(i18n(    "Logout Message"));
  eventlist->insertItem(i18n(    "Window Move Start"));
  eventlist->insertItem(i18n(    "Window Move End"));
  eventlist->insertItem(i18n(    "Window Resize Start"));
  eventlist->insertItem(i18n(    "Window Resize End"));

  eventlist->setMinimumSize(25+eventlist->fontMetrics().width(i18n("Change to Desktop 1")), 150);
  //
  // CC: Now set up the list of known WAV Files
  //

  soundlist = new QListBox(this);
  soundlist->setAcceptDrops(true);
  setAcceptDrops(true);
  soundlist->installEventFilter(this);

  soundlist->insertItem(i18n("(none)"));

  list = KGlobal::dirs()->findAllResources("sound");

  soundlist->insertStringList(list);

  sounds_enabled = new QCheckBox(this);
  sounds_enabled->setText(i18n("E&nable system sounds"));
  connect(sounds_enabled, SIGNAL(clicked()), this, SLOT(changed()));

  btn_test = new QPushButton(this);
  btn_test->setText(i18n("&Test"));

  eventlabel = new QLabel(eventlist, i18n("&Events:"), this);
  soundlabel = new QLabel(soundlist, i18n("&Sounds:"), this);
  statustext = new QLabel(i18n(
	       "Additional WAV files can be dropped onto the sound list."
	       ),this);
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

  status_layout = new QHBoxLayout();
  status_layout->addWidget(statustext, 1);
  status_layout->addWidget(btn_test);

  top_layout = new QVBoxLayout(this, 10);
  top_layout->addWidget(sounds_enabled,0,AlignLeft);
  top_layout->addLayout(grid_layout, 1);
  top_layout->addLayout(status_layout);

  top_layout->activate();

  setUpdatesEnabled(TRUE);

  load();

  connect(eventlist, SIGNAL(highlighted(int)), this, SLOT(eventSelected(int)));
  connect(soundlist, SIGNAL(highlighted(const QString &)),
	  this, SLOT(soundSelected(const QString &)));
  connect(btn_test, SIGNAL(clicked()), this, SLOT(playCurrentSound()));

};

KSoundWidget::~KSoundWidget(){

 // delete audiodrop;

}

void KSoundWidget::load(){

  QString *str;
  QString hlp;
  KConfig *config;
  int lf;

  // CC: we need to read/write the config file of "kwmsound" and not
  // our own (that would be called syssoundrc)

  config = new KConfig("kwmsoundrc");

  config->setGroup("SoundConfiguration");

  soundnames.clear();
  for( lf = 0; lf < EVENT_COUNT; lf++) {

    str = new QString;
    *str = config->readEntry(eventNames[0][lf],"(none)");

    if (str->at(0) == '/') {
      // CC: a file that is not in the default
      // sound directory-> add it to the soundlist too

      addToSoundList(*str);
    }

    soundnames.append(str);

  }

  soundnames.setAutoDelete(TRUE);

  config->setGroup("GlobalConfiguration");

  hlp = config->readEntry("EnableSounds","No");

  if (!stricmp(hlp.ascii(),"Yes"))
    sounds_enabled->setChecked(True);
  else
    sounds_enabled->setChecked(False);

  delete config;

  eventlist->setCurrentItem(0);
  soundlist->setCurrentItem(0);
}


void KSoundWidget::eventSelected(int index){


  int i;
  uint listlen;
  char found;
  QString *sname, hlp;

  selected_event = index;

  if (0 == index)
    soundlist->setCurrentItem(0);
  else {
    // CC: at first, get the name of the sound file we want to select
    sname = soundnames.at(index-1);
    CHECK_PTR(sname); // CC: should never happen anyways...
    kDebugInfo(0, "event %d wants sound %s", index, debugString(*sname));

    i = 1;
    listlen = soundlist->count();
    found = 0;
    while ( (!found) && (i < (int)listlen) ) {
      hlp = soundlist->text(i);
      if (hlp == *sname)
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
  QString *snd;

  if (selected_event > 0) {
    snd = soundnames.at(selected_event-1);
    *snd= filename;

    emit KCModule::changed(true);
  }
}


void KSoundWidget::save(){

  KConfig *config;
  QString *sname, helper;
  int lf;
  KWM kwm;

  // config = kapp->getConfig();
  config = new KConfig("kwmsoundrc");

  config->setGroup("SoundConfiguration");

  for( lf = 0; lf < EVENT_COUNT; lf++) {
    sname = soundnames.at(lf);

    if (sname->isEmpty())
      config->writeEntry(eventNames[0][lf],"(none)");
    else {
      // keep configuration files language--independent

      if (i18n("(none)") == *sname)
	config->writeEntry(eventNames[0][lf], "(none)");
      else
	config->writeEntry(eventNames[0][lf], *sname);

    }

  }

  config->setGroup("GlobalConfiguration");

  if (sounds_enabled->isChecked())
    config->writeEntry("EnableSounds", "Yes");
  else
    config->writeEntry("EnableSounds", "No");

  config->sync();
  delete config;
  kwm.sendKWMCommand("syssnd_restart");

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

    found = (sound == *soundnames.at(i));
    i++;
  }

 if (!found) {

   // CC: Fine, the sound is not already in the sound list!

   QString *tmp = new QString(sound); // CC: take a copy...
   //   soundnames.append(tmp);
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
    soundnames.append(&none);

  sounds_enabled->setChecked(False);

  eventlist->setCurrentItem(0);
  soundlist->setCurrentItem(0);
}


void KSoundWidget::changed()
{
  emit KCModule::changed(true);
}
