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
#include <qradiobutton.h>
#include <qwhatsthis.h>
#include <qslider.h>
#include <qspinbox.h>

#include <kconfig.h>
#include <kglobal.h>
#include <klocale.h>
#include <klineedit.h>

#include "positiontab_impl.h"
#include "positiontab_impl.moc"


extern int kickerconfig_screen_number;


PositionTab::PositionTab( QWidget *parent, const char* name )
  : PositionTabBase (parent, name)
{
    // connections
    connect(m_locationGroup, SIGNAL(clicked(int)), SLOT(locationChanged()));
    connect(m_sizeGroup, SIGNAL(clicked(int)), SIGNAL(changed()));
    connect(m_alignGroup, SIGNAL(clicked(int)), SIGNAL(changed()));
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

    QWhatsThis::add(m_alignGroup, i18n("This setting determines how the panel is aligned, i.e."
				       " how it's positioned on the panel edge."
				       " Note that in order for this setting to have any effect,"
				       " the panel size has to be set to a value of less than 100%"));

    load();
}

void PositionTab::load()
{
    QCString configname;
    if (kickerconfig_screen_number == 0)
	configname = "kickerrc";
    else
	configname.sprintf("kicker-screen-%drc", kickerconfig_screen_number);
    KConfig *c = new KConfig(configname, false, false);

    c->setGroup("General");

    // Magic numbers stolen from kdebase/kicker/core/global.cpp
    // PGlobal::sizeValue()
    switch(c->readNumEntry("Size", 46)) {
    case 24: m_sizeGroup->setButton(0); break;
    case 30: m_sizeGroup->setButton(1); break;
    case 46: m_sizeGroup->setButton(2); break;
    case 58: m_sizeGroup->setButton(3); break;
    default: m_sizeGroup->setButton(4); break;
    }

    m_locationGroup->setButton(c->readNumEntry("Position", 3));
    m_alignGroup->setButton(c->readNumEntry("Alignment",
		    QApplication::reverseLayout() ? 2 : 0));

    // if the panel is horizontal...
    if (m_locationGroup->id(m_locationGroup->selected()) > 1) {
	m_alignLeftTop->setText(i18n("Le&ft"));
	m_alignRightBottom->setText(i18n("R&ight"));
    } else {
	m_alignLeftTop->setText(i18n("T&op"));
	m_alignRightBottom->setText(i18n("Bottom"));
    }

    int sizepercentage = c->readNumEntry( "SizePercentage", 100 );
    m_percentSlider->setValue( sizepercentage );
    m_percentSpinBox->setValue( sizepercentage );

    m_expandCheckBox->setChecked( c->readBoolEntry( "ExpandSize", true ) );

    delete c;
}

void PositionTab::locationChanged()
{
    // if the panel is horizontal...
    if ( m_locationGroup->id(m_locationGroup->selected()) > 1) {
    	m_alignLeftTop->setText(i18n("Le&ft"));
	m_alignRightBottom->setText(i18n("R&ight"));
    } else {
	m_alignLeftTop->setText(i18n("T&op"));
	m_alignRightBottom->setText(i18n("Bottom"));
    }
	
    emit changed();
}

void PositionTab::save()
{
    QCString configname;
    if (kickerconfig_screen_number == 0)
	configname = "kickerrc";
    else
	configname.sprintf("kicker-screen-%drc", kickerconfig_screen_number);
    KConfig *c = new KConfig(configname, false, false);

    c->setGroup("General");

    // Magic numbers stolen from kdebase/kicker/core/global.cpp
    // PGlobal::sizeValue()
    switch( m_sizeGroup->id(m_sizeGroup->selected()) ) {
    case 0: c->writeEntry("Size",24); break;
    case 1: c->writeEntry("Size",30); break;
    case 2: c->writeEntry("Size",46); break;
    case 3: c->writeEntry("Size",58); break;
    default: break; // Just leave the old entry
    }

    c->writeEntry("Position", m_locationGroup->id(m_locationGroup->selected()));
    c->writeEntry("Alignment", m_alignGroup->id(m_alignGroup->selected()));
    c->writeEntry( "SizePercentage", m_percentSlider->value() );
    c->writeEntry( "ExpandSize", m_expandCheckBox->isChecked() );
    c->sync();

    delete c;
}

void PositionTab::defaults()
{
    m_sizeGroup->setButton(2);
    m_locationGroup->setButton(3);
    m_alignGroup->setButton( QApplication::reverseLayout() ? 2 : 0 );
    m_expandCheckBox->setChecked( true );
    m_percentSlider->setValue( 100 );
    m_percentSpinBox->setValue( 100 );
}
