////////////////////////////////////////////////////////////////////////////////
//
// Class Name    : CKfiCmModule
// Author        : Craig Drummond
// Project       : K Font Installer (kfontinst-kcontrol)
// Creation Date : 07/05/2001
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

#include "KfiCmModule.h"
#include "KfiMainWidget.h"
#include "Kfi.h"
#include "KfiGlobal.h"
#include "Config.h"
#include <klocale.h>
#include <qtimer.h>
#include <qlayout.h>
#include <kmessagebox.h>

static bool firstTime=true;

static CKfiCmModule *cm=NULL;  // very hacky...

CKfiCmModule::CKfiCmModule(QWidget *parent, const char *name)
            : KCModule(parent, name)
{
    QGridLayout *topLayout=new QGridLayout(this);

    topLayout->setSpacing(0);
    topLayout->setMargin(-5);
    itsMainWidget=CKfi::create(this);
    topLayout->addWidget(itsMainWidget, 0, 0);
    setButtons(0);
    firstTime=true;
    cm=this;
}

CKfiCmModule::~CKfiCmModule()
{
    cm=NULL;
    CKfi::destroy();
}

void CKfiCmModule::madeChanges(bool m)
{
    if(NULL!=cm)
        cm->emitChanged(m);
}

void CKfiCmModule::emitChanged(bool m)
{
    emit changed(m);
}

void CKfiCmModule::scanFonts()
{
    if(CKfiGlobal::cfg().getModifiedDirs().count()>0 || CKfiGlobal::cfg().firstTime())
        emitChanged(true);

    itsMainWidget->scanFonts();
}

void CKfiCmModule::show()
{
    KCModule::show();

    if(firstTime)
    {
        firstTime=false;
        QTimer::singleShot(0, this, SLOT(scanFonts()));
    }
}

void CKfiCmModule::save()
{
    // This should onlt be called when the user selects to unload the module, and the system hasn't been configured...
    itsMainWidget->configureSystem();
}

QString CKfiCmModule::quickHelp() const
{
    return i18n("<h1>Font Installer</h1> This module allows you to"
                " install TrueType, Type1, Speedo, and Bitmap"
                " fonts. If you have StarOffice installed on your"
                " system, then this can also be configured.");
}

extern "C"
{
    KCModule *create_kfontinst(QWidget *parent, const char *name)
    {
        CKfiCmModule *mod=new CKfiCmModule(parent, name);

        return mod;
    };
}
#include "KfiCmModule.moc"
