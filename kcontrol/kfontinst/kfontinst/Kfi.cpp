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
#include "Misc.h"
#include "XConfig.h"
#include <qapplication.h>
#include <qnamespace.h>
#include <qfile.h>
#include <fstream.h>
#include "kxftconfig.cpp" // CPD: Hack!!, this source file is located in kcontrol/fonts

const char * CKfi::constVersion = "0.12";
 
CKfiMainWidget * CKfi::create(QWidget *parent)
{
    CKfiGlobal::create(parent);
 
    if(CKfiGlobal::cfg().firstTime())
    {
        QApplication::setOverrideCursor(Qt::arrowCursor);
        CSettingsWizard *wiz=new CSettingsWizard(parent);
        wiz->exec();
        QApplication::restoreOverrideCursor();
        CKfiGlobal::cfg().configured();

        if(!CMisc::root())
        {
            if(CKfiGlobal::xcfg().ok() && CKfiGlobal::cfg().getModifiedDirs().count())
            {
                int i;

                for(i=0; i<CKfiGlobal::cfg().getModifiedDirs().count(); ++i)
                {
                    ofstream fontsDir(QFile::encodeName(CKfiGlobal::cfg().getModifiedDirs()[i]+"/fonts.dir"));

                    if(fontsDir)
                    {
                        fontsDir << 0 << endl;
                        fontsDir.close();
                    }
                    CKfiGlobal::xcfg().addPath(CKfiGlobal::cfg().getModifiedDirs()[i]);
                }
            }

            QStringList lst;

            CKfiGlobal::xcfg().writeConfig();
            CKfiGlobal::xcfg().getTTandT1Dirs(lst);

            if(lst.count())
            {
                KXftConfig            xft(KXftConfig::Dirs, CMisc::root());
                QStringList::Iterator it;

                xft.clearDirs();
                for(it=lst.begin(); it!=lst.end(); ++it)
                    xft.addDir(*it);

                xft.apply();
            }

            CKfiGlobal::cfg().clearModifiedDirs();
            CKfiGlobal::cfg().save();
        }
    }
    return new CKfiMainWidget(parent);
}

void CKfi::destroy()
{
    CKfiGlobal::destroy();
}
