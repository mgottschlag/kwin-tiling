/*
 *  advancedDialog.cpp
 *
 *  Copyright (c) 2002 Aaron J. Seigo <aseigo@olympusproject.org>
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
#include <qframe.h>
#include <qlayout.h>
#include <qpushbutton.h>

#include <kconfig.h>
#include <klocale.h>
#include <knuminput.h>

#include "advancedDialog.h"
#include "advancedOptions.h" 
#include "main.h"

advancedDialog::advancedDialog(QWidget* parent, char* name)
    : KDialogBase(KDialogBase::Plain,
                  i18n("Advanced Options"),
                  KDialogBase::Ok |
                  KDialogBase::Apply |
                  KDialogBase::Cancel,
                  KDialogBase::Cancel,
                  parent,
                  name,
                  true, true)
{
    connect(this, SIGNAL(applyClicked()),
            this, SLOT(save()));
    connect(this, SIGNAL(okClicked()),
            this, SLOT(save()));
    actionButton(Apply)->setEnabled(false);
    QFrame* page = plainPage();
    QVBoxLayout* layout = new QVBoxLayout(page);
    m_advancedWidget = new advancedKickerOptions(page);
    layout->addWidget(m_advancedWidget);

    connect(m_advancedWidget->fadeOutHandles, SIGNAL(toggled(bool)),
            this, SLOT(changed()));
    connect(m_advancedWidget->hideButtonSize, SIGNAL(valueChanged(int)),
            this, SLOT(changed()));
    load();
}

advancedDialog::~advancedDialog()
{
}

void advancedDialog::load()
{
    KConfig c(KickerConfig::configName(), false, false);
    c.setGroup("General");
    bool fadedOut = c.readBoolEntry("FadeOutAppletHandles", false);
    m_advancedWidget->fadeOutHandles->setChecked(fadedOut);
    int defaultHideButtonSize = c.readNumEntry("HideButtonSize", 14);
    m_advancedWidget->hideButtonSize->setValue(defaultHideButtonSize);
    actionButton(Apply)->setEnabled(false);
}

void advancedDialog::save()
{
    KConfig c(KickerConfig::configName(), false, false);

    c.setGroup("General");
    c.writeEntry("FadeOutAppletHandles",
                 m_advancedWidget->fadeOutHandles->isChecked());
    c.writeEntry("HideButtonSize",
                 m_advancedWidget->hideButtonSize->value());
    c.sync();

    KickerConfig::notifyKicker();
    actionButton(Apply)->setEnabled(false);
}

void advancedDialog::changed()
{
    actionButton(Apply)->setEnabled(true);
}

#include "advancedDialog.moc"

