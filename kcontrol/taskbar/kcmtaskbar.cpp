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
#include <qlayout.h>
#include <qwhatsthis.h>

#include <dcopclient.h>
#include <kapp.h>
#include <kconfig.h>
#include <kdialog.h>
#include <kglobal.h>
#include <klocale.h>

#include "kcmtaskbar.h"
#include "kcmtaskbar.moc"

extern "C"
{
  KCModule *create_taskbar(QWidget *parent, const char *name)
  {
    KGlobal::locale()->insertCatalogue("kcmtaskbar");
    return new TaskbarConfig(parent, name);
  };
}

TaskbarConfig::TaskbarConfig( QWidget *parent, const char* name )
  : KCModule (parent, name)
{
    QGroupBox *taskbar_group = new QGroupBox(i18n("Taskbar"), this);

    QVBoxLayout *vbox = new QVBoxLayout(taskbar_group, KDialog::marginHint(),
                                        KDialog::spacingHint());
    vbox->addSpacing(fontMetrics().lineSpacing());

    showAllCheck = new QCheckBox(i18n("&Show all windows"), taskbar_group);
    connect(showAllCheck, SIGNAL(clicked()), SLOT(configChanged()));
    QWhatsThis::add(showAllCheck, i18n("Check this option if you want"
                                       " the taskbar to display all of the existing windows at once.  By"
                                       " default, the taskbar will only show those windows that are on"
                                       " the current desktop."));

    m_pShowListBtn = new QCheckBox(i18n("Show windows list &button"), taskbar_group);
    connect(m_pShowListBtn, SIGNAL(clicked()), SLOT(configChanged()));
    QWhatsThis::add(m_pShowListBtn, i18n("Check this option if you want"
                                         " the taskbar to display a small popup which gives you easy access"
                                         " to all applications on other desktops and some further options."));

    vbox->addWidget(showAllCheck);
    vbox->addWidget(m_pShowListBtn);

    QVBoxLayout *top_layout = new QVBoxLayout(this, KDialog::marginHint(),
                                              KDialog::spacingHint());
    top_layout->addWidget(taskbar_group);
    top_layout->addStretch(1);

    load();
}

TaskbarConfig::~TaskbarConfig()
{
}

void TaskbarConfig::configChanged()
{
    emit changed(true);
}

void TaskbarConfig::load()
{
    KConfig *c = new KConfig("ktaskbarrc", false, false);
    { // group for the benefit of the group saver
        KConfigGroupSaver saver(c, "General");

        showAllCheck->setChecked(c->readBoolEntry("ShowAllWindows", false));
	m_pShowListBtn->setChecked(c->readBoolEntry("ShowWindowListBtn", true));
    }

    delete c;
    emit changed(false);
}

void TaskbarConfig::save()
{
    KConfig *c = new KConfig("ktaskbarrc", false, false);
    { // group for the benefit of the group saver
        KConfigGroupSaver saver(c, "General");

        c->writeEntry("ShowAllWindows", showAllCheck->isChecked());
	c->writeEntry("ShowWindowListBtn", m_pShowListBtn->isChecked());
	c->sync();
    }

    delete c;

    emit changed(false);

    // Tell kicker about the new config file.
    if (!kapp->dcopClient()->isAttached())
        kapp->dcopClient()->attach();
    QByteArray data;
    kapp->dcopClient()->send( "kicker", "Panel", "configure()", data );
}

void TaskbarConfig::defaults()
{
    showAllCheck->setChecked(false);
    m_pShowListBtn->setChecked(true);
    emit changed(true);
}

QString TaskbarConfig::quickHelp() const
{
    return i18n("<h1>Taskbar</h1> You can configure the taskbar here."
                " This includes options such as whether or not the taskbar should show all"
		" windows at once or only those on the current desktop."
                " You can also configure whether or not Windows list button will be displayed.");
}
