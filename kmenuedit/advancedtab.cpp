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

#include <klocale.h>
#include <kstddirs.h>
#include <kglobal.h>
#include <kdialog.h>
#include <klineedit.h>
#include <kicondialog.h>
#include <kdesktopfile.h>

#include "advancedtab.h"
#include "advancedtab.moc"

AdvancedTab::AdvancedTab( QWidget *parent, const char *name )
  : QWidget(parent, name)
{
}

void AdvancedTab::setDesktopFile(const QString& desktopFile)
{
    _desktopFile = desktopFile;

    // KDesktopFile can also handle relative pathes, so we don't have
    // to make sure it's absolute.
    KDesktopFile df(desktopFile);
}

void AdvancedTab::apply()
{
}

void AdvancedTab::reset()
{
    if(_desktopFile != "")
	setDesktopFile(_desktopFile);
}
