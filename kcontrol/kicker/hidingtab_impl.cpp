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

#include <qwhatsthis.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qstring.h>
#include <qslider.h>
#include <qspinbox.h>
#include <qradiobutton.h>
#include <qgroupbox.h>
#include <qbuttongroup.h>

#include <kconfig.h>
#include <kcombobox.h>
#include <kglobal.h>
#include <klocale.h>
#include <kstandarddirs.h>

#include "main.h"
#include "hidingtab_impl.h"
#include "hidingtab_impl.moc"


extern int kickerconfig_screen_number;


HidingTab::HidingTab( KickerConfig *parent, const char* name )
  : HidingTabBase (parent, name)
{
    kconf = parent;

    // connections
    connect(m_manual,SIGNAL(toggled(bool)), SIGNAL(changed()));
    connect(m_automatic,SIGNAL(toggled(bool)), SIGNAL(changed()));
    connect(m_background,SIGNAL(toggled(bool)), SIGNAL(changed()));
    connect(m_lHB, SIGNAL(toggled(bool)), SLOT(hideButtonsClicked()));
    connect(m_rHB, SIGNAL(toggled(bool)), SLOT(hideButtonsClicked()));
    connect(m_hideButtonSlider, SIGNAL(valueChanged(int)), SIGNAL(changed()));
    connect(m_manualHideSlider, SIGNAL(valueChanged(int)), SIGNAL(changed()));
    connect(m_delaySpinBox, SIGNAL(valueChanged(int)), SIGNAL(changed()));
    connect(m_autoHideSlider, SIGNAL(valueChanged(int)), SIGNAL(changed()));
    connect(m_autoHideSwitch, SIGNAL(toggled(bool)), SIGNAL(changed()));
    connect(m_none, SIGNAL(toggled(bool)), SIGNAL(changed()));
    connect(m_top, SIGNAL(toggled(bool)), SIGNAL(changed()));
    connect(m_topRight, SIGNAL(toggled(bool)), SIGNAL(changed()));
    connect(m_right, SIGNAL(toggled(bool)), SIGNAL(changed()));
    connect(m_bottomRight, SIGNAL(toggled(bool)), SIGNAL(changed()));
    connect(m_bottomLeft, SIGNAL(toggled(bool)), SIGNAL(changed()));
    connect(m_left, SIGNAL(toggled(bool)), SIGNAL(changed()));
    connect(m_topLeft, SIGNAL(toggled(bool)), SIGNAL(changed()));

    // whats this help
    QWhatsThis::add(m_manual, i18n(
        "If this option is selected, the only way to hide the panel "
        "will be via the hide buttons at its side."));
    QWhatsThis::add(m_automatic, i18n(
        "If this option is selected, the panel will automatically hide "
        "after some time and reappear when you move the mouse to the "
        "screen edge where the panel is hidden. "
        "This is particularly useful for small screen resolutions, "
        "for example, on laptops."));
    QWhatsThis::add(m_background, i18n(
        "If this option is selected, the panel will allow itself to "
        "be covered by other windows. Raise the panel to the top by "
        "moving the mouse to the screen edge specified by the unhide location "
        "below."));
    QWhatsThis::add(m_hideButtonSlider, i18n(
        "Here you can change the size of the hide buttons."));
    QWhatsThis::add(m_manualHideSlider, i18n(
        "Determines the speed of the animation shown when you click "
        "the panel's hide buttons. To disable the animation, move the "
        "slider all the way to the left."));
    QWhatsThis::add(m_delaySpinBox, i18n(
        "Here you can change the delay after which the panel will disappear "
        "if not used."));
    QWhatsThis::add(m_autoHideSlider, i18n(
        "Determines the speed of the animation shown when the panel "
        "automatically hides itself. To disable the animation, move the "
        "slider all the way to the left."));
    QWhatsThis::add(m_autoHideSwitch, i18n(
        "If this option is enabled, the panel will automatically show "
        "itself for a brief period of time when the desktop is switched "
        "so you can see which desktop you are on.") );
    QWhatsThis::add(m_backgroundGroup, i18n(
        "Here you can set the location on the screen's edge that will "
        "bring the panel to front."));
    load();
}

void HidingTab::load()
{
    QCString configname;
    if (kickerconfig_screen_number == 0)
	configname = "kickerrc";
    else
	configname.sprintf("kicker-screen-%drc", kickerconfig_screen_number);
    KConfig *c = new KConfig(configname, false, false);

    c->setGroup("General");

    if( c->readBoolEntry("AutoHidePanel", false) ) {
       m_automatic->setChecked(true);
    } else if( c->readBoolEntry("BackgroundHide", false) ) {
       m_background->setChecked(true);
    } else {
       m_manual->setChecked(true);
    }
    m_automaticGroup->setEnabled(m_automatic->isChecked());

    bool showLHB = c->readBoolEntry("ShowLeftHideButton", false);
    bool showRHB = c->readBoolEntry("ShowRightHideButton", true);
    m_lHB->setChecked( showLHB );
    m_rHB->setChecked( showRHB );
    m_hideButtonSlider->setValue(c->readNumEntry("HideButtonSize", 14));
    m_hideButtonSlider->setEnabled(showLHB || showRHB);

    if( c->readBoolEntry("HideAnimation", true) ) {
       m_manualHideSlider->setValue(c->readNumEntry("HideAnimationSpeed", 40)/10);
    } else {
       m_manualHideSlider->setValue(0);
    }

    m_delaySpinBox->setValue( c->readNumEntry("AutoHideDelay", 3) );
    if( c->readBoolEntry("AutoHideAnimation", true) ) {
       m_autoHideSlider->setValue(c->readNumEntry("AutoHideAnimationSpeed", 40)/10);
    } else {
       m_manualHideSlider->setValue(0);
    }
    m_autoHideSwitch->setChecked( c->readBoolEntry("AutoHideSwitch", false) );

    m_backgroundGroup->setButton( c->readNumEntry("UnhideLocation",BottomLeft) );
    m_backgroundGroup->setEnabled(m_background->isChecked());

    delete c;
}

void HidingTab::save()
{
    QCString configname;
    if (kickerconfig_screen_number == 0)
	configname = "kickerrc";
    else
	configname.sprintf("kicker-screen-%drc", kickerconfig_screen_number);
    KConfig *c = new KConfig(configname, false, false);

    c->setGroup("General");

    c->writeEntry("AutoHidePanel", m_automatic->isChecked());
    c->writeEntry("BackgroundHide", m_background->isChecked());

    c->writeEntry("ShowLeftHideButton", m_lHB->isChecked());
    c->writeEntry("ShowRightHideButton", m_rHB->isChecked());
    c->writeEntry("HideButtonSize", m_hideButtonSlider->value());
    c->writeEntry("HideAnimation", m_manualHideSlider->value() != 0);
    c->writeEntry("HideAnimationSpeed", m_manualHideSlider->value()*10);

    c->writeEntry("AutoHideDelay", m_delaySpinBox->value());
    c->writeEntry("AutoHideAnimation", m_autoHideSlider->value() != 0);
    c->writeEntry("AutoHideAnimationSpeed", m_autoHideSlider->value()*10);
    c->writeEntry("AutoHideSwitch", m_autoHideSwitch->isChecked());

    c->writeEntry("UnhideLocation", m_backgroundGroup->id(m_backgroundGroup->selected()));

    c->sync();

    delete c;
}

void HidingTab::defaults()
{
    m_manual->setChecked(true);

    m_lHB->setChecked( false );
    m_rHB->setChecked( true );
    m_hideButtonSlider->setValue(10);
    m_manualHideSlider->setValue(100);

    m_delaySpinBox->setValue(3);
    m_autoHideSlider->setValue(30);
    m_autoHideSwitch->setChecked(false);

    m_backgroundGroup->setButton(BottomLeft);
}

void HidingTab::hideButtonsClicked()
{
    m_hideButtonSlider->setEnabled( m_lHB->isChecked() || m_rHB->isChecked() );
    emit changed();
}
