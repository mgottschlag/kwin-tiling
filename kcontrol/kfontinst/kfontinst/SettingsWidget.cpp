////////////////////////////////////////////////////////////////////////////////
//
// Class Name    : CSettingsWidget
// Author        : Craig Drummond
// Project       : K Font Installer (kfontinst-kcontrol)
// Creation Date : 29/04/2001
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

#include "SettingsWidget.h"
#include "DirSettingsWidget.h"
#include "DisplaySettingsWidget.h"
#include "InstUninstSettingsWidget.h"
#include "StarOfficeSettingsWidget.h"
#include "SysCfgSettingsWidget.h"
#include <klocale.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kjanuswidget.h>
#include <qframe.h>
#include <qsizepolicy.h>
#include <qlayout.h>

CSettingsWidget::CSettingsWidget(QWidget *parent, const char *name)
               : QWidget(parent, name) //, KJanusWidget::IconList)
{
    QGridLayout *topLayout=new QGridLayout(this);
    QFrame      *frame=NULL;
    QVBoxLayout *layout=NULL;

    topLayout->setSpacing(6);
    topLayout->setMargin(11);

    KJanusWidget *janus=new KJanusWidget(this, "JanusWidget", KJanusWidget::IconList);

    topLayout->addWidget(janus, 0, 0); 

    // Appearance page...
    frame=janus->addPage(i18n("Appearance"), i18n("Customize the look & operation"),
                         KGlobal::iconLoader()->loadIcon("appearance", KIcon::Desktop));
    layout=new QVBoxLayout(frame, 0);

    CDisplaySettingsWidget *disp=new CDisplaySettingsWidget(frame);

    layout->addWidget(disp);

    // Folders page...
    frame=janus->addPage(i18n("Folders & Files"), i18n("Set folders and files"),
                         KGlobal::iconLoader()->loadIcon("folder", KIcon::Desktop));
    layout=new QVBoxLayout(frame, 0);
 
    CDirSettingsWidget *dirs=new CDirSettingsWidget(frame);
 
    layout->addWidget(dirs);

    // Install page...
    frame=janus->addPage(i18n("Install/Uninstall"), i18n("Configure the install and uninstall operations"),
                         KGlobal::iconLoader()->loadIcon("editcopy", KIcon::Desktop));
    layout=new QVBoxLayout(frame, 0);
 
    CInstUninstSettingsWidget *inst=new CInstUninstSettingsWidget(frame);
 
    layout->addWidget(inst);

    // StarOffice page...
    KGlobal::iconLoader()->addAppDir("kcmfontinst");
    frame=janus->addPage(i18n("StarOffice"), i18n("StarOffice configuration"),
                         KGlobal::iconLoader()->loadIcon("kcmfontinst_star_office", KIcon::User));
    layout=new QVBoxLayout(frame, 0);
 
    CStarOfficeSettingsWidget *so=new CStarOfficeSettingsWidget(frame);
 
    layout->addWidget(so);

    // System config page...
    frame=janus->addPage(i18n("System"), i18n("System configuration options"),
                         KGlobal::iconLoader()->loadIcon("xapp", KIcon::Desktop));
    layout=new QVBoxLayout(frame, 0);
 
    CSysCfgSettingsWidget *sys=new CSysCfgSettingsWidget(frame);
 
    layout->addWidget(sys);

    connect(so, SIGNAL(cfgSelected()), sys, SLOT(enableAfmGeneration()));
    connect(sys, SIGNAL(afmGenerationDeselected()), so, SLOT(disable()));
    connect(dirs, SIGNAL(encodingsDirChanged()), sys, SLOT(scanEncodings()));
}
#include "SettingsWidget.moc"
