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


#include "smartcard.h"
#include <klocale.h>
#include <kglobal.h>
#include <kaboutdata.h>
#include <qcheckbox.h>
#include <kconfig.h>
#include <kdialog.h>
#include <qlayout.h>
#include <qlabel.h>
#include <dcopclient.h>
#include <kapplication.h>


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

     // The config backend
     connect(base->launchManager, SIGNAL(clicked()), this, SLOT(configChanged()));
     connect(base->beepOnInsert, SIGNAL(clicked()), this, SLOT(configChanged()));
     connect(base->enableSupport, SIGNAL(clicked()), this, SLOT(configChanged()));
     connect(base->enablePolling, SIGNAL(clicked()), this, SLOT(configChanged()));
     load();
  } else {
     layout->add(new NoSmartcardBase(this));
  }
}

KSmartcardConfig::~KSmartcardConfig()
{
    delete config;
}

void KSmartcardConfig::configChanged()
{
    emit changed(true);
}


void KSmartcardConfig::load()
{
if (_ok) {
  base->enableSupport->setChecked(config->readBoolEntry("Enable Support", false));
  base->enablePolling->setChecked(config->readBoolEntry("Enable Polling", true));
  base->beepOnInsert->setChecked(config->readBoolEntry("Beep on Insert", true));
  base->launchManager->setChecked(config->readBoolEntry("Launch Manager", true));
}
  emit changed(false);
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
  } else {
	QByteArray data, retval;
	QCString rettype;
	QDataStream arg(data, IO_WriteOnly);
	QCString modName = "kardsvc";
	arg << modName;
	kapp->dcopClient()->call("kded", "kded", "unloadModule(QCString)", 
			         data, rettype, retval);
  }

  config->sync();
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
     " for smartcards.  These can be used for various tasks such as storing"
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
  KCModule *create_smartcard(QWidget *parent, const char *name)
  {
    KGlobal::locale()->insertCatalogue("kcmsmartcard");
    return new KSmartcardConfig(parent, name);
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

