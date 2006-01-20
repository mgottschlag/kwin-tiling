/*
 *  Copyright (c) 2000 Matthias Elter <elter@kde.org>
 *  Copyright (c) 2002 Aaron Seigo <aseigo@olympusproject.org>
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

#ifndef __hidingtab_impl_h__
#define __hidingtab_impl_h__

#include "hidingtab.h"

class KickerConfig;
class ExtensionInfo;

class HidingTab : public HidingTabBase
{
    Q_OBJECT

public:
    HidingTab(QWidget *parent = 0, const char* name = 0);

    void load();
    void save();
    void defaults();

Q_SIGNALS:
    void changed();

public Q_SLOTS:
    void panelPositionChanged(int);

protected Q_SLOTS:
    void backgroundModeClicked();
    void infoUpdated();
    void storeInfo();
    void extensionAdded(ExtensionInfo*);
    void extensionRemoved(ExtensionInfo*);
    void switchPanel(int);

private:
    enum Trigger { None = 0, Top, TopRight, Right, BottomRight, Bottom, BottomLeft, Left, TopLeft };

    // these convert between the combobox and the config file for trigger
    // this is why storing enums vs strings can be a BAD thing
    int triggerComboToConfig(int trigger);
    int triggerConfigToCombo(int trigger);

    KickerConfig* m_kcm;
    ExtensionInfo* m_panelInfo;
};

#endif
