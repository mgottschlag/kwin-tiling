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

#include <qcheckbox.h>
#include <qgroupbox.h>
#include <qradiobutton.h>
#include <qslider.h>

#include <kcombobox.h>
#include <kdebug.h>
#include <klocale.h>
#include <knuminput.h>

#include "main.h"
#include "positiontab_impl.h"

#include "hidingtab_impl.h"
#include "hidingtab_impl.moc"


HidingTab::HidingTab(QWidget *parent, const char* name)
  : HidingTabBase(parent, name),
    m_panelInfo(0)
{
    // connections
    connect(m_manual,SIGNAL(toggled(bool)), SIGNAL(changed()));
    connect(m_automatic, SIGNAL(toggled(bool)), SIGNAL(changed()));
    connect(m_automatic, SIGNAL(toggled(bool)), SLOT(backgroundModeClicked()));
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

    connect(KickerConfig::the(), SIGNAL(extensionInfoChanged()),
            SLOT(infoUpdated()));
    connect(KickerConfig::the(), SIGNAL(extensionAdded(ExtensionInfo*)),
            SLOT(extensionAdded(ExtensionInfo*)));
    connect(KickerConfig::the(), SIGNAL(extensionRemoved(ExtensionInfo*)),
            SLOT(extensionRemoved(ExtensionInfo*)));
    // position tab tells hiding tab about extension selections and vice versa
    connect(KickerConfig::the(), SIGNAL(positionPanelChanged(int)),
            SLOT(switchPanel(int)));
    connect(m_panelList, SIGNAL(activated(int)),
            KickerConfig::the(), SIGNAL(hidingPanelChanged(int)));
}

void HidingTab::load()
{
    m_panelInfo = 0;
    KickerConfig::the()->populateExtensionInfoList(m_panelList);
    m_panelsGroupBox->setHidden(m_panelList->count() < 2);

    switchPanel(KickerConfig::the()->currentPanelIndex());
}

void HidingTab::extensionAdded(ExtensionInfo* info)
{
    m_panelList->insertItem(info->_name);
    m_panelsGroupBox->setHidden(m_panelList->count() < 2);
}

void HidingTab::extensionRemoved(ExtensionInfo* info)
{
    int count = m_panelList->count();
    int extensionCount = KickerConfig::the()->extensionsInfo().count();
    int index = 0;
    for (; index < count && index < extensionCount; ++index)
    {
        if (KickerConfig::the()->extensionsInfo()[index] == info)
        {
            break;
        }
    }

    bool isCurrentlySelected = index == m_panelList->currentItem();
    m_panelList->removeItem(index);
    m_panelsGroupBox->setHidden(m_panelList->count() < 2);

    if (isCurrentlySelected)
    {
        m_panelList->setCurrentItem(0);
    }
}

void HidingTab::switchPanel(int panelItem)
{
    blockSignals(true);
    ExtensionInfo* panelInfo = (KickerConfig::the()->extensionsInfo())[panelItem];

    if (!panelInfo)
    {
        m_panelList->setCurrentItem(0);
        panelInfo = (KickerConfig::the()->extensionsInfo())[panelItem];

        if (!panelInfo)
        {
            return;
        }
    }

    if (m_panelInfo)
    {
        storeInfo();
    }

    m_panelList->setCurrentItem(panelItem);

    m_panelInfo = panelInfo;

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

    panelPositionChanged(m_panelInfo->_position);

    backgroundModeClicked();
    blockSignals(false);
}

void HidingTab::save()
{
    storeInfo();
    KickerConfig::the()->saveExtentionInfo();
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
    m_backgroundRaise->setChecked( false );
}

void HidingTab::panelPositionChanged(int position)
{
    if (position == PositionTab::PosTop ||
        position == PositionTab::PosBottom)
    {
        m_lHB->setText(i18n("Show left panel-hiding bu&tton"));
        m_rHB->setText(i18n("Show right panel-hiding bu&tton"));
    }
    else
    {
        m_lHB->setText(i18n("Show top panel-hiding bu&tton"));
        m_rHB->setText(i18n("Show bottom panel-hiding bu&tton"));
    }
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
    m_backgroundPos->setEnabled((m_automatic->isChecked() ||
                                m_background->isChecked()) &&
                                m_backgroundRaise->isChecked());
}

void HidingTab::infoUpdated()
{
    switchPanel(0);
}
