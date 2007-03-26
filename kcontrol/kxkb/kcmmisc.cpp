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


#include <config.h>
#include <config-X11.h>
#include <math.h>

#include <QSlider>
#include <QFileInfo>
#include <QCheckBox>
#include <QString>
#include <QLayout>
#include <QWhatsThis>
#include <Q3ButtonGroup>
#include <QRadioButton>

#include <klocale.h>
#include <kconfig.h>
#include <knuminput.h>
#include <kapplication.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <k3process.h>
#include <kdialog.h>
#include <kgenericfactory.h>

#include "kcmmisc.h"
#include "ui_kcmmiscwidget.h"
#include <X11/Xlib.h>

typedef KGenericFactory<KeyboardConfig> KeyboardConfigFactory;
K_EXPORT_COMPONENT_FACTORY(keyboard, KeyboardConfigFactory("kcmmisc"))

KeyboardConfig::KeyboardConfig(QWidget *parent, const QStringList &)
	: KCModule(KeyboardConfigFactory::componentData(), parent)
{
  QString wtstr;
//   QBoxLayout* lay = new QVBoxLayout(this, 0, KDialog::spacingHint());
  ui = new Ui_KeyboardConfigWidget();
  ui->setupUi(this);
//   lay->addWidget(ui);
//   lay->addStretch();

  ui->click->setRange(0, 100, 10);
  ui->delay->setRange(100, 5000, 50, false);
  ui->rate->setRange(0.2, 50, 5, false);

  sliderMax = (int)floor (0.5 + 2*(log(5000)-log(100)) / (log(5000)-log(4999)));
  ui->delaySlider->setRange(0, sliderMax);
  ui->delaySlider->setSingleStep(sliderMax/100);
  ui->delaySlider->setPageStep(sliderMax/10);
  ui->delaySlider->setTickInterval(sliderMax/10);

  ui->rateSlider->setRange(20, 5000);
  ui->rateSlider->setSingleStep(30);
  ui->rateSlider->setPageStep(500);
  ui->rateSlider->setTickInterval(498);

  connect(ui->repeatBox, SIGNAL(clicked()), this, SLOT(changed()));
  connect(ui->delay, SIGNAL(valueChanged(int)), this, SLOT(delaySpinboxChanged(int)));
  connect(ui->delaySlider, SIGNAL(valueChanged(int)), this, SLOT(delaySliderChanged(int)));
  connect(ui->rate, SIGNAL(valueChanged(double)), this, SLOT(rateSpinboxChanged(double)));
  connect(ui->rateSlider, SIGNAL(valueChanged(int)), this, SLOT(rateSliderChanged(int)));

  connect(ui->click, SIGNAL(valueChanged(int)), this, SLOT(changed()));
  connect(ui->numlockGroup, SIGNAL(released(int)), this, SLOT(changed()));

#if !defined(HAVE_XTEST) && !defined(HAVE_XKB)
  ui->numlockGroup->setDisabled( true );
#endif
#if !defined(HAVE_XKB) && !defined(HAVE_XF86MISC)
//  delay->setDisabled( true );
//  rate->setDisabled( true );
#endif
//  lay->addStretch();
  load();
}

int  KeyboardConfig::getClick()
{
    return ui->click->value();
}

// set the slider and LCD values
void KeyboardConfig::setRepeat(int r, int delay_, double rate_)
{
    ui->repeatBox->setChecked(r == AutoRepeatModeOn);
    ui->delay->setValue(delay_);
    ui->rate->setValue(rate_);
}

void KeyboardConfig::setClick(int v)
{
    ui->click->setValue(v);
}

int KeyboardConfig::getNumLockState()
{
    QAbstractButton* selected = ui->numlockGroup->selected();
    if( selected == NULL )
        return 2;
    int ret = ui->numlockGroup->id( selected );
    if( ret == -1 )
        ret = 2;
    return ret;
}

void KeyboardConfig::setNumLockState( int s )
{
    ui->numlockGroup->setButton( s );
}

void KeyboardConfig::load()
{
  KConfigGroup config(KSharedConfig::openConfig("kcminputrc", KConfig::NoGlobals), "Keyboard");

  XKeyboardState kbd;

  XGetKeyboardControl(QX11Info::display(), &kbd);

  bool key = config.readEntry("KeyboardRepeating", true);
  keyboardRepeat = (key ? AutoRepeatModeOn : AutoRepeatModeOff);
  ui->delay->setValue(config.readEntry( "RepeatDelay", 660 ));
  ui->rate->setValue(config.readEntry( "RepeatRate", 25. ));
  clickVolume = config.readEntry("ClickVolume", kbd.key_click_percent);
  numlockState = config.readEntry( "NumLock", 2 );

  setClick(kbd.key_click_percent);
  setRepeat(kbd.global_auto_repeat, ui->delay->value(), ui->rate->value());
  setNumLockState( numlockState );
}

void KeyboardConfig::save()
{
  KConfigGroup config(KSharedConfig::openConfig("kcminputrc", KConfig::NoGlobals), "Keyboard");

  XKeyboardControl kbd;

  clickVolume = getClick();
  keyboardRepeat = ui->repeatBox->isChecked() ? AutoRepeatModeOn : AutoRepeatModeOff;
  numlockState = getNumLockState();

  kbd.key_click_percent = clickVolume;
  kbd.auto_repeat_mode = keyboardRepeat;
  XChangeKeyboardControl(QX11Info::display(),
                           KBKeyClickPercent | KBAutoRepeatMode,
                           &kbd);
  if( keyboardRepeat ) {
    set_repeatrate(ui->delay->value(), ui->rate->value());
  }

  config.writeEntry("ClickVolume",clickVolume);
  config.writeEntry("KeyboardRepeating", (keyboardRepeat == AutoRepeatModeOn));
  config.writeEntry("RepeatRate", ui->rate->value() );
  config.writeEntry("RepeatDelay", ui->delay->value() );
  config.writeEntry("NumLock", numlockState );
  config.sync();
}

void KeyboardConfig::defaults()
{
    setClick(50);
    setRepeat(true, 660, 25);
    setNumLockState( 2 );
}

QString KeyboardConfig::quickHelp() const
{
  return QString::null;

  /* "<h1>Keyboard</h1> This module allows you to choose options"
     " for the way in which your keyboard works. The actual effect of"
     " setting these options depends upon the features provided by your"
     " keyboard hardware and the X server on which KDE is running.<p>"
     " For example, you may find that changing the key click volume"
     " has no effect because this feature is not available on your system." */
}

void KeyboardConfig::delaySliderChanged (int value) {
	double alpha  = sliderMax / (log(5000) - log(100));
	double linearValue = exp (value/alpha + log(100));

	ui->delay->setValue((int)floor(0.5 + linearValue));

	emit KCModule::changed(true);
}

void KeyboardConfig::delaySpinboxChanged (int value) {
	double alpha  = sliderMax / (log(5000) - log(100));
	double logVal = alpha * (log(value)-log(100));

	ui->delaySlider->setValue ((int)floor (0.5 + logVal));

	emit KCModule::changed(true);
}

void KeyboardConfig::rateSliderChanged (int value) {
	ui->rate->setValue(value/100.0);

	emit KCModule::changed(true);
}

void KeyboardConfig::rateSpinboxChanged (double value) {
	ui->rateSlider->setValue ((int)(value*100));

	emit KCModule::changed(true);
}

void KeyboardConfig::changed()
{
  emit KCModule::changed(true);
}

/*
 Originally comes from NumLockX http://dforce.sh.cvut.cz/~seli/en/numlockx

 NumLockX

 Copyright (C) 2000-2001 Lubos Lunak        <l.lunak@kde.org>
 Copyright (C) 2001      Oswald Buddenhagen <ossi@kde.org>

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.

****************************************************************************/

#include <X11/Xlib.h>

#ifdef HAVE_XTEST
#include <X11/extensions/XTest.h>
#endif

#ifdef HAVE_XKB
#define explicit myexplicit
#include <X11/XKBlib.h>
#undef explicit
#endif

#include <X11/keysym.h>

#if defined(HAVE_XTEST) || defined(HAVE_XKB)

/* the XKB stuff is based on code created by Oswald Buddenhagen <ossi@kde.org> */
#ifdef HAVE_XKB
int xkb_init()
    {
    int xkb_opcode, xkb_event, xkb_error;
    int xkb_lmaj = XkbMajorVersion;
    int xkb_lmin = XkbMinorVersion;
    return XkbLibraryVersion( &xkb_lmaj, &xkb_lmin )
			&& XkbQueryExtension( QX11Info::display(), &xkb_opcode, &xkb_event, &xkb_error,
			       &xkb_lmaj, &xkb_lmin );
    }

unsigned int xkb_mask_modifier( XkbDescPtr xkb, const char *name )
    {
    int i;
    if( !xkb || !xkb->names )
	return 0;
    for( i = 0;
         i < XkbNumVirtualMods;
	 i++ )
	{
	char* modStr = XGetAtomName( xkb->dpy, xkb->names->vmods[i] );
	if( modStr != NULL && strcmp(name, modStr) == 0 )
	    {
	    unsigned int mask;
	    XkbVirtualModsToReal( xkb, 1 << i, &mask );
	    return mask;
	    }
	}
    return 0;
    }

unsigned int xkb_numlock_mask()
    {
    XkbDescPtr xkb;
    if(( xkb = XkbGetKeyboard( QX11Info::display(), XkbAllComponentsMask, XkbUseCoreKbd )) != NULL )
	{
        unsigned int mask = xkb_mask_modifier( xkb, "NumLock" );
        XkbFreeKeyboard( xkb, 0, True );
        return mask;
        }
    return 0;
    }

int xkb_set_on()
    {
    unsigned int mask;
    if( !xkb_init())
        return 0;
    mask = xkb_numlock_mask();
    if( mask == 0 )
        return 0;
    XkbLockModifiers ( QX11Info::display(), XkbUseCoreKbd, mask, mask);
    return 1;
    }

int xkb_set_off()
    {
    unsigned int mask;
    if( !xkb_init())
        return 0;
    mask = xkb_numlock_mask();
    if( mask == 0 )
        return 0;
    XkbLockModifiers ( QX11Info::display(), XkbUseCoreKbd, mask, 0);
    return 1;
    }
#endif

#ifdef HAVE_XTEST
int xtest_get_numlock_state()
    {
    int i;
    int numlock_mask = 0;
    Window dummy1, dummy2;
    int dummy3, dummy4, dummy5, dummy6;
    unsigned int mask;
    KeyCode numlock_keycode = XKeysymToKeycode( QX11Info::display(), XK_Num_Lock );
    if( numlock_keycode == NoSymbol )
        return 0;
    XModifierKeymap* map = XGetModifierMapping( QX11Info::display() );
    for( i = 0;
         i < 8;
         ++i )
        {
	if( map->modifiermap[ map->max_keypermod * i ] == numlock_keycode )
		numlock_mask = 1 << i;
	}
    XQueryPointer( QX11Info::display(), DefaultRootWindow( QX11Info::display() ), &dummy1, &dummy2,
        &dummy3, &dummy4, &dummy5, &dummy6, &mask );
    XFreeModifiermap( map );
    return mask & numlock_mask;
    }

void xtest_change_numlock()
    {
    XTestFakeKeyEvent( QX11Info::display(), XKeysymToKeycode( QX11Info::display(), XK_Num_Lock ), True, CurrentTime );
    XTestFakeKeyEvent( QX11Info::display(), XKeysymToKeycode( QX11Info::display(), XK_Num_Lock ), False, CurrentTime );
    }

void xtest_set_on()
    {
    if( !xtest_get_numlock_state())
        xtest_change_numlock();
    }

void xtest_set_off()
    {
    if( xtest_get_numlock_state())
        xtest_change_numlock();
    }
#endif

void numlock_set_on()
    {
#ifdef HAVE_XKB
    if( xkb_set_on())
        return;
#endif
#ifdef HAVE_XTEST
    xtest_set_on();
#endif
    }

void numlock_set_off()
    {
#ifdef HAVE_XKB
    if( xkb_set_off())
        return;
#endif
#ifdef HAVE_XTEST
    xtest_set_off();
#endif
    }

void numlockx_change_numlock_state( bool set_P )
    {
    if( set_P )
        numlock_set_on();
    else
        numlock_set_off();
    }
#else
void numlockx_change_numlock_state( bool ) {} // dummy
#endif // defined(HAVE_XTEST) || defined(HAVE_XKB)


// This code is taken from xset utility from XFree 4.3 (http://www.xfree86.org/)


#if 0
//HAVE_XF86MISC
#include <X11/extensions/xf86misc.h>
void set_repeatrate(int delay, double rate)
{
  Display* dpy = QX11Info::display();
  XF86MiscKbdSettings values;

  XF86MiscGetKbdSettings(dpy, &values);
  values.delay = delay;
  values.rate = rate;
  XF86MiscSetKbdSettings(dpy, &values);
  return;
}
#else
void set_repeatrate(int delay, double rate)
{
#if HAVE_XKB
  Display* dpy = QX11Info::display();
  int xkbmajor = XkbMajorVersion, xkbminor = XkbMinorVersion;
  int xkbopcode, xkbevent, xkberror;

  if (XkbQueryExtension(dpy, &xkbopcode, &xkbevent, &xkberror, &xkbmajor,
				&xkbminor)) {
     XkbDescPtr xkb = XkbAllocKeyboard();
     if (xkb) {
        int res = XkbGetControls(dpy, XkbRepeatKeysMask, xkb);
        xkb->ctrls->repeat_delay = delay;
        xkb->ctrls->repeat_interval = (int)floor(1000/rate + 0.5);
        res = XkbSetControls(dpy, XkbRepeatKeysMask, xkb);
        return;
     }
  }
#endif
  // Fallback: use the xset utility.

  // Unfortunately xset does only support int parameters, so
  // really slow repeat rates cannot be supported this way.
  // (the FSG Accessibility standard requires support for repeat rates
  // of several seconds per character)
  int r;
  if (rate < 1)
     r = 1;
  else
     r = (int)floor(rate + 0.5);

  QString exe = KGlobal::dirs()->findExe("xset");
  if (exe.isEmpty())
    return;

  K3Process p;
  p << exe << "r" << "rate" << QString::number(delay) << QString::number(r);

  p.start(K3Process::Block);
}
#endif

void KeyboardConfig::init_keyboard()
{
        KConfigGroup config(KSharedConfig::openConfig( "kcminputrc" ), "Keyboard");

	XKeyboardState   kbd;
	XKeyboardControl kbdc;

	XGetKeyboardControl(QX11Info::display(), &kbd);
	bool key = config.readEntry("KeyboardRepeating", true);
	kbdc.key_click_percent = config.readEntry("ClickVolume", kbd.key_click_percent);
	kbdc.auto_repeat_mode = (key ? AutoRepeatModeOn : AutoRepeatModeOff);

	XChangeKeyboardControl(QX11Info::display(),
						   KBKeyClickPercent | KBAutoRepeatMode,
						   &kbdc);

	if( key ) {
		int delay_ = config.readEntry("RepeatDelay", 250);
		double rate_ = config.readEntry("RepeatRate", 30.);
		set_repeatrate(delay_, rate_);
	}


	int numlockState = config.readEntry( "NumLock", 2 );
	if( numlockState != 2 )
		numlockx_change_numlock_state( numlockState == 0 );
}

#include "kcmmisc.moc"

