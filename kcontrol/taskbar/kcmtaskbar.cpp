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
#include <qradiobutton.h>
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

    // create windows list controls
    QButtonGroup *winlist_group = new QButtonGroup(i18n("Windows list"), this);
    winlist_group->setRadioButtonExclusive(true);
    connect(winlist_group, SIGNAL(clicked(int)), SLOT(windowListClicked()));

    vbox = new QVBoxLayout(winlist_group, KDialog::marginHint(),
                           KDialog::spacingHint());
    vbox->addSpacing(fontMetrics().lineSpacing());

    m_pAllWindows = new QRadioButton( i18n("Display &full windows list"), winlist_group);
    QWhatsThis::add(m_pAllWindows, i18n("Check this option if you want"
                                        " the Windows list to include all of the existing windows at once."));
    m_pCurrent = new QRadioButton( i18n("Display c&urrent desktop list"), winlist_group);
    QWhatsThis::add(m_pCurrent, i18n("Check this option if you want"
                                     " the Windows list to display windows from current desktop only."));
    vbox->addWidget(m_pAllWindows);
    vbox->addWidget(m_pCurrent);

    // create list order controls
    QButtonGroup *listorder_group = new QButtonGroup(i18n("List order"), this);
    listorder_group->setRadioButtonExclusive(true);
    connect(listorder_group, SIGNAL(clicked(int)), SLOT(configChanged()));

    vbox = new QVBoxLayout(listorder_group, KDialog::marginHint(),
                           KDialog::spacingHint());
    vbox->addSpacing(fontMetrics().lineSpacing());

    m_pName = new QRadioButton( i18n("Sort by &name"), listorder_group);
    QWhatsThis::add(m_pName, i18n("Check this option if you want"
                                  " the Windows list to be sorted by name."));
    m_pLastUse = new QRadioButton( i18n("Sort by &last use"), listorder_group);
    QWhatsThis::add(m_pLastUse, i18n("Check this option if you want"
                                     " the Windows list to be sorted by last use."));
    m_pDesktop = new QRadioButton( i18n("Sort by deskto&p"), listorder_group);
    QWhatsThis::add(m_pDesktop, i18n("Check this option if you want"
                                     " the Windows list to be sorted by desktop."));
    vbox->addWidget(m_pName);
    vbox->addWidget(m_pLastUse);
    vbox->addWidget(m_pDesktop);

    QVBoxLayout *top_layout = new QVBoxLayout(this, KDialog::marginHint(),
                                              KDialog::spacingHint());
    top_layout->addWidget(taskbar_group);
    top_layout->addWidget(winlist_group);
    top_layout->addWidget(listorder_group);
    top_layout->addStretch(1);

    load();
}

TaskbarConfig::~TaskbarConfig()
{
}

void TaskbarConfig::windowListClicked()
{
    if (m_pAllWindows->isChecked())
        m_pDesktop->setEnabled(true);
    else {
        if (m_pDesktop->isChecked())
            m_pName->setChecked(true);
        m_pDesktop->setEnabled(false);
    }
    emit changed(true);
}

void TaskbarConfig::configChanged()
{
    emit changed(true);
}

void TaskbarConfig::load()
{
    KConfig *c = new KConfig("ktaskbarappletrc", false, false);
    { // group for the benefit of the group saver
        KConfigGroupSaver saver(c, "General");

        showAllCheck->setChecked(c->readBoolEntry("ShowAllWindows", false));

	m_pShowListBtn->setChecked(c->readBoolEntry("ShowWindowListBtn", true));
	bool bFullVsCurrent = c->readBoolEntry("FullVsCurrent", true);
	if (bFullVsCurrent)
            m_pAllWindows->setChecked(true);
	else
            m_pCurrent->setChecked(true);

	QString strOrder(c->readEntry("ListOrder", "desktop"));

	if (strOrder == "desktop" && bFullVsCurrent)
            m_pDesktop->setChecked(true);
	else if (strOrder == "lastuse")
            m_pLastUse->setChecked(true);
	else
            m_pName->setChecked(true);

	if (!bFullVsCurrent)
            m_pDesktop->setEnabled(false);
    }

    delete c;
    emit changed(false);
}

void TaskbarConfig::save()
{
    KConfig *c = new KConfig("ktaskbarappletrc", false, false);
    { // group for the benefit of the group saver
        KConfigGroupSaver saver(c, "General");

        c->writeEntry("ShowAllWindows", showAllCheck->isChecked());

	c->writeEntry("ShowWindowListBtn", m_pShowListBtn->isChecked());
	c->writeEntry("FullVsCurrent", m_pAllWindows->isChecked());
	QString strOrder;
	if (m_pName->isChecked())
            strOrder = "name";
	else if (m_pLastUse->isChecked())
            strOrder = "lastuse";
	else
            strOrder = "desktop";
	c->writeEntry("ListOrder", strOrder);
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
    m_pAllWindows->setChecked(true);
    m_pDesktop->setChecked(true);
    emit changed(true);
}

QString TaskbarConfig::quickHelp() const
{
    return i18n("<h1>Taskbar</h1> You can configure the taskbar here."
                " This includes options such as whether or not the taskbar should show all"
		" windows at once or only those on the current desktop."
                " You can also configure whether or not Windows list button will be displayed"
                " and how Windows list will appear.");
}
