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

#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qwhatsthis.h>
#include <qslider.h>
#include <qspinbox.h>

#include <kconfig.h>
#include <kglobal.h>
#include <klocale.h>
#include <klineedit.h>

#include "generaltab_impl.h"
#include "generaltab_impl.moc"


extern int kickerconfig_screen_number;


GeneralTab::GeneralTab( QWidget *parent, const char* name )
  : GeneralTabBase (parent, name)
{
    // connections
    connect(m_locationGroup, SIGNAL(clicked(int)), SIGNAL(changed()));
    connect(m_sizeGroup, SIGNAL(clicked(int)), SIGNAL(changed()));
    connect(m_autoHide, SIGNAL(clicked()), SIGNAL(changed()));
    connect(m_autoHideSwitch, SIGNAL(clicked()), SIGNAL(changed()));
    connect(m_delaySlider, SIGNAL(valueChanged(int)), SIGNAL(changed()));
    connect(m_delaySpinBox, SIGNAL(valueChanged(int)), SIGNAL(changed()));
    connect(m_percentSlider, SIGNAL(valueChanged(int)), SIGNAL(changed()));
    connect(m_percentSpinBox, SIGNAL(valueChanged(int)), SIGNAL(changed()));
    connect(m_expandCheckBox, SIGNAL(clicked()), SIGNAL(changed()));

    // whats this help
    QWhatsThis::add(m_locationGroup, i18n("This sets the position of the panel"
                                          " i.e. the screen border it is attached to. You can also change this"
                                          " position by left-clicking on some free space on the panel and"
                                          " dragging it to a screen border."));

    QWhatsThis::add(m_sizeGroup, i18n("This sets the size of the panel."
                                      " You can also access this option via the panel context menu, i.e."
                                      " by right-clicking on some free space on the panel."));

    QWhatsThis::add(m_autoHide, i18n("If this option is enabled, the panel will automatically hide "
                                     "after some time and reappear when you move the mouse to the "
                                     "screen edge the panel is attached to. "
                                     "This is particularly useful for small screen resolutions, "
                                     "for example, on laptops.") );

    QWhatsThis::add(m_autoHideSwitch, i18n("If this option is enabled, the panel will automatically show "
					   "itself for a brief period of time when the desktop is switched "
					   "so you can see which desktop you are on.") );

    QString delaystr = i18n("Here you can change the delay after which the panel will disappear"
                            " if not used.");

    QWhatsThis::add(m_delaySlider, delaystr);
    QWhatsThis::add(m_delaySpinBox, delaystr);

    load();
}

void GeneralTab::load()
{
    QCString configname;
    if (kickerconfig_screen_number == 0)
	configname = "kickerrc";
    else
	configname.sprintf("kicker-screen-%drc", kickerconfig_screen_number);
    KConfig *c = new KConfig(configname, false, false);

    c->setGroup("General");

    m_sizeGroup->setButton(c->readNumEntry("Size", 2));
    m_locationGroup->setButton(c->readNumEntry("Position", 3));

    bool ah = c->readBoolEntry("AutoHidePanel", false);
    bool ahs = c->readBoolEntry("AutoHideSwitch", false);
    int delay = c->readNumEntry("AutoHideDelay", 3);

    m_autoHide->setChecked(ah);
    m_autoHideSwitch->setChecked(ahs);
    m_delaySlider->setValue(delay);
    m_delaySpinBox->setValue(delay);
    m_delaySlider->setEnabled(ah);
    m_delaySpinBox->setEnabled(ah);

    int sizepercentage = c->readNumEntry( "SizePercentage", 100 );
    m_percentSlider->setValue( sizepercentage );
    m_percentSpinBox->setValue( sizepercentage );

    m_expandCheckBox->setChecked( c->readBoolEntry( "ExpandSize", true ) );

    delete c;
}

void GeneralTab::save()
{
    QCString configname;
    if (kickerconfig_screen_number == 0)
	configname = "kickerrc";
    else
	configname.sprintf("kicker-screen-%drc", kickerconfig_screen_number);
    KConfig *c = new KConfig(configname, false, false);

    c->setGroup("General");

    c->writeEntry("Size", m_sizeGroup->id(m_sizeGroup->selected()));
    c->writeEntry("Position", m_locationGroup->id(m_locationGroup->selected()));
    c->writeEntry("AutoHidePanel", m_autoHide->isChecked());
    c->writeEntry("AutoHideSwitch", m_autoHideSwitch->isChecked());
    c->writeEntry("AutoHideDelay", m_delaySlider->value());
    c->writeEntry( "SizePercentage", m_percentSlider->value() );
    c->writeEntry( "ExpandSize", m_expandCheckBox->isChecked() );

    c->sync();

    delete c;
}

void GeneralTab::defaults()
{
    m_sizeGroup->setButton(2);
    m_locationGroup->setButton(3);

    m_autoHide->setChecked(false);
    m_delaySlider->setValue(3);
    m_delaySpinBox->setValue(3);
    m_delaySlider->setEnabled(false);
    m_delaySpinBox->setEnabled(false);
    m_expandCheckBox->setChecked( true );
    m_percentSlider->setValue( 100 );
    m_percentSpinBox->setValue( 100 );
}
