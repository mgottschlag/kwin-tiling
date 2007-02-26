/*
    Copyright (C) 2001, S.R.Haque <srhaque@iee.org>. 
	Copyright (C) 2006, Andriy Rysin <rysin@kde.org>. Derived from an
    original by Matthias H�zer-Klpfel released under the QPL.
    This file is part of the KDE project

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

DESCRIPTION

    KDE Keyboard Tool. Manages XKB keyboard mappings.
*/
#ifndef __K_XKBAPP_H__
#define __K_XKBAPP_H__


#include <QString>
#include <QStringList>
#include <QHash>
#include <QQueue>

#include <kuniqueapplication.h>

#include "kxkbcore.h"

class LayoutUnit;
class QAction;
/* This is the main Kxkb class responsible for reading options
    and switching layouts
*/

class KXKBApp : public KUniqueApplication
{
    Q_OBJECT
//     K_DCOP

public:
	KXKBApp(bool allowStyles=true, bool GUIenabled=true);
	~KXKBApp();

	virtual int newInstance();

	bool setLayout(const LayoutUnit& layoutUnit, int group=-1);
// k_dcop:
public slots:
	bool setLayout(const QString& layoutPair);
	QString getCurrentLayout() { return m_kxkbCore->getCurrentLayout(); }
	QStringList getLayoutsList() { return m_kxkbCore->getLayoutsList(); }
	void forceSetXKBMap( bool set );

protected slots:
    void windowChanged(WId winId);

    void slotSettingsChanged(int category);

protected:
    // Read settings, and apply them.
    bool settingsRead();
    void layoutApply();

private:
	KxkbCore* m_kxkbCore;
};

#endif
