/*
 * main.cpp for the samba kcontrol module
 *
 * Requires the Qt widget libraries, available at no cost at
 * http://www.troll.no/
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

#include "ksmbstatus.h"
#include "kcmsambaimports.h"
#include "kcmsambalog.h"
#include "kcmsambastatistics.h"
#include <klocale.h>
#include <qtabwidget.h>
#include <kglobal.h>
#include <qlayout.h>
#include <kconfig.h>

#include <kcmodule.h>

class SambaContainer:public KCModule
{
   public:
      SambaContainer(QWidget *parent=0, const char * name=0);
      virtual ~SambaContainer() {};
      virtual void load();
      virtual void save();
      virtual void defaults() {};
   private:
      QVBoxLayout layout;
      KConfig config;
      QTabWidget tabs;
      NetMon status;
      ImportsView imports;
      LogView logView;
      StatisticsView statisticsView;
};

SambaContainer::SambaContainer(QWidget *parent, const char* name)
:KCModule(parent,name)
,layout(this)
,config("kcmsambarc",false,true)
,tabs(this)
,status(&tabs,&config)
,imports(&tabs,&config)
,logView(&tabs,&config)
,statisticsView(&tabs,&config)
{
   layout.addWidget(&tabs);
   tabs.addTab(&status,i18n("&Exports"));
   tabs.addTab(&imports,i18n("&Imports"));
   tabs.addTab(&logView,i18n("&Log"));
   tabs.addTab(&statisticsView,i18n("&Statistics"));
   connect(&logView,SIGNAL(contentsChanged(QListView* , int, int)),&statisticsView,SLOT(setListInfo(QListView *, int, int)));
   setButtons(Ok|Help);
   load();
};

#include <iostream.h>

void SambaContainer::load()
{
//   cout<<"SambaContainer::load"<<endl;
   status.load();
   imports.load();
   logView.load();
   statisticsView.load();
}

void SambaContainer::save()
{
//   cout<<"SambaContainer::save"<<endl;   
   status.save();
   imports.save();
   logView.save();
   statisticsView.save();
   config.sync();
}

extern "C"
{

  KCModule *create_samba(QWidget *parent, const char *name)
  { 
    KGlobal::locale()->insertCatalogue("kcmsamba");
    return new SambaContainer(parent, name);
  }
}
