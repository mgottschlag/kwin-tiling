/*
 *  Copyright (C) 2010 Andriy Rysin (rysin@kde.org)
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

#include <kdebug.h>
#include <kconfiggroup.h>
#include <ksharedconfig.h>

#include <QtGui/QX11Info>
#include <QtGui/QCursor>	// WTF? - otherwise compiler complains

#include <X11/Xlib.h>

#include <math.h>

#include "x11_helper.h"

// from numlockx.c
extern "C" void numlockx_change_numlock_state(Display* dpy, int state);

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

#include <X11/XKBlib.h>
#include <X11/keysym.h>


// This code is taken from xset utility from XFree 4.3 (http://www.xfree86.org/)

static
void set_repeatrate(int delay, double rate)
{
	if( !X11Helper::xkbSupported(NULL) ) {
		kError() << "Failed to set keyboard repeat rate: xkb is not supported";
		return;
	}

	XkbDescPtr xkb = XkbAllocKeyboard();
	if (xkb) {
		Display* dpy = QX11Info::display();
		int res = XkbGetControls(dpy, XkbRepeatKeysMask, xkb);
		xkb->ctrls->repeat_delay = delay;
		xkb->ctrls->repeat_interval = (int)floor(1000/rate + 0.5);
		res = XkbSetControls(dpy, XkbRepeatKeysMask, xkb);
		return;
	}
}

static
void set_volume(int click_percent, bool auto_repeat_mode)
{
	XKeyboardState   kbd;
	XKeyboardControl kbdc;

	XGetKeyboardControl(QX11Info::display(), &kbd);

	if( click_percent != -1 ) {
		kbdc.key_click_percent = click_percent;
	}
	kbdc.auto_repeat_mode = (auto_repeat_mode ? AutoRepeatModeOn : AutoRepeatModeOff);

	XChangeKeyboardControl(QX11Info::display(),
						   KBKeyClickPercent | KBAutoRepeatMode,
						   &kbdc);
}

void init_keyboard_hardware()
{
    KConfigGroup config(KSharedConfig::openConfig( "kcminputrc" ), "Keyboard");

	bool key_repeat = config.readEntry("KeyboardRepeating", true);
	int click_percent = config.readEntry("ClickVolume", -1);

	set_volume(click_percent, key_repeat);

	if( key_repeat ) {
		int delay_ = config.readEntry("RepeatDelay", 250);
		double rate_ = config.readEntry("RepeatRate", 30.);
		set_repeatrate(delay_, rate_);
	}


	int numlockState = config.readEntry( "NumLock", 2 );
	if( numlockState != 2 ) {
		numlockx_change_numlock_state(QX11Info::display(), numlockState == 0 );
	}
}
