/**
 * smartcard.cpp
 *
 * Copyright (c) 2001 George Staikos <staikos@kde.org>
 * Copyright (c) 2001 Fernando Llobregat <fernando.llobregat@free.fr>
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <QCheckBox>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QPushButton>

#include <dcopclient.h>

#include <kaboutdata.h>
#include <kapplication.h>
#include <kcarddb.h>
#include <kcardfactory.h>
#include <kcardgsm_impl.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kdialog.h>
#include <kglobal.h>
#include <k3listview.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kmenu.h>
#include <kpluginfactory.h>

#include "smartcard.h"

K_PLUGIN_FACTORY(KSmartcardConfigFactory, registerPlugin<KSmartcardConfig>();)
K_EXPORT_PLUGIN(KSmartcardConfigFactory("kcmsmartcard"))

KSmartcardConfig::KSmartcardConfig(QWidget *parent, const QVariantList &)
  : KCModule(KSmartcardConfig::componentData(), parent)
  , DCOPObject("kcmsmartcard")
{
  QVBoxLayout *layout = new QVBoxLayout(this);
  layout->setSpacing(KDialog::spacingHint());
  layout->setMargin(KDialog::marginHint());
  config = new KConfig("ksmartcardrc", false, false);

  DCOPClient *dc = KApplication::kApplication()->dcopClient();

  _ok = false;
  dc->remoteInterfaces("kded", "kardsvc", &_ok);

  KAboutData *about =
  new KAboutData(I18N_NOOP("kcmsmartcard"), 0, ki18n("KDE Smartcard Control Module"),
                0, KLocalizedString(), KAboutData::License_GPL,
                ki18n("(c) 2001 George Staikos"));

  about->addAuthor(ki18n("George Staikos"), KLocalizedString(), "staikos@kde.org");
  setAboutData( about );

  if (_ok) {


     base = new SmartcardBase(this);
     layout->add(base);

     _popUpKardChooser = new KMenu(this,"KpopupKardChooser");
     _popUpKardChooser->insertItem(i18n("Change Module..."),
				   this,
				   SLOT(slotLaunchChooser()));
     // The config backend

     connect(base->launchManager, SIGNAL(clicked()), SLOT(changed()));
     connect(base->beepOnInsert, SIGNAL(clicked()), SLOT(changed()));
     connect(base->enableSupport, SIGNAL(clicked()), SLOT(changed()));


     connect(base->enablePolling, SIGNAL(clicked()), SLOT(changed()));
     connect(base->_readerHostsListView,
	     SIGNAL(rightButtonPressed(QListViewItem*,QPoint,int)),
	     this,
	     SLOT(slotShowPopup(QListViewItem*,QPoint,int)));



     if (!connectDCOPSignal("",
			    "",
			    "signalReaderListChanged(QStringList)",
			    "loadReadersTab(QStringList)",
			    false))

       kDebug()<<"Error connecting to DCOP server";


     if (!connectDCOPSignal("",
			    "",
			    "signalCardStateChanged(QString,bool,QString)",
			    "updateReadersState (QString,bool,QString) ",
			    false))

       kDebug()<<"Error connecting to DCOP server";
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

void KSmartcardConfig::loadSmartCardSupportTab(){



  //Update the toggle buttons with the current configuration

  if (_ok) {
  base->enableSupport->setChecked(config->readEntry("Enable Support",
							false));
  base->enablePolling->setChecked(config->readEntry("Enable Polling",
							true));
  base->beepOnInsert->setChecked(config->readEntry("Beep on Insert",
						       true));
  base->launchManager->setChecked(config->readEntry("Launch Manager",
							true));



  }
}

void KSmartcardConfig::updateReadersState (QString &readerName,
                                           bool isCardPresent,
                                           QString &atr) {

    K3ListViewItem * tID=(K3ListViewItem *) base->_readerHostsListView->findItem(readerName, 0);
    if (tID==0) return;

    K3ListViewItem * tIDChild=(K3ListViewItem*) tID->firstChild();
    if (tIDChild==NULL) return;

    delete tIDChild;

    if (!isCardPresent)
                (void) new K3ListViewItem(tID,i18n("No card inserted"));
    else{

        getSupportingModule(tID,atr);
    }


}



void KSmartcardConfig::loadReadersTab( QStringList &lr){

  //Prepare data for dcop calls
  QByteArray data, retval;
  QCString rettype;
  QDataStream arg(&data, QIODevice::WriteOnly);

  arg.setVersion(QDataStream::Qt_3_1);
  DCOPCString modName = "kardsvc";
  arg << modName;

  //  New view items
  K3ListViewItem * temp;

  //If the smartcard support is disabled we unload the kardsvc KDED module
  //  and return

  base->_readerHostsListView->clear();

  if (!config->readEntry("Enable Support", false)) {




    //  New view items
    K3ListViewItem * temp;
    kapp->dcopClient()->call("kded", "kded", "unloadModule(QCString)",
			     data, rettype, retval);

    (void) new K3ListViewItem(base->_readerHostsListView,
			     i18n("Smart card support disabled"));


    return;

  }

  if (lr.isEmpty()){


    (void) new K3ListViewItem(base->_readerHostsListView,
			     i18n("No readers found. Check 'pcscd' is running"));
    return;
  }

  for (QStringList::Iterator _slot=lr.begin();_slot!=lr.end();++_slot){

   temp= new K3ListViewItem(base->_readerHostsListView,*_slot);


   QByteArray dataATR;
   QDataStream argATR(&dataATR,QIODevice::WriteOnly);

   argATR.setVersion(QDataStream::Qt_3_1);
   argATR << *_slot;

   kapp->dcopClient()->call("kded", "kardsvc", "getCardATR(QString)",
			   dataATR, rettype, retval);


   QString cardATR;
   QDataStream retReaderATR(retval);
   retReaderATR>>cardATR;

   if (cardATR.isNull()){

     (void) new K3ListViewItem(temp,i18n("NO ATR or no card inserted"));
     continue;
   }

   getSupportingModule(temp,cardATR);




  }

}


void KSmartcardConfig::getSupportingModule( K3ListViewItem * ant,
                                            QString & cardATR) const{


    if (cardATR.isNull()){

        (void) new K3ListViewItem(ant,i18n("NO ATR or no card inserted"));
        return;
    }


    QString modName=_cardDB->getModuleName(cardATR);
    if (!modName.isNull()){
        QStringList mng= modName.split( ",");
        QString type=mng[0];
        QString subType=mng[1];
        QString subSubType=mng[2];
        K3ListViewItem * hil =new K3ListViewItem(ant,
                                               i18n("Managed by: "),
                                               type,
                                               subType,
                                               subSubType);
        hil->setSelectable(false);
    }
    else{


        K3ListViewItem * hil =new K3ListViewItem(ant,
                                               i18n("No module managing this card"));
        hil->setSelectable(false);
    }

  }

void KSmartcardConfig::load()
{


  //Prepare data for dcop calls
  QByteArray data, retval;
  QCString rettype;
  QDataStream arg(&data, QIODevice::WriteOnly);

  arg.setVersion(QDataStream::Qt_3_1);
  DCOPCString modName = "kardsvc";
  arg << modName;

  loadSmartCardSupportTab();


  // We call kardsvc to retrieve the current readers
  kapp->dcopClient()->call("kded", "kardsvc", "getSlotList ()",
			   data, rettype, retval);
  QStringList readers;
  readers.clear();
  QDataStream retReader(retval);
  retReader>>readers;

  //And we update the panel
  loadReadersTab(readers);

  emit changed(false);

}


void KSmartcardConfig::save()
{
if (_ok) {
  config->writeEntry("Enable Support", base->enableSupport->isChecked());
  config->writeEntry("Enable Polling", base->enablePolling->isChecked());
  config->writeEntry("Beep on Insert", base->beepOnInsert->isChecked());
  config->writeEntry("Launch Manager", base->launchManager->isChecked());


  QByteArray data, retval;
  QCString rettype;
  QDataStream arg(&data, QIODevice::WriteOnly);

  arg.setVersion(QDataStream::Qt_3_1);
  DCOPCString modName = "kardsvc";
  arg << modName;

  // Start or stop the server as needed
  if (base->enableSupport->isChecked()) {

    kapp->dcopClient()->call("kded", "kded", "loadModule(QCString)",
			     data, rettype, retval);
    config->sync();

    kapp->dcopClient()->call("kded", "kardsvc", "reconfigure()",
			     data, rettype, retval);
  } else {



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

extern "C"
{
  KDE_EXPORT void kcminit_smartcard()
  {
    KConfig *config = new KConfig("ksmartcardrc", false, false);
    bool start = config->readEntry("Enable Support", false);
    delete config;

    if (start) {
	QByteArray data, retval;
	QCString rettype;
	QDataStream arg(&data, QIODevice::WriteOnly);

	arg.setVersion(QDataStream::Qt_3_1);
	DCOPCString modName = "kardsvc";
	arg << modName;
	kapp->dcopClient()->call("kded", "kded", "loadModule(QCString)",
			         data, rettype, retval);
    }
  }
}


#include "smartcard.moc"

