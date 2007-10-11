/*
 *  Copyright (C) 2006 Andriy Rysin (rysin@kde.org)
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


#include <QX11Info>
#include <kwindowsystem.h>

#include "layoutmap.h"

#include "x11helper.h"


LayoutMap::LayoutMap(const KxkbConfig& kxkbConfig_):
	m_kxkbConfig(kxkbConfig_),
	m_currentWinId( X11Helper::UNKNOWN_WINDOW_ID )
{
}

// private
void LayoutMap::clearMaps()
{
	m_appLayouts.clear();
	m_winLayouts.clear();
	m_globalLayouts.clear();
	//setCurrentWindow( -1 );
}

void LayoutMap::reset()
{
    clearMaps();
    ownerChanged();
    m_currentWinId = X11Helper::UNKNOWN_WINDOW_ID;
}


void LayoutMap::ownerChanged()
{
    if( m_kxkbConfig.m_switchingPolicy == SWITCH_POLICY_DESKTOP ) {
	m_currentDesktop = KWindowSystem::currentDesktop();
    }
    else {
	m_currentWinId = KWindowSystem::activeWindow();
	if( m_kxkbConfig.m_switchingPolicy == SWITCH_POLICY_WIN_CLASS )
	    m_currentWinClass = X11Helper::getWindowClass(m_currentWinId, QX11Info::display());
    }
}

// private
QQueue<int>& LayoutMap::getCurrentLayoutQueueInternal()
{
	if( m_currentWinId == X11Helper::UNKNOWN_WINDOW_ID )
		return m_globalLayouts;
	
	switch( m_kxkbConfig.m_switchingPolicy ) {
		case SWITCH_POLICY_WIN_CLASS:
			return m_appLayouts[ m_currentWinClass ];
		case SWITCH_POLICY_WINDOW:
			return m_winLayouts[ m_currentWinId ];
		case SWITCH_POLICY_DESKTOP:
			return m_winLayouts[ m_currentDesktop ];
		default:
			return m_globalLayouts;
	}
}

//private
QString LayoutMap::getOwner()
{
	switch( m_kxkbConfig.m_switchingPolicy ) {
		case SWITCH_POLICY_WIN_CLASS:
			return QString("winclass: %1").arg(m_currentWinClass);
		case SWITCH_POLICY_WINDOW:
			return QString("window: %1").arg(m_currentWinId);
		case SWITCH_POLICY_DESKTOP:
			return QString("desktop: %1").arg(m_currentDesktop);
		default:
			return "global";
	}
}

// private
QQueue<int>& LayoutMap::getCurrentLayoutQueue()
{
	QQueue<int>& layoutQueue = getCurrentLayoutQueueInternal();
	if( layoutQueue.count() == 0 ) {
		initLayoutQueue(layoutQueue);
		kDebug() << "Created queue for " << getOwner() << " size: " << layoutQueue.count();
	}
	
	return layoutQueue;
}

int LayoutMap::getCurrentLayout() {
	return getCurrentLayoutQueue().head();
}

int LayoutMap::getNextLayout() {
	LayoutQueue& layoutQueue = getCurrentLayoutQueue(/*m_currentWinId*/);
	int layoutState = layoutQueue.dequeue();
	layoutQueue.enqueue(layoutState);
	
	kDebug() << "map: Next layout: " << layoutQueue.head() << " for " << getOwner();
	
	return layoutQueue.head();
}

void LayoutMap::setCurrentLayout(int layoutUnit) {
	LayoutQueue& layoutQueue = getCurrentLayoutQueue();
	kDebug() << "map: Storing layout: " << layoutUnit << " for " << getOwner();
	
	int queueSize = (int)layoutQueue.count();
	for(int ii=0; ii<queueSize; ii++) {
		if( layoutQueue.head() == layoutUnit )
			return;	// if present return when it's in head
		
		int layoutState = layoutQueue.dequeue();
		if( ii < queueSize - 1 ) {
			layoutQueue.enqueue(layoutState);
		}
		else {
			layoutQueue.enqueue(layoutUnit);
		}
	}
	for(int ii=0; ii<queueSize - 1; ii++) {
		int layoutState = layoutQueue.dequeue();
		layoutQueue.enqueue(layoutState);
	}
}

// private
void LayoutMap::initLayoutQueue(LayoutQueue& layoutQueue) {
	int queueSize = ( m_kxkbConfig.m_stickySwitching ) 
			? m_kxkbConfig.m_stickySwitchingDepth : m_kxkbConfig.m_layouts.count();
	for(int ii=0; ii<queueSize; ii++) {
		layoutQueue.enqueue( ii );
	}
}
