//
// C++ Implementation: layoutmap
//
// Description: 
//
//
// Author: Andriy Rysin <rysin@kde.org>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include <QX11Info>

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
	setCurrentWindow( X11Helper::UNKNOWN_WINDOW_ID );
}



void LayoutMap::setCurrentWindow(WId winId)
{
	m_currentWinId = winId;
	if( m_kxkbConfig.m_switchingPolicy == SWITCH_POLICY_WIN_CLASS )
		m_currentWinClass = X11Helper::getWindowClass(winId, QX11Info::display());
}

// private
//LayoutQueue& 
QQueue<int>& LayoutMap::getCurrentLayoutQueueInternal(WId winId)
{
	if( winId == X11Helper::UNKNOWN_WINDOW_ID )
		return m_globalLayouts;
	
	switch( m_kxkbConfig.m_switchingPolicy ) {
		case SWITCH_POLICY_WIN_CLASS: {
//			QString winClass = X11Helper::getWindowClass(winId, qt_xdisplay());
			return m_appLayouts[ m_currentWinClass ];
		}
		case SWITCH_POLICY_WINDOW:
			return m_winLayouts[ winId ];

		default:
			return m_globalLayouts;
	}
}

// private
//LayoutQueue& 
QQueue<int>& LayoutMap::getCurrentLayoutQueue(WId winId)
{
	QQueue<int>& layoutQueue = getCurrentLayoutQueueInternal(winId);
	if( layoutQueue.count() == 0 ) {
		initLayoutQueue(layoutQueue);
		kDebug() << "map: Created queue for " << winId << " size: " << layoutQueue.count() << endl;
	}
	
	return layoutQueue;
}

int LayoutMap::getCurrentLayout() {
	return getCurrentLayoutQueue(m_currentWinId).head();
}

int LayoutMap::getNextLayout() {
	LayoutQueue& layoutQueue = getCurrentLayoutQueue(m_currentWinId);
	int layoutState = layoutQueue.dequeue();
	layoutQueue.enqueue(layoutState);
	
	kDebug() << "map: Next layout: " << layoutQueue.head() 
// 			<< " group: " << layoutQueue.head()->layoutUnit.defaultGroup 
			<< " for " << m_currentWinId << endl;
	
	return layoutQueue.head();
}

void LayoutMap::setCurrentLayout(int layoutUnit) {
	LayoutQueue& layoutQueue = getCurrentLayoutQueue(m_currentWinId);
	kDebug() << "map: Storing layout: " << layoutUnit 
// 			<< " group: " << layoutUnit.defaultGroup 
			<< " for " << m_currentWinId << endl;
	
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
