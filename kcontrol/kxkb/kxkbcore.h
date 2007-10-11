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
#ifndef __KXKBCORE_H__
#define __KXKBCORE_H__


#include <QHash>
#include <QQueue>

#include "kxkbconfig.h"
#include "kxkbwidget.h"

class XKBExtension;
class XkbRules;
class KActionCollection;
class LayoutMap;
//class KxkbWidget;
class QAction;

/* This is the main Kxkb class responsible for reading options
    and switching layouts
*/

class KxkbCore : public QObject
{
    Q_OBJECT

public:
    enum { MAIN_MODULE=1, NO_INIT=2 };

    KxkbCore(QWidget* parentWidget, int mode=MAIN_MODULE, int controlType=KxkbWidget::MENU_FULL, int widgetType=KxkbWidget::WIDGET_TRAY);
    ~KxkbCore();

    virtual int newInstance();
    bool setLayout(int layout);
    int getStatus() { return m_status; }
    bool x11EventFilter ( XEvent * event );

// DBUS:
public slots:
    bool setLayout(const QString& layoutPair);
    QString getCurrentLayout() { return m_kxkbConfig.m_layouts[m_currentLayout].toPair(); }
    QStringList getLayoutsList() { return m_kxkbConfig.getLayoutStringList(); }

protected slots:
    void iconMenuTriggered(QAction*);
    void iconToggled();
    void windowChanged(WId winId);
    void desktopChanged(int desktop);

    void slotSettingsChanged(int category);

protected:
    bool settingsRead();
    void layoutApply();
    
signals:
    void quit();
		
private:
    KxkbConfig m_kxkbConfig;

    LayoutMap* m_layoutOwnerMap;
    
    int m_mode;
    int m_currentLayout;
    int m_controlType;
    int m_widgetType;
    int m_status;

    XKBExtension *m_extension;
    XkbRules *m_rules;
    QWidget *m_parentWidget;
    KxkbWidget *m_kxkbWidget;
    KActionCollection *m_keys;
    
    void updateIndicator(int layout, int res);
    void initTray();
    void initKeys();
    void initWidget();
    void initLayoutGroups();
    void initSwitchingPolicy();
    int updateGroupsFromServer();
};

#endif
