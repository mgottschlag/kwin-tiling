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
#include "Misc.h"
#include <klocale.h>
#include <qtimer.h>
#include <qlayout.h>
#include <kmessagebox.h>
#include <kaboutdata.h>

static bool firstTime=true;

CKfiCmModule * CKfiCmModule::theirInstance=NULL;

CKfiCmModule::CKfiCmModule(QWidget *parent, const char *name)
            : KCModule(parent, name),
              itsAboutData(NULL)
{
    QGridLayout *topLayout=new QGridLayout(this);

    topLayout->setSpacing(0);
    topLayout->setMargin(-5);
    itsMainWidget=CKfi::create(this);
    topLayout->addWidget(itsMainWidget, 0, 0);
    setButtons(0);
    setUseRootOnlyMsg(false);
    firstTime=true;
    theirInstance=this;
}

CKfiCmModule::~CKfiCmModule()
{
    theirInstance=NULL;
    CKfi::destroy();

    if(itsAboutData)
        delete itsAboutData;
}

const KAboutData * CKfiCmModule::aboutData() const
{
    if(!itsAboutData)
    {
        CKfiCmModule *that = const_cast<CKfiCmModule *>(this);

        that->itsAboutData=new KAboutData("kcmfontinst",
                                          I18N_NOOP("KFontinst"),
                                          CKfi::constVersion,
                                          I18N_NOOP("Font installer and previewer"),
                                          KAboutData::License_GPL,
                                          I18N_NOOP("(C) Craig Drummond, 2000, 2001"),
                                          I18N_NOOP("(TQMM, PS - MBFM y CGD)"));

        that->itsAboutData->addAuthor("Craig Drummond", "Developer and maintainer", "cpdrummond@uklinux.net");
        that->itsAboutData->addCredit("Michael Davis", I18N_NOOP("StarOffice xprinter.prolog patch"));
#ifdef HAVE_XFT
        that->itsAboutData->addCredit("Keith Packard", I18N_NOOP("XftConfig parser"));
#endif
    }

    return itsAboutData;
}

void CKfiCmModule::madeChanges(bool m)
{
    if(NULL!=theirInstance)
        theirInstance->emitChanged(m);
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
    QString help(i18n("<h1>Font Installer</h1><p> This module allows you to"
                      " install TrueType, Type1, Speedo, and Bitmap"
                      " fonts. If you have StarOffice installed on your"
                      " system, then this can also be configured.</p>")),
            rootHelp(i18n("<p><b>NOTE:</b> As you are not logged in as \"root\", any"
                          " fonts installed will only be available to you. To install"
                          " fonts system wide, then use the \"Aministrator Mode\""
                          " button to run this module as \"root\".</p>"));

    return CMisc::root() ? help : help+rootHelp;
}

extern "C"
{
    KCModule * create_fontinst(QWidget *parent, const char *name)
    {
        return new CKfiCmModule(parent, name);
    };
}
#include "KfiCmModule.moc"
