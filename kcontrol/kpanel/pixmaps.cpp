/*
 * pixmaps.cpp
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

#include "pixmaps.h"
#include "pixmaps.moc"
#include <qlayout.h>
#include <qlabel.h>
#include <ksimpleconfig.h>
#include <klocale.h>

extern KConfigBase *config;

KPixmapConfig::KPixmapConfig(QWidget *parent, const char* name)
    : KConfigWidget(parent, name)
{
    ldr.getDirList()->clear();
    ldr.insertDirectory(0, kapp->localkdedir()+"/share/apps/kpanel/pics");
    ldr.insertDirectory(1, kapp->kde_datadir()+"/kpanel/pics");
    
    currentIcon = new KIconLoaderButton(&ldr, this);
    enableBox = new QCheckBox(i18n("Enable pixmaps for this item."), this);
    enableBox->setChecked(true);
    
    itemCombo = new QComboBox(this);
    itemCombo->insertItem(i18n("Panel background"), Panel_ID);
    itemCombo->insertItem(i18n("Application icon background"), AppIcon_ID);
    itemCombo->insertItem(i18n("Taskbar background"), Taskbar_ID);
    itemCombo->insertItem(i18n("Taskbar button"), TaskBtn_ID);

    QLabel *itemLbl = new QLabel(i18n("KPanel item:"), this);

    currentIcon->setMinimumSize(QSize(52,52));
    enableBox->setMinimumSize(enableBox->sizeHint());
    itemCombo->setMinimumSize(itemCombo->sizeHint());
    itemLbl->setMinimumSize(itemLbl->sizeHint());
    
    QVBoxLayout *mainLayout = new QVBoxLayout(this, 4);
    QHBoxLayout *pixLayout = new QHBoxLayout;
    QHBoxLayout *ctlLayout = new QHBoxLayout;

    mainLayout->addSpacing(8);
    mainLayout->addLayout(pixLayout);
    mainLayout->addLayout(ctlLayout);
    mainLayout->addSpacing(4);
    mainLayout->addWidget(enableBox);
    mainLayout->addStretch(1);

    pixLayout->addWidget(currentIcon);
    pixLayout->addStretch(1);
    
    ctlLayout->addWidget(itemLbl);
    ctlLayout->addSpacing(4);
    ctlLayout->addWidget(itemCombo);
    ctlLayout->addStretch(1);

    connect(itemCombo, SIGNAL(activated(int)), SLOT(itemSlot(int)));
    connect(enableBox, SIGNAL(clicked()), SLOT(enableSlot()));
    connect(currentIcon, SIGNAL(iconChanged(const char *)),
            SLOT(iconSlot(const char *)));

    
    loadSettings();
    mainLayout->activate();

    enabled[0] = enabled[1] = enabled[2] = enabled[3] = true;
}

KPixmapConfig::~KPixmapConfig()
{
    ;
}

void KPixmapConfig::itemSlot(int index)
{
    currentIcon->setIcon((const char *)icons[index]);
    currentIcon->setPixmap(ldr.loadIcon(icons[index])); // hack :P
    enableBox->setChecked(enabled[index]);
}

void KPixmapConfig::enableSlot()
{
    enabled[itemCombo->currentItem()] = enableBox->isChecked();
}

void KPixmapConfig::iconSlot(const char *icon)
{
    icons[itemCombo->currentItem()] = icon;
}

void KPixmapConfig::loadSettings(){

    config->setGroup("kpanel");
    icons[Panel_ID] = config->readEntry("BackgroundTexture", " ");
    icons[AppIcon_ID] = config->readEntry("IconTexture", " ");
    icons[Taskbar_ID] = config->readEntry("TaskbarFrameTexture", " ");
    icons[TaskBtn_ID] = config->readEntry("TaskbarTexture", " ");
    currentIcon->setIcon(icons[itemCombo->currentItem()]);
    currentIcon->setPixmap(ldr.loadIcon(icons[Panel_ID])); // hack :P
}

void KPixmapConfig::saveSettings(){
    config->setGroup("kpanel");

    if(enabled[Panel_ID])
        config->writeEntry("BackgroundTexture", icons[Panel_ID]);
    else
        config->writeEntry("BackgroundTexture", " ");

    if(enabled[AppIcon_ID])
        config->writeEntry("IconTexture", icons[AppIcon_ID]);
    else
        config->writeEntry("IconTexture", " ");

    if(enabled[Taskbar_ID])
        config->writeEntry("TaskbarFrameTexture", icons[Taskbar_ID]);
    else
        config->writeEntry("TaskbarFrameTexture", " ");

    if(enabled[TaskBtn_ID])
        config->writeEntry("TaskbarTexture", icons[TaskBtn_ID]);
    else
        config->writeEntry("TaskbarTexture", " ");

    config->sync();
}

void KPixmapConfig::applySettings() {
    saveSettings();
}

