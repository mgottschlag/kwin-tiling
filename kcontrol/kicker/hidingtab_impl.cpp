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

#include <qwhatsthis.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qstring.h>
#include <qslider.h>
#include <qradiobutton.h>
#include <qgroupbox.h>
#include <qbuttongroup.h>

#include <kconfig.h>
#include <kcombobox.h>
#include <kglobal.h>
#include <knuminput.h>
#include <klocale.h>
#include <kstandarddirs.h>

#include "main.h"
#include "hidingtab_impl.h"
#include "hidingtab_impl.moc"


extern int kickerconfig_screen_number;


HidingTab::HidingTab( KickerConfig *kcmKicker, const char* name )
  : HidingTabBase (kcmKicker, name),
    m_kcm(kcmKicker),
    m_panelInfo(0)
{
    m_panelList->setSorting(-1);

    // connections
    connect(m_manual,SIGNAL(toggled(bool)), SIGNAL(changed()));
    connect(m_automatic, SIGNAL(toggled(bool)), SIGNAL(changed()));
    connect(m_background,SIGNAL(toggled(bool)), SIGNAL(changed()));
    connect(m_background, SIGNAL(toggled(bool)), SLOT(backgroundModeClicked()));
    connect(m_hideSlider, SIGNAL(valueChanged(int)), SIGNAL(changed()));
    connect(m_delaySpinBox, SIGNAL(valueChanged(int)), SIGNAL(changed()));
    connect(m_animateHiding, SIGNAL(toggled(bool)), SIGNAL(changed()));
    connect(m_delaySpinBox, SIGNAL(valueChanged(int)), SIGNAL(changed()));
    connect(m_autoHideSwitch, SIGNAL(toggled(bool)), SIGNAL(changed()));
    connect(m_backgroundRaise, SIGNAL(toggled(bool)), SIGNAL(changed()));
    connect(m_backgroundPos, SIGNAL(activated(int)), SIGNAL(changed()));
    connect(m_lHB, SIGNAL(toggled(bool)), SIGNAL(changed()));
    connect(m_rHB, SIGNAL(toggled(bool)), SIGNAL(changed()));

    connect(m_kcm, SIGNAL(extensionInfoChanged()), SLOT(infoUpdated()));

    load();
}

void HidingTab::load()
{
    if (m_panelList->firstChild())
    {
        m_kcm->reloadExtensionInfo();
        m_panelList->clear();
    }

    m_kcm->populateExtensionInfoList(m_panelList);
    if (m_kcm->extensionsInfo().count() == 1)
    {
        m_panelList->hide();
    }
    
    switchPanel(0);
 }
 
void HidingTab::switchPanel(QListViewItem* panelItem)
{
    blockSignals(true);
    extensionInfoItem* listItem = reinterpret_cast<extensionInfoItem*>(panelItem);

    if (!listItem)
    {
        m_panelList->setSelected(m_panelList->firstChild(), true);
        listItem = reinterpret_cast<extensionInfoItem*>(m_panelList->firstChild());
    }
    
    if (m_panelInfo)
    {
        storeInfo();
    }

    m_panelInfo = listItem->info();

    if(m_panelInfo->_autohidePanel) 
    {
       m_automatic->setChecked(true);
    } 
    else if(m_panelInfo->_backgroundHide) 
    {
       m_background->setChecked(true);
    } 
    else 
    {
       m_manual->setChecked(true);
    }

    m_delaySpinBox->setValue(m_panelInfo->_autoHideDelay);
    m_autoHideSwitch->setChecked(m_panelInfo->_autoHideSwitch);

    m_lHB->setChecked( m_panelInfo->_showLeftHB );
    m_rHB->setChecked( m_panelInfo->_showRightHB );

    m_animateHiding->setChecked(m_panelInfo->_hideAnim);
    m_hideSlider->setValue(m_panelInfo->_hideAnimSpeed/10);

    if (m_panelInfo->_unhideLocation > 0)
    {
        m_backgroundRaise->setChecked(true);
        m_backgroundPos->setCurrentItem(triggerConfigToCombo(m_panelInfo->_unhideLocation));
    }
    else
    {
        m_backgroundRaise->setChecked(false);
    }

    backgroundModeClicked();
    blockSignals(false);
}

void HidingTab::save()
{
    storeInfo();
    m_kcm->saveExtentionInfo();
}

void HidingTab::storeInfo()
{
    if (!m_panelInfo)
    {
        return;
    }

    m_panelInfo->_autohidePanel = m_automatic->isChecked();
    m_panelInfo->_backgroundHide = m_background->isChecked();

    m_panelInfo->_showLeftHB = m_lHB->isChecked();
    m_panelInfo->_showRightHB = m_rHB->isChecked();
    m_panelInfo->_hideAnim = m_animateHiding->isChecked();
    m_panelInfo->_hideAnimSpeed = m_hideSlider->value() * 10;

    m_panelInfo->_autoHideDelay = m_delaySpinBox->value();
    m_panelInfo->_autoHideSwitch = m_autoHideSwitch->isChecked();

    m_panelInfo->_unhideLocation = m_backgroundRaise->isChecked() ?
                                   triggerComboToConfig(m_backgroundPos->currentItem()) : 0;
}

void HidingTab::defaults()
{
    m_manual->setChecked(true);
    m_delaySpinBox->setValue(3);
    m_autoHideSwitch->setChecked(false);
    m_lHB->setChecked( false );
    m_rHB->setChecked( true );
    m_animateHiding->setChecked(true);
    m_hideSlider->setValue(10);
    m_delaySpinBox->setValue(3);
    m_backgroundPos->setCurrentItem(triggerConfigToCombo(BottomLeft));
}

int HidingTab::triggerComboToConfig(int trigger)
{
    if (trigger == 0)
        return TopLeft;
    else if (trigger == 1)
        return Top;
    else if (trigger == 2)
        return TopRight;
    else if (trigger == 3)
        return Right;
    else if (trigger == 4)
        return BottomRight;
    else if (trigger == 5)
        return Bottom;
    else if (trigger == 6)
        return BottomLeft;
    else if (Left == 7)
        return Left;
    
    return 0;
}

int HidingTab::triggerConfigToCombo(int trigger)
{
    if (trigger == TopLeft)
        return 0;
    else if (trigger == Top)
        return 1;
    else if (trigger == TopRight)
        return 2;
    else if (trigger == Right)
        return 3;
    else if (trigger == BottomRight)
        return 4;
    else if (trigger == Bottom)
        return 5;
    else if (trigger == BottomLeft)
        return 6;
    else if (Left == Left)
        return 7;
    
    return 0;
}

void HidingTab::backgroundModeClicked()
{
    m_backgroundPos->setEnabled(m_background->isChecked() &&
                                m_backgroundRaise->isChecked());
}

void HidingTab::infoUpdated()
{
    switchPanel(0);
}

