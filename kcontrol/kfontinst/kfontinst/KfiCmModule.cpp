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
#include <kapplication.h>

#if KDE_VERSION >= 292
#include <kgenericfactory.h>

typedef KGenericFactory<CKfiCmModule, QWidget> FontInstallFactory;
K_EXPORT_COMPONENT_FACTORY( kcm_fontinst, FontInstallFactory );
#endif

static bool firstTime=true;

CKfiCmModule * CKfiCmModule::theirInstance=NULL;

CKfiCmModule::CKfiCmModule(QWidget *parent, const char *name, const QStringList&)
            : KCModule(parent, "fontinst"),
              itsAboutData(NULL)
{
    QGridLayout *topLayout=new QGridLayout(this);

    topLayout->setSpacing(0);
    topLayout->setMargin(-5);
    itsMainWidget=CKfi::create(this);
    topLayout->addWidget(itsMainWidget, 0, 0);
    connect(itsMainWidget, SIGNAL(madeChanges()), SLOT(emitChanged()));
    setButtons(Apply);
#if KDE_VERSION >= 291
    setUseRootOnlyMsg(false);
#endif
    firstTime=true;
    theirInstance=this;
}

CKfiCmModule::~CKfiCmModule()
{
    itsMainWidget->storeSettings();
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
                                          I18N_NOOP("KDE Font Installer"),
                                          CKfi::constVersion, 0,
                                          KAboutData::License_GPL,
                                          I18N_NOOP("(C) Craig Drummond, 2000 - 2002"),
                                          I18N_NOOP("(TQMM, PS - MBFM y CGD)"));

        that->itsAboutData->addAuthor("Craig Drummond", "Developer and maintainer", "Craig.Drummond@gmx.net");
        that->itsAboutData->addCredit("Michael Davis", I18N_NOOP("StarOffice xprinter.prolog patch"));
    }

    return itsAboutData;
}

void CKfiCmModule::emitChanged()
{
    emit changed(true);
}

void CKfiCmModule::scanFonts()
{
    if(CKfiGlobal::cfg().getModifiedDirs().count()>0 || CKfiGlobal::cfg().firstTime())
        emit changed(true);

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

void CKfiCmModule::load()
{
    itsMainWidget->reset();
}

void CKfiCmModule::save()
{
    itsMainWidget->configureSystem();
    emit changed(false);
}

QString CKfiCmModule::quickHelp() const
{
    QString help(i18n("<h1>Font Installer</h1><p> This module allows you to"
                      " install TrueType, Type1, Speedo, and Bitmap"
                      " fonts. If you have StarOffice installed on your"
                      " system, this can also be configured.</p>")),
            rootHelp(i18n("<p><b>NOTE:</b> As you are not logged in as \"root\", any"
                          " fonts installed will only be available to you. To install"
                          " fonts system wide, use the \"Administrator Mode\""
                          " button to run this module as \"root\".</p>"));

    return CMisc::root() ? help : help+rootHelp;
}

#if KDE_VERSION <= 291
extern "C"
{
    KCModule * create_fontinst(QWidget *parent, const char *name)
    {
        return new CKfiCmModule(parent, name);
    };
}
#endif

#include "KfiCmModule.moc"
