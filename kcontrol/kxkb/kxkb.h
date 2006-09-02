/*
    Copyright (C) 2001, S.R.Haque <srhaque@iee.org>. Derived from an
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
#ifndef __K_XKB_H__
#define __K_XKB_H__


#include <QString>
#include <QStringList>
#include <QHash>
#include <QQueue>

#include <kuniqueapplication.h>

#include "kxkbtraywindow.h"
#include "kxkbconfig.h"


class XKBExtension;
class XkbRules;
class KActionCollection;
class KWinModule;
class LayoutMap;

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
	QString getCurrentLayout() { return m_currentLayout.toPair(); }
	QStringList getLayoutsList() { return kxkbConfig.getLayoutStringList(); }
	void forceSetXKBMap( bool set );

protected slots:
    void menuActivated(int id);
    void trayActivated(QSystemTrayIcon::ActivationReason reason);
    void windowChanged(WId winId);

    void slotSettingsChanged(int category);

protected:
    // Read settings, and apply them.
    bool settingsRead();
    void layoutApply();
    
private:
	void initTray();

private:
	KxkbConfig kxkbConfig;

    WId m_prevWinId;	// for tricky part of saving xkb group
    LayoutMap* m_layoutOwnerMap;

	LayoutUnit m_currentLayout;

    XKBExtension *m_extension;
    XkbRules *m_rules;
    KxkbLabelController *m_tray;
    KActionCollection *keys;
    KWinModule* kWinModule;
    bool m_forceSetXKBMap;
};

#endif
