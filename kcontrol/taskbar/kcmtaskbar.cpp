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
#include <qtimer.h>
#include <qvaluelist.h>

#include <dcopclient.h>

#include <kaboutdata.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kdialog.h>
#include <kgenericfactory.h>
#include <kwin.h>

#include "kcmtaskbarui.h"
#include "taskbarsettings.h"

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

TaskbarConfig::TaskbarConfig(QWidget *parent, const char* name, const QStringList&)
  : KCModule(TaskBarFactory::instance(), parent, name)
{
    QVBoxLayout *layout = new QVBoxLayout(this, 0, KDialog::spacingHint());
    m_widget = new TaskbarConfigUI(this);
    layout->addWidget(m_widget);
    
    addConfig(TaskBarSettings::self(), m_widget);

    setQuickHelp(i18n("<h1>Taskbar</h1> You can configure the taskbar here."
                " This includes options such as whether or not the taskbar should show all"
                " windows at once or only those on the current desktop."
                " You can also configure whether or not the Window List button will be displayed."));

    QStringList list = i18nActionList();
    m_widget->kcfg_LeftButtonAction->insertStringList(list);
    m_widget->kcfg_MiddleButtonAction->insertStringList(list);
    m_widget->kcfg_RightButtonAction->insertStringList(list);
    m_widget->kcfg_GroupTasks->insertStringList(i18nGroupModeList());

    connect(m_widget->kcfg_GroupTasks, SIGNAL(activated(int)),
            this, SLOT(slotUpdateComboBox()));

    if (KWin::numberOfDesktops() < 2)
    {
        m_widget->kcfg_ShowAllWindows->hide();
        m_widget->kcfg_SortByDesktop->hide();
    }

    if (!QApplication::desktop()->isVirtualDesktop() ||
        QApplication::desktop()->numScreens() == 1) // No Ximerama
    {
        m_widget->kcfg_ShowCurrentScreenOnly->hide();
    }

    KAboutData *about = new KAboutData(I18N_NOOP("kcmtaskbar"),
                                       I18N_NOOP("KDE Taskbar Control Module"),
                                       0, 0, KAboutData::License_GPL,
                                       I18N_NOOP("(c) 2000 - 2001 Matthias Elter"));

    about->addAuthor("Matthias Elter", 0, "elter@kde.org");
    about->addCredit("Stefan Nikolaus", I18N_NOOP("KConfigXT conversion"),
                     "stefan.nikolaus@kdemail.net");
    setAboutData(about);

    load();
    QTimer::singleShot(0, this, SLOT(notChanged()));
}

void TaskbarConfig::slotUpdateComboBox()
{
    int pos = TaskBarSettings::ActivateRaiseOrMinimize;
    // If grouping is enabled, call "Activate, Raise or Iconify something else,
    // though the config key used is the same.
    if(m_widget->kcfg_GroupTasks->currentItem() != TaskBarSettings::GroupNever)
    {
        m_widget->kcfg_LeftButtonAction->changeItem(i18n("Cycle Through Windows"), pos);
        m_widget->kcfg_MiddleButtonAction->changeItem(i18n("Cycle Through Windows"), pos);
        m_widget->kcfg_RightButtonAction->changeItem(i18n("Cycle Through Windows"), pos);
    }
    else
    {
        QString action = i18nActionList()[pos];
        m_widget->kcfg_LeftButtonAction->changeItem(action,pos);
        m_widget->kcfg_MiddleButtonAction->changeItem(action,pos);
        m_widget->kcfg_RightButtonAction->changeItem(action,pos);
    }
}

void TaskbarConfig::load()
{
    KCModule::load();
    slotUpdateComboBox();
}

void TaskbarConfig::save()
{
    KCModule::save();
    
    QByteArray data;
    kapp->dcopClient()->emitDCOPSignal("kdeTaskBarConfigChanged()", data);
}

void TaskbarConfig::defaults()
{
    KCModule::defaults();
    slotUpdateComboBox();
}

void TaskbarConfig::notChanged() 
{
    emit changed(false);
}
