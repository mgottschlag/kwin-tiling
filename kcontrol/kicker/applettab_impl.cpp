/*
 *  applettab.cpp
 *
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

#include <qlayout.h>
#include <qgroupbox.h>
#include <qvbuttongroup.h>
#include <qwhatsthis.h>
#include <qradiobutton.h>
#include <qpushbutton.h>
#include <qtoolbutton.h>
#include <qvbox.h>
#include <qfileinfo.h>

#include <kconfig.h>
#include <kglobal.h>
#include <klocale.h>
#include <kdialog.h>
#include <kstandarddirs.h>
#include <k3listview.h>
#include <kdebug.h>

#include "applettab_impl.h"
#include "applettab_impl.moc"

AppletTab::AppletTab( QWidget *parent, const char* name )
  : AppletTabBase (parent, name)
{

  connect(level_group, SIGNAL(clicked(int)), SLOT(level_changed(int)));

  connect(lb_trusted, SIGNAL(selectionChanged(QListViewItem*)),
          SLOT(trusted_selection_changed(QListViewItem*)));

  connect(pb_add, SIGNAL(clicked()), SLOT(add_clicked()));
  connect(pb_remove, SIGNAL(clicked()), SLOT(remove_clicked()));

  connect(lb_available, SIGNAL(selectionChanged(QListViewItem*)),
          SLOT(available_selection_changed(QListViewItem*)));

  pb_add->setEnabled(false);
  pb_remove->setEnabled(false);

  QWhatsThis::add( level_group, i18n("Panel applets can be started in two different ways:"
    " internally or externally. While 'internally' is the preferred way to load applets, this can"
    " raise stability or security problems when you are using poorly-programmed third-party applets."
    " To address these problems, applets can be marked 'trusted'. You might want to configure"
    " Kicker to treat trusted applets differently to untrusted ones; your options are:"
    " <ul><li><em>Load only trusted applets internally:</em> All applets but the ones marked 'trusted'"
    " will be loaded using an external wrapper application.</li>"
    " <li><em>Load startup config applets internally:</em> The applets shown on KDE startup"
    " will be loaded internally, others will be loaded using an external wrapper application.</li>"
    " <li><em>Load all applets internally</em></li></ul>") );

  QWhatsThis::add( lb_trusted, i18n("Here you can see a list of applets that are marked"
    " 'trusted', i.e. will be loaded internally by Kicker in any case. To move an applet"
    " from the list of available applets to the trusted ones, or vice versa, select it and"
    " press the left or right buttons.") );

  QWhatsThis::add( pb_add, i18n("Click here to add the selected applet from the list of available,"
    " untrusted applets to the list of trusted applets.") );

  QWhatsThis::add( pb_remove, i18n("Click here to remove the selected applet from the list of trusted"
    " applets to the list of available, untrusted applets.") );

  QWhatsThis::add( lb_available, i18n("Here you can see a list of available applets that you"
    " currently do not trust. This does not mean you cannot use those applets, but rather that"
    " the panel's policy using them depends on your applet security level. To move an applet"
    " from the list of available applets to the trusted ones or vice versa, select it and"
    " press the left or right buttons.") );

  load();
}

void AppletTab::load()
{
    KConfig c(KickerConfig::self()->configName(), false, false);
  c.setGroup("General");

  available.clear();
  l_available.clear();
  l_trusted.clear();

  int level = c.readEntry("SecurityLevel", 1);

  switch(level)
    {
    case 0:
    default:
      trusted_rb->setChecked(true);
      break;
    case 1:
      new_rb->setChecked(true);
      break;
    case 2:
      all_rb->setChecked(true);
      break;
    }

  list_group->setEnabled(trusted_rb->isChecked());

  QStringList list = KGlobal::dirs()->findAllResources("applets", "*.desktop");
  for ( QStringList::Iterator it = list.begin(); it != list.end(); ++it )
    {
      QFileInfo fi(*it);
      available << fi.baseName();
    }

  if(c.hasKey("TrustedApplets"))
    {
      QStringList list = c.readEntry("TrustedApplets", QStringList());
      for ( QStringList::Iterator it = list.begin(); it != list.end(); ++it )
        {
          if(available.contains(*it))
            l_trusted << (*it);
        }
    }
  else
      l_trusted << "clockapplet" << "ksystemtrayapplet" << "krunapplet" << "quicklauncher"
                << "kminipagerapplet" << "ktaskbarapplet" << "eyesapplet" << "kmixapplet";

  for ( QStringList::Iterator it = available.begin(); it != available.end(); ++it )
    {
      if(!l_trusted.contains(*it))
        l_available << (*it);
    }

  updateTrusted();
  updateAvailable();
}

void AppletTab::save()
{
    KConfig c(KickerConfig::self()->configName(), false, false);
  c.setGroup("General");

  int level = 0;
  if(new_rb->isChecked()) level = 1;
  else if (all_rb->isChecked()) level = 2;

  c.writeEntry("SecurityLevel", level);
  c.writeEntry("TrustedApplets", l_trusted);
  c.sync();
}

void AppletTab::defaults()
{
  new_rb->setChecked(true);
  list_group->setEnabled(false);
}

QString AppletTab::quickHelp() const
{
  return i18n("");
}

void AppletTab::level_changed(int)
{
  list_group->setEnabled(trusted_rb->isChecked());
  setChanged();
}

void AppletTab::updateTrusted()
{
  lb_trusted->clear();
  for ( QStringList::Iterator it = l_trusted.begin(); it != l_trusted.end(); ++it )
    (void) new QListViewItem(lb_trusted, (*it));
}

void AppletTab::updateAvailable()
{
  lb_available->clear();
  for ( QStringList::Iterator it = l_available.begin(); it != l_available.end(); ++it )
    (void) new QListViewItem(lb_available, (*it));
}

void AppletTab::trusted_selection_changed(QListViewItem * item)
{
  pb_remove->setEnabled(item != 0);
  setChanged();
}

void AppletTab::available_selection_changed(QListViewItem * item)
{
  pb_add->setEnabled(item != 0);
  setChanged();
}

void AppletTab::add_clicked()
{
  QListViewItem *item = lb_available->selectedItem();
  if (!item) return;
  l_available.remove(item->text(0));
  l_trusted.append(item->text(0));

  updateTrusted();
  updateAvailable();
  updateAddRemoveButton();
}

void AppletTab::remove_clicked()
{
  QListViewItem *item = lb_trusted->selectedItem();
  if (!item) return;
  l_trusted.remove(item->text(0));
  l_available.append(item->text(0));

  updateTrusted();
  updateAvailable();
  updateAddRemoveButton();
}


void AppletTab::updateAddRemoveButton()
{
    pb_remove->setEnabled(l_trusted.count ()>0);
    pb_add->setEnabled(l_available.count()>0);
}
