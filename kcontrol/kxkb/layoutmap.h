//
// C++ Interface: layoutmap
//
// Description: 
//
//
// Author: Andriy Rysin <rysin@kde.org>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef __LAYOUTMAP_H
#define __LAYOUTMAP_H

#include <assert.h>

#include <QQueue>
#include <QMap>
#include <qwidget.h>

#include <kdebug.h>

#include "kxkbconfig.h"


// LayoutMap is used for per-window or per-application switching policy
class LayoutMap {
	typedef QQueue<int> LayoutQueue;
	typedef QMap<WId, LayoutQueue> WinLayoutMap;
	typedef QMap<QString, LayoutQueue> WinClassLayoutMap;

public:
	LayoutMap(const KxkbConfig& kxkbConfig);
//	void setConfig(const KxkbConfig& kxkbConfig);
	
	void setCurrentLayout(int layout);
	int getNextLayout();
	int getCurrentLayout();
	
	void setCurrentWindow(WId winId);
	void reset();
	
private:
    // pseudo-union
	LayoutQueue m_globalLayouts;
	WinLayoutMap m_winLayouts;
	WinClassLayoutMap m_appLayouts;
	
	const KxkbConfig& m_kxkbConfig;
	WId m_currentWinId;
	QString m_currentWinClass; // only for SWITCH_POLICY_WIN_CLASS
	
	void initLayoutQueue(LayoutQueue& layoutQueue);
	LayoutQueue& getCurrentLayoutQueue(WId winId);
	LayoutQueue& getCurrentLayoutQueueInternal(WId winId);
	void clearMaps();
};

#endif
