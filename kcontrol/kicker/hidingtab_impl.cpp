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


HidingTab::HidingTab( KickerConfig *parent, const char* name )
  : HidingTabBase (parent, name)
{
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
    connect(m_backgroundPos, SIGNAL(activated(bool)), SIGNAL(changed()));
    connect(m_lHB, SIGNAL(toggled(bool)), SIGNAL(changed()));
    connect(m_rHB, SIGNAL(toggled(bool)), SIGNAL(changed()));

    load();
}

void HidingTab::load()
{
    QCString configname;
    if (kickerconfig_screen_number == 0)
	configname = "kickerrc";
    else
	configname.sprintf("kicker-screen-%drc", kickerconfig_screen_number);
    KConfig c(configname, false, false);

    c.setGroup("General");

    if( c.readBoolEntry("AutoHidePanel", false) ) {
       m_automatic->setChecked(true);
    } else if( c.readBoolEntry("BackgroundHide", false) ) {
       m_background->setChecked(true);
    } else {
       m_manual->setChecked(true);
    }

    m_delaySpinBox->setValue(c.readNumEntry("AutoHideDelay"));
    m_autoHideSwitch->setChecked( c.readBoolEntry("AutoHideSwitch", false) );

    bool showLHB = c.readBoolEntry("ShowLeftHideButton", false);
    bool showRHB = c.readBoolEntry("ShowRightHideButton", true);
    m_lHB->setChecked( showLHB );
    m_rHB->setChecked( showRHB );

    m_animateHiding->setChecked(c.readBoolEntry("HideAnimation", true));
    m_hideSlider->setValue(c.readNumEntry("HideAnimationSpeed", 40)/10);

    int unhideLocation = c.readNumEntry("UnhideLocation",BottomLeft);
    if (unhideLocation > 0)
    {
        m_backgroundRaise->setChecked(true);
        m_backgroundPos->setCurrentItem(triggerConfigToCombo(unhideLocation));
    }
    else
    {
        m_backgroundRaise->setChecked(false);
    }

    backgroundModeClicked();
}

void HidingTab::save()
{
    QCString configname;
    if (kickerconfig_screen_number == 0)
        configname = "kickerrc";
    else
        configname.sprintf("kicker-screen-%drc", kickerconfig_screen_number);
    KConfig c(configname, false, false);

    c.setGroup("General");

    c.writeEntry("AutoHidePanel", m_automatic->isChecked());
    c.writeEntry("BackgroundHide", m_background->isChecked());

    c.writeEntry("ShowLeftHideButton", m_lHB->isChecked());
    c.writeEntry("ShowRightHideButton", m_rHB->isChecked());
    c.writeEntry("HideAnimation", m_animateHiding->isChecked());
    c.writeEntry("HideAnimationSpeed", m_hideSlider->value()*10);

    c.writeEntry("AutoHideDelay", m_delaySpinBox->value());
    c.writeEntry("AutoHideSwitch", m_autoHideSwitch->isChecked());

    c.writeEntry("UnhideLocation", m_backgroundRaise->isChecked() ?
                                    triggerComboToConfig(m_backgroundPos->currentItem()) : 0);

    c.sync();
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

