/*
 *
 * Copyright (c) 1998 Matthias Ettrich <ettrich@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <qlabel.h>

#include <kapp.h>
#include <dcopclient.h>
#include <klocale.h>
#include <kglobal.h>
#include <kconfig.h>
#include <kdialog.h>
#include <qcombobox.h>

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "mouse.h"
#include "geom.h"
#include <qlayout.h>
#include <qwhatsthis.h>


extern "C" {
  KCModule *create_kwinmouse ( QWidget *parent, const char* name)
  {
    //CT there's need for decision: kwm or kwin?
    KGlobal::locale()->insertCatalogue("kcmkwm");
    return new KMouseConfig( parent, name);
  }
}


KMouseConfig::~KMouseConfig ()
{

}

KMouseConfig::KMouseConfig (QWidget * parent, const char *name)
  : KCModule (parent, name)
{
  QString strWin1, strWin2, strWin3, strAll1, strAll2, strAll3;
  QGridLayout *layout = new QGridLayout( this, 15, 5,
                     KDialog::marginHint(), 1);
  layout->setColStretch( 1, 50 );
  layout->setColStretch( 2, 20 ); // very wide comboboxes look ugly (DF)
  layout->setColStretch( 3, 20 ); // very wide comboboxes look ugly (DF)
  layout->setColStretch( 4, 50 );

  QLabel* label;

  label = new QLabel(i18n("Active"), this);
  label->setFixedSize(label->sizeHint());
  layout->addWidget(label, 0,2, AlignHCenter);
  QWhatsThis::add( label, i18n("In this column you can customize mouse clicks into the titlebar"
    " or the frame of an active window.") );

  label = new QLabel(i18n("Inactive"), this);
  label->setFixedSize(label->sizeHint());
  layout->addWidget(label, 0,3, AlignHCenter);
  QWhatsThis::add( label, i18n("In this column you can customize mouse clicks into the titlebar"
    " or the frame of an inactive window.") );

  label = new QLabel(i18n("Titlebar and frame:"), this);
  layout->addMultiCellWidget(label, 0,0,0,1);
  layout->setRowStretch(0, 1);
  QWhatsThis::add( label, i18n("Here you can customize mouse click behavior when clicking on the"
    " titlebar or the frame of a window.") );

  label = new QLabel(this);
  label->setFrameStyle(QFrame::HLine|QFrame::Sunken);
  layout->addMultiCellWidget(label, 4, 4, 0, 3);

  label = new QLabel(i18n("Inactive inner window:"), this);
  layout->addMultiCellWidget(label, 5,5,0,3);
  layout->setRowStretch(5, 1);
  QWhatsThis::add( label, i18n("Here you can customize mouse click behavior when clicking on an inactive"
    " inner window ('inner' means: not titlebar, not frame).") );

  label = new QLabel(this);
  label->setFrameStyle(QFrame::HLine|QFrame::Sunken);
  layout->addMultiCellWidget(label, 9, 9, 0, 3);

  label = new QLabel(i18n("Inner window, titlebar and frame:"), this);
  layout->addMultiCellWidget(label, 10,10,0,3);
  layout->setRowStretch(10, 1);
  QWhatsThis::add( label, i18n("Here you can customize KDE's behaviour when clicking somewhere into"
    " a window while pressing a modifier key."));

  label = new QLabel(i18n("Left Button"), this);
  layout->addWidget(label, 1,1);
  QWhatsThis::add( label, i18n("In this row you can customize left click behavior when clicking into"
    " the titlebar or the frame.") );

  label = new QLabel(i18n("Middle Button"), this);
  layout->addWidget(label, 2,1);
  QWhatsThis::add( label, i18n("In this row you can customize middle click behavior when clicking into"
    " the titlebar or the frame.") );

  label = new QLabel(i18n("Right Button"), this);
  layout->addWidget(label, 3,1);
  QWhatsThis::add( label, i18n("In this row you can customize right click behavior when clicking into"
    " the titlebar or the frame.") );

  label = new QLabel(i18n("Left Button"), this);
  layout->addWidget(label, 6,1);
  strWin1 = i18n("In this row you can customize left click behavior when clicking into"
    " an inactive inner window ('inner' means: not titlebar, not frame).");
  QWhatsThis::add( label, strWin1 );

  label = new QLabel(i18n("Middle Button"), this);
  layout->addWidget(label, 7,1);
  strWin2 = i18n("In this row you can customize middle click behavior when clicking into"
    " an inactive inner window ('inner' means: not titlebar, not frame).");
  QWhatsThis::add( label, strWin2 );

  label = new QLabel(i18n("Right Button"), this);
  layout->addWidget(label, 8,1);
  strWin3 = i18n("In this row you can customize right click behavior when clicking into"
    " an inactive inner window ('inner' means: not titlebar, not frame).");
  QWhatsThis::add( label, strWin3 );

  label = new QLabel(i18n("ALT + Left Button"), this);
  layout->addWidget(label, 11,1);
  strAll1 = i18n("Here you can customize KDE's behavior when left clicking into a window"
    " while pressing the ALT key.");
  QWhatsThis::add( label, strAll1 );

  label = new QLabel(i18n("ALT + Middle Button"), this);
  layout->addWidget(label, 12,1);
  strAll2 = i18n("Here you can customize KDE's behavior when middle clicking into a window"
    " while pressing the ALT key.");
  QWhatsThis::add( label, strAll2 );

  label = new QLabel(i18n("ALT + Right Button"), this);
  layout->addWidget(label, 13,1);
  strAll3 = i18n("Here you can customize KDE's behavior when right clicking into a window"
    " while pressing the ALT key.");
  QWhatsThis::add( label, strAll3 );

  QComboBox* combo;

  combo = new QComboBox(this);
  combo->insertItem(i18n("Raise"));
  combo->insertItem(i18n("Lower"));
  combo->insertItem(i18n("Operations menu"));
  combo->insertItem(i18n("Toggle raise and lower"));
  connect(combo, SIGNAL(activated(int)), this, SLOT(slotChanged()));
  layout->addWidget(combo, 1,2);
  coTiAct1 = combo;
  QWhatsThis::add(combo, "Behavior on <em>left</em> click into the titlebar or frame of an <em>active</em> window.");

  combo = new QComboBox(this);
  combo->insertItem(i18n("Raise"));
  combo->insertItem(i18n("Lower"));
  combo->insertItem(i18n("Operations menu"));
  combo->insertItem(i18n("Toggle raise and lower"));
  combo->insertItem(i18n("Nothing"));
  connect(combo, SIGNAL(activated(int)), this, SLOT(slotChanged()));
  layout->addWidget(combo, 2,2);
  coTiAct2 = combo;
  QWhatsThis::add(combo, "Behavior on <em>middle</em> click into the titlebar or frame of an <em>active</em> window.");

  combo = new QComboBox(this);
  combo->insertItem(i18n("Raise"));
  combo->insertItem(i18n("Lower"));
  combo->insertItem(i18n("Operations menu"));
  combo->insertItem(i18n("Toggle raise and lower"));
  combo->insertItem(i18n("Nothing"));
  connect(combo, SIGNAL(activated(int)), this, SLOT(slotChanged()));
  layout->addWidget(combo, 3,2);
  coTiAct3 =  combo;
  QWhatsThis::add(combo, "Behavior on <em>right</em> click into the titlebar or frame of an <em>active</em> window.");

  combo = new QComboBox(this);
  combo->insertItem(i18n("Activate and raise"));
  combo->insertItem(i18n("Activate and lower"));
  combo->insertItem(i18n("Activate"));
  connect(combo, SIGNAL(activated(int)), this, SLOT(slotChanged()));
  layout->addWidget(combo, 1,3);
  coTiInAct1 = combo;
  QWhatsThis::add(combo, "Behavior on <em>left</em> click into the titlebar or frame of an <em>inactive</em> window.");

  combo = new QComboBox(this);
  combo->insertItem(i18n("Activate and raise"));
  combo->insertItem(i18n("Activate and lower"));
  combo->insertItem(i18n("Activate"));
  connect(combo, SIGNAL(activated(int)), this, SLOT(slotChanged()));
  layout->addWidget(combo, 2,3);
  coTiInAct2 = combo;
  QWhatsThis::add(combo, "Behavior on <em>middle</em> click into the titlebar or frame of an <em>inactive</em> window.");

  combo = new QComboBox(this);
  combo->insertItem(i18n("Activate and raise"));
  combo->insertItem(i18n("Activate and lower"));
  combo->insertItem(i18n("Activate"));
  connect(combo, SIGNAL(activated(int)), this, SLOT(slotChanged()));
  layout->addWidget(combo, 3,3);
  coTiInAct3 = combo;
  QWhatsThis::add(combo, "Behavior on <em>right</em> click into the titlebar or frame of an <em>inactive</em> window.");

  combo = new QComboBox(this);
  combo->insertItem(i18n("Activate, raise and pass click"));
  combo->insertItem(i18n("Activate and pass click"));
  combo->insertItem(i18n("Activate"));
  combo->insertItem(i18n("Activate and raise"));
  connect(combo, SIGNAL(activated(int)), this, SLOT(slotChanged()));
  layout->addMultiCellWidget(combo, 6,6, 2, 3);
  coWin1 = combo;
  QWhatsThis::add( combo, strWin1 );

  combo = new QComboBox(this);
  combo->insertItem(i18n("Activate, raise and pass click"));
  combo->insertItem(i18n("Activate and pass click"));
  combo->insertItem(i18n("Activate"));
  combo->insertItem(i18n("Activate and raise"));
  connect(combo, SIGNAL(activated(int)), this, SLOT(slotChanged()));
  layout->addMultiCellWidget(combo, 7,7, 2, 3);
  coWin2 = combo;
  QWhatsThis::add( combo, strWin2 );

  combo = new QComboBox(this);
  combo->insertItem(i18n("Activate, raise and pass click"));
  combo->insertItem(i18n("Activate and pass click"));
  combo->insertItem(i18n("Activate"));
  combo->insertItem(i18n("Activate and raise"));
  connect(combo, SIGNAL(activated(int)), this, SLOT(slotChanged()));
  layout->addMultiCellWidget(combo, 8,8, 2, 3);
  coWin3 = combo;
  QWhatsThis::add( combo, strWin3 );

  combo = new QComboBox(this);
  combo->insertItem(i18n("Move"));
  combo->insertItem(i18n("Toggle raise and lower"));
  combo->insertItem(i18n("Resize"));
  combo->insertItem(i18n("Raise"));
  combo->insertItem(i18n("Lower"));
  combo->insertItem(i18n("Nothing"));
  connect(combo, SIGNAL(activated(int)), this, SLOT(slotChanged()));
  layout->addMultiCellWidget(combo, 11,11, 2, 3);
  coAll1 = combo;
  QWhatsThis::add( combo, strAll1 );

  combo = new QComboBox(this);
  combo->insertItem(i18n("Move"));
  combo->insertItem(i18n("Toggle raise and lower"));
  combo->insertItem(i18n("Resize"));
  combo->insertItem(i18n("Raise"));
  combo->insertItem(i18n("Lower"));
  combo->insertItem(i18n("Nothing"));
  connect(combo, SIGNAL(activated(int)), this, SLOT(slotChanged()));
  layout->addMultiCellWidget(combo, 12,12, 2, 3);
  coAll2 = combo;
  QWhatsThis::add( combo, strAll2 );

  combo = new QComboBox(this);
  combo->insertItem(i18n("Move"));
  combo->insertItem(i18n("Toggle raise and lower"));
  combo->insertItem(i18n("Resize"));
  combo->insertItem(i18n("Raise"));
  combo->insertItem(i18n("Lower"));
  combo->insertItem(i18n("Nothing"));
  connect(combo, SIGNAL(activated(int)), this, SLOT(slotChanged()));
  layout->addMultiCellWidget(combo, 13,13, 2, 3);
  coAll3 =  combo;
  QWhatsThis::add( combo, strAll3 );

  layout->setRowStretch(14, 1);

  load();
}

void KMouseConfig::setComboText(QComboBox* combo, const char* text){
  int i;
  QString s = i18n(text); // no problem. These are already translated!
  for (i=0;i<combo->count();i++){
    if (s==combo->text(i)){
      combo->setCurrentItem(i);
      return;
    }
  }
}

const char*  KMouseConfig::functionTiAc(int i)
{
  switch (i){
  case 0: return "Raise"; break;
  case 1: return "Lower"; break;
  case 2: return "Operations menu"; break;
  case 3: return "Toggle raise and lower"; break;
  case 4: return "Nothing"; break;
  case 5: return ""; break;
  }
  return "";
}
const char*  KMouseConfig::functionTiInAc(int i)
{
  switch (i){
  case 0: return "Activate and raise"; break;
  case 1: return "Activate and lower"; break;
  case 2: return "Activate"; break;
  case 3: return ""; break;
  case 4: return ""; break;
  case 5: return ""; break;
  }
  return "";
}
const char*  KMouseConfig::functionWin(int i)
{
  switch (i){
  case 0: return "Activate, raise and pass click"; break;
  case 1: return "Activate and pass click"; break;
  case 2: return "Activate"; break;
  case 3: return "Activate and raise"; break;
  case 4: return ""; break;
  case 5: return ""; break;
  }
  return "";
}
const char*  KMouseConfig::functionAll(int i)
{
  switch (i){
  case 0: return "Move"; break;
  case 1: return "Toggle raise and lower"; break;
  case 2: return "Resize"; break;
  case 3: return "Raise"; break;
  case 4: return "Lower"; break;
  case 5: return "Nothing"; break;
  }
  return "";
}


void KMouseConfig::load()
{
  KConfig *config = new KConfig("kwinrc", false, false);

  config->setGroup( "MouseBindings");
  setComboText(coTiAct1,config->readEntry("CommandActiveTitlebar1","Raise").ascii());
  setComboText(coTiAct2,config->readEntry("CommandActiveTitlebar2","Lower").ascii());
  setComboText(coTiAct3,config->readEntry("CommandActiveTitlebar3","Operations menu").ascii());
  setComboText(coTiInAct1,config->readEntry("CommandInactiveTitlebar1","Activate and raise").ascii());
  setComboText(coTiInAct2,config->readEntry("CommandInactiveTitlebar2","Activate and lower").ascii());
  setComboText(coTiInAct3,config->readEntry("CommandInactiveTitlebar3","Activate").ascii());
  setComboText(coWin1,config->readEntry("CommandWindow1","Activate, raise and pass click").ascii());
  setComboText(coWin2,config->readEntry("CommandWindow2","Activate and pass click").ascii());
  setComboText(coWin3,config->readEntry("CommandWindow3","Activate and pass click").ascii());
  setComboText(coAll1,config->readEntry("CommandAll1","Move").ascii());
  setComboText(coAll2,config->readEntry("CommandAll2","Toggle raise and lower").ascii());
  setComboText(coAll3,config->readEntry("CommandAll3","Resize").ascii());
}

// many widgets connect to this slot
void KMouseConfig::slotChanged()
{
  emit changed(true);
}

void KMouseConfig::save()
{
  KConfig *config = new KConfig("kwinrc", false, false);

  config->setGroup("MouseBindings");
  config->writeEntry("CommandActiveTitlebar1", functionTiAc(coTiAct1->currentItem()));
  config->writeEntry("CommandActiveTitlebar2", functionTiAc(coTiAct2->currentItem()));
  config->writeEntry("CommandActiveTitlebar3", functionTiAc(coTiAct3->currentItem()));
  config->writeEntry("CommandInactiveTitlebar1", functionTiInAc(coTiInAct1->currentItem()));
  config->writeEntry("CommandInactiveTitlebar2", functionTiInAc(coTiInAct2->currentItem()));
  config->writeEntry("CommandInactiveTitlebar3", functionTiInAc(coTiInAct3->currentItem()));
  config->writeEntry("CommandWindow1", functionWin(coWin1->currentItem()));
  config->writeEntry("CommandWindow2", functionWin(coWin2->currentItem()));
  config->writeEntry("CommandWindow3", functionWin(coWin3->currentItem()));
  config->writeEntry("CommandAll1", functionAll(coAll1->currentItem()));
  config->writeEntry("CommandAll2", functionAll(coAll2->currentItem()));
  config->writeEntry("CommandAll3", functionAll(coAll3->currentItem()));

  config->sync();
  if ( !kapp->dcopClient()->isAttached() )
      kapp->dcopClient()->attach();
  kapp->dcopClient()->send("kwin", "", "reconfigure()", "");
}

void KMouseConfig::defaults()
{
  setComboText(coTiAct1,"Raise");
  setComboText(coTiAct2,"Lower");
  setComboText(coTiAct3,"Operations menu");
  setComboText(coTiInAct1,"Activate and raise");
  setComboText(coTiInAct2,"Activate and lower");
  setComboText(coTiInAct3,"Activate");
  setComboText(coWin1,"Activate, raise and pass click");
  setComboText(coWin2,"Activate and pass click");
  setComboText(coWin3,"Activate and pass click");
  setComboText (coAll1,"Move");
  setComboText(coAll2,"Toggle raise and lower");
  setComboText(coAll3,"Resize");
}

QString KMouseConfig::quickHelp() const
{
  return i18n("<h1>Mouse Behavior</h1> Here you can customize the way the KDE window manager handles"
    " mouse button clicks. <p>Please note that this configuration will not take effect if you don't use"
    " KWin as your window manager. If you do use a different window manager, please refer to its documentation"
    " for how to customize mouse behavior.");
}

#include "mouse.moc"
