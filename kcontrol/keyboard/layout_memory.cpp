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

#include "layout_memory.h"

#include <QtGui/QX11Info>

#include <kdebug.h>
#include <kwindowsystem.h>
//#include <netwm.h>

#include "x11_helper.h"


LayoutMemory::LayoutMemory():
	switchingPolicy(KeyboardConfig::SWITCH_POLICY_GLOBAL)
{
	registerListeners();
}

LayoutMemory::~LayoutMemory()
{
	unregisterListeners();
}

void LayoutMemory::setSwitchingPolicy(KeyboardConfig::SwitchingPolicy switchingPolicy)
{
	this->layoutMap.clear();
	this->switchingPolicy = switchingPolicy;
	unregisterListeners();
	registerListeners();
}

void LayoutMemory::registerListeners()
{
	if( switchingPolicy ==  KeyboardConfig::SWITCH_POLICY_WINDOW
			|| switchingPolicy ==  KeyboardConfig::SWITCH_POLICY_APPLICATION ) {
		connect(KWindowSystem::self(), SIGNAL(activeWindowChanged(WId)), this, SLOT(windowChanged(WId)));
//		connect(KWindowSystem::self(), SIGNAL(windowRemoved(WId)), this, SLOT(windowRemoved(WId)));
	}
	if( switchingPolicy ==  KeyboardConfig::SWITCH_POLICY_DESKTOP ) {
		connect(KWindowSystem::self(), SIGNAL(currentDesktopChanged(int)), this, SLOT(desktopChanged(int)));
	}
}

void LayoutMemory::unregisterListeners()
{
    disconnect(KWindowSystem::self(), SIGNAL(activeWindowChanged(WId)), this, SLOT(windowChanged(WId)));
    disconnect(KWindowSystem::self(), SIGNAL(currentDesktopChanged(int)), this, SLOT(desktopChanged(int)));
//	disconnect(KWindowSystem::self(), SIGNAL(windowRemoved(WId)), this, SLOT(windowRemoved(WId)));
}

QString LayoutMemory::getCurrentMapKey() {
	switch(switchingPolicy) {
	case KeyboardConfig::SWITCH_POLICY_WINDOW: {
		WId wid = KWindowSystem::self()->activeWindow();
		KWindowInfo winInfo = KWindowSystem::windowInfo(wid, NET::WMWindowType);
		NET::WindowType windowType = winInfo.windowType( NET::NormalMask | NET::DesktopMask | NET::DialogMask );
		kDebug() << "window type" << windowType;

		// we ignore desktop type so that our keybaord layout applet on desktop could change layout properly
		if( windowType == NET::Desktop )
			return previousLayoutMapKey;
		if( windowType != NET::Unknown && windowType != NET::Normal && windowType != NET::Dialog )
			return QString();

		return QString::number(wid);
	}
	case KeyboardConfig::SWITCH_POLICY_APPLICATION: {
		WId wid = KWindowSystem::self()->activeWindow();
		KWindowInfo winInfo = KWindowSystem::windowInfo(wid, NET::WMWindowType, NET::WM2WindowClass);
		NET::WindowType windowType = winInfo.windowType( NET::NormalMask | NET::DesktopMask | NET::DialogMask );
		kDebug() << "window type" << windowType;

		// we ignore desktop type so that our keybaord layout applet on desktop could change layout properly
		if( windowType == NET::Desktop )
			return previousLayoutMapKey;
		if( windowType != NET::Unknown && windowType != NET::Normal && windowType != NET::Dialog )
			return QString();

		// shall we use pid or window class ??? - class seems better (see e.g. https://bugs.kde.org/show_bug.cgi?id=245507)
		// for window class shall we use class.class or class.name? (seem class.class is a bit better - more app-oriented)
//		kDebug() << "window class.class: " << winInfo.windowClassClass();
		return QString(winInfo.windowClassClass());
//		NETWinInfo winInfoForPid( QX11Info::display(), wid, QX11Info::appRootWindow(), NET::WMPid);
//		return QString::number(winInfoForPid.pid());
	}
	case KeyboardConfig::SWITCH_POLICY_DESKTOP:
		return QString::number(KWindowSystem::self()->currentDesktop());
	default:
		return QString();
	}
}

void LayoutMemory::clear()
{
	layoutMap.clear();
}

void LayoutMemory::layoutChanged()
{
	QString layoutMapKey = getCurrentMapKey();
	if( layoutMapKey.isEmpty() )
		return;

	layoutMap[ layoutMapKey ] = X11Helper::getCurrentLayout();
}

void LayoutMemory::setCurrentLayoutFromMap()
{
	QString layoutMapKey = getCurrentMapKey();
	if( layoutMapKey.isEmpty() )
		return;

	LayoutUnit layoutFromMap = layoutMap[layoutMapKey];
	kDebug() << "layout map item" << layoutFromMap.toString() << "for container key" << layoutMapKey;
	if( layoutFromMap.isEmpty() ) {
		if( ! X11Helper::isDefaultLayout() ) {
//			kDebug() << "setting default layout for container key" << layoutMapKey;
			X11Helper::setDefaultLayout();
		}
	}
	else if( ! (layoutFromMap == X11Helper::getCurrentLayout()) ) {
//		kDebug() << "setting layout" <<  layoutFromMap << "for container key" << layoutMapKey;
		X11Helper::setLayout( layoutFromMap );
	}
	previousLayoutMapKey = layoutMapKey;
}

void LayoutMemory::windowChanged(WId /*wId*/)
{
	setCurrentLayoutFromMap();
}

void LayoutMemory::desktopChanged(int /*desktop*/)
{
	setCurrentLayoutFromMap();
}

//TODO: window removed
//void LayoutMemory::windowRemoved(WId wId)
//{
//	setCurrentLayoutFromMap();
//}
