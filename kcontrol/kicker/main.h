/*
 *  Copyright (c) 2000 Matthias Elter <elter@kde.org>
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
 */

#ifndef __main_h__
#define __main_h__

#include <dcopobject.h>

#include <kcmodule.h>
#include <kconfig.h>

#include "extensionInfo.h"

class QComboBox;
class QTabWidget;
class KDirWatch;
class PositionTab;
class HidingTab;
class MenuTab;
class LookAndFeelTab;
//class AppletTab;
class ExtensionsTab;

class KickerConfig : public KCModule, public DCOPObject
{
    Q_OBJECT
    K_DCOP

public:
    KickerConfig(QWidget *parent = 0L, const char *name = 0L);
    ~KickerConfig();
    void load();
    void save();
    void defaults();

    void populateExtensionInfoList(QComboBox* list);
    void reloadExtensionInfo();
    void saveExtentionInfo();
    const extensionInfoList& extensionsInfo();

    // now that it's all split up, bring the code dupe under control
    static void initScreenNumber();
    static QString configName();
    static void notifyKicker();

k_dcop:
    void jumpToPanel(const QString& panelConfig);

signals:
    void extensionInfoChanged();
    void extensionAdded(extensionInfo*);
    void extensionChanged(const QString&);
    void extensionAboutToChange(const QString&);

protected slots:
    void positionPanelChanged(int);
    void hidingPanelChanged(int);
    void configChanged(const QString&);

private:
    void setupExtensionInfo(KConfig& c, bool checkExists, bool reloadIfExists = false);

    KDirWatch      *configFileWatch;
    PositionTab    *positiontab;
    HidingTab      *hidingtab;
    LookAndFeelTab *lookandfeeltab;
    MenuTab        *menutab;
//    AppletTab      *applettab;
    extensionInfoList m_extensionInfo;
    static int kickerconfig_screen_number;
};

#endif // __main_h__
