/**
 * smartcard.cpp
 *
 * Copyright (c) 2001 George Staikos <staikos@kde.org>
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


#include <kcardfactory.h>
#include <kcardgsm_impl.h>

#include "smartcard.h"
#include <klocale.h>
#include <kglobal.h>
#include <kaboutdata.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <kconfig.h>
#include <kdialog.h>
#include <qlayout.h>
#include <qlabel.h>
#include <dcopclient.h>
#include <klistview.h>
#include <kpopupmenu.h>
#include <kmessagebox.h>
#include <kapplication.h>

#include <kcarddb.h>

#include <kdebug.h>
KSmartcardConfig::KSmartcardConfig(QWidget *parent, const char *name)
  : KCModule(parent, name)
{
  
  
  QVBoxLayout *layout = new QVBoxLayout(this, KDialog::marginHint(), KDialog::spacingHint());
  config = new KConfig("ksmartcardrc", false, false);

  DCOPClient *dc = KApplication::kApplication()->dcopClient();
  dc->attach();
  _ok = false;
  dc->remoteInterfaces("kded", "kardsvc", &_ok);

  if (_ok) {

    
     base = new SmartcardBase(this);
     layout->add(base);
     
     _popUpKardChooser = new KPopupMenu(this,"KpopupKardChooser");
     _popUpKardChooser->insertItem(i18n("Change module"),
				   this, 
				   SLOT(slotLaunchChooser()));
     // The config backend

     connect(base->launchManager, SIGNAL(clicked()), this, SLOT(configChanged()));
     connect(base->beepOnInsert,  SIGNAL(clicked()), this, SLOT(configChanged()));
     connect(base->enableSupport, SIGNAL(clicked()), this, SLOT(configChanged()));
     connect(base->enablePolling, SIGNAL(clicked()), this, SLOT(configChanged()));
     connect(base->_readerHostsListView,
	     SIGNAL(rightButtonPressed(QListViewItem *,const QPoint &,int)),
	     this,
	     SLOT(slotShowPopup(QListViewItem *,const QPoint &,int)));


  
     _cardDB= new KCardDB();
     load();
  } else {
     layout->add(new NoSmartcardBase(this));
  }
}

KSmartcardConfig::~KSmartcardConfig()
{
    delete config;
    delete _cardDB;
}

void KSmartcardConfig::configChanged()
{
    emit changed(true);
}



void KSmartcardConfig::slotLaunchChooser(){

  
  if ( KCardDB::launchSelector(base->_readerHostsListView->currentItem()->parent()->text(0))){
    
    KMessageBox::sorry(this,i18n("Unable to launch KCardChooser"));
  }
  
  
}

void KSmartcardConfig::slotShowPopup(QListViewItem * item ,const QPoint & _point,int i)
{
  
  //The popup only appears in cards, not in the slots1
  if (item->isSelectable()) return;
  _popUpKardChooser->exec(_point);

}




void KSmartcardConfig::load()
{

  QByteArray data, retval;
  QCString rettype;
  QDataStream arg(data, IO_WriteOnly);
  QCString modName = "kardsvc";
  arg << modName;
  KListViewItem * temp;

if (_ok) {
  base->enableSupport->setChecked(config->readBoolEntry("Enable Support", false));
  base->enablePolling->setChecked(config->readBoolEntry("Enable Polling", true));
  base->beepOnInsert->setChecked(config->readBoolEntry("Beep on Insert", true));
  base->launchManager->setChecked(config->readBoolEntry("Launch Manager", true));
  
  base->_readerHostsListView->clear();
  if (!config->readBoolEntry("Enable Support", false)){
    
    kapp->dcopClient()->call("kded", "kded", "unloadModule(QCString)", 
			     data, rettype, retval);
    
    (void) new KListViewItem(base->_readerHostsListView,i18n("Smart card support disabled"));
    return;

  }
  
  kapp->dcopClient()->call("kded", "kardsvc", "getSlotList ()", 
			   data, rettype, retval);
  QStringList _readers;
  _readers.clear();
  QDataStream _retReader(retval, IO_ReadOnly);
  _retReader>>_readers;

  if (_readers.isEmpty()){


    (void) new KListViewItem(base->_readerHostsListView,i18n("No readers found.Check 'pcscd' is running"));
    return;
  }

  for (QStringList::Iterator _slot=_readers.begin();_slot!=_readers.end();++_slot){

   temp= new KListViewItem(base->_readerHostsListView,*_slot);
   

   QByteArray dataATR;
   QDataStream argATR(dataATR,IO_WriteOnly);
   argATR << *_slot;

   kapp->dcopClient()->call("kded", "kardsvc", "getCardATR(QString)", 
			   dataATR, rettype, retval);

   
   QString cardATR;
   QDataStream _retReaderATR(retval, IO_ReadOnly);
   _retReaderATR>>cardATR;
   
   if (cardATR.isNull()){
     
     (void) new KListViewItem(temp,i18n("No card inserted"));
     continue;
   }

   QString modName=_cardDB->getModuleName(cardATR);
   
   if (modName!=QString::null){
     QStringList mng= QStringList::split(",",modName);
     QString type=mng[0];
     QString subType=mng[1];
     QString subSubType=mng[2];
     KListViewItem * hil =new KListViewItem(temp,
					    i18n("Managed by: "),
					    type,
					    subType,
					    subSubType);
     hil->setSelectable(FALSE);
   }
   else{

     
     KListViewItem * hil =new KListViewItem(temp,
					    i18n("No module managing this card"));
     hil->setSelectable(FALSE);
   }

   
  }
  emit changed(false);
}
}


void KSmartcardConfig::save()
{
if (_ok) {
  config->writeEntry("Enable Support", base->enableSupport->isChecked());
  config->writeEntry("Enable Polling", base->enablePolling->isChecked());
  config->writeEntry("Beep on Insert", base->beepOnInsert->isChecked());
  config->writeEntry("Launch Manager", base->launchManager->isChecked());



  // Start or stop the server as needed
  if (base->enableSupport->isChecked()) {
    
	QByteArray data, retval;
	QCString rettype;
	QDataStream arg(data, IO_WriteOnly);
	QCString modName = "kardsvc";
	arg << modName;
	kapp->dcopClient()->call("kded", "kded", "loadModule(QCString)", 
			         data, rettype, retval);

	config->sync();
       
	kapp->dcopClient()->call("kded", "kardsvc", "reconfigure()", 
			         data, rettype, retval);
  } else {
   
	QByteArray data, retval;
	QCString rettype;
	QDataStream arg(data, IO_WriteOnly);
	QCString modName = "kardsvc";
	arg << modName;
	kapp->dcopClient()->call("kded", "kded", "unloadModule(QCString)", 
			         data, rettype, retval);
  }


}
  emit changed(false);
}

void KSmartcardConfig::defaults()
{
if (_ok) {
  base->enableSupport->setChecked(false);
  base->enablePolling->setChecked(true);
  base->beepOnInsert->setChecked(true);
  base->launchManager->setChecked(true);
}
  emit changed(true);
}




QString KSmartcardConfig::quickHelp() const
{
  return i18n("<h1>smartcard</h1> This module allows you to configure KDE support"
     " for smartcards. These can be used for various tasks such as storing"
     " SSL certificates and logging in to the system.");
}

const KAboutData* KSmartcardConfig::aboutData() const
{
 
    KAboutData *about =
    new KAboutData(I18N_NOOP("kcmsmartcard"), I18N_NOOP("KDE Smartcard Control Module"),
                  0, 0, KAboutData::License_GPL,
                  I18N_NOOP("(c) 2001 George Staikos"));
 
    about->addAuthor("George Staikos", 0, "staikos@kde.org");
 
    return about;
}

extern "C"
{
  KCModule *create_smartcard(QWidget *parent, const char *)
  {
    return new KSmartcardConfig(parent, "kcmsmartcard");
  }

  void init_smartcard()
  {
    KConfig *config = new KConfig("ksmartcardrc", false, false);
    bool start = config->readBoolEntry("Enable Support", false);
    delete config;

    if (start) {
	QByteArray data, retval;
	QCString rettype;
	QDataStream arg(data, IO_WriteOnly);
	QCString modName = "kardsvc";
	arg << modName;
	kapp->dcopClient()->call("kded", "kded", "loadModule(QCString)", 
			         data, rettype, retval);
    }
  }
}


#include "smartcard.moc"

