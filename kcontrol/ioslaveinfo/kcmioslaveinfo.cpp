/*
 * kcmioslaveinfo.cpp
 *
 * Copyright 2001 Alexander Neundorf <alexander.neundorf@rz.tu-ilmenau.de>
 * Copyright 2001 George Staikos  <staikos@kde.org>
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

#include "kcmioslaveinfo.h"

#include <qlabel.h>
#include <qlayout.h>
#include <qhbox.h>
#include <qfile.h>
#include <qspinbox.h>
#include <qtabwidget.h>
#include <qvbox.h>
#include <qwhatsthis.h>

#include <kprotocolinfo.h>
#include <kstddirs.h>
#include <klocale.h>
#include <kconfig.h>
#include <kdialog.h>
#include <kglobal.h>


KCMIOSlaveInfo::KCMIOSlaveInfo(QWidget *parent, const char *name)
               :KCModule(parent,name),m_ioslavesLb(0)
{
   QVBoxLayout *layout=new QVBoxLayout(this,10,15);

   QLabel* label=new QLabel(i18n("Available IOSlaves"),this);
   QHBox *hbox=new QHBox(this);
   m_ioslavesLb=new QListBox(hbox);
   m_ioslavesLb->setMinimumSize(fontMetrics().width("blahfaselwhatever----"),10);
   //TODO make something useful after 2.1 is released
   m_info=new KTextBrowser(hbox);
   hbox->setSpacing(15);

   layout->addWidget(label);
   layout->addWidget(hbox);
   hbox->setStretchFactor(m_ioslavesLb,1);
   hbox->setStretchFactor(m_info,5);

   QStringList protocols=KProtocolInfo::protocols();
   for (QStringList::Iterator it=protocols.begin(); it!=protocols.end(); it++)
   {
      m_ioslavesLb->insertItem(*it);
   };
   m_ioslavesLb->sort();
   
   setButtons(buttons());
   load();
};


KCMIOSlaveInfo::~KCMIOSlaveInfo() {

}


void KCMIOSlaveInfo::load() {
   emit changed(false);
}

void KCMIOSlaveInfo::defaults() {
   emit changed(true);
}

void KCMIOSlaveInfo::save() {
   emit changed(true);
}

int KCMIOSlaveInfo::buttons () {
return KCModule::Default|KCModule::Apply|KCModule::Help;
}

void KCMIOSlaveInfo::configChanged() {
  emit changed(true);
}

void KCMIOSlaveInfo::childChanged(bool really) {
  emit changed(really);
}


QString KCMIOSlaveInfo::quickHelp() const
{
   return i18n("Gives you an overview over the installed ioslaves and allows"
               " you to configure the network timeout values for those slaves.");
}

void KCMIOSlaveInfo::showInfo(const QString& protocol)
{
   //m_info->setText(QString("Some info about protocol %1:/ ...").arg(protocol));
/*   QStringList dirs=KGlobal::dirs()->resourceDirs("html");
   for ( QStringList::Iterator it = dirs.begin(); it != dirs.end(); ++it )
   {
      printf( "%s \n", (*it).latin1() );
   };*/

   QString tmp("default/kioslave/");
   tmp+=protocol+".html";
   QString file=KGlobal::dirs()->findResource("html",tmp);
   /*QString tmp("kioslave/");
   tmp+=protocol+".html";
   QString file=KGlobal::dirs()->findResource("data",tmp);*/

   //cout<<"found for -"<<tmp.latin1()<<"- file -"<<file.latin1()<<"-"<<endl;
   if (!file.isEmpty())
   {
      QFile theFile( file );
      theFile.open( IO_ReadOnly );
      uint size = theFile.size();
      char* buffer = new char[ size + 1 ];
      theFile.readBlock( buffer, size );
      buffer[ size ] = 0;
      theFile.close();

      QString text = QString::fromUtf8( buffer, size );
      delete[] buffer;
      m_info->setText(text);
      return;
   };
   m_info->setText(QString("Some info about protocol %1:/ ...").arg(protocol));
};

/*void KCMIOSlaveInfo::showInfo(QListBoxItem *item)
{
   if (item==0)
      return;
   m_info->setText(QString("Some info about protocol %1 :/ ...").arg(item->text()));
};*/


extern "C"
{

  KCModule *create_ioslaveinfo(QWidget *parent, const char *name)
  {
    KGlobal::locale()->insertCatalogue("kcmioslaveinfo");
    return new KCMIOSlaveInfo(parent, name);
  }
}

#include "kcmioslaveinfo.moc"
