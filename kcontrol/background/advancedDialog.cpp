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
#include "bgadvanced.h" 
#include "bgsettings.h"

AdvancedDialog::AdvancedDialog(KGlobalBackgroundSettings* settings,
                               QWidget* parent, char* name)
    : KDialogBase(KDialogBase::Plain,
                  i18n("Advanced Options"),
                  KDialogBase::Ok |
                  KDialogBase::Apply |
                  KDialogBase::Default |
                  KDialogBase::Cancel,
                  KDialogBase::Cancel,
                  parent,
                  name,
                  true, true),
     m_pGlobals(settings)
{
    connect(this, SIGNAL(applyClicked()),
            this, SLOT(save()));
    connect(this, SIGNAL(defaultClicked()),
            this, SLOT(defaults()));
    actionButton(Apply)->setEnabled(false);
    QFrame* page = plainPage();
    QVBoxLayout* layout = new QVBoxLayout(page);
    m_advancedWidget = new AdvancedOptions(page);
    layout->addWidget(m_advancedWidget);

    connect(m_advancedWidget->m_pCBLimit, SIGNAL(toggled(bool)),
            this, SLOT(changed()));
    connect(m_advancedWidget->m_pCacheBox, SIGNAL(valueChanged(int)),
            this, SLOT(changed()));
    load();
}

AdvancedDialog::~AdvancedDialog()
{
}

void AdvancedDialog::load()
{
    m_advancedWidget->m_pCBLimit->setChecked(m_pGlobals->limitCache());
    m_advancedWidget->m_pCacheBox->setValue(m_pGlobals->cacheSize());
    actionButton(Apply)->setEnabled(true);
}

void AdvancedDialog::save()
{
    m_pGlobals->setLimitCache(m_advancedWidget->m_pCBLimit->isChecked());
    m_pGlobals->setCacheSize(m_advancedWidget->m_pCacheBox->value());
    m_pGlobals->writeSettings();
    actionButton(Apply)->setEnabled(false);
}

void AdvancedDialog::changed()
{
    actionButton(Apply)->setEnabled(true);
}

void AdvancedDialog::defaults()
{
    m_advancedWidget->m_pCBLimit->setChecked(true);
    m_advancedWidget->m_pCacheBox->setValue(2048);
}

#include "advancedDialog.moc"

