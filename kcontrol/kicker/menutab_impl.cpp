/*
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
#include <qslider.h>
#include <qwhatsthis.h>
#include <qlineedit.h>
#include <qlistview.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <qvalidator.h>
#include <qtoolbutton.h>
#include <qdir.h>
#include <qtooltip.h>
#include <qlayout.h>

#include <kconfig.h>
#include <kglobal.h>
#include <knuminput.h>
#include <klocale.h>
#include <klistview.h>
#include <kmessagebox.h>
#include <kprocess.h>
#include <kdesktopfile.h>
#include <kstandarddirs.h>
#include <kiconloader.h>

#include "menutab_impl.h"
#include "menutab_impl.moc"


extern int kickerconfig_screen_number;

kSubMenuItem::kSubMenuItem(QListView* parent, 
                           const QString& visibleName,
                           const QString& desktopFile,
                           const QPixmap& icon,
                           bool checked)
    : QCheckListItem(parent, visibleName, QCheckListItem::CheckBox),
      m_desktopFile(desktopFile)
{
    setPixmap(0, icon);
    setOn(checked);
}

QString kSubMenuItem::desktopFile() 
{ 
    return m_desktopFile; 
}

void kSubMenuItem::stateChange(bool state)
{
    emit toggled(state);
}

MenuTab::MenuTab( QWidget *parent, const char* name )
  : MenuTabBase (parent, name),
    m_preferencesMenu(0),
    m_bookmarkMenu(0),
    m_quickBrowserMenu(0),
    m_recentDocumentsMenu(0)
{
    // connections
    connect(m_formatSimple, SIGNAL(clicked()), SIGNAL(changed()));
    connect(m_formatNameDesc, SIGNAL(clicked()), SIGNAL(changed()));
    connect(m_formDescName, SIGNAL(clicked()), SIGNAL(changed()));
    connect(m_showPixmap, SIGNAL(clicked()), SIGNAL(changed()));  
    connect(m_hiddenFiles, SIGNAL(clicked()), SIGNAL(changed()));
    connect(m_maxQuickBrowserItems, SIGNAL(valueChanged(int)), SIGNAL(changed()));
    connect(m_showRecent, SIGNAL(clicked()), SIGNAL(changed()));
    connect(m_showFrequent, SIGNAL(clicked()), SIGNAL(changed()));
    connect(m_maxQuickStartItems, SIGNAL(valueChanged(int)), SIGNAL(changed()));

    m_browserGroupLayout->setColStretch( 1, 1 );
    m_pRecentOrderGroupLayout->setColStretch( 1, 1 );

    load();
}

void MenuTab::load()
{
    QCString configname;
    if (kickerconfig_screen_number == 0)
        configname = "kickerrc";
    else
        configname.sprintf("kicker-screen-%drc", kickerconfig_screen_number);
    KConfig c(configname, false, false);

    c.setGroup("KMenu");

    m_showPixmap->setChecked(c.readBoolEntry("UseSidePixmap", true));

    c.setGroup("menus");
 
    m_hiddenFiles->setChecked(c.readBoolEntry("ShowHiddenFiles", false));
    m_maxQuickBrowserItems->setValue(c.readNumEntry("MaxEntries2", 30));

    if (c.readBoolEntry("DetailedMenuEntries", true))
    {
        if (c.readBoolEntry("DetailedEntriesNamesFirst", true))
        {
            m_formatNameDesc->setChecked(true);
        }
        else
        {
            m_formDescName->setChecked(true);
        }
    }
    else
    {
        m_formatSimple->setChecked(true);
    }

    m_subMenus->clear();

    // show the preferences menu?
    m_preferencesMenu = new kSubMenuItem(m_subMenus,
                                         i18n("Preferences"),
                                         QString::null,
                                         SmallIcon("package_settings"),
                                         c.readBoolEntry("UsePreferences", true));
    connect(m_preferencesMenu, SIGNAL(toggled(bool)), SIGNAL(changed()));

    // show the bookmark menu?
    m_bookmarkMenu = new kSubMenuItem(m_subMenus, 
                                      i18n("Bookmarks"),
                                      QString::null,
                                      SmallIcon("bookmark"),
                                      c.readBoolEntry("UseBookmarks", true));
    connect(m_bookmarkMenu, SIGNAL(toggled(bool)), SIGNAL(changed()));
    
    // show the quick menus menu?
    m_quickBrowserMenu = new kSubMenuItem(m_subMenus,
                                          i18n("Quick Browser"),
                                          QString::null,
                                          SmallIcon("kdisknav"),
                                          c.readBoolEntry("UseBrowser", true));
    connect(m_quickBrowserMenu, SIGNAL(toggled(bool)), SIGNAL(changed()));
    
    // show the recent documents menu?
    m_recentDocumentsMenu = new kSubMenuItem(m_subMenus, 
                                             i18n("Recent Documents"),
                                              QString::null,
                                              SmallIcon("document"),
                                              c.readBoolEntry("UseRecent", true));
    connect(m_recentDocumentsMenu, SIGNAL(toggled(bool)), SIGNAL(changed()));

    QStringList ext = c.readListEntry("Extensions");
    QStringList dirs = KGlobal::dirs()->findDirs("data", "kicker/menuext");
    kSubMenuItem* menuItem(0);
    for (QStringList::ConstIterator dit=dirs.begin(); dit!=dirs.end(); ++dit)
    {
        QDir d(*dit, "*.desktop");
        QStringList av = d.entryList();
        for (QStringList::ConstIterator it=av.begin(); it!=av.end(); ++it)
        {
            KDesktopFile df(d.absFilePath(*it), true);
            menuItem = new kSubMenuItem(m_subMenus, 
                                        df.readName(),
                                        *it,
                                        SmallIcon(df.readIcon()),
                                        qFind(ext.begin(), ext.end(), *it) != ext.end());
            connect(menuItem, SIGNAL(toggled(bool)), SIGNAL(changed()));
        }
    }

    if (c.readBoolEntry("RecentVsOften", false))
        m_showRecent->setChecked(true);
    else
        m_showFrequent->setChecked(true);

    m_maxQuickStartItems->setValue(c.readNumEntry("NumVisibleEntries", 5));
}

void MenuTab::save()
{
    QCString configname;
    if (kickerconfig_screen_number == 0)
        configname = "kickerrc";
    else
        configname.sprintf("kicker-screen-%drc", kickerconfig_screen_number);
    KConfig c(configname, false, false);


    c.setGroup("KMenu");

    c.writeEntry("UseSidePixmap", m_showPixmap->isChecked());

    c.setGroup("menus");

    c.writeEntry("MaxEntries2", m_maxQuickBrowserItems->value());

    c.writeEntry("DetailedMenuEntries", !m_formatSimple->isChecked());
    c.writeEntry("DetailedEntriesNamesFirst", m_formatNameDesc->isChecked());
    c.writeEntry("ShowHiddenFiles", m_hiddenFiles->isChecked());
    c.writeEntry("NumVisibleEntries", m_maxQuickStartItems->value());
    c.writeEntry("RecentVsOften", m_showRecent->isChecked());

    QStringList ext;
    QListViewItem *item(0);
    for (item = m_subMenus->firstChild(); item; item = item->nextSibling())
    {
        bool isOn = static_cast<kSubMenuItem*>(item)->isOn();
        if( item == m_preferencesMenu )
	{
       	    c.writeEntry("UsePreferences", isOn);
        }
        else if (item == m_bookmarkMenu)
        {
            c.writeEntry("UseBookmarks", isOn);
        }
        else if (item == m_quickBrowserMenu)
        {
            c.writeEntry("UseBrowser", isOn);
        }
        else if (item == m_recentDocumentsMenu)
        {
            c.writeEntry("UseRecent", isOn);
        }
        else if (isOn)
        {
            ext << static_cast<kSubMenuItem*>(item)->desktopFile();
        }
    }
    c.writeEntry("Extensions", ext);

    c.sync();
}

void MenuTab::defaults()
{
  m_showPixmap->setChecked(true);
  m_maxQuickBrowserItems->setValue(30);
  m_formatNameDesc->setChecked(true);
  m_showRecent->setChecked(true);
  m_hiddenFiles->setChecked(false);
  m_preferencesMenu->setOn(false);
  m_bookmarkMenu->setOn(true);
  m_quickBrowserMenu->setOn(true);
  m_recentDocumentsMenu->setOn(true);

  m_showFrequent->setChecked(true);
  m_maxQuickStartItems->setValue(5);
}

void MenuTab::launchMenuEditor()
{
    QString strAppPath(locate("exe", "kmenuedit"));
    if (!strAppPath.isEmpty())
    {
        KProcess process;
        process << strAppPath;
        process.start(KProcess::DontCare);
    }
    else
    {
        KMessageBox::error(this,
                           "The KDE menu editor (kmenuedit) could not be launched.\n"
                           "Perhaps it is not installed or not in your path.",
                           "Application Missing");
    }
}

