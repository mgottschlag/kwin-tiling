////////////////////////////////////////////////////////////////////////////////
//
// Class Name    : CFontsWidget
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

#include "FontsWidget.h"
#include "KfiGlobal.h"
#include "FontEngine.h"
#include "Config.h"
#include "SysConfigurer.h"
#include "FontPreview.h"
#include "Misc.h"
#include "KfiCmModule.h"

#include <kprogress.h>
#include <kapplication.h>
#include <klocale.h>
#include <kbuttonbox.h>

#include <qbitmap.h>
#include <qlabel.h>
#include <qgroupbox.h>
#include <qpalette.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qsplitter.h>

#include <stdlib.h>

CFontsWidget::CFontsWidget(QWidget *parent, const char *)
            : QWidget(parent),
              itsSysConfigurer(NULL)
{
    QSplitter   *splitter=new QSplitter(this);
    QGroupBox   *fontsBox=new QGroupBox(splitter, "fontsBox"),
                *previewBox=new QGroupBox(splitter, "previewBox");

    itsProgressBox=new QGroupBox(fontsBox, "itsProgressBox");

    QGridLayout *fontsLayout=new QGridLayout(fontsBox, 3, 3, 11, 6),
                *progressLayout=new QGridLayout(itsProgressBox, 2, 2, 11, 6),
                *previewLayout=new QGridLayout(previewBox, 1, 1, 6, 6),
                *layout=new QGridLayout(this, 1, 1, 11, 6);
    KButtonBox  *buttonBox=new KButtonBox(fontsBox, Qt::Horizontal);
    QPalette    pal(itsProgressBox->palette());
    QColorGroup dis(pal.disabled());

    dis.setColor(QColorGroup::Text, pal.active().text());
    pal.setDisabled(dis);
    itsProgressBox->setPalette(pal);

    itsFontList=new CFontListWidget(fontsBox);
    itsFontList->setSizePolicy(QSizePolicy((QSizePolicy::SizeType)3, (QSizePolicy::SizeType)3, 0, 0, itsFontList->sizePolicy().hasHeightForWidth()));

    itsButtonAdd = buttonBox->addButton(i18n("Add..."));
    itsButtonRemove = buttonBox->addButton(i18n("Remove"));
    itsButtonDisable = buttonBox->addButton(i18n("Disable"));
    itsButtonEnable = buttonBox->addButton(i18n("Enable"));
    itsAdvancedCB = new QCheckBox( i18n("Advanced Mode"), fontsBox);
    itsAdvancedCB->setChecked(CKfiGlobal::cfg().getAdvancedMode());

    itsLabel=new QLabel(itsProgressBox, "itsLabel");
    itsLabel->setSizePolicy(QSizePolicy((QSizePolicy::SizeType)3, (QSizePolicy::SizeType)1, 0, 0, itsLabel->sizePolicy().hasHeightForWidth()));
    itsLabel->setText("Label...");
    itsProgress=new KProgress(itsProgressBox, "itsProgress");
    itsProgress->setMinimumSize(QSize(160, 0));
    progressLayout->addItem(new QSpacerItem(8, 8, QSizePolicy::Fixed, QSizePolicy::Fixed), 0, 0);
    progressLayout->addWidget(itsLabel, 1, 0);
    progressLayout->addWidget(itsProgress, 1, 1);
    itsProgressBox->hide();
    itsProgressBox->setTitle("Title...");

    fontsLayout->addMultiCellWidget(itsFontList, 0, 0, 0, 2);
    fontsLayout->addWidget(buttonBox, 1, 2);
    fontsLayout->addWidget(itsAdvancedCB, 1, 0);
    fontsLayout->addItem(new QSpacerItem(10, 10, QSizePolicy::Expanding, QSizePolicy::Minimum), 1, 1);
    fontsLayout->addMultiCellWidget(itsProgressBox, 2, 2, 0, 2);

    itsPreview = new CFontPreview(previewBox, "itsPreview");
    itsPreview->setSizePolicy(QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)3, 0, 0, itsPreview->sizePolicy().hasHeightForWidth()));
    itsPreview->setMinimumSize(QSize(96, 64));
    previewLayout->addWidget(itsPreview, 0, 0);

    layout->addWidget(splitter, 0, 0);

    connect(itsButtonAdd, SIGNAL(clicked()), itsFontList, SLOT(install()));
    connect(itsButtonRemove, SIGNAL(clicked()), itsFontList, SLOT(uninstall()));
    connect(itsButtonDisable, SIGNAL(clicked()), itsFontList, SLOT(disable()));
    connect(itsButtonEnable, SIGNAL(clicked()), itsFontList, SLOT(enable()));
    connect(itsAdvancedCB, SIGNAL(toggled(bool)), this, SLOT(setAdvanced(bool)));
    connect(itsFontList, SIGNAL(fontSelected(const QString &)), itsPreview, SLOT(showFont(const QString &)));
    connect(itsFontList, SIGNAL(configureSystem()), this, SLOT(configureSystem()));
    connect(itsFontList, SIGNAL(initProgress(const QString &, int)), SLOT(initProgress(const QString &, int)));
    connect(itsFontList, SIGNAL(progress(const QString &)), SLOT(progress(const QString &)));
    connect(itsFontList, SIGNAL(stopProgress()), SLOT(stopProgress()));
    connect(itsFontList, SIGNAL(madeChanges()), SLOT(flMadeChanges()));
}

CFontsWidget::~CFontsWidget()
{
    if(itsSysConfigurer)
        delete itsSysConfigurer;
}

void CFontsWidget::initProgress(const QString &title, int numSteps)
{
    emit progressActive(true);
    if(topLevelWidget())
        topLevelWidget()->setEnabled(false);
    itsProgress->setTotalSteps(numSteps);
    if(numSteps>0)
    {
        itsProgress->show();
        itsProgress->setProgress(0);
    }
    else
        itsProgress->hide();

    itsLabel->setText("");
    itsProgressBox->setTitle(title);
    itsProgressBox->show();
}

void CFontsWidget::progress(const QString &str)
{
    if(QString::null!=str)
        itsLabel->setText(" "+str);

    if(itsProgress->totalSteps()>0)
        itsProgress->advance(1);

    kapp->processEvents();

    //
    // If KControl has been closed by the user, then exit the application now.
    // This is needed because we're in the middle of updating the font list, and can't continue
    // as they've been deleted...
    if(!CKfiCmModule::instance())
        ::exit(0);
}

void CFontsWidget::stopProgress()
{
    if(itsProgress->totalSteps()>0)
    {
        itsProgress->setProgress(0);
        itsProgress->repaint();
    }

    itsProgressBox->hide();

    kapp->processEvents();

    if(topLevelWidget())
        topLevelWidget()->setEnabled(true);
    emit progressActive(false);
}

void CFontsWidget::configureSystem()
{
    kapp->processEvents();
    itsFontList->applyChanges();
    if(NULL==itsSysConfigurer)
    {
        itsSysConfigurer=new CSysConfigurer(this);

        connect(itsSysConfigurer, SIGNAL(initProgress(const QString &, int)), SLOT(initProgress(const QString &, int)));
        connect(itsSysConfigurer, SIGNAL(progress(const QString &)), SLOT(progress(const QString &)));
        connect(itsSysConfigurer, SIGNAL(stopProgress()), SLOT(stopProgress()));
        connect(itsSysConfigurer, SIGNAL(successful()), SLOT(systemConfigured()));
    }

    itsSysConfigurer->go();
}

void CFontsWidget::systemConfigured()
{
    CKfiGlobal::cfg().clearModifiedDirs();
}

void CFontsWidget::setAdvanced(bool b)
{
    CKfiGlobal::cfg().setAdvancedMode(b);
    itsFontList->setAdvanced(b);
}

void CFontsWidget::flMadeChanges()
{
    emit madeChanges();
}

#include "FontsWidget.moc"
