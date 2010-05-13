/*
 * kcmmisc.cpp
 *
 * Copyright (c) 1997 Patrick Dowler dowler@morgul.fsh.uvic.ca
 *
 * Layout management, cleanups:
 * Copyright (c) 1999 Dirk A. Mueller <dmuell@gmx.net>
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "kcmmisc.h"
#include "ui_kcmmiscwidget.h"

#include <math.h>

#include <QtGui/QCheckBox>
#include <QtGui/QWhatsThis>
#include <QtGui/QX11Info>

#include <klocale.h>
#include <kconfig.h>
#include <knuminput.h>

#include <X11/Xlib.h>


KCMiscKeyboardWidget::KCMiscKeyboardWidget(QWidget *parent)
	: QWidget(parent),
	  ui(*new Ui_KeyboardConfigWidget)
{
  ui.setupUi(this);

  ui.delay->setRange(100, 5000, 50);
  ui.delay->setSliderEnabled(false);
  ui.rate->setRange(0.2, 50, 5, false);

  sliderMax = (int)floor (0.5 + 2*(log(5000.0L)-log(100.0L)) / (log(5000.0L)-log(4999.0L)));
  ui.delaySlider->setRange(0, sliderMax);
  ui.delaySlider->setSingleStep(sliderMax/100);
  ui.delaySlider->setPageStep(sliderMax/10);
  ui.delaySlider->setTickInterval(sliderMax/10);

  ui.rateSlider->setRange(20, 5000);
  ui.rateSlider->setSingleStep(30);
  ui.rateSlider->setPageStep(500);
  ui.rateSlider->setTickInterval(498);

  connect(ui.repeatBox, SIGNAL(clicked()), this, SLOT(changed()));
  connect(ui.delay, SIGNAL(valueChanged(int)), this, SLOT(delaySpinboxChanged(int)));
  connect(ui.delaySlider, SIGNAL(valueChanged(int)), this, SLOT(delaySliderChanged(int)));
  connect(ui.rate, SIGNAL(valueChanged(double)), this, SLOT(rateSpinboxChanged(double)));
  connect(ui.rateSlider, SIGNAL(valueChanged(int)), this, SLOT(rateSliderChanged(int)));

  connect(ui.click, SIGNAL(valueChanged(int)), this, SLOT(changed()));
  connect(ui.numlockGroup, SIGNAL(released(int)), this, SLOT(changed()));

#if !defined(HAVE_XTEST) && !defined(HAVE_XKB)
  ui.numlockGroup->setDisabled( true );
#endif
#if !defined(HAVE_XKB) && !defined(HAVE_XF86MISC)
//  delay->setDisabled( true );
//  rate->setDisabled( true );
#endif
}

KCMiscKeyboardWidget::~KCMiscKeyboardWidget()
{
	delete &ui;
}

int  KCMiscKeyboardWidget::getClick()
{
    return ui.click->value();
}

// set the slider and LCD values
void KCMiscKeyboardWidget::setRepeat(int r, int delay_, double rate_)
{
    ui.repeatBox->setChecked(r == AutoRepeatModeOn);
    ui.delay->setValue(delay_);
    ui.rate->setValue(rate_);
    delaySpinboxChanged(delay_);
    rateSpinboxChanged(rate_);
}

void KCMiscKeyboardWidget::setClick(int v)
{
    ui.click->setValue(v);
}

int KCMiscKeyboardWidget::getNumLockState()
{
    int selected = ui.numlockGroup->selected();
    if( selected < 0 )
        return 2;
    return selected;
}

void KCMiscKeyboardWidget::setNumLockState( int s )
{
    ui.numlockGroup->setSelected( s );
}

void KCMiscKeyboardWidget::load()
{
  KConfigGroup config(KSharedConfig::openConfig("kcminputrc", KConfig::NoGlobals), "Keyboard");

  XKeyboardState kbd;

  XGetKeyboardControl(QX11Info::display(), &kbd);

  ui.delay->blockSignals(true);
  ui.rate->blockSignals(true);
  ui.click->blockSignals(true);

  bool key = config.readEntry("KeyboardRepeating", true);
  keyboardRepeat = (key ? AutoRepeatModeOn : AutoRepeatModeOff);
  float delay = config.readEntry( "RepeatDelay", 660 );
  float rate = config.readEntry( "RepeatRate", 25. );
  setRepeat(keyboardRepeat, delay, rate);

  clickVolume = config.readEntry("ClickVolume", kbd.key_click_percent);
  numlockState = config.readEntry( "NumLock", 2 );

  setClick(kbd.key_click_percent);
  setRepeat(kbd.global_auto_repeat, ui.delay->value(), ui.rate->value());
  setNumLockState( numlockState );

  ui.delay->blockSignals(false);
  ui.rate->blockSignals(false);
  ui.click->blockSignals(false);
}

void KCMiscKeyboardWidget::save()
{
  KConfigGroup config(KSharedConfig::openConfig("kcminputrc", KConfig::NoGlobals), "Keyboard");

  XKeyboardControl kbd;

  clickVolume = getClick();
  keyboardRepeat = ui.repeatBox->isChecked() ? AutoRepeatModeOn : AutoRepeatModeOff;
  numlockState = getNumLockState();

  kbd.key_click_percent = clickVolume;
  kbd.auto_repeat_mode = keyboardRepeat;
  XChangeKeyboardControl(QX11Info::display(),
                           KBKeyClickPercent | KBAutoRepeatMode,
                           &kbd);
  if( keyboardRepeat ) {
    set_repeatrate(ui.delay->value(), ui.rate->value());
  }

  config.writeEntry("ClickVolume",clickVolume);
  config.writeEntry("KeyboardRepeating", (keyboardRepeat == AutoRepeatModeOn));
  config.writeEntry("RepeatRate", ui.rate->value() );
  config.writeEntry("RepeatDelay", ui.delay->value() );
  config.writeEntry("NumLock", numlockState );
  config.sync();
}

void KCMiscKeyboardWidget::defaults()
{
    setClick(50);
    setRepeat(true, 660, 25);
    setNumLockState( 2 );
    emit changed(true);
}

QString KCMiscKeyboardWidget::quickHelp() const
{
  return QString();

  /* "<h1>Keyboard</h1> This module allows you to choose options"
     " for the way in which your keyboard works. The actual effect of"
     " setting these options depends upon the features provided by your"
     " keyboard hardware and the X server on which KDE is running.<p>"
     " For example, you may find that changing the key click volume"
     " has no effect because this feature is not available on your system." */
}

void KCMiscKeyboardWidget::delaySliderChanged (int value) {
	double alpha  = sliderMax / (log(5000.0L) - log(100.0L));
	double linearValue = exp (value/alpha + log(100.0L));

	ui.delay->setValue((int)floor(0.5 + linearValue));

	emit changed(true);
}

void KCMiscKeyboardWidget::delaySpinboxChanged (int value) {
	double alpha  = sliderMax / (log(5000.0L) - log(100.0L));
	double logVal = alpha * (log((double)value)-log(100.0L));

	ui.delaySlider->setValue ((int)floor (0.5 + logVal));

	emit changed(true);
}

void KCMiscKeyboardWidget::rateSliderChanged (int value) {
	ui.rate->setValue(value/100.0);

	emit changed(true);
}

void KCMiscKeyboardWidget::rateSpinboxChanged (double value) {
	ui.rateSlider->setValue ((int)(value*100));

	emit changed(true);
}

void KCMiscKeyboardWidget::changed()
{
  emit changed(true);
}
