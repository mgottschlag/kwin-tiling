#ifndef __ERRORDIALOG_H__
#define __ERRORDIALOG_H__

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

#include "ErrorDialogData.h"
#include <klistview.h>

class QString;

class CErrorDialog : public CErrorDialogData
{
    Q_OBJECT

    public:

    CErrorDialog(QWidget *parent = NULL, const char *name = NULL);

    virtual ~CErrorDialog();

    void clear() { itsListView->clear(); }
    void add(const QString &file, const QString &reason);
    void open(const QString &str);
};

#endif // __ERRORDIALOG_H__
