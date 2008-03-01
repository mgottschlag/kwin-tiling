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


class XKBExtension;
class XkbRules;
class KActionCollection;
class LayoutMap;
class KxkbWidget;
class QAction;
class KShortcut;

//typedef KxkbWidget* (*KxkbWidgetCreateFn(KxkbWidget*));

/* This is the main Kxkb class responsible for reading options
    and switching layouts
*/

class KxkbCore : public QObject
{
    Q_OBJECT

public:
    enum { KXKB_MAIN=1, KXKB_COMPONENT=2 };

    KxkbCore(int mode=KXKB_MAIN);
    ~KxkbCore();

    virtual int newInstance();
    bool setLayout(int layout);
    int getStatus() { return m_status; }
    bool x11EventFilter ( XEvent * event );
    void setWidget(KxkbWidget* kxkbWidet);
    void cleanup();
    const KShortcut* getKDEShortcut();

// DBUS:
public slots:
    bool setLayout(const QString& layoutPair);
    QString getCurrentLayout() { return m_kxkbConfig.m_layouts[m_currentLayout].toPair(); }
    QStringList getLayoutsList() { return m_kxkbConfig.getLayoutStringList(); }
    void toggled();

protected slots:
    void iconMenuTriggered(QAction*);
    void windowChanged(WId winId);
    void desktopChanged(int desktop);

    void settingsChanged(int category);

protected:
    bool settingsRead();
    void layoutApply();
    
private:
    int m_mode;
    int m_currentLayout;
    int m_status;
    bool m_eventsHandled;

    KxkbConfig m_kxkbConfig;
    LayoutMap* m_layoutOwnerMap;
    
    XKBExtension *m_extension;
    XkbRules *m_rules;
    
    KxkbWidget *m_kxkbWidget;
    KActionCollection *actionCollection;
    
    QWidget* m_dummyWidget;
    
    void updateIndicator(int layout, int res);
    void initTray();
    void initKDEShortcut();
    void stopKDEShortcut();
    void initReactions();
    void initLayoutGroups();
    void initSwitchingPolicy();
    int updateGroupsFromServer();
};

#endif
