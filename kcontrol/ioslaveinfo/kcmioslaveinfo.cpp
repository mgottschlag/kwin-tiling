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

#include <kprotocolinfo.h>
#include <klocale.h>
#include <kconfig.h>
#include <kdialog.h>
#include <kglobal.h>
#include <qspinbox.h>
#include <qtabwidget.h>
#include <qvbox.h>
#include <qwhatsthis.h>


KCMIOSlaveInfo::KCMIOSlaveInfo(QWidget *parent, const char *name)
:KCModule(parent,name)
,m_ioslavesLb(0)
{
QBoxLayout *top = new QVBoxLayout(this);

   tabs = new QTabWidget(this);
   top->addWidget(tabs);

   ////////////////////////////////////////
   // Add the main tab with the information
   ////////////////////////////////////////
   tabInfo = new QFrame(this);

   setButtons(Help);
   QVBoxLayout *layout=new QVBoxLayout(tabInfo,10,15);

   QLabel* label=new QLabel(i18n("Available IOSlaves"),tabInfo);

   QHBox *hbox=new QHBox(tabInfo);
   m_ioslavesLb=new QListBox(hbox);
   //TODO make something useful after 2.1 is released
   //m_info=new QTextView(hbox);
/*   QWidget *dummy=new QWidget(hbox);
   hbox->setStretchFactor(dummy,1);
   hbox->setStretchFactor(m_ioslavesLb,1);*/
   hbox->setSpacing(15);

   layout->addWidget(label);
   layout->addWidget(hbox);

   QStringList protocols=KProtocolInfo::protocols();
   for (QStringList::Iterator it=protocols.begin(); it!=protocols.end(); it++)
   {
      m_ioslavesLb->insertItem(*it);
   };
   m_ioslavesLb->sort();
   //connect(m_ioslavesLb,SIGNAL(highlighted( const QString&)),this,SLOT(showInfo(const QString&)));
   //connect(m_ioslavesLb,SIGNAL(highlighted( QListBoxItem *item )),this,SLOT(showInfo(QListBoxItem *item)));
   //showInfo(m_ioslavesLb->text(0));
   //showInfo(m_ioslavesLb->firstItem());


   ////////////////////////////////////////
   //// Add the tab for timeout values
   ////////////////////////////////////////
   tabTimeouts = new KIOTimeoutControl(this);

   connect(tabTimeouts, SIGNAL(changed(bool)), SLOT(childChanged(bool)));

   tabs->addTab(tabInfo, i18n("IOSlaves Installed"));
   tabs->addTab(tabTimeouts, i18n("Timeout Values"));
   tabs->resize(tabs->sizeHint());

   setButtons(buttons());
   load();
};


KCMIOSlaveInfo::~KCMIOSlaveInfo() {

}


void KCMIOSlaveInfo::load() {
   tabTimeouts->load();
   emit changed(false);
}

void KCMIOSlaveInfo::defaults() {
   tabTimeouts->defaults();
   emit changed(true);
}

void KCMIOSlaveInfo::save() {
   tabTimeouts->save();
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


////////////////////////////////////////////////////////////////////////////
/////       Timeout class definitions go under here                    /////
////////////////////////////////////////////////////////////////////////////

KIOTimeoutControl::KIOTimeoutControl(QWidget *parent, const char *name) 
                 : QWidget(parent, name) {
QGridLayout *grid = new QGridLayout(this, 5, 2, KDialog::marginHint(),
                                                KDialog::spacingHint());

QLabel *anonylabel;
QString whatstr;

   anonylabel = new QLabel(i18n("Timeout values for KDE IO subsystem (in seconds)."), this);
   grid->addMultiCellWidget(anonylabel, 0, 0, 0, 1);

   anonylabel = new QLabel(i18n("Socket read timeout:"), this);
   _to_read = new QSpinBox(1, 9999, 1, this);
   grid->addWidget(anonylabel, 1, 0);
   grid->addWidget(_to_read, 1, 1);
   whatstr = i18n("This is the amount of time that KDE will wait on a socket"
                  " for requested data before disconnecting.");
   QWhatsThis::add(anonylabel, whatstr);
   QWhatsThis::add(_to_read, whatstr);
   connect(_to_read, SIGNAL(valueChanged(int)), SLOT(timeoutChanged(int)));

   anonylabel = new QLabel(i18n("Server response timeout:"), this);
   _to_response = new QSpinBox(1, 9999, 1, this);
   grid->addWidget(anonylabel, 2, 0);
   grid->addWidget(_to_response, 2, 1);
   whatstr = i18n("This is the amount of time that KDE will wait on a socket"
                  " for a server or peer to respond before disconnecting.");
   QWhatsThis::add(anonylabel, whatstr);
   QWhatsThis::add(_to_response, whatstr);
   connect(_to_response, SIGNAL(valueChanged(int)), SLOT(timeoutChanged(int)));

   anonylabel = new QLabel(i18n("Server connect timeout:"), this);
   _to_connect = new QSpinBox(1, 9999, 1, this);
   grid->addWidget(anonylabel, 3, 0);
   grid->addWidget(_to_connect, 3, 1);
   whatstr = i18n("This is the amount of time that KDE will wait for a"
                  " connection to succeed before giving up.");
   QWhatsThis::add(anonylabel, whatstr);
   QWhatsThis::add(_to_connect, whatstr);
   connect(_to_connect, SIGNAL(valueChanged(int)), SLOT(timeoutChanged(int)));

   anonylabel = new QLabel(i18n("Proxy connect timeout:"), this);
   _to_proxy = new QSpinBox(1, 9999, 1, this);
   grid->addWidget(anonylabel, 4, 0);
   grid->addWidget(_to_proxy, 4, 1);
   whatstr = i18n("This is the amount of time that KDE will wait for a"
                  " proxy server connection to succeed before giving up.");
   QWhatsThis::add(anonylabel, whatstr);
   QWhatsThis::add(_to_proxy, whatstr);
   connect(_to_proxy, SIGNAL(valueChanged(int)), SLOT(timeoutChanged(int)));

   _cfg = new KConfig("kioslaverc", false, false);

   load();
}


KIOTimeoutControl::~KIOTimeoutControl() {
  delete _cfg;
}

 
void KIOTimeoutControl::timeoutChanged(int val) {
  emit changed(true);
}

void KIOTimeoutControl::load() {
  _to_read->setValue(_cfg->readNumEntry("ReadTimeout", 15));
  _to_response->setValue(_cfg->readNumEntry("ResponseTimeout", 60));
  _to_connect->setValue(_cfg->readNumEntry("ConnectTimeout", 20));
  _to_proxy->setValue(_cfg->readNumEntry("ProxyConnectTimeout", 10));
  emit changed(false);
}


void KIOTimeoutControl::save() {
  _cfg->writeEntry("ReadTimeout", _to_read->value());
  _cfg->writeEntry("ResponseTimeout", _to_response->value());
  _cfg->writeEntry("ConnectTimeout", _to_connect->value());
  _cfg->writeEntry("ProxyConnectTimeout", _to_proxy->value());
  emit changed(true);
}


void KIOTimeoutControl::defaults() {
  _to_read->setValue(15);
  _to_response->setValue(60);
  _to_connect->setValue(20);
  _to_proxy->setValue(10);
  emit changed(true);
}





#include "kcmioslaveinfo.moc"
