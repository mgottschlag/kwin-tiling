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


#ifndef __LAYOUTMAP_H
#define __LAYOUTMAP_H

#include <assert.h>

#include <QQueue>
#include <QMap>
#include <QtGui/QWidget>

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
	
	void ownerChanged();
	void reset();
	
private:
    // pseudo-union
	LayoutQueue m_globalLayouts;
	WinLayoutMap m_winLayouts;
	WinClassLayoutMap m_appLayouts;
	
	const KxkbConfig& m_kxkbConfig;
	WId m_currentWinId;
	int m_currentDesktop;
	QString m_currentWinClass; // only for SWITCH_POLICY_WIN_CLASS
	
	void initLayoutQueue(LayoutQueue& layoutQueue);
	LayoutQueue& getCurrentLayoutQueue();
	LayoutQueue& getCurrentLayoutQueueInternal();
	void clearMaps();
	QString getOwner();
};

#endif
