////////////////////////////////////////////////////////////////////////////////
//
// Class Name    : CErrorDialogDialog
// Author        : Craig Drummond
// Project       : K Font Installer (kfontinst-kcontrol)
// Creation Date : 02/05/2001
// Version       : $Revision$ $Date$
//
////////////////////////////////////////////////////////////////////////////////
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
////////////////////////////////////////////////////////////////////////////////
// (C) Craig Drummond, 2001
////////////////////////////////////////////////////////////////////////////////

#include "ErrorDialog.h"
#include <qgroupbox.h>

CErrorDialog::CErrorDialog(QWidget *parent, const char *name)
             : CErrorDialogData(parent, name, true)
{
}

CErrorDialog::~CErrorDialog()
{
}

void CErrorDialog::add(const QString &file, const QString &reason)
{
    new QListViewItem(itsListView, file, reason);
}

void CErrorDialog::open(const QString &str)
{
    itsGroupBox->setTitle(str);
    exec();
}
