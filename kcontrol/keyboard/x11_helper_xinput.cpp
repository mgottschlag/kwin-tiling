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

#include "x11_helper.h"

#include <QtCore/QDebug>
#include <QtGui/QX11Info>
#include <kapplication.h>

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/XKBlib.h>
#include <X11/extensions/XKBrules.h>

#if HAVE_XINPUT==1
#include <X11/extensions/XInput.h>
#endif



#if HAVE_XINPUT==1

extern "C" {
    extern int _XiGetDevicePresenceNotifyEvent(Display *);
}

bool XEventNotifier::isNewDeviceEvent(XEvent* event)
{
	if( xinputEventType != -1 && event->type == xinputEventType ) {
		XDevicePresenceNotifyEvent *xdpne = (XDevicePresenceNotifyEvent*) event;
		if( xdpne->devchange == DeviceEnabled ) {
			bool keyboard_device = false;
			int ndevices;
			XDeviceInfo	*devices = XListInputDevices(xdpne->display, &ndevices);
			if( devices != NULL ) {
				qDebug() << "New device id:" << xdpne->deviceid;
				for(int i=0; i<ndevices; i++) {
					qDebug() << "id:" << devices[i].id << "name:" << devices[i].name << "used as:" << devices[i].use;
					if( devices[i].id == xdpne->deviceid
							&& (devices[i].use == IsXKeyboard || devices[i].use == IsXExtensionKeyboard) ) {
						keyboard_device = true;
						break;
					}
				}
				XFreeDeviceList(devices);
			}
			return keyboard_device;
		}
	}
	return false;
}

int XEventNotifier::registerForNewDeviceEvent(Display* display)
{
	int xitype;
	XEventClass xiclass;

	DevicePresence(display, xitype, xiclass);
	XSelectExtensionEvent(display, DefaultRootWindow(display), &xiclass, 1);
	qDebug() << "Registered for new device events from XInput, class" << xitype;
	xinputEventType = xitype;
	return xitype;
}

#else

#warning "Keyboard kded daemon is compiled without XInput, xkb configuration will be reset when new keyboard device is plugged in!"

int XEventNotifier::registerForNewDeviceEvent(Display* display)
{
	qWarning() << "Keyboard kded daemon is compiled without XInput, xkb configuration will be reset when new keyboard device is plugged in!";
	return -1;
}

bool XEventNotifier::isNewDeviceEvent(XEvent* event)
{
	return false;
}

#endif
