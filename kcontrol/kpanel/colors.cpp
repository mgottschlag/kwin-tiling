/*
 * colors.cpp
 *
 * Copyright (C) 1999 Daniel M. Duley <mosfet@jorsm.com>
 *
 * Requires the Qt widget libraries, available at no cost at
 * http://www.troll.no/
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
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "colors.h"
#include "colors.moc"
#include <qlayout.h>
#include <qlabel.h>
#include <ksimpleconfig.h>
#include <klocale.h>

extern KConfigBase *config;

KColorConfig::KColorConfig(QWidget *parent, const char* name)
    : KConfigWidget(parent, name)
{
    currentColor = new KColorButton(this);

    itemCombo = new QComboBox(this);
    itemCombo->insertItem(i18n("Panel background"), Panel_ID);
    itemCombo->insertItem(i18n("Desktop button foreground (not yet :P)"), DeskFg_ID);
    itemCombo->insertItem(i18n("Desktop button background"), DeskBg_ID);
    itemCombo->insertItem(i18n("Application icon background"), AppIcon_ID);
    itemCombo->insertItem(i18n("Taskbar background"), Taskbar_ID);
    itemCombo->insertItem(i18n("Taskbar button foreground"), TaskBtnFg_ID);
    itemCombo->insertItem(i18n("Taskbar button background"), TaskBtnBg_ID);
    itemCombo->insertItem(i18n("Busy indicator foreground"), BlinkHigh_ID);
    itemCombo->insertItem(i18n("Busy indicator background"), BlinkLow_ID);

    QLabel *warnLbl = new QLabel(i18n("Notice: These settings should be merged with\n\
kcmdisplay's color preview widget after beta status."), this);
    
    currentColor->setMinimumSize(currentColor->sizeHint());
    itemCombo->setMinimumSize(itemCombo->sizeHint());
    warnLbl->setMinimumSize(warnLbl->sizeHint());
    
    QVBoxLayout *mainLayout = new QVBoxLayout(this, 4);
    QHBoxLayout *ctlLayout = new QHBoxLayout;

    mainLayout->addSpacing(8);
    mainLayout->addWidget(warnLbl);
    mainLayout->addSpacing(8);
    mainLayout->addLayout(ctlLayout);
    mainLayout->addStretch(1);
    
    ctlLayout->addWidget(itemCombo);
    ctlLayout->addSpacing(8);
    ctlLayout->addWidget(currentColor);
    ctlLayout->addStretch(1);

    connect(itemCombo, SIGNAL(activated(int)), SLOT(itemSlot(int)));
    connect(currentColor, SIGNAL(changed(const QColor &)),
            SLOT(colorSlot(const QColor &)));
    
    loadSettings();
    mainLayout->activate();
}

KColorConfig::~KColorConfig()
{
    ;
}

void KColorConfig::itemSlot(int index)
{
    currentColor->setColor(colors[index]);
}

void KColorConfig::colorSlot(const QColor &color)
{
    colors[itemCombo->currentItem()] = color;
}

void KColorConfig::loadSettings()
{

    config->setGroup("kpanel");
    colors[Panel_ID] = config->readColorEntry("PanelBackground",
                                              &backgroundColor());
    colors[DeskFg_ID] = config->readColorEntry("ButtonForeground",
                                               &foregroundColor());
    colors[DeskBg_ID] = config->readColorEntry("ButtonBackground",
                                               &backgroundColor());
    colors[AppIcon_ID] = config->readColorEntry("IconBackground",
                                                &backgroundColor());
    colors[Taskbar_ID] = config->readColorEntry("TaskbarFrameBackground",
                                                &backgroundColor());
    colors[TaskBtnFg_ID] = config->readColorEntry("TaskbarForeground",
                                                  &foregroundColor());
    colors[TaskBtnBg_ID] = config->readColorEntry("TaskbarBackground",
                                                  &backgroundColor());
    colors[BlinkHigh_ID] = config->readColorEntry("BlinkHighlight",
                                                  &green);
    colors[BlinkLow_ID] = config->readColorEntry("BlinkLowlight",
                                                  &darkGreen);
}

void KColorConfig::saveSettings()
{
    config->setGroup("kpanel");
    config->writeEntry("PanelBackground", colors[Panel_ID]);
    config->writeEntry("ButtonForeground", colors[DeskFg_ID]);
    config->writeEntry("ButtonBackground", colors[DeskBg_ID]);
    config->writeEntry("IconBackground", colors[AppIcon_ID]);
    config->writeEntry("TaskbarFrameBackground", colors[Taskbar_ID]);
    config->writeEntry("TaskbarForeground", colors[TaskBtnFg_ID]);
    config->writeEntry("TaskbarBackground", colors[TaskBtnBg_ID]);
    config->writeEntry("BlinkHighlight", colors[BlinkHigh_ID]);
    config->writeEntry("BlinkLowlight", colors[BlinkLow_ID]);
    config->sync();
}

void KColorConfig::applySettings()
{
    saveSettings();
}



