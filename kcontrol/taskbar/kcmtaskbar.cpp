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

#include <qcheckbox.h>
#include <qcombobox.h>
#include <qlayout.h>

#include <dcopclient.h>

#include <kaboutdata.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kdialog.h>
#include <kgenericfactory.h>
#include <kwin.h>

#include "kcmtaskbar.h"
#include "kcmtaskbar.moc"

typedef KGenericFactory<TaskbarConfig, QWidget > TaskBarFactory;
K_EXPORT_COMPONENT_FACTORY (kcm_taskbar, TaskBarFactory("kcmtaskbar") )

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

// These are the strings that are actually stored in the config file.
const QStringList& TaskbarConfig::groupModeList()
{
    static QStringList list(
            QStringList() << I18N_NOOP("Never") << I18N_NOOP("When Taskbar Full")
            << I18N_NOOP("Always"));
    return list;
}

// Get a translated version of the above string list.
QStringList TaskbarConfig::i18nGroupModeList()
{
   QStringList i18nList;
   for( QStringList::ConstIterator it = groupModeList().begin(); it != groupModeList().end(); ++it ) {
      i18nList << i18n((*it).latin1());
   }
   return i18nList;
}

// Translate from config entry name to enumeration
enum TaskbarConfig::GroupMode TaskbarConfig::groupMode(const QString& groupModeName )
{
   int index = groupModeList().findIndex( groupModeName );
   if( index != -1 ) return static_cast<GroupMode>(index);

   // Translate old entries
   if( groupModeName == "true" ) return GroupAlways;
   if( groupModeName == "false" ) return GroupNever;

   // Otherwise return the default.
   return GroupWhenFull;
}

// Translate from enum (or integer) to config entry name.
QString TaskbarConfig::groupMode( int groupModeNum )
{
   return groupModeList()[groupModeNum];
}

TaskbarConfig::TaskbarConfig( QWidget *parent, const char* name, const QStringList & )
  : KCModule (TaskBarFactory::instance(), parent, name)
{
    ui = new TaskbarConfigUI(this);

    setQuickHelp( i18n("<h1>Taskbar</h1> You can configure the taskbar here."
                " This includes options such as whether or not the taskbar should show all"
                " windows at once or only those on the current desktop."
                " You can also configure whether or not the Window List button will be displayed."));

    QVBoxLayout *vbox = new QVBoxLayout(this, 0, KDialog::spacingHint());
    vbox->addWidget(ui);
    connect(ui->showAllCheck, SIGNAL(clicked()), SLOT( changed()));
    connect(ui->showAllScreensCheck, SIGNAL(clicked()), SLOT( changed()));
    connect(ui->showListBtnCheck, SIGNAL(clicked()), SLOT( changed()));
    connect(ui->sortCheck, SIGNAL(clicked()), SLOT( changed()));
    connect(ui->iconCheck, SIGNAL(clicked()), SLOT( changed()));
    connect(ui->iconifiedCheck, SIGNAL(clicked()), SLOT( changed()));

    QStringList list = i18nActionList();
    ui->leftButtonComboBox->insertStringList( list );
    ui->middleButtonComboBox->insertStringList( list );
    ui->rightButtonComboBox->insertStringList( list );
    ui->groupComboBox->insertStringList( i18nGroupModeList() );

    connect(ui->leftButtonComboBox, SIGNAL(activated(int)), SLOT( changed()));
    connect(ui->middleButtonComboBox, SIGNAL(activated(int)), SLOT( changed()));
    connect(ui->rightButtonComboBox, SIGNAL(activated(int)), SLOT( changed()));
    connect(ui->groupComboBox, SIGNAL(activated(int)), SLOT( changed()));
    connect(ui->groupComboBox, SIGNAL(activated(int)), SLOT(slotUpdateComboBox()));

    if (KWin::numberOfDesktops() < 2)
    {
        ui->showAllCheck->hide();
        ui->sortCheck->hide();
    }

    if (!QApplication::desktop()->isVirtualDesktop() ||
        QApplication::desktop()->numScreens()==1 ) // No Ximerama
    {
           ui->showAllScreensCheck->hide();
    }

    KAboutData *about =
    new KAboutData(I18N_NOOP("kcmtaskbar"), I18N_NOOP("KDE Taskbar Control Module"),
                  0, 0, KAboutData::License_GPL,
                  I18N_NOOP("(c) 2000 - 2001 Matthias Elter"));

    about->addAuthor("Matthias Elter", 0, "elter@kde.org");
    setAboutData(about);

    load();
}

void TaskbarConfig::slotUpdateComboBox()
{
    // If grouping is enabled, call "Activate, Raise or Iconify something else,
    // though the config key used is the same.
    if( ui->groupComboBox->currentItem() != GroupNever ) {
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
    KConfig *c = new KConfig("ktaskbarrc", true, false);
    { // group for the benefit of the group saver
        KConfigGroupSaver saver(c, "General");

        ui->showAllCheck->setChecked(c->readBoolEntry("ShowAllWindows", true));
        ui->showAllScreensCheck->setChecked(!c->readBoolEntry("ShowCurrentScreenOnly", false));
        ui->showListBtnCheck->setChecked(c->readBoolEntry("ShowWindowListBtn", false));
        ui->sortCheck->setChecked(c->readBoolEntry("SortByDesktop", true));
        ui->iconCheck->setChecked(c->readBoolEntry("ShowIcon", true));
        ui->iconifiedCheck->setChecked(c->readBoolEntry("ShowOnlyIconified", false));
        ui->leftButtonComboBox->setCurrentItem(buttonAction(LeftButton, c->readEntry("LeftButtonAction")));
        ui->middleButtonComboBox->setCurrentItem(buttonAction(MidButton, c->readEntry("MiddleButtonAction")));
        ui->rightButtonComboBox->setCurrentItem(buttonAction(RightButton, c->readEntry("RightButtonAction")));
        ui->groupComboBox->setCurrentItem(groupMode(c->readEntry("GroupTasks")));
    }

    delete c;
    slotUpdateComboBox();
}

void TaskbarConfig::save()
{
    KConfig *c = new KConfig("ktaskbarrc", false, false);
    { // group for the benefit of the group saver
        KConfigGroupSaver saver(c, "General");

        c->writeEntry("ShowAllWindows", ui->showAllCheck->isChecked());
        c->writeEntry("ShowCurrentScreenOnly", !ui->showAllScreensCheck->isChecked());
        c->writeEntry("ShowWindowListBtn", ui->showListBtnCheck->isChecked());
        c->writeEntry("SortByDesktop", ui->sortCheck->isChecked());
        c->writeEntry("ShowIcon", ui->iconCheck->isChecked());
        c->writeEntry("ShowOnlyIconified", ui->iconifiedCheck->isChecked());
        c->writeEntry("LeftButtonAction", buttonAction(ui->leftButtonComboBox->currentItem()));
        c->writeEntry("MiddleButtonAction", buttonAction(ui->middleButtonComboBox->currentItem()));
        c->writeEntry("RightButtonAction", buttonAction(ui->rightButtonComboBox->currentItem()));
        c->writeEntry("GroupTasks", groupMode(ui->groupComboBox->currentItem()));
        c->sync();
    }

    delete c;

    QByteArray data;
    kapp->dcopClient()->emitDCOPSignal("kdeTaskBarConfigChanged()", data);
}

void TaskbarConfig::defaults()
{
    ui->showAllCheck->setChecked(true);
    ui->showAllScreensCheck->setChecked(true);
    ui->showListBtnCheck->setChecked(false);
    ui->sortCheck->setChecked(true);
    ui->iconCheck->setChecked(true);
    ui->iconifiedCheck->setChecked(false);
    ui->leftButtonComboBox->setCurrentItem( buttonAction( LeftButton ) );
    ui->middleButtonComboBox->setCurrentItem( buttonAction( MidButton ) );
    ui->rightButtonComboBox->setCurrentItem( buttonAction( RightButton ) );
    ui->groupComboBox->setCurrentItem( groupMode() );
    slotUpdateComboBox();
}

