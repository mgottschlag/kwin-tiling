/**************************************************************************

    config.cpp  - KPager config dialog
    Copyright (C) 2000  Antonio Larrosa Jimenez <larossa@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

    Send comments and bug fixes to larrosa@kde.org

***************************************************************************/

#include <QCheckBox>
#include <QRadioButton>
#include <q3buttongroup.h>
#include <QWidget>
#include <kvbox.h>
#include <khbox.h>
#include <QLayout>
//Added by qt3to4:
#include <QVBoxLayout>

#include <klocale.h>
#include <kconfig.h>
#include <kseparator.h>
#include <kdebug.h>
#include <kglobal.h>

#include "config.h"
#include "config.moc"
#include "desktop.h"
#include "kpager.h"

KPagerConfigDialog::KPagerConfigDialog (QWidget *parent)
 : KDialog( parent )
{
    setCaption( i18n("Configuration") );
    setButtons( Ok | Cancel );
    setObjectName( "configdialog" );
    setModal( true );
    showButtonSeparator( true );

    KVBox *box = new KVBox( this );
    m_chkWindowDragging=new QCheckBox(i18n("Enable window dragging"),box);
    (void ) new KSeparator( box );
    connect(m_chkWindowDragging, SIGNAL(toggled(bool)), this, SLOT(enableWindowDragging(bool)));

    KHBox *page = new KHBox( box );
    KVBox *lpage = new KVBox( page );
    setMainWidget(box);

    m_chkShowName=new QCheckBox(i18n("Show name"),lpage);
    connect(m_chkShowName, SIGNAL(toggled(bool)), this, SLOT(setShowName(bool)));
    m_chkShowNumber=new QCheckBox(i18n("Show number"),lpage);
    connect(m_chkShowNumber, SIGNAL(toggled(bool)), this, SLOT(setShowNumber(bool)));
    m_chkShowBackground=new QCheckBox(i18n("Show background"),lpage);
    connect(m_chkShowBackground, SIGNAL(toggled(bool)), this, SLOT(setShowBackground(bool)));
    m_chkShowWindows=new QCheckBox(i18n("Show windows"),lpage);
    connect(m_chkShowWindows, SIGNAL(toggled(bool)), this, SLOT(setShowWindows(bool)));

    m_grpWindowDrawMode=new Q3ButtonGroup(i18n("Type of Window"),page);
    m_grpWindowDrawMode->setExclusive(true);
    QVBoxLayout *vbox = new QVBoxLayout(m_grpWindowDrawMode);
    vbox->setMargin(KDialog::marginHint());
    vbox->setSpacing(KDialog::spacingHint());
    vbox->addSpacing(fontMetrics().lineSpacing());
    vbox->addWidget(new QRadioButton(i18n("Plain"),m_grpWindowDrawMode));
    vbox->addWidget(new QRadioButton(i18n("Icon"),m_grpWindowDrawMode));

    QRadioButton *rbpix = new QRadioButton(i18n("Pixmap"),m_grpWindowDrawMode);
//    rbpix->setEnabled(false);
    vbox->addWidget(rbpix);

    connect(m_grpWindowDrawMode, SIGNAL(clicked(int)), this, SLOT(setWindowDrawMode(int)));

    m_grpLayoutType=new Q3ButtonGroup(i18n("Layout"),page);
    m_grpLayoutType->setExclusive(true);
    vbox = new QVBoxLayout(m_grpLayoutType);
    vbox->setMargin(KDialog::marginHint());
    vbox->setSpacing(KDialog::spacingHint());
    vbox->addSpacing(fontMetrics().lineSpacing());
    vbox->addWidget(new QRadioButton(i18n("Classical"),m_grpLayoutType));
    vbox->addWidget(new QRadioButton(i18n("Horizontal"),m_grpLayoutType));
    vbox->addWidget(new QRadioButton(i18n("Vertical"),m_grpLayoutType));

    connect(m_grpLayoutType, SIGNAL(clicked(int)), this, SLOT(setLayout(int)));
    connect(this,SIGNAL(okClicked()),this,SLOT(slotOk()));
    loadConfiguration();
    setMinimumSize(360, 160);
}

void KPagerConfigDialog::setShowName(bool show)
{
    m_tmpShowName=show;
}

void KPagerConfigDialog::setShowNumber(bool show)
{
    m_tmpShowNumber=show;
}

void KPagerConfigDialog::setShowBackground(bool show)
{
    m_tmpShowBackground=show;
}

void KPagerConfigDialog::setShowWindows(bool show)
{
    m_tmpShowWindows=show;
}

void KPagerConfigDialog::enableWindowDragging(bool enable)
{
    m_tmpWindowDragging = enable;
}

void KPagerConfigDialog::setWindowDrawMode(int type)
{
    m_tmpWindowDrawMode=type;
}

void KPagerConfigDialog::setLayout(int layout)
{
    m_tmpLayoutType=layout;
}

void KPagerConfigDialog::loadConfiguration()
{
    m_chkShowName->setChecked(m_showName);
    m_chkShowNumber->setChecked(m_showNumber);
    m_chkShowBackground->setChecked(m_showBackground);
    m_chkShowWindows->setChecked(m_showWindows);
    m_grpWindowDrawMode->setButton(m_windowDrawMode);
    m_grpLayoutType->setButton(m_layoutType);
    m_chkWindowDragging->setChecked( m_windowDragging );
    m_tmpShowName=m_showName;
    m_tmpShowNumber=m_showNumber;
    m_tmpShowBackground=m_showBackground;
    m_tmpShowWindows=m_showWindows;
    m_tmpWindowDrawMode=m_windowDrawMode;
    m_tmpLayoutType=m_layoutType;
    m_tmpWindowDragging=m_windowDragging;
}

void KPagerConfigDialog::initConfiguration(void)
{
  KSharedConfig::Ptr cfg = KGlobal::config();
  cfg->setGroup("KPager");

  m_windowDrawMode=cfg->readEntry("windowDrawMode", int(Desktop::c_defWindowDrawMode));
  m_showName=cfg->readEntry("showName", Desktop::c_defShowName);
  m_showNumber=cfg->readEntry("showNumber", Desktop::c_defShowNumber);
  m_showBackground=cfg->readEntry("showBackground", Desktop::c_defShowBackground);
  m_showWindows=cfg->readEntry("showWindows", Desktop::c_defShowWindows);
  m_layoutType=cfg->readEntry("layoutType", int(KPager::c_defLayout));
  m_windowDragging=cfg->readEntry("windowDragging", true);
}

void KPagerConfigDialog::slotOk()
{
  m_showName=m_tmpShowName;
  m_showNumber=m_tmpShowNumber;
  m_showBackground=m_tmpShowBackground;
  m_showWindows=m_tmpShowWindows;
  m_windowDrawMode=m_tmpWindowDrawMode;
  m_layoutType=m_tmpLayoutType;
  m_windowDragging=m_tmpWindowDragging;
  accept();
}

bool KPagerConfigDialog::m_showName=Desktop::c_defShowName;
bool KPagerConfigDialog::m_showNumber=Desktop::c_defShowNumber;
bool KPagerConfigDialog::m_showBackground=Desktop::c_defShowBackground;
bool KPagerConfigDialog::m_showWindows=Desktop::c_defShowWindows;
bool KPagerConfigDialog::m_windowDragging=Desktop::c_defWindowDragging;
int  KPagerConfigDialog::m_windowDrawMode=Desktop::c_defWindowDrawMode;
int  KPagerConfigDialog::m_layoutType=KPager::c_defLayout;

