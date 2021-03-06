/********************************************************************
 KWin - the KDE window manager
 This file is part of the KDE project.

 Copyright (C) 2009 Martin Gräßlin <kde@martin-graesslin.com>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*********************************************************************/

#ifndef KWIN_CUBESLIDE_CONFIG_H
#define KWIN_CUBESLIDE_CONFIG_H

#include <kcmodule.h>

#include "ui_cubeslide_config.h"


namespace KWin
{

class CubeSlideEffectConfigForm : public QWidget, public Ui::CubeSlideEffectConfigForm
{
    Q_OBJECT
public:
    explicit CubeSlideEffectConfigForm(QWidget* parent);
};

class CubeSlideEffectConfig : public KCModule
{
    Q_OBJECT
public:
    explicit CubeSlideEffectConfig(QWidget* parent = 0, const QVariantList& args = QVariantList());

public slots:
    virtual void save();
    virtual void load();
    virtual void defaults();
private:
    CubeSlideEffectConfigForm* m_ui;
};

} // namespace

#endif
