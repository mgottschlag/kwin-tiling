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
#include <kconfig.h>

#include "extensionInfo.h"

class QComboBox;
class KAboutData;
class KDirWatch;

class KickerConfig : public QObject, public DCOPObject
{
    Q_OBJECT
    K_DCOP

public:
    static KickerConfig *the();
    ~KickerConfig();

    void populateExtensionInfoList(QComboBox* list);
    void reloadExtensionInfo();
    void saveExtentionInfo();
    const ExtensionInfoList& extensionsInfo();

    QString configName();
    void notifyKicker();

    QString quickHelp() const;
    KAboutData *aboutData();

    int currentPanelIndex() const { return m_currentPanelIndex; }

k_dcop:
    void jumpToPanel(const QString& panelConfig);

signals:
    void positionPanelChanged(int);
    void hidingPanelChanged(int);
    void extensionInfoChanged();
    void extensionAdded(ExtensionInfo*);
    void extensionRemoved(ExtensionInfo*);
    void extensionChanged(const QString&);
    void extensionAboutToChange(const QString&);
    void aboutToNotifyKicker();

protected:
    void init();
    void setupExtensionInfo(KConfig& c, bool checkExists, bool reloadIfExists = false);

protected slots:
    void configChanged(const QString&);
    void setCurrentPanelIndex(int);

private:
    KickerConfig(QWidget *parent = 0, const char *name = 0);

    static KickerConfig *m_self;

    KDirWatch *configFileWatch;
    ExtensionInfoList m_extensionInfo;
    int m_screenNumber;
    uint m_currentPanelIndex;
};

#endif // __main_h__
