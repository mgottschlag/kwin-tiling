/*
 *  Copyright (c) 2002 Stephan Binner <binner@kde.org>
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

#ifndef __menuconfig_h__
#define __menuconfig_h__

#include <kcmodule.h>

class MenuTab;

class MenuConfig : public KCModule
{
    Q_OBJECT

public:
    MenuConfig(QWidget *parent = 0L, const char *name = 0L);

    void load();
    void save();
    void defaults();
    QString quickHelp() const;
    const KAboutData* aboutData() const;

public slots:
    void configChanged();

private:
    MenuTab        *menutab;
};

#endif // __menuconfig_h__
