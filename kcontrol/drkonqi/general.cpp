/**
 *  general.cpp
 *
 *  Copyright (c) 2000 Timo Hummel <timo.hummel@sap.com>
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
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <qlayout.h>
#include <qlistbox.h>
#include <qvgroupbox.h>
#include <qvbox.h>

#include <kconfig.h>
#include <kglobal.h>
#include <kstddirs.h>
#include <klocale.h>
#include <kdialog.h>
#include <kdebug.h>

#include "main.h"
#include "general.h"

KDrKonqiGeneral::KDrKonqiGeneral(KConfig *config,
				 QString group,
				 QWidget *parent, const char *name)
  : KCModule(parent, name), g_pConfig(config), groupname(group)
{
  QBoxLayout *layout = new QHBoxLayout(this, KDialog::spacingHint());
  layout->setAutoAdd( TRUE );

  QGroupBox *grp;

  grp = new QVGroupBox(i18n("Presets"), this);
  lb_presets = new QListBox(grp);

  grp = new QVGroupBox(i18n("Debugger"), this);
  lb_debuggers = new QListBox(grp);

  connect(lb_presets, SIGNAL(selectionChanged()), SLOT(changed()));
  connect(lb_presets, SIGNAL(highlighted(int)), SLOT(slotPresetChanged(int)));
  connect(lb_debuggers, SIGNAL(selectionChanged()), SLOT(changed()));

  loadLists();
}

void KDrKonqiGeneral::loadLists()
{
  // load all available presets
  lb_presets->clear();
  QStringList qldd;
  qldd = KGlobal::dirs()->findAllResources("data",
					   "drkonqi/presets/*rc",
					   false, true);
  for (QStringList::Iterator it = qldd.begin();  it != qldd.end(); ++it)
    {
      // Read the configuration titles
      KConfig config (*it, true, true, "data");
      config.setGroup("General");
      
      QString ConfigNames = config.readEntry(QString::fromLatin1("Name"),
					     i18n("Unknown"));
      lb_presets->insertItem(ConfigNames);

      m_presets << (*it).mid((*it).findRev('/') + 1);
    }

  // load all available debuggers
  lb_debuggers->clear();
  qldd = KGlobal::dirs()->findAllResources("data",
					   "drkonqi/debuggers/*rc",
					   false, true);
  for (QStringList::Iterator it = qldd.begin();  it != qldd.end(); ++it)
    {
      // Read the configuration titles
      KConfig config (*it, true, true, "data");
      config.setGroup("General");
      
      QString ConfigNames = config.readEntry(QString::fromLatin1("Name"),
					     i18n("Unknown"));
      lb_debuggers->insertItem(ConfigNames);

      m_debuggers << (*it).mid((*it).findRev('/') + 1);
    }
}

void KDrKonqiGeneral::load()
{
  // set preset from config
  QString preset = g_pConfig->readEntry(QString::fromLatin1("ConfigName"),
					QString::fromLatin1("enduser"));
  preset += QString::fromLatin1("rc"); // the list includes the rc extention
  lb_presets->setCurrentItem( m_presets.findIndex(preset) );

  // set debugger from config
  QString debugger = g_pConfig->readEntry(QString::fromLatin1("Debugger"),
					  QString::fromLatin1("gdb"));
  debugger += QString::fromLatin1("rc"); // the list includes the rc extention
  lb_debuggers->setCurrentItem( m_debuggers.findIndex(debugger) );
}

void KDrKonqiGeneral::defaults()
{
  // Load the defaults here

  // set preset to "enduser"
  lb_presets->setCurrentItem( m_presets.findIndex
			      ( QString::fromLatin1("enduserrc") ) );

  // set debugger to "gdb"
  lb_debuggers->setCurrentItem( m_debuggers.findIndex
				( QString::fromLatin1("gdbrc") ) );
}

void KDrKonqiGeneral::save()
{
  QString str;

  // save preset
  str = m_presets[ lb_presets->currentItem() ];
  str.truncate( str.length() - 2 ); // do not include the rc extention
  g_pConfig->writeEntry( QString::fromLatin1("ConfigName"), str);

  // save debugger
  str = m_debuggers[ lb_debuggers->currentItem() ];
  str.truncate( str.length() - 2 ); // do not include the rc extention
  g_pConfig->writeEntry( QString::fromLatin1("Debugger"), str);

  g_pConfig->sync();
}

void KDrKonqiGeneral::changed()
{
  emit KCModule::changed(true);
}

void KDrKonqiGeneral::slotPresetChanged(int index)
{
  // This code was supposed to grey out the list box on the right side.
  // Unfortantly setEnable doesn't grey out, but only disables the list box.

#if 0
  QString str;
  str = QString::fromLatin1("drkonqi/presets/%1").arg(m_presets[ index ]);

  KConfig config(str, true, true, "data");
  config.setGroup(QString::fromLatin1("General"));
  bool b = config.readBoolEntry(QString::fromLatin1("ShowDebugButton"), false);

  lb_debuggers->setEnabled(b);
#endif
}

#include "general.moc"

