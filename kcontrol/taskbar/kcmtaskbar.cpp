/*
 *  Copyright (c) 2000 Kurt Granroth <granroth@kde.org>
 *  Copyright (c) 2000 Matthias Elter <elter@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 */

#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qgroupbox.h>
#include <qcombobox.h>
#include <qlayout.h>
#include <qwhatsthis.h>

#include <dcopclient.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kdialog.h>
#include <kglobal.h>
#include <klocale.h>
#include <kgenericfactory.h>

#include "kcmtaskbar.h"
#include <kaboutdata.h>
#include "kcmtaskbar.moc"

typedef KGenericFactory<TaskbarConfig, QWidget > TaskBarFactory;
K_EXPORT_COMPONENT_FACTORY (libkcm_taskbar, TaskBarFactory("kcmtaskbar") );

// These are the strings that are actually stored in the config file.
const QStringList& TaskbarConfig::actionList()
{
    static QStringList list(
            QStringList() << I18N_NOOP("Show Task List") << I18N_NOOP("Show Operations Menu")
            << I18N_NOOP("Activate, Raise or Minimize Task")
            << I18N_NOOP("Activate Task") << I18N_NOOP("Raise Task")
            << I18N_NOOP("Lower Task") << I18N_NOOP("Minimize Task") );
    return list;
}

// Get a translated version of the above string list.
QStringList TaskbarConfig::i18nActionList()
{
   QStringList i18nList;
   for( QStringList::ConstIterator it = actionList().begin(); it != actionList().end(); ++it ) {
      i18nList << i18n((*it).latin1());
   }
   return i18nList;
}

// Translate from config entry name to enumeration
enum TaskbarConfig::Action TaskbarConfig::buttonAction( ButtonState button, const QString& actionName )
{
   int index = actionList().findIndex( actionName );
   if( index != -1 ) return static_cast<Action>(index);

   // Otherwise return the default.
   switch( button ) {
   case MidButton: return ActivateRaiseOrIconify;
   case RightButton: return ShowOperationsMenu;
   case LeftButton: // fall through
   default: return ShowTaskList;
   }
}

// Translate from enum (or integer) to config entry name.
QString TaskbarConfig::buttonAction( int action )
{
   return actionList()[action];
}

TaskbarConfig::TaskbarConfig( QWidget *parent, const char* name, const QStringList & )
  : KCModule (TaskBarFactory::instance(), parent, name)
{
    ui = new TaskbarConfigUI(this);

    QVBoxLayout *vbox = new QVBoxLayout(this, KDialog::marginHint(),
                                        KDialog::spacingHint());
    vbox->addWidget(ui);
    connect(ui->showAllCheck, SIGNAL(clicked()), SLOT(configChanged()));
    connect(ui->showListBtnCheck, SIGNAL(clicked()), SLOT(configChanged()));
    connect(ui->groupCheck, SIGNAL(clicked()), SLOT(configChanged()));
    connect(ui->sortCheck, SIGNAL(clicked()), SLOT(configChanged()));
    connect(ui->iconCheck, SIGNAL(clicked()), SLOT(configChanged()));

    QStringList list = i18nActionList();
    ui->leftButtonComboBox->insertStringList( list );
    ui->middleButtonComboBox->insertStringList( list );
    ui->rightButtonComboBox->insertStringList( list );
    connect(ui->leftButtonComboBox, SIGNAL(activated(int)), SLOT(configChanged()));
    connect(ui->middleButtonComboBox, SIGNAL(activated(int)), SLOT(configChanged()));
    connect(ui->rightButtonComboBox, SIGNAL(activated(int)), SLOT(configChanged()));

    connect(ui->groupCheck, SIGNAL(clicked()), SLOT(slotUpdateComboBox()));

    load();
}

TaskbarConfig::~TaskbarConfig()
{
}

void TaskbarConfig::configChanged()
{
    emit changed(true);
}

void TaskbarConfig::slotUpdateComboBox()
{
    // If grouping is enabled, call "Activate, Raise or Iconify something else,
    // though the config key used is the same.
    if( ui->groupCheck->isChecked() ) {
	ui->leftButtonComboBox->changeItem(i18n("Cycle Through Windows"),ActivateRaiseOrIconify);
	ui->middleButtonComboBox->changeItem(i18n("Cycle Through Windows"),ActivateRaiseOrIconify);
	ui->rightButtonComboBox->changeItem(i18n("Cycle Through Windows"),ActivateRaiseOrIconify);
    } else {
	QString action = i18nActionList()[ActivateRaiseOrIconify];
	ui->leftButtonComboBox->changeItem(action,ActivateRaiseOrIconify);
	ui->middleButtonComboBox->changeItem(action,ActivateRaiseOrIconify);
	ui->rightButtonComboBox->changeItem(action,ActivateRaiseOrIconify);
    }
}

void TaskbarConfig::load()
{
    KConfig *c = new KConfig("ktaskbarrc", false, false);
    { // group for the benefit of the group saver
        KConfigGroupSaver saver(c, "General");

        ui->showAllCheck->setChecked(c->readBoolEntry("ShowAllWindows", true));
        ui->showListBtnCheck->setChecked(c->readBoolEntry("ShowWindowListBtn", true));
        ui->groupCheck->setChecked(c->readBoolEntry("GroupTasks", true));
	ui->sortCheck->setChecked(c->readBoolEntry("SortByDesktop", true));
	ui->iconCheck->setChecked(c->readBoolEntry("ShowIcon", true));
	ui->leftButtonComboBox->setCurrentItem(buttonAction(LeftButton, c->readEntry("LeftButtonAction")));
	ui->middleButtonComboBox->setCurrentItem(buttonAction(MidButton, c->readEntry("MiddleButtonAction")));
	ui->rightButtonComboBox->setCurrentItem(buttonAction(RightButton, c->readEntry("RightButtonAction")));
    }

    delete c;
    emit changed(false);
    slotUpdateComboBox();
}

void TaskbarConfig::save()
{
    KConfig *c = new KConfig("ktaskbarrc", false, false);
    { // group for the benefit of the group saver
        KConfigGroupSaver saver(c, "General");

        c->writeEntry("ShowAllWindows", ui->showAllCheck->isChecked());
        c->writeEntry("ShowWindowListBtn", ui->showListBtnCheck->isChecked());
        c->writeEntry("GroupTasks", ui->groupCheck->isChecked());
        c->writeEntry("SortByDesktop", ui->sortCheck->isChecked());
        c->writeEntry("ShowIcon", ui->iconCheck->isChecked());
        c->writeEntry("LeftButtonAction", buttonAction(ui->leftButtonComboBox->currentItem()));
        c->writeEntry("MiddleButtonAction", buttonAction(ui->middleButtonComboBox->currentItem()));
        c->writeEntry("RightButtonAction", buttonAction(ui->rightButtonComboBox->currentItem()));
        c->sync();
    }

    delete c;

    emit changed(false);

    // Tell kicker about the new config file.
    if (!kapp->dcopClient()->isAttached())
        kapp->dcopClient()->attach();
    QByteArray data;
    kapp->dcopClient()->send( "kicker", "Panel", "restart()", data );
}

void TaskbarConfig::defaults()
{
    ui->showAllCheck->setChecked(true);
    ui->showListBtnCheck->setChecked(true);
    ui->groupCheck->setChecked(true);
    ui->sortCheck->setChecked(true);
    ui->iconCheck->setChecked(true);
    ui->leftButtonComboBox->setCurrentItem( buttonAction( LeftButton ) );
    ui->middleButtonComboBox->setCurrentItem( buttonAction( MidButton ) );
    ui->rightButtonComboBox->setCurrentItem( buttonAction( RightButton ) );
    emit changed(true);
    slotUpdateComboBox();
}

QString TaskbarConfig::quickHelp() const
{
    return i18n("<h1>Taskbar</h1> You can configure the taskbar here."
                " This includes options such as whether or not the taskbar should show all"
		" windows at once or only those on the current desktop."
                " You can also configure whether or not the Window List button will be displayed.");
}

const KAboutData* TaskbarConfig::aboutData() const
{

    KAboutData *about =
    new KAboutData(I18N_NOOP("kcmtaskbar"), I18N_NOOP("KDE Taskbar Control Module"),
                  0, 0, KAboutData::License_GPL,
                  I18N_NOOP("(c) 2000 - 2001 Matthias Elter"));

    about->addAuthor("Matthias Elter", 0, "elter@kde.org");

    return about;
}
