/**
 * email.cpp - $Id$
 *
 * Copyright (c) 1999, 2000 Preston Brown <pbrown@kde.org>
 * Copyright (c) 2000 Frerich Raabe <raabe@kde.org>
 *
 * $Id$
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

#include <sys/types.h>
#include <sys/stat.h>

#include <stdlib.h>
#include <pwd.h>
#include <unistd.h>

#include <qlayout.h>
#include <qvbox.h>
#include <qlabel.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <qwhatsthis.h>
#include <qpushbutton.h>
#include <qfileinfo.h>
#include <qcheckbox.h>

#include <kfiledialog.h>
#include <klineedit.h>
#include <kconfig.h>
#include <kglobal.h>
#include <kstddirs.h>
#include <klocale.h>
#include <kdialog.h>
#include <kcombobox.h>
#include <kemailsettings.h>

#include "email.h"

KEmailConfig::KEmailConfig(QWidget *parent, const char *name)
  : KCModule(parent, name)
{
	blah=false;
	QString wtstr;
	QVBoxLayout *topLayout = new QVBoxLayout(this, KDialog::marginHint(), KDialog::spacingHint());
	QLabel *label;

	QHBoxLayout *l = new QHBoxLayout(topLayout);
	cProfiles = new KComboBox(true, this);
	cProfiles->setTrapReturnKey(true);
	connect(cProfiles, SIGNAL(returnPressed()), this, SLOT(newProfile()));
	label = new QLabel("Current Profile:", this);
	label->setBuddy(cProfiles);
	l->addWidget(label, 0);
	l->addWidget(cProfiles, 1);
	connect (cProfiles, SIGNAL(activated(const QString&)), this, SLOT(profileChanged(const QString&)));

	QGroupBox *uBox = new QGroupBox(2, Qt::Horizontal, i18n("User information"), this);
	topLayout->addWidget(uBox);

	label = new QLabel(i18n("&Full Name:"), uBox);

	fullName = new KLineEdit(uBox);
	connect(fullName, SIGNAL(textChanged(const QString&)), this, SLOT(configChanged()));
	label->setBuddy(fullName);

	wtstr = i18n(	"Enter your full name here, e.g. \"John Doe\" (without the quotation"
			" marks).  Some people like to provide a nick name only. You can leave this field"
			" blank and still use email. However, providing your full name is <em>recommended</em> as"
			" this makes it much easier for your recipient to browse his or her email.");
	QWhatsThis::add( label, wtstr );
	QWhatsThis::add( fullName, wtstr );

	label = new QLabel(i18n("Or&ganization:"), uBox);

	organization = new KLineEdit(uBox);
	connect(organization, SIGNAL(textChanged(const QString&)), this, SLOT(configChanged()));
	label->setBuddy(organization);

	wtstr = i18n(	"Here you can enter the name of your organization, company"
			" or university. This field is <em>optional</em>. However, if"
			" you are using a business account and communicate with persons working for other"
			" companies, providing the name of your organization is recommended.");
	QWhatsThis::add( label, wtstr );
	QWhatsThis::add( organization, wtstr );

	label = new QLabel(i18n("Email &Address:"), uBox);

	emailAddr = new KLineEdit(uBox);
	connect(emailAddr, SIGNAL(textChanged(const QString&)), this, SLOT(configChanged()));
	label->setBuddy(emailAddr);

	wtstr = i18n(	"Enter your email address here, e.g. \"john@doe.com\" (without "
			"the quotation marks). This information is mandatory if you want to use email.<p>"
			"Do <em>not</em> enter something like \"John Doe &lt;john@doe.com&gt;\", just a plain email address. "
			"Your email address may not contain any blank spaces.");
	QWhatsThis::add( label, wtstr );
	QWhatsThis::add( emailAddr, wtstr );

	label = new QLabel(i18n("&Reply Address:"), uBox);

	replyAddr = new KLineEdit(uBox);
	connect(replyAddr, SIGNAL(textChanged(const QString&)), this, SLOT(configChanged()));
	label->setBuddy(replyAddr);

  wtstr = i18n("You can set a reply address if you want replies to your e-mail messages"
     " to go to a different address than the e-mail address above. Most likely, you should"
     " leave the reply address blank, so replies go to the e-mail address you entered above.<p>"
     " <em>Please note:</em> <ul><li>You do not need to enter the same email"
     " address as above.</li><li>You should not use a reply address if you frequently"
     " use discussion mailing lists.</li></ul>");
  QWhatsThis::add( label, wtstr );
  QWhatsThis::add( replyAddr, wtstr );

  uBox = new QGroupBox(2, Qt::Horizontal, i18n("Server information"),
               this);
  topLayout->addWidget(uBox);

  label = new QLabel(i18n("User &name:"), uBox);

  userName = new KLineEdit(uBox);
  connect(userName, SIGNAL(textChanged(const QString&)), this, SLOT(configChanged()));
  label->setBuddy(userName);

  wtstr = i18n("The user name you use to login to your e-mail server (sometimes just called \"login\")."
     " Your e-mail provider should have supplied this information. Your login name is often (but"
     " not always) identical to the part of your email address that comes before the \"@\".");
  QWhatsThis::add( label, wtstr );
  QWhatsThis::add( userName, wtstr );

  label = new QLabel(i18n("&Password:"), uBox);

  password = new KLineEdit(uBox);
  password->setEchoMode(QLineEdit::Password);
  connect(password, SIGNAL(textChanged(const QString&)), this, SLOT(configChanged()));
  label->setBuddy(password);

  wtstr = i18n("Your password for the e-mail server. Your e-mail provider should have supplied"
     " this information along with your user name. <br>Your password will not"
     " appear on screen and will not be readable by other normal users on the system.");
  QWhatsThis::add( label, wtstr );
  QWhatsThis::add( password, wtstr );

  label = new QLabel(i18n("&Incoming host:"), uBox);

  inServer = new KLineEdit(uBox);
  connect(inServer, SIGNAL(textChanged(const QString&)), this, SLOT(configChanged()));
  label->setBuddy(inServer);

  wtstr = i18n("The server you get incoming e-mail from (this <em>may</em> be identical to your outgoing host)."
     " Your e-mail provider should have supplied this information. It may have been labeled \"POP3 server/host\" or"
     " \"IMAP server/host\". If you are using a local mailbox, you may leave this blank.");
  QWhatsThis::add( label, wtstr );
  QWhatsThis::add( inServer, wtstr );

  label = new QLabel(i18n("O&utgoing host:"), uBox);

  outServer = new KLineEdit(uBox);
  connect(outServer, SIGNAL(textChanged(const QString&)), this, SLOT(configChanged()));

  label->setBuddy(outServer);

  wtstr = i18n("The server you use for sending e-mail (this <em>may</em> be identical to your incoming host)."
     " Your e-mail provider should have supplied this information. It may have been labeled \"SMTP server\" or"
     " \"SMTP host\". If you are using a local mailbox, you may leave this blank.");
  QWhatsThis::add( label, wtstr );
  QWhatsThis::add( outServer, wtstr );

  bGrp = new QButtonGroup(1, Qt::Vertical,
               i18n("Incoming mail server type"), this);
  connect(bGrp, SIGNAL(clicked(int)), this, SLOT(configChanged()));

  topLayout->addWidget(bGrp);

  imapButton = new QRadioButton(i18n("&IMAP"), bGrp);
  pop3Button = new QRadioButton(i18n("P&OP3"), bGrp);
  localButton = new QRadioButton(i18n("&Local mailbox"), bGrp);

  wtstr = i18n("This is the protocol used by your incoming e-mail server. Your e-mail provider should have"
     " supplied this information. If you use dial-up networking, you are probably using a POP3 server.");
  QWhatsThis::add( bGrp, wtstr );
  QWhatsThis::add( imapButton, wtstr );
  QWhatsThis::add( pop3Button, wtstr );
  QWhatsThis::add( localButton, wtstr );

  uBox = new QGroupBox(2, Qt::Horizontal, i18n("Preferred email client"),
                        this);
  topLayout->addWidget(uBox);

  emailClient = new KLineEdit(uBox);
  connect(emailClient, SIGNAL(textChanged(const QString &)), this, SLOT(configChanged()));

  bEmailClient = new QPushButton(i18n("Bro&wse..."), uBox);
  connect(bEmailClient, SIGNAL(clicked()), this, SLOT(selectEmailClient()));

  cTerminalClient = new QCheckBox(i18n("Run in &terminal"), uBox);
  connect(cTerminalClient, SIGNAL(clicked()), this, SLOT(configChanged()));

  wtstr = i18n("Enter the path to your preferred email client (KMail, Mutt, etc.) here or"
        " choose it with the <em>Browse...</em> button. If no client is specified here,"
        " KMail will be used (if available) instead.");

  QWhatsThis::add(emailClient, wtstr);
  QWhatsThis::add(bEmailClient, i18n("Press this button to select your favorite email client. Please"
        " note that the file you select has to have the executable attribute set in order to be"
        " accepted."));
	QWhatsThis::add(cTerminalClient, i18n(	"Activate this option if you want the selected email client"
						" to be executed in a terminal (e.g. <em>Konsole</em>)."));

	topLayout->addSpacing(KDialog::spacingHint());

	pSettings = new KEMailSettings();

	load();
}

KEmailConfig::~KEmailConfig()
{
}

void KEmailConfig::configChanged()
{
	emit changed(true);
	blah=true;
}


void KEmailConfig::load(const QString &s)
{
	if (s == QString::null) {
		cProfiles->insertStringList(pSettings->profiles());
		if (pSettings->defaultProfileName() != QString::null)
			load(pSettings->defaultProfileName());
		else {
			if (cProfiles->count()) {
				pSettings->setProfile(cProfiles->text(0));
				load(cProfiles->text(0));
				pSettings->setDefault(cProfiles->text(0));
			} else {
				cProfiles->insertItem(i18n("Default"));
				pSettings->setProfile(i18n("Default"));
				pSettings->setDefault(i18n("Default"));
			}
		}
	} else {
		pSettings->setProfile(s);
		emailAddr->setText(pSettings->getSetting(KEMailSettings::EmailAddress));
		replyAddr->setText(pSettings->getSetting(KEMailSettings::ReplyToAddress));
		organization->setText(pSettings->getSetting(KEMailSettings::Organization));
		userName->setText(pSettings->getSetting(KEMailSettings::InServerLogin));
		password->setText(pSettings->getSetting(KEMailSettings::InServerPass));
		inServer->setText(pSettings->getSetting(KEMailSettings::InServer));
		outServer->setText(pSettings->getSetting(KEMailSettings::OutServer));
		fullName->setText(pSettings->getSetting(KEMailSettings::RealName));

		QString intype = pSettings->getSetting(KEMailSettings::InServerType);
		if (intype == "imap4")
			bGrp->setButton(0);
		else if (intype == "pop3")
			bGrp->setButton(1);
		else if (intype == "localbox")
			bGrp->setButton(2);

		emailClient->setText(pSettings->getSetting(KEMailSettings::ClientProgram));
		cTerminalClient->setChecked((pSettings->getSetting(KEMailSettings::ClientTerminal) == "true"));
		emit changed(false);
		blah=false;
	}
}

void KEmailConfig::save()
{
	pSettings->setSetting(KEMailSettings::RealName, fullName->text());
	pSettings->setSetting(KEMailSettings::EmailAddress, emailAddr->text());
	pSettings->setSetting(KEMailSettings::Organization, organization->text());
	pSettings->setSetting(KEMailSettings::ReplyToAddress, replyAddr->text());
	pSettings->setSetting(KEMailSettings::OutServer, outServer->text());
	pSettings->setSetting(KEMailSettings::InServerLogin, userName->text());
	pSettings->setSetting(KEMailSettings::InServerPass, password->text());
	pSettings->setSetting(KEMailSettings::InServer, inServer->text());

	if (imapButton->isChecked())
		pSettings->setSetting(KEMailSettings::InServerType, "imap4");
	else if (pop3Button->isChecked())
		pSettings->setSetting(KEMailSettings::InServerType, "pop3");
	else if (localButton->isChecked())
		pSettings->setSetting(KEMailSettings::InServerType, "localbox");

	pSettings->setSetting(KEMailSettings::ClientProgram, emailClient->text());
	pSettings->setSetting(KEMailSettings::ClientTerminal, (cTerminalClient->isChecked()) ? "true" : "false");

	// insure proper permissions -- contains sensitive data
	QString cfgName(KGlobal::dirs()->findResource("config", "emaildefaults"));
	if (!cfgName.isEmpty())
		::chmod(cfgName.utf8(), 0600);

	emit changed(false);
	blah=false;
}

void KEmailConfig::defaults()
{
  char hostname[80];
  struct passwd *p;

  p = getpwuid(getuid());
  gethostname(hostname, 80);

  fullName->setText(p->pw_gecos);

  QString tmp = p->pw_name;
  tmp += "@"; tmp += hostname;

  emailAddr->setText(tmp);

  QString client = KGlobal::dirs()->findResource("exe", "kmail");

  if (client.isEmpty())
    client = "kmail";

  emailClient->setText(client);

  cTerminalClient->setChecked(false);

  emit changed(true);
	blah=true;
}

QString KEmailConfig::quickHelp() const
{
  return i18n("<h1>e-mail</h1> This module allows you to enter basic e-mail"
     " information for the current user. The information here is used,"
     " among other things, for sending bug reports to the KDE developers"
     " when you use the bug report dialog.<p>"
     " Note that e-mail programs like KMail and Empath offer many more"
     " features, but they provide their own configuration facilities.");
}

void KEmailConfig::selectEmailClient()
{
  QString client = KFileDialog::getOpenFileName(QString::null, "*", this);

  QFileInfo *clientInfo = new QFileInfo(client);
  if (clientInfo->exists() && clientInfo->isExecutable() && clientInfo->filePath().contains(' ') == 0)
    emailClient->setText(client);
}

void KEmailConfig::profileChanged(const QString &s)
{
	save(); // Save our changes...
	load(s);
}

void KEmailConfig::newProfile()
{
	return; //...
}

extern "C"
{
  KCModule *create_email(QWidget *parent, const char *name)
  {
    KGlobal::locale()->insertCatalogue("kcmemail");
    return new KEmailConfig(parent, name);
  };
}

#include "email.moc"
