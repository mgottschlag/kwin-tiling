////////////////////////////////////////////////////////////////////////////////
//
// Class Name    : CKfi
// Author        : Craig Drummond
// Project       : K Font Installer (kfontinst-kcontrol)
// Creation Date : 01/05/2001
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

#include "Kfi.h"
#include "KfiMainWidget.h"
#include "KfiGlobal.h"
#include "Config.h"
#include "SettingsWizard.h"
#include <qapplication.h>
#include <qnamespace.h>

const QString CKfi::constVersion("0.10/CVS");
 
CKfiMainWidget * CKfi::create(QWidget *parent)
{
    CKfiGlobal::create(parent);
 
    if(CKfiGlobal::cfg().firstTime())
    {
        QApplication::setOverrideCursor(Qt::arrowCursor);
        CSettingsWizard *wiz=new CSettingsWizard(parent);
        wiz->exec();
        QApplication::restoreOverrideCursor();

        //
        // Mark TrueType and Type1 dirs as having ben modified...
        //
        CKfiGlobal::cfg().addModifiedDir(CKfiGlobal::cfg().getFontsDir()+CKfiGlobal::cfg().getTTSubDir());
        CKfiGlobal::cfg().addModifiedDir(CKfiGlobal::cfg().getFontsDir()+CKfiGlobal::cfg().getT1SubDir());
    }
    return new CKfiMainWidget(parent);
}

void CKfi::destroy()
{
    if(CKfiGlobal::cfg().firstTime())
        CKfiGlobal::cfg().configured();

    CKfiGlobal::destroy();
}
