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
 *   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */
#include <qlayout.h>
#include <qpushbutton.h>

#include <klocale.h>
#include <kglobal.h>
#include <kseparator.h>

#include "basictab.h"
#include "desktopfileeditor.h"
#include "desktopfileeditor.moc"

DesktopFileEditor::DesktopFileEditor( QWidget *parent, const char *name )
  : QWidget(parent, name)
{
    QGridLayout *layout = new QGridLayout(this, 3, 3, 2, 2);

    // setup tab
    _basicTab = new BasicTab(this);
    connect(_basicTab, SIGNAL(changed(bool)), SLOT(slotChanged(bool)));
    layout->addMultiCellWidget(_basicTab, 0, 0, 0, 2);

    // setup separator
    KSeparator *_separator = new KSeparator(KSeparator::HLine, this);
    layout->addMultiCellWidget(_separator, 1, 1, 0, 2);

    // setup buttons
    _apply = new QPushButton(i18n("&Apply"), this);
    _reset = new QPushButton(i18n("&Reset"), this);
    _apply->setEnabled(false);
    _reset->setEnabled(false);
    connect(_apply, SIGNAL(clicked()), SLOT(slotApply()));
    connect(_reset, SIGNAL(clicked()), SLOT(slotReset()));
    _desktopFileNeedsSave = false;
    layout->addWidget(_apply, 2, 1);
    layout->addWidget(_reset, 2, 2);
    layout->setColStretch(0, 9);
    layout->setColStretch(1, 3);
    layout->setColStretch(2, 3);
}

void DesktopFileEditor::setDesktopFile(const QString& desktopFile, const QString &name, bool isDeleted)
{
    _basicTab->setDesktopFile(desktopFile, name, isDeleted);
    _apply->setEnabled(false);
    _reset->setEnabled(false);
    _desktopFileNeedsSave = false;
}

void DesktopFileEditor::slotChanged( bool desktopFileNeedsSave )
{
    if( desktopFileNeedsSave )
        _desktopFileNeedsSave = true;
    _apply->setEnabled(true);
    _reset->setEnabled(true);
}

void DesktopFileEditor::slotApply()
{
    _basicTab->apply( _desktopFileNeedsSave );
    _apply->setEnabled(false);
    _reset->setEnabled(false);
    _desktopFileNeedsSave = false;
    emit changed(_basicTab->desktopFile());
}

void DesktopFileEditor::slotReset()
{
    _basicTab->reset();
    _apply->setEnabled(false);
    _reset->setEnabled(false);
    _desktopFileNeedsSave = false;
}

