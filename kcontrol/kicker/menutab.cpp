/*
 *  menutab.cpp
 *
 *  Copyright (c) 2000 Matthias Elter <elter@kde.org>
 *  Copyright (c) 2000 Preston Brown <pbrown@kde.org>
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

#include <qlayout.h>
#include <qcheckbox.h>
#include <qgroupbox.h>
#include <qwhatsthis.h>

#include <kconfig.h>
#include <kdialog.h>
#include <kglobal.h>
#include <klocale.h>
#include <knuminput.h>

#include "menutab.h"
#include "menutab.moc"


MenuTab::MenuTab( QWidget *parent, const char* name )
  : QWidget (parent, name)
{
  layout = new QGridLayout(this, 3, 1,
                           KDialog::marginHint(),
                           KDialog::spacingHint());

  // general group
  general_group = new QGroupBox(i18n("General"), this);
  
  QVBoxLayout *vbox = new QVBoxLayout(general_group, KDialog::marginHint(),
                                      KDialog::spacingHint());
  vbox->addSpacing(fontMetrics().lineSpacing());

  clear_cache_cb = new QCheckBox(i18n("Clear menu cache."), general_group);
  connect(clear_cache_cb, SIGNAL(clicked()), SLOT(clear_cache_clicked()));
  vbox->addWidget(clear_cache_cb);

  cache_time_input = new KIntNumInput(60, general_group);
  cache_time_input->setRange(1, 600, 1, true);
  cache_time_input->setLabel(i18n("Clear after n seconds:"));
  connect(cache_time_input, SIGNAL(valueChanged(int)), SLOT(cache_time_changed(int)));
  vbox->addWidget(cache_time_input);

  layout->addWidget(general_group,0,0);

  // browser menu group
  browser_group = new QGroupBox(i18n("Browser Menus"), this);
  
  vbox = new QVBoxLayout(browser_group, KDialog::marginHint(),
                                      KDialog::spacingHint());
  vbox->addSpacing(fontMetrics().lineSpacing());

  show_hidden_cb = new QCheckBox(i18n("Show hidden files."), browser_group);
  connect(show_hidden_cb, SIGNAL(clicked()), SIGNAL(changed()));
  vbox->addWidget(show_hidden_cb);

  max_entries_input = new KIntNumInput(200, browser_group);
  max_entries_input->setRange(20, 1000, 1, true);
  max_entries_input->setLabel(i18n("Maximum browser menu entries:"));
  connect(max_entries_input, SIGNAL(valueChanged(int)), SLOT(max_entries_changed(int)));
  vbox->addWidget(max_entries_input);

  layout->addWidget(browser_group,1,0);

  // kmenu group
  kmenu_group = new QGroupBox(i18n("K Menu"), this);

  vbox = new QVBoxLayout(kmenu_group,KDialog::marginHint(),
                         KDialog::spacingHint());
  vbox->addSpacing(fontMetrics().lineSpacing());

  merge_cb = new QCheckBox(i18n("Merge different menu locations."), kmenu_group);
  connect(merge_cb, SIGNAL(clicked()), SIGNAL(changed()));
  vbox->addWidget(merge_cb);

  QWhatsThis::add(merge_cb, i18n("KDE can support several different locations "
                                 "on the system for storing program "
                                 "information, including (but not limited to) "
                                 "a system-wide and a personal directory. "
                                 "Enabling this option makes the KDE panel "
                                 "merge these different locations into a "
                                 "single logical tree of programs."));

  show_recent_cb = new QCheckBox(i18n("Show recent documents submenu."), kmenu_group);
  connect(show_recent_cb, SIGNAL(clicked()), SIGNAL(changed()));
  vbox->addWidget(show_recent_cb);
  QWhatsThis::add( show_recent_cb, i18n("Enabling this option will make the panel show"
    " a recent documents menu in your KDE menu, containing shortcuts to your most recently"
    " edited documents. This assumes you've been using KDE applications to edit those documents,"
    " as other applications will not be able to take advantage of this feature."));

  show_qb_cb = new QCheckBox(i18n("Show quickbrowser submenu."), kmenu_group);
  connect(show_qb_cb, SIGNAL(clicked()), SIGNAL(changed()));
  vbox->addWidget(show_qb_cb);
  QWhatsThis::add( show_qb_cb, i18n("Enabling this option will show the 'Quick Browser' in your"
    " KDE menu, a fast way of accessing your files via submenus. You can also add a Quick Browser"
    " as a panel button, using the panel context menu."));

  layout->addWidget(kmenu_group,2,0);

  load();
}

void MenuTab::cache_time_changed(int)
{
  emit changed();
}

void MenuTab::max_entries_changed(int)
{
  emit changed();
}

void MenuTab::clear_cache_clicked()
{
  cache_time_input->setEnabled(clear_cache_cb->isChecked());
  emit changed();
}

void MenuTab::load()
{
  KConfig *c = new KConfig("kickerrc", false, false);
  
  c->setGroup("menus");

  bool cc = c->readBoolEntry("ClearMenuCache", true);
  clear_cache_cb->setChecked(cc);
  cache_time_input->setValue(c->readNumEntry("MenuCacheTime", 60) / 100);
  cache_time_input->setEnabled(cc);

  max_entries_input->setValue(c->readNumEntry("MaxEntries", 200));

  merge_cb->setChecked(c->readBoolEntry("MergeKDEDirs", true));
  show_recent_cb->setChecked(c->readBoolEntry("UseRecent", true));
  show_qb_cb->setChecked(c->readBoolEntry("UseBrowser", true));

  show_hidden_cb->setChecked(c->readBoolEntry("ShowHiddenFiles", true));

  delete c;
}

void MenuTab::save()
{
  KConfig *c = new KConfig("kickerrc", false, false);
  
  c->setGroup("menus");

  c->writeEntry("ClearMenuCache", clear_cache_cb->isChecked());
  c->writeEntry("MenuCacheTime", cache_time_input->value() * 100);
  c->writeEntry("MaxEntries", max_entries_input->value());
  c->writeEntry("MergeKDEDirs", merge_cb->isChecked());
  c->writeEntry("UseRecent", show_recent_cb->isChecked());
  c->writeEntry("UseBrowser", show_qb_cb->isChecked());
  c->writeEntry("ShowHiddenFiles", show_hidden_cb->isChecked());

  c->sync();

  delete c;
}

void MenuTab::defaults()
{
  clear_cache_cb->setChecked(true);
  cache_time_input->setValue(60);
  cache_time_input->setEnabled(true);
  max_entries_input->setValue(200);
  merge_cb->setChecked(true);
  show_recent_cb->setChecked(true);
  show_qb_cb->setChecked(true);
  show_hidden_cb->setChecked(true);
}

QString MenuTab::quickHelp()
{
  return i18n("");
}
