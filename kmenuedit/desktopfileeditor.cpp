/*
 *   Copyright (C) 2000 Matthias Elter <elter@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include <qlayout.h>
#include <qframe.h>
#include <qpushbutton.h>
#include <qtabwidget.h>

#include <klocale.h>
#include <kglobal.h>

#include "basictab.h"
#include "advancedtab.h"
#include "desktopfileeditor.h"
#include "desktopfileeditor.moc"

DesktopFileEditor::DesktopFileEditor( QWidget *parent, const char *name )
  : QWidget(parent, name)
{
    QGridLayout *layout = new QGridLayout(this, 3, 3, 2, 2);

    // setup tabs
    _tabs = new QTabWidget(this);
    _basicTab = new BasicTab(this);
    _advancedTab = new AdvancedTab(this);
    _tabs->addTab(_basicTab, i18n("General"));
    _tabs->addTab(_advancedTab, i18n("Advanced"));
    connect(_basicTab, SIGNAL(changed()), SLOT(slotChanged()));
    connect(_advancedTab, SIGNAL(changed()), SLOT(slotChanged()));
    layout->addMultiCellWidget(_tabs, 0, 0, 0, 2);

    // setup separator
    _separator = new QFrame(this);
    _separator->setFrameStyle(QFrame::HLine | QFrame::Sunken);
    layout->addMultiCellWidget(_separator, 1, 1, 0, 2);

    // setup buttons
    _apply = new QPushButton(i18n("&Apply"), this);
    _reset = new QPushButton(i18n("&Reset"), this);
    _apply->setEnabled(false);
    _reset->setEnabled(false);
    connect(_apply, SIGNAL(clicked()), SLOT(slotApply()));
    connect(_reset, SIGNAL(clicked()), SLOT(slotReset()));
    layout->addWidget(_reset, 2, 1);
    layout->addWidget(_apply, 2, 2);
    layout->setColStretch(0, 9);
    layout->setColStretch(1, 3);
    layout->setColStretch(2, 3);
}

void DesktopFileEditor::setDesktopFile(const QString& desktopFile)
{
    _basicTab->setDesktopFile(desktopFile);
    _advancedTab->setDesktopFile(desktopFile);
    _apply->setEnabled(false);
    _reset->setEnabled(false);
}

void DesktopFileEditor::slotChanged()
{
    _apply->setEnabled(true);
    _reset->setEnabled(true);
}

void DesktopFileEditor::slotApply()
{
    _basicTab->apply();
    _advancedTab->apply();
    _apply->setEnabled(false);
    _reset->setEnabled(false);
    emit changed();
}

void DesktopFileEditor::slotReset()
{
    _basicTab->reset();
    _advancedTab->reset();
    _apply->setEnabled(false);
    _reset->setEnabled(false);
}
