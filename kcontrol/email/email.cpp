/**
 * email.cpp
 *
 * Copyright (c) 1999 Preston Brown <pbrown@kde.org>
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

#include <unistd.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <malloc.h>

#include <qlayout.h>
#include <qvbox.h>
#include <qlabel.h>
#include <klined.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <kconfig.h>
#include <kglobal.h>
#include <kstddirs.h>
#include <klocale.h>

#include "email.h"

#define SPACE 6

KEmailConfig::KEmailConfig(QWidget *parent, const char *name)
  : KCModule(parent, name)
{
  QVBoxLayout *topLayout = new QVBoxLayout(this, SPACE);
  QGroupBox *uBox = new QGroupBox(2, Qt::Horizontal, i18n("User information"),
				  this);
  topLayout->addWidget(uBox);

  QLabel *label = new QLabel(i18n("&Full Name:"), uBox);

  fullName = new KLineEdit(uBox);
  connect(fullName, SIGNAL(textChanged(const QString&)), this, SLOT(configChanged()));
  label->setBuddy(fullName);

  label = new QLabel(i18n("Or&ganization:"), uBox);

  organization = new KLineEdit(uBox);
  connect(organization, SIGNAL(textChanged(const QString&)), this, SLOT(configChanged()));
  label->setBuddy(organization);

  label = new QLabel(i18n("E-mail &Address:"), uBox);

  emailAddr = new KLineEdit(uBox);
  connect(emailAddr, SIGNAL(textChanged(const QString&)), this, SLOT(configChanged()));
  label->setBuddy(emailAddr);

  label = new QLabel(i18n("&Reply Address:"), uBox);

  replyAddr = new KLineEdit(uBox);
  connect(replyAddr, SIGNAL(textChanged(const QString&)), this, SLOT(configChanged()));
  label->setBuddy(replyAddr);

  uBox = new QGroupBox(2, Qt::Horizontal, i18n("Server information"),
		       this);
  topLayout->addWidget(uBox);

  label = new QLabel(i18n("User &name:"), uBox);

  userName = new KLineEdit(uBox);
  connect(userName, SIGNAL(textChanged(const QString&)), this, SLOT(configChanged()));
  label->setBuddy(userName);
  label = new QLabel(i18n("&Password:"), uBox);

  password = new KLineEdit(uBox);
  password->setEchoMode(QLineEdit::Password);
  connect(password, SIGNAL(textChanged(const QString&)), this, SLOT(configChanged()));
  label->setBuddy(password);

  label = new QLabel(i18n("&Incoming host:"), uBox);

  inServer = new KLineEdit(uBox);
  connect(inServer, SIGNAL(textChanged(const QString&)), this, SLOT(configChanged()));
  label->setBuddy(inServer);

  label = new QLabel(i18n("O&utgoing host:"), uBox);

  outServer = new KLineEdit(uBox);
  connect(outServer, SIGNAL(textChanged(const QString&)), this, SLOT(configChanged()));

  label->setBuddy(outServer);

  bGrp = new QButtonGroup(1, Qt::Vertical,
			   i18n("Incoming mail server type"), this);
  connect(bGrp, SIGNAL(clicked(int)), this, SLOT(configChanged()));
  
  topLayout->addWidget(bGrp);

  imapButton = new QRadioButton(i18n("&IMAP"), bGrp);
  pop3Button = new QRadioButton(i18n("P&OP3"), bGrp);
  localButton = new QRadioButton(i18n("&Local mailbox"), bGrp);

  topLayout->addSpacing(SPACE);

  load();
}

KEmailConfig::~KEmailConfig()
{
}

void KEmailConfig::configChanged()
{
    emit changed(true);
}


void KEmailConfig::load()
{
  KConfig *config = KGlobal::config();
  char hostname[80];
  struct passwd *p;

  p = getpwuid(getuid());
  gethostname(hostname, 80);

  config->setGroup("UserInfo");
  fullName->setText(config->readEntry("FullName", p->pw_gecos));
  QString tmp = p->pw_name;
  tmp += "@"; tmp += hostname;
  emailAddr->setText(config->readEntry("EmailAddress", tmp));
  organization->setText(config->readEntry("Organization"));
  replyAddr->setText(config->readEntry("ReplyAddr"));

  config->setGroup("ServerInfo");
  userName->setText(config->readEntry("UserName", p->pw_name));
  password->setText(config->readEntry("Password"));
  inServer->setText(config->readEntry("Incoming"));
  outServer->setText(config->readEntry("Outgoing", hostname));

  bGrp->setButton(config->readNumEntry("ServerType", 0));

  emit changed(false);
}

void KEmailConfig::save()
{
    debug("KEmailConfig save called");
    
  KConfig *config = KGlobal::config();

  config->setGroup("UserInfo");
  config->writeEntry("FullName", fullName->text());
  config->writeEntry("EmailAddress", emailAddr->text());
  config->writeEntry("Organization", organization->text());
  config->writeEntry("ReplyAddr", replyAddr->text());

  config->setGroup("ServerInfo");
  config->writeEntry("UserName", userName->text());
  config->writeEntry("Password", password->text());
  config->writeEntry("Incoming", inServer->text());
  config->writeEntry("Outgoing", outServer->text());
  int sType;
  if (imapButton->isChecked())
    sType = 0;
  else if (pop3Button->isChecked())
    sType = 1;
  else
    sType = 2;
  config->writeEntry("ServerType", sType);

  config->sync();

  // insure proper permissions -- contains sensitive data
  QString cfgName(KGlobal::dirs()->findResource("config", "kcmemailrc"));
  if (!cfgName.isEmpty())
    ::chmod(cfgName.utf8(), 0600);

  emit changed(false);
}

void KEmailConfig::defaults()
{
    // as there is nothing really reset, we shouldn't call this yet
    //emit changed(false);
}



extern "C"
{
  KCModule *create_email(QWidget *parent, const char *name)
  {
    return new KEmailConfig(parent, name);
  };
}
