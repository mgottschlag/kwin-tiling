/*
 * windows.cpp
 *
 * Copyright (c) 1997 Patrick Dowler dowler@morgul.fsh.uvic.ca
 *
 * Requires the Qt widget libraries, available at no cost at
 * http://www.troll.no/
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

#include <qlayout.h>
#include <qspinbox.h>
#include <qslider.h>
#include <qwhatsthis.h>

#include <kapp.h>
#include <klocale.h>
#include <kconfig.h>
#include <knuminput.h>
#include <kdialog.h>
#include <kapp.h>
#include <dcopclient.h>

#include <kglobal.h>

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "windows.h"
#include "geom.h"


// kwin config keywords
#define KWIN_FOCUS     "FocusPolicy"
#define KWIN_PLACEMENT "WindowsPlacement"
#define KWIN_MOVE      "MoveMode"
#define KWIN_MAXIMIZE  "MaximizeOnlyVertically"
#define KWIN_RESIZE_ANIM    "ResizeAnimation"
#define KWIN_RESIZE_OPAQUE    "ResizeMode"
#define KWIN_AUTORAISE_INTERVAL "AutoRaiseInterval"
#define KWIN_AUTORAISE "AutoRaise"
#define KWIN_CLICKRAISE "ClickRaise"


extern "C" {
    KCModule *create_kwinoptions ( QWidget *parent, const char *name )
    {
	//CT there's need for decision: kwm or kwin?
	KGlobal::locale()->insertCatalogue("kcmkwm");
	return new KWindowConfig( parent, name );
    }
}

KWindowConfig::~KWindowConfig ()
{
}

KWindowConfig::KWindowConfig (QWidget * parent, const char *name)
  : KCModule (parent, name)
{
  QString wtstr;
  QBoxLayout *lay = new QVBoxLayout (this, KDialog::marginHint(),
				     KDialog::spacingHint());

  windowsBox = new QButtonGroup(i18n("Windows"), this);

  QBoxLayout *wLay = new QVBoxLayout (windowsBox,KDialog::marginHint(),
				      KDialog::spacingHint());
  wLay->addSpacing(fontMetrics().lineSpacing());

  QBoxLayout *bLay = new QVBoxLayout;
  wLay->addLayout(bLay);

  QGridLayout *rLay = new QGridLayout(2,3);
  wLay->addLayout(rLay);
  rLay->setColStretch(0,0);
  rLay->setColStretch(1,1);

  //CT checkboxes: maximize, move, resize behaviour
  vertOnly = new QCheckBox(i18n("Vertical maximization only by default"), windowsBox);
  QWhatsThis::add( vertOnly, i18n("If this option is enabled, windows will only be maximized"
    " vertically while keeping there width.") );

  // CT: disabling is needed as long as functionality misses in kwin
  vertOnly->setEnabled(false);
  bLay->addWidget(vertOnly);

  opaque = new QCheckBox(i18n("Display content in moving windows"), windowsBox);
  bLay->addWidget(opaque);
  QWhatsThis::add( opaque, i18n("Enable this option if you want a window's content to be fully shown"
    " while moving it, instead of just showing a window 'skeleton'. The result may not be satisfying"
    " on slow machines without graphic acceleration.") );

  resizeOpaqueOn = new QCheckBox(i18n("Display content in resizing windows"), windowsBox);
  bLay->addWidget(resizeOpaqueOn);
  QWhatsThis::add( resizeOpaqueOn, i18n("Enable this option if you want a window's content to be shown"
    " while resizing it, instead of just showing a window 'skeleton'. The result may not be satisfying"
    " on slow machines.") );

  // resize animation - CT 27May98; 19Oct1998
  resizeAnimTitleLabel = new QLabel(i18n("Resize animation:"),
				    windowsBox);
  rLay->addWidget(resizeAnimTitleLabel,0,0);

  resizeAnimSlider = new QSlider(0,10,10,0,QSlider::Horizontal, windowsBox);
  resizeAnimSlider->setSteps(10,1);
  rLay->addMultiCellWidget(resizeAnimSlider,0,0,1,2);

  resizeAnimNoneLabel= new QLabel(i18n("None"),windowsBox);
  resizeAnimNoneLabel->setAlignment(AlignTop|AlignLeft);
  rLay->addWidget(resizeAnimNoneLabel,1,1);

  resizeAnimFastLabel= new QLabel(i18n("Fast"),windowsBox);
  resizeAnimFastLabel->setAlignment(AlignTop|AlignRight);
  rLay->addWidget(resizeAnimFastLabel,1,2);

  wtstr = i18n("Here you can set the speed for the resize animation shown when windows are"
    " maximized or minimized. Drag the slider to the left edge to avoid a resize animation.");
  QWhatsThis::add( resizeAnimTitleLabel, wtstr );
  QWhatsThis::add( resizeAnimSlider, wtstr );
  QWhatsThis::add( resizeAnimNoneLabel, wtstr );
  QWhatsThis::add( resizeAnimFastLabel, wtstr );

#ifdef __GNUC__
#warning CT: disabling is needed as long as functionality misses in kwin
#endif
  resizeAnimTitleLabel->setEnabled(false);
  resizeAnimSlider->setEnabled(false);
  resizeAnimNoneLabel->setEnabled(false);
  resizeAnimFastLabel->setEnabled(false);

  lay->addWidget(windowsBox);

  // placement policy --- CT 19jan98, 13mar98 ---
  plcBox = new QButtonGroup(i18n("Placement policy"),this);

  QGridLayout *pLay = new QGridLayout(plcBox,3,3,
				      KDialog::marginHint(),
				      KDialog::spacingHint());
  pLay->addRowSpacing(0,fontMetrics().lineSpacing());

  placementCombo = new QComboBox(false, plcBox);
  placementCombo->insertItem(i18n("Smart"), SMART_PLACEMENT);
  placementCombo->insertItem(i18n("Cascade"), CASCADE_PLACEMENT);
  placementCombo->insertItem(i18n("Random"), RANDOM_PLACEMENT);
  // CT: disabling is needed as long as functionality misses in kwin
  //placementCombo->insertItem(i18n("Interactive"), INTERACTIVE_PLACEMENT);
  //placementCombo->insertItem(i18n("Manual"), MANUAL_PLACEMENT);
  placementCombo->setCurrentItem(SMART_PLACEMENT);
  pLay->addWidget(placementCombo,1,0);

  // FIXME, when more policies have been added to KWin
  QWhatsThis::add( placementCombo, i18n("The placement policy determines where a new window"
    " will appear on the desktop. For now, there are three different policies:"
    " <ul><li><em>Smart</em> will try to achieve a minimum overlap of windows</li>"
    " <li><em>Cascade</em> will cascade the windows</li>"
    " <li><em>Random</em> will use a random position</li></ul>") );

  connect(placementCombo, SIGNAL(activated(int)),this,
	  SLOT(ifPlacementIsInteractive()) );

  iTLabel = new QLabel(i18n("  Allowed overlap:\n"
					       "(% of desktop space)"),
		       plcBox);
  iTLabel->setAlignment(AlignTop|AlignHCenter);
  pLay->addWidget(iTLabel,1,1);

  interactiveTrigger = new QSpinBox(0, 500, 1, plcBox);
  pLay->addWidget(interactiveTrigger,1,2);

  pLay->addRowSpacing(2,KDialog::spacingHint());

  lay->addWidget(plcBox);

  // focus policy
  fcsBox = new QButtonGroup(i18n("Focus policy"),this);

  QGridLayout *fLay = new QGridLayout(fcsBox,5,3,
				      KDialog::marginHint(),
				      KDialog::spacingHint());
  fLay->addRowSpacing(0,fontMetrics().lineSpacing());
  fLay->setColStretch(0,0);
  fLay->setColStretch(1,1);
  fLay->setColStretch(2,1);


  focusCombo =  new QComboBox(false, fcsBox);
  focusCombo->insertItem(i18n("Click to focus"), CLICK_TO_FOCUS);
  focusCombo->insertItem(i18n("Focus follows mouse"), FOCUS_FOLLOWS_MOUSE);
  focusCombo->insertItem(i18n("Focus under mouse"), FOCUS_UNDER_MOUSE);
  focusCombo->insertItem(i18n("Focus strictly under mouse"), FOCUS_STRICTLY_UNDER_MOUSE);
  fLay->addMultiCellWidget(focusCombo,1,1,0,1);

  // FIXME, when more policies have been added to KWin
  QWhatsThis::add( focusCombo, i18n("The focus policy is used to determin the active window, i.e."
    " the window you can work in. <ul>"
    " <li><em>Click to focus:</em> a window becomes active when you click into it. This is the behavior"
    " you might know from other operating systems.</li>"
    " <li><em>Focus follows mouse:</em> Moving the mouse pointer actively onto a"
    " normal window activates it. Very practical if you are using the mouse a lot.</li>"
    " <li><em>FocusUnderMouse</em> - The window that happens to be under the"
    "  mouse pointer becomes active. The invariant is: no window can"
    "  have focus that is not under the mouse. </li>"
    " <li><em>FocusStrictlyUnderMouse</em> - this is even worse than"
    " FocusUnderMouse. Only the window under the mouse pointer is"
    " active. If the mouse points nowhere, nothing has the focus. "
    " </ul>"
    "  Note that FocusUnderMouse and FocusStrictlyUnderMouse are not"
    " particulary useful. They are only provided for old-fashined"
    " die-hard UNIX people ;-)"
    ) );

  connect(focusCombo, SIGNAL(activated(int)),this,
	  SLOT(setAutoRaiseEnabled()) );

  // autoraise delay

  autoRaiseOn = new QCheckBox(i18n("Auto Raise"), fcsBox);
  fLay->addWidget(autoRaiseOn,2,0);
  connect(autoRaiseOn,SIGNAL(toggled(bool)), this, SLOT(autoRaiseOnTog(bool)));

  clickRaiseOn = new QCheckBox(i18n("Click Raise"), fcsBox);
  fLay->addWidget(clickRaiseOn,4,0);

  connect(clickRaiseOn,SIGNAL(toggled(bool)), this, SLOT(clickRaiseOnTog(bool)));

  alabel = new QLabel(i18n("Delay (ms)"), fcsBox);
  alabel->setAlignment(AlignVCenter|AlignHCenter);
  fLay->addWidget(alabel,2,1,AlignLeft);

  s = new QLCDNumber (4, fcsBox);
  s->setFrameStyle( QFrame::NoFrame );
  s->setFixedHeight(30);
  s->adjustSize();
  s->setMinimumSize(s->size());
  fLay->addWidget(s,2,2);

  autoRaise = new KIntNumInput(500, fcsBox);
  autoRaise->setRange(0, 3000, 100, true);
  autoRaise->setSteps(100,100);
  fLay->addMultiCellWidget(autoRaise,3,3,1,2);
  connect( autoRaise, SIGNAL(valueChanged(int)), s, SLOT(display(int)) );

  fLay->addColSpacing(0,QMAX(autoRaiseOn->sizeHint().width(),
                             clickRaiseOn->sizeHint().width()) + 15);

  QWhatsThis::add( autoRaiseOn, i18n("If Auto Raise is enabled, a window in the background will automatically"
    " come to front when the mouse pointer has been over it for some time.") );
  wtstr = i18n("This is the delay after which the window the mouse pointer is over will automatically"
    " come to front.");
  QWhatsThis::add( autoRaise, wtstr );
  QWhatsThis::add( s, wtstr );
  QWhatsThis::add( alabel, wtstr );

  QWhatsThis::add( clickRaiseOn, i18n("Disable this option if you don't want windows to be brought to"
    " front automatically when you click somewhere into the window contents.") );

  // CT: disabling is needed as long as functionality misses in kwin
  autoRaiseOn->setEnabled(false);
  clickRaiseOn->setEnabled(false);
  alabel->setEnabled(false);
  s->setEnabled(false);
  autoRaise->setEnabled(false);

  lay->addWidget(fcsBox);

  // Any changes goes to slotChanged()
  connect(vertOnly, SIGNAL(clicked()), this, SLOT(slotChanged()));
  connect(opaque, SIGNAL(clicked()), this, SLOT(slotChanged()));
  connect(resizeOpaqueOn, SIGNAL(clicked()), this, SLOT(slotChanged()));
  connect(resizeAnimSlider, SIGNAL(valueChanged(int)), this, SLOT(slotChanged()));
  connect(placementCombo, SIGNAL(activated(int)), this, SLOT(slotChanged()));
  connect(focusCombo, SIGNAL(activated(int)), this, SLOT(slotChanged()));
  connect(fcsBox, SIGNAL(clicked(int)), this, SLOT(slotChanged()));
  connect(autoRaise, SIGNAL(valueChanged(int)), this, SLOT(slotChanged()));

  load();

}

// many widgets connect to this slot
void KWindowConfig::slotChanged()
{
  emit changed(true);
}

int KWindowConfig::getMove()
{
  if (opaque->isChecked())
    return OPAQUE;
  else
    return TRANSPARENT;
}

void KWindowConfig::setMove(int trans)
{
  if (trans == TRANSPARENT)
    opaque->setChecked(false);
  else
    opaque->setChecked(true);
}

// placement policy --- CT 31jan98 ---
int KWindowConfig::getPlacement()
{
  return placementCombo->currentItem();
}

void KWindowConfig::setPlacement(int plac)
{
  placementCombo->setCurrentItem(plac);
}

int KWindowConfig::getFocus()
{
    return focusCombo->currentItem();
}

void KWindowConfig::setFocus(int foc)
{
  focusCombo->setCurrentItem(foc);

  // this will disable/hide the auto raise delay widget if focus==click
  setAutoRaiseEnabled();
}

int KWindowConfig::getResizeAnim()
{
  return resizeAnimSlider->value();
}

void KWindowConfig::setResizeAnim(int anim)
{
  resizeAnimSlider->setValue(anim);
}

int KWindowConfig::getResizeOpaque()
{
  if (resizeOpaqueOn->isChecked())
    return RESIZE_OPAQUE;
  else
    return RESIZE_TRANSPARENT;
}

void KWindowConfig::setResizeOpaque(int opaque)
{
  if (opaque == RESIZE_OPAQUE)
    resizeOpaqueOn->setChecked(true);
  else
    resizeOpaqueOn->setChecked(false);
}

void KWindowConfig::setMaximize(int tb)
{
  if (tb == MAXIMIZE_FULL)
    vertOnly->setChecked(false);
  else
    vertOnly->setChecked(true);
}

int KWindowConfig::getMaximize()
{
  if (vertOnly->isChecked())
    return MAXIMIZE_VERT;
  else
    return MAXIMIZE_FULL;
}

void KWindowConfig::setAutoRaiseInterval(int tb)
{
    autoRaise->setValue(tb);
    s->display(tb);
}

int KWindowConfig::getAutoRaiseInterval()
{
    return s->intValue();
}

void KWindowConfig::setAutoRaise(bool on)
{
    autoRaiseOn->setChecked(on);
}

void KWindowConfig::setClickRaise(bool on)
{
    clickRaiseOn->setChecked(on);
}

void KWindowConfig::setAutoRaiseEnabled()
{
  // the auto raise related widgets are: autoRaise, alabel, s, sec
  if ( focusCombo->currentItem() != CLICK_TO_FOCUS )
    {
      autoRaiseOn->setEnabled(true);
      autoRaiseOnTog(autoRaiseOn->isChecked());
      clickRaiseOn->setEnabled(true);
      clickRaiseOnTog(clickRaiseOn->isChecked());
    }
  else
    {
      autoRaiseOn->setEnabled(false);
      autoRaiseOnTog(false);
      clickRaiseOn->setEnabled(false);
      clickRaiseOnTog(false);
    }
}

// CT 13mar98 interactiveTrigger configured by this slot
void KWindowConfig::ifPlacementIsInteractive( )
{
  if( placementCombo->currentItem() == INTERACTIVE_PLACEMENT) {
    iTLabel->setEnabled(true);
    interactiveTrigger->show();
  }
  else {
    iTLabel->setEnabled(false);
    interactiveTrigger->hide();
  }
}
//CT

//CT 23Oct1998 make AutoRaise toggling much clear
void KWindowConfig::autoRaiseOnTog(bool a) {
  autoRaise->setEnabled(a);
  s->setEnabled(a);
  alabel->setEnabled(a);
}
//CT

void KWindowConfig::clickRaiseOnTog(bool ) {
}

void KWindowConfig::load( void )
{
  QString key;

  KConfig *config = new KConfig("kwinrc", false, false);
  config->setGroup( "Windows" );

  key = config->readEntry(KWIN_MOVE, "Opaque");
  if( key == "Transparent")
    setMove(TRANSPARENT);
  else if( key == "Opaque")
    setMove(OPAQUE);

  //CT 17Jun1998 - variable animation speed from 0 (none!!) to 10 (max)
  // CT: disabling is needed as long as functionality misses in kwin
//   int anim = 1;
//   if (config->hasKey(KWIN_RESIZE_ANIM)) {
//     anim = config->readNumEntry(KWIN_RESIZE_ANIM);
//     if( anim < 1 ) anim = 0;
//     if( anim > 10 ) anim = 10;
//     setResizeAnim(anim);
//   }
//   else{
//     setResizeAnim(1);
//     config->writeEntry(KWIN_RESIZE_ANIM, 1);
//   }

  key = config->readEntry(KWIN_RESIZE_OPAQUE, "Transparent");
  if( key == "Opaque")
    setResizeOpaque(RESIZE_OPAQUE);
  else if ( key == "Transparent")
    setResizeOpaque(RESIZE_TRANSPARENT);

  // placement policy --- CT 19jan98 ---
  key = config->readEntry(KWIN_PLACEMENT);
  //CT 13mar98 interactive placement
  if( key.left(11) == "interactive") {
    setPlacement(INTERACTIVE_PLACEMENT);
    int comma_pos = key.find(',');
    if (comma_pos < 0)
      interactiveTrigger->setValue(0);
    else
      interactiveTrigger->setValue (key.right(key.length()
					      - comma_pos).toUInt(0));
    iTLabel->setEnabled(true);
    interactiveTrigger->show();
  }
  else {
    interactiveTrigger->setValue(0);
    iTLabel->setEnabled(false);
    interactiveTrigger->hide();
    if( key == "Random")
      setPlacement(RANDOM_PLACEMENT);
    else if( key == "Cascade")
      setPlacement(CASCADE_PLACEMENT); //CT 31jan98
    //CT 31mar98 manual placement
    else if( key == "manual")
      setPlacement(MANUAL_PLACEMENT);

    else
      setPlacement(SMART_PLACEMENT);
  }

  key = config->readEntry(KWIN_FOCUS);
  if( key == "ClickToFocus")
    setFocus(CLICK_TO_FOCUS);
  else if( key == "FocusFollowsMouse")
    setFocus(FOCUS_FOLLOWS_MOUSE);
  else if(key == "FocusUnderMouse")
    setFocus(FOCUS_UNDER_MOUSE);
  else if(key == "FocusStrictlyUnderMouse")
    setFocus(FOCUS_STRICTLY_UNDER_MOUSE);

  key = config->readEntry(KWIN_MAXIMIZE);
  if( key == "on")
    setMaximize(MAXIMIZE_VERT);
  else if( key == "off")
    setMaximize(MAXIMIZE_FULL);

  int k = config->readNumEntry(KWIN_AUTORAISE_INTERVAL,0);
  setAutoRaiseInterval(k);

  key = config->readEntry(KWIN_AUTORAISE);
  setAutoRaise(key == "on");
  key = config->readEntry(KWIN_CLICKRAISE);
  setClickRaise(key != "off");
  setAutoRaiseEnabled();      // this will disable/hide the auto raise delay widget if focus==click

  delete config;
}

void KWindowConfig::save( void )
{
  int v;

  KConfig *config = new KConfig("kwinrc", false, false);
  config->setGroup( "Windows" );

  v = getMove();
  if (v == TRANSPARENT)
    config->writeEntry(KWIN_MOVE,"Transparent");
  else
    config->writeEntry(KWIN_MOVE,"Opaque");


  // placement policy --- CT 31jan98 ---
  v =getPlacement();
  if (v == RANDOM_PLACEMENT)
    config->writeEntry(KWIN_PLACEMENT, "Random");
  else if (v == CASCADE_PLACEMENT)
    config->writeEntry(KWIN_PLACEMENT, "Cascade");
  //CT 13mar98 manual and interactive placement
  else if (v == MANUAL_PLACEMENT)
    config->writeEntry(KWIN_PLACEMENT, "Manual");
  else if (v == INTERACTIVE_PLACEMENT) {
      QString tmpstr = QString("Interactive,%1").arg(interactiveTrigger->value());
      config->writeEntry(KWIN_PLACEMENT, tmpstr);
  }
  else
    config->writeEntry(KWIN_PLACEMENT, "Smart");


  v = getFocus();
  if (v == CLICK_TO_FOCUS)
    config->writeEntry(KWIN_FOCUS,"ClickToFocus");
  else if (v == FOCUS_UNDER_MOUSE)
    config->writeEntry(KWIN_FOCUS,"FocusUnderMouse");
  else if (v == FOCUS_STRICTLY_UNDER_MOUSE)
    config->writeEntry(KWIN_FOCUS,"FocusStrictlyUnderMouse");
  else
    config->writeEntry(KWIN_FOCUS,"FocusFollowsMouse");

  //CT - 17Jun1998
  //  config->writeEntry(KWIN_RESIZE_ANIM, getResizeAnim());


  v = getResizeOpaque();
  if (v == RESIZE_OPAQUE)
    config->writeEntry(KWIN_RESIZE_OPAQUE, "Opaque");
  else
    config->writeEntry(KWIN_RESIZE_OPAQUE, "Transparent");

//   v = getMaximize();
//   if (v == MAXIMIZE_VERT)
//     config->writeEntry(KWIN_MAXIMIZE, "on");
//   else
//     config->writeEntry(KWIN_MAXIMIZE, "off");

   v = getAutoRaiseInterval();
   if (v <0) v = 0;
   config->writeEntry(KWIN_AUTORAISE_INTERVAL,v);

   if (autoRaiseOn->isChecked())
     config->writeEntry(KWIN_AUTORAISE, "on");
   else
     config->writeEntry(KWIN_AUTORAISE, "off");

   if (clickRaiseOn->isChecked())
     config->writeEntry(KWIN_CLICKRAISE, "on");
   else
     config->writeEntry(KWIN_CLICKRAISE, "off");

  config->sync();

  delete config;

  if ( !kapp->dcopClient()->isAttached() )
      kapp->dcopClient()->attach();
  kapp->dcopClient()->send("kwin", "", "reconfigure()", "");
}

void KWindowConfig::defaults()
{
    setMove(OPAQUE);
    setResizeOpaque(RESIZE_TRANSPARENT);
    setPlacement(SMART_PLACEMENT);
    setFocus(CLICK_TO_FOCUS);
}

QString KWindowConfig::quickHelp()
{
    return i18n("<h1>Window Behavior</h1> Here you can modify the way windows behave when being"
      " moved or resized and KWin's policies regarding window placement and window focus.<p>"
      " Please note that changes in this module will only take effect if you use KWin as your"
      " window manager. If you do use a different window manager, please consult its documentation"
      " on how to change these options.");
}

void KWindowConfig::loadSettings()
{
  load();
}

void KWindowConfig::applySettings()
{
  save();
}

#include "windows.moc"
