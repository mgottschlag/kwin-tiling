// $Id$

#include <sys/types.h>
#include <sys/stat.h>

#include <pwd.h>
#include <unistd.h>

#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qframe.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qlayout.h>
#include <qvbox.h>
#include <qvariant.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qfile.h>
#include <qfileinfo.h>

#include <kdebug.h>
#include <klineedit.h>
#include <kcombobox.h>
#include <kdialog.h>
#include <kglobal.h>
#include <klocale.h>
#include <kfiledialog.h>
#include <kstddirs.h>
#include <kmessagebox.h>
#include <kemailsettings.h>

#include "email.h"
#include "usernamedlg.h"

topKCMEmail::topKCMEmail (QWidget *parent,  const char *name)
	: KCModule (parent, name)
{
	QString wtstr;
	m_bChanged=false;
	QVBoxLayout *topLayout = new QVBoxLayout(this, KDialog::marginHint(), KDialog::spacingHint());

	QHBoxLayout *layCurProfile = new QHBoxLayout(topLayout);

	lblCurrentProfile = new QLabel( this, "lblCurrentProfile" );
	lblCurrentProfile->setGeometry( QRect( 6, 6, 83, 22 ) );
	lblCurrentProfile->setText( i18n("Cu&rrent Profile:" ) );
	lblCurrentProfile->setMinimumSize( QSize( 0, 0 ) );
	layCurProfile->addWidget(lblCurrentProfile);

	cmbCurProfile = new KComboBox( FALSE, this, "cmbCurProfile" );
	cmbCurProfile->setGeometry( QRect( 95, 6, 444, 22 ) );
	cmbCurProfile->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)0, cmbCurProfile->sizePolicy().hasHeightForWidth() ) );
	layCurProfile->addWidget(cmbCurProfile);
	connect(cmbCurProfile, SIGNAL(activated(const QString &)), this, SLOT(slotComboChanged(const QString &)));
	lblCurrentProfile->setBuddy(cmbCurProfile);

	QHBoxLayout *layCurProfile2 = new QHBoxLayout(topLayout);
	btnNewProfile = new QPushButton( this, "btnNewProfile" );
	btnNewProfile->setGeometry( QRect( 25, 35, 102, 26 ) );
	btnNewProfile->setText( i18n( "&New Profile.." ) );
	btnNewProfile->setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed));
	connect(btnNewProfile, SIGNAL(clicked()), this, SLOT(slotNewProfile()));
	layCurProfile2->addWidget(btnNewProfile);
	layCurProfile2->addStretch(9);

	grpUserInfo = new QGroupBox( 2, Qt::Horizontal, this, "grpUserInfo" );
	grpUserInfo->setGeometry( QRect( 5, 70, 535, 140 ) );
	grpUserInfo->setTitle( i18n( "User Information" ) );
	grpUserInfo->setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed));
	topLayout->addWidget(grpUserInfo);

	lblFullName = new QLabel( grpUserInfo, "lblFullName" );
	lblFullName->setGeometry( QRect( 7, 22, 107, 22 ) );
	lblFullName->setText(i18n("&Full Name:"));
	lblFullName->setTextFormat( QLabel::PlainText );
	txtFullName = new KLineEdit( grpUserInfo, "txtFullName" );
	txtFullName->setGeometry( QRect( 122, 22, 406, 22 ) );
	connect(txtFullName, SIGNAL(textChanged(const QString&)), this, SLOT(configChanged()));
	lblFullName->setBuddy(txtFullName);

	wtstr = i18n(	"Enter your full name here, e.g. \"John Doe\" (without the quotation"
			" marks).  Some people like to provide a nick name only. You can leave this field"
			" blank and still use email. However, providing your full name is <em>recommended</em> as"
			" this makes it much easier for your recipient to browse his or her email.");
	QWhatsThis::add( lblFullName, wtstr );
	QWhatsThis::add( txtFullName, wtstr );

	lblOrganization = new QLabel( grpUserInfo, "lblOrganization" );
	lblOrganization->setGeometry( QRect( 7, 50, 107, 22 ) );
	lblOrganization->setText(i18n( "Or&ganization:" ) );
	lblOrganization->setTextFormat( QLabel::PlainText );
	txtOrganization = new KLineEdit( grpUserInfo, "txtOrganization" );
	txtOrganization->setGeometry( QRect( 122, 50, 406, 22 ) );
	connect(txtOrganization, SIGNAL(textChanged(const QString&)), this, SLOT(configChanged()));
	lblOrganization->setBuddy(txtOrganization);
	wtstr = i18n(	"Here you can enter the name of your organization, company"
			" or university. This field is <em>optional</em>. However, if"
			" you are using a business account and communicate with persons working for other"
			" companies, providing the name of your organization is recommended.");
	QWhatsThis::add( txtOrganization, wtstr );
	QWhatsThis::add( lblOrganization, wtstr );

	lblEMailAddr = new QLabel( grpUserInfo, "lblEMailAddr" );
	lblEMailAddr->setGeometry( QRect( 7, 78, 107, 22 ) );
	lblEMailAddr->setText( i18n( "&E-Mail Address:" ) );
	txtEMailAddr = new KLineEdit( grpUserInfo, "txtEMailAddr" );
	txtEMailAddr->setGeometry( QRect( 122, 78, 406, 22 ) );
	connect(txtEMailAddr, SIGNAL(textChanged(const QString&)), this, SLOT(configChanged()));
	lblEMailAddr->setBuddy(txtEMailAddr);
	wtstr = i18n(	"Enter your email address here, e.g. \"john@doe.com\" (without "
			"the quotation marks). This information is mandatory if you want to use email.<p>"
			"Do <em>not</em> enter something like \"John Doe &lt;john@doe.com&gt;\", just a plain email address. "
			"Your email address may not contain any blank spaces.");
	QWhatsThis::add( txtEMailAddr, wtstr );
	QWhatsThis::add( lblEMailAddr, wtstr );


	lblReplyTo = new QLabel( grpUserInfo, "lblReplyTo" );
	lblReplyTo->setGeometry( QRect( 7, 106, 107, 22 ) );
	lblReplyTo->setText( i18n( "Reply-&To Address:" ) );
	txtReplyTo = new KLineEdit( grpUserInfo, "txtReplyTo" );
	txtReplyTo->setGeometry( QRect( 122, 106, 406, 22 ) );
	connect(txtReplyTo, SIGNAL(textChanged(const QString&)), this, SLOT(configChanged()));
	lblReplyTo->setBuddy(txtReplyTo);
	wtstr = i18n(	"You can set a reply address if you want replies to your e-mail messages"
			" to go to a different address than the e-mail address above. Most likely, you should"
			" leave the reply address blank, so replies go to the e-mail address you entered above.<p>"
			" <em>Please note:</em> <ul><li>You do not need to enter the same email"
			" address as above.</li><li>You should not use a reply address if you frequently"
			" use discussion mailing lists.</li></ul>");
	QWhatsThis::add( lblReplyTo, wtstr );
	QWhatsThis::add( txtReplyTo, wtstr );

	grpClient = new QGroupBox (2, Qt::Horizontal, this, "grpClient");
	grpClient->setGeometry( QRect( 5, 215, 535, 70 ) );
	grpClient->setTitle( i18n( "Preferred E-Mail Client" ) );
	grpClient->setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed));
	topLayout->addWidget(grpClient);

	txtEMailClient = new KLineEdit( grpClient, "txtEMailClient" );
	txtEMailClient->setGeometry( QRect( 5, 20, 410, 22 ) );
	connect(txtEMailClient, SIGNAL(textChanged(const QString&)), this, SLOT(configChanged()));

	btnBrowseClient = new QPushButton( grpClient, "btnBrowseClient" );
	btnBrowseClient->setGeometry( QRect( 420, 20, 102, 26 ) );
	btnBrowseClient->setText(i18n( "&Browse..." ) );
	connect(btnBrowseClient, SIGNAL(clicked()), this, SLOT(selectEmailClient()));

	chkRunTerminal = new QCheckBox( grpClient, "chkRunTerminal" );
	chkRunTerminal->setGeometry( QRect( 20, 45, 106, 19 ) );
	chkRunTerminal->setText(i18n( "Run in &terminal" ) );
	connect(chkRunTerminal, SIGNAL(clicked()), this, SLOT(configChanged()));

	wtstr = i18n(	"Enter the path to your preferred email client (KMail, Mutt, etc.) here or"
			" choose it with the <em>Browse...</em> button. If no client is specified here,"
			" KMail will be used (if available) instead.");
	QWhatsThis::add(txtEMailClient, wtstr);
	QWhatsThis::add(btnBrowseClient, i18n(	"Press this button to select your favorite email client. Please"
						" note that the file you select has to have the executable attribute set in order to be"
						" accepted."));
	QWhatsThis::add(chkRunTerminal, i18n(	"Activate this option if you want the selected email client"
						" to be executed in a terminal (e.g. <em>Konsole</em>)."));
	grpIncoming = new QButtonGroup(2, Qt::Horizontal, this, "grpIncoming");
	grpIncoming->setGeometry( QRect( 5, 290, 535, 80 ) );
	grpIncoming->setTitle(i18n( "Incoming Mail" ) );
	grpIncoming->setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed));
	topLayout->addWidget(grpIncoming);

	grpICM = new QVBox(grpIncoming);

	radIMAP = new QRadioButton( grpICM, "radIMAP" );
	radIMAP->setGeometry( QRect( 10, 20, 52, 19 ) );
	radIMAP->setText(i18n( "I&MAP" ) );
	connect(radIMAP, SIGNAL(clicked()), this, SLOT(configChanged()));

	radPOP = new QRadioButton( grpICM, "radPOP" );
	radPOP->setGeometry( QRect( 70, 20, 47, 19 ) );
	radPOP->setText(i18n( "&POP" ) );
	connect(radPOP, SIGNAL(clicked()), this, SLOT(configChanged()));

	radICMLocal = new QRadioButton( grpICM, "radICMLocal" );
	radICMLocal->setGeometry( QRect( 120, 20, 98, 19 ) );
	radICMLocal->setText(i18n( "Local M&ailbox" ) );
	connect(radICMLocal, SIGNAL(clicked()), this, SLOT(configChanged()));

	grpIncoming->insert(radIMAP);
	grpIncoming->insert(radPOP);
	grpIncoming->insert(radICMLocal);

	wtstr = i18n(	"This is the protocol used by your incoming e-mail server. Your e-mail provider should have"
			" supplied this information. If you use dial-up networking, you are probably using a POP3 server.");
	QWhatsThis::add(grpIncoming, wtstr);
	QWhatsThis::add(radPOP, wtstr);
	QWhatsThis::add(radIMAP, wtstr);
	QWhatsThis::add(radICMLocal, wtstr);

	btnICMSettings = new QPushButton( grpIncoming, "btnICMRemoteSettings" );
	btnICMSettings->setGeometry( QRect( 245, 45, 175, 26 ) );
	btnICMSettings->setText(i18n( "&Incoming mailbox settings..." ) );
	btnICMSettings->setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed));
	connect(btnICMSettings, SIGNAL(clicked()), this, SLOT(slotICMSettings()));

	grpOutgoing = new QButtonGroup( 2, Qt::Horizontal, this, "grpOutgoing" );
	grpOutgoing->setGeometry( QRect( 5, 370, 535, 80 ) );
	grpOutgoing->setTitle( i18n( "Outgoing Mail" ) );
	grpOutgoing->setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed));
	topLayout->addWidget(grpOutgoing, 5);

	grpOGM = new QVBox(grpOutgoing);

	radSMTP = new QRadioButton( grpOGM, "radSMTP" );
	radSMTP->setGeometry( QRect( 10, 20, 55, 19 ) );
	radSMTP->setText( i18n("&SMTP") );
	connect(radSMTP, SIGNAL(clicked()), this, SLOT(configChanged()));

	radOGMLocal = new QRadioButton( grpOGM, "radOGMLocal" );
	radOGMLocal->setGeometry( QRect( 75, 20, 103, 19 ) );
	radOGMLocal->setText( i18n("&Local Delivery") );
	connect(radOGMLocal, SIGNAL(clicked()), this, SLOT(configChanged()));

	grpOutgoing->insert(radSMTP);
	grpOutgoing->insert(radOGMLocal);

	btnOGMSettings = new QPushButton( grpOutgoing, "btnOGMSMTPSettings" );
	btnOGMSettings->setGeometry( QRect( 245, 15, 175, 26 ) );
	btnOGMSettings->setText(i18n( "&Outgoing mailbox settings..." ));
	btnOGMSettings->setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed));
	connect(btnOGMSettings, SIGNAL(clicked()), this, SLOT(slotOGMSettings()));

	grpOGM->setGeometry( QRect( 5, 20, 230, 50 ) );
	grpICM->setGeometry( QRect( 5, 20, 230, 50 ) );

	btnOGMSettings->setEnabled(false);
	btnICMSettings->setEnabled(false);

	// tab order
	setTabOrder( cmbCurProfile, btnNewProfile );
	setTabOrder( btnNewProfile, txtFullName );
	setTabOrder( txtFullName, txtOrganization );
	setTabOrder( txtOrganization, txtEMailAddr );
	setTabOrder( txtEMailAddr, txtReplyTo );
	setTabOrder( txtReplyTo, txtEMailClient );
	setTabOrder( txtEMailClient, btnBrowseClient );
	setTabOrder( btnBrowseClient, chkRunTerminal );
	setTabOrder( chkRunTerminal, radIMAP );
	setTabOrder( radIMAP, radPOP );
	setTabOrder( radPOP, radICMLocal );
	setTabOrder( radICMLocal, btnICMSettings );
	setTabOrder( btnICMSettings, radSMTP );
	setTabOrder( radSMTP, radOGMLocal );
	setTabOrder( radOGMLocal, btnOGMSettings );

	pSettings = new KEMailSettings();
	load();
}

void topKCMEmail::load()
{
       cmbCurProfile->clear();
       load(QString::null);
}

void topKCMEmail::load(const QString &s)
{
	if (s == QString::null) {
		cmbCurProfile->insertStringList(pSettings->profiles());
		if (pSettings->defaultProfileName() != QString::null) {
			kdDebug() << "prfile is: " << pSettings->defaultProfileName()<< "line is " << __LINE__ << endl;
			load(pSettings->defaultProfileName());
		} else {
			if (cmbCurProfile->count()) {
				pSettings->setProfile(cmbCurProfile->text(0));
				kdDebug() << "prfile is: " << cmbCurProfile->text(0) << endl;
				load(cmbCurProfile->text(0));
				pSettings->setDefault(cmbCurProfile->text(0));
			} else {
				cmbCurProfile->insertItem(i18n("Default"));
				pSettings->setProfile(i18n("Default"));
				pSettings->setDefault(i18n("Default"));
			}
		}
	} else {
		pSettings->setProfile(s);
		txtEMailAddr->setText(pSettings->getSetting(KEMailSettings::EmailAddress));
		txtReplyTo->setText(pSettings->getSetting(KEMailSettings::ReplyToAddress));
		txtOrganization->setText(pSettings->getSetting(KEMailSettings::Organization));
		txtFullName->setText(pSettings->getSetting(KEMailSettings::RealName));

		QString intype = pSettings->getSetting(KEMailSettings::InServerType);
		if (intype == "imap4")
			grpIncoming->setButton(0);
		else if (intype == "pop3")
			grpIncoming->setButton(1);
		else if (intype == "localbox")
			grpIncoming->setButton(2);

		QString outtype = pSettings->getSetting(KEMailSettings::OutServerType);
		if (outtype == "smtp") {
			radSMTP->setChecked(true);
		} else if (outtype == "local") {
			radOGMLocal->setChecked(true);
		}

		txtEMailClient->setText(pSettings->getSetting(KEMailSettings::ClientProgram));
		chkRunTerminal->setChecked((pSettings->getSetting(KEMailSettings::ClientTerminal) == "true"));

		m_sICMPassword = pSettings->getSetting(KEMailSettings::InServerPass);
		m_sICMUsername = pSettings->getSetting(KEMailSettings::InServerLogin);
		if (radICMLocal->isChecked()) {
			m_sICMPath = pSettings->getSetting(KEMailSettings::InServer);
			m_sICMHost = QString::null;
		} else {
			m_sICMHost = pSettings->getSetting(KEMailSettings::InServer);
			m_sICMPath = QString::null;
		}

		m_sOGMPassword = pSettings->getSetting(KEMailSettings::OutServerPass);
		m_sOGMUsername = pSettings->getSetting(KEMailSettings::OutServerLogin);
		m_sOGMHost = pSettings->getSetting(KEMailSettings::OutServer);
		m_sOGMCommand = pSettings->getSetting(KEMailSettings::OutServerCommand);

		configChanged(false);
	}
}

topKCMEmail::~topKCMEmail()
{
}

void topKCMEmail::clearData()
{
	txtEMailAddr->setText(QString::null);
	txtReplyTo->setText(QString::null);
	txtOrganization->setText(QString::null);
	txtFullName->setText(QString::null);

	radIMAP->setChecked(false);
	radPOP->setChecked(false);
	radICMLocal->setChecked(false);
	radSMTP->setChecked(false);
	radOGMLocal->setChecked(false);

	txtEMailClient->setText(QString::null);
	chkRunTerminal->setChecked(false);

	m_sICMPassword = QString::null;
	m_sICMUsername = QString::null;
	m_sICMPath = QString::null;
	m_sICMHost = QString::null;
	m_bICMSecure = false;
	m_uICMPort = 0;

	m_sOGMPassword = QString::null;
	m_sOGMUsername = QString::null;
	m_sOGMCommand = QString::null;
	m_sOGMHost = QString::null;
	m_bOGMSecure = false;
	m_uOGMPort = 0;

	configChanged(false);
}

void topKCMEmail::slotNewProfile()
{
	KDialog *dlgAskName = new KDialog(this, "noname", true);
	dlgAskName->setCaption(i18n("New E-Mail Profile"));

	QVBoxLayout *vlayout = new QVBoxLayout(dlgAskName, KDialog::marginHint(), KDialog::spacingHint());

	QHBoxLayout *layout = new QHBoxLayout(vlayout);

	QLabel *lblName = new QLabel(dlgAskName);
	lblName->setText(i18n("Name:"));
	lblName->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed));
	KLineEdit *txtName = new KLineEdit(dlgAskName);
	lblName->setBuddy(txtName);
	layout->addWidget(lblName);
	layout->addWidget(txtName);

	layout = new QHBoxLayout(vlayout);
	QPushButton *btnOK = new QPushButton(dlgAskName);
	btnOK->setText(i18n("&OK"));
	btnOK->setFixedSize(btnOK->sizeHint());
	QPushButton *btnCancel = new QPushButton(dlgAskName);
	btnCancel->setText(i18n("&Cancel"));
	btnCancel->setFixedSize(btnCancel->sizeHint());
	layout->addWidget(btnOK);
	layout->addWidget(btnCancel);
	connect(btnOK, SIGNAL(clicked()), dlgAskName, SLOT(accept()));
	connect(btnCancel, SIGNAL(clicked()), dlgAskName, SLOT(reject()));
	txtName->setFocus();

	if (dlgAskName->exec() == QDialog::Accepted) {
		if (txtName->text().isEmpty()) {
			KMessageBox::sorry(this, i18n("Oops, you need to enter a name please, thanks."));
		} else if (cmbCurProfile->contains(txtName->text()))
			KMessageBox::sorry(this, i18n("This e-mail profile already exists, and cannot be created again"), i18n("Oops"));
		else {
			pSettings->setProfile(txtName->text());
			cmbCurProfile->insertItem(txtName->text());
			// should probbaly load defaults instead
			clearData();
			cmbCurProfile->setCurrentItem(cmbCurProfile->count()-1);
		}
	} else { // rejected
	}

	delete dlgAskName;
}

void topKCMEmail::configChanged(bool c)
{
	btnOGMSettings->setEnabled((radSMTP->isChecked() || radOGMLocal->isChecked()));
	btnICMSettings->setEnabled((radIMAP->isChecked() || radPOP->isChecked() || radICMLocal->isChecked()));
	emit changed(c);
	m_bChanged=c;
}

void topKCMEmail::configChanged()
{
	configChanged(true);
}

void topKCMEmail::save()
{
	//pSettings->setProfile(cmbCurProfile->text(cmbCurProfile->currentItem()));
	pSettings->setSetting(KEMailSettings::RealName, txtFullName->text());
	pSettings->setSetting(KEMailSettings::EmailAddress, txtEMailAddr->text());
	pSettings->setSetting(KEMailSettings::Organization, txtOrganization->text());
	pSettings->setSetting(KEMailSettings::ReplyToAddress, txtReplyTo->text());

	if (radIMAP->isChecked() || radPOP->isChecked()) {
		if (radPOP->isChecked())
			pSettings->setSetting(KEMailSettings::InServerType, "pop3");
		else
			pSettings->setSetting(KEMailSettings::InServerType, "imap4");
		pSettings->setSetting(KEMailSettings::InServerLogin, m_sICMUsername);
		pSettings->setSetting(KEMailSettings::InServerPass, m_sICMPassword);
		pSettings->setSetting(KEMailSettings::InServerTLS, m_bICMSecure ? "true" : "false");
		pSettings->setSetting(KEMailSettings::InServer, m_sICMHost);
	} else if (radICMLocal->isChecked()) {
		pSettings->setSetting(KEMailSettings::InServerType, "localbox");
		pSettings->setSetting(KEMailSettings::InServerLogin, QString::null);
		pSettings->setSetting(KEMailSettings::InServerPass, QString::null);
		pSettings->setSetting(KEMailSettings::InServer, m_sICMPath);
	}

	if (radSMTP->isChecked()) {
		pSettings->setSetting(KEMailSettings::OutServerType, "smtp");
		pSettings->setSetting(KEMailSettings::OutServer, m_sOGMHost);
		pSettings->setSetting(KEMailSettings::OutServerLogin, m_sOGMUsername);
		pSettings->setSetting(KEMailSettings::OutServerPass, m_sOGMPassword);
		pSettings->setSetting(KEMailSettings::OutServerCommand, QString::null);
		pSettings->setSetting(KEMailSettings::OutServerTLS, m_bICMSecure ? "true" : "false");
	} else if (radOGMLocal->isChecked()) {
		pSettings->setSetting(KEMailSettings::OutServerType, "local");
		pSettings->setSetting(KEMailSettings::OutServer, QString::null);
		pSettings->setSetting(KEMailSettings::OutServerLogin, QString::null);
		pSettings->setSetting(KEMailSettings::OutServerPass, QString::null);
		pSettings->setSetting(KEMailSettings::OutServerCommand, m_sOGMCommand);
	}

	pSettings->setSetting(KEMailSettings::ClientProgram, txtEMailClient->text());
	pSettings->setSetting(KEMailSettings::ClientTerminal, (chkRunTerminal->isChecked()) ? "true" : "false");

	// insure proper permissions -- contains sensitive data
	QString cfgName(KGlobal::dirs()->findResource("config", "emaildefaults"));
	if (!cfgName.isEmpty())
		::chmod(cfgName.utf8(), 0600);

	configChanged(false);
}

void topKCMEmail::defaults()
{
	char hostname[80];
	struct passwd *p;

	p = getpwuid(getuid());
	gethostname(hostname, 80);

	txtFullName->setText(p->pw_gecos);

	QString tmp = p->pw_name;
	tmp += "@"; tmp += hostname;

	txtEMailAddr->setText(tmp);

	QString client = KGlobal::dirs()->findResource("exe", "kmail");

	if (client.isEmpty())
		client = "kmail";

	txtEMailClient->setText(client);

	chkRunTerminal->setChecked(false);

	radPOP->setChecked(true);
	m_bICMSecure = false;
	m_sICMUsername = QString::null;
	m_sICMPassword = QString::null;
	m_sICMHost = QString::fromLatin1("pop");
	m_uICMPort = 110;

	radSMTP->setChecked(true);
	m_bOGMSecure = false;
	m_sOGMUsername = QString::null;
	m_sOGMPassword = QString::null;
	m_sOGMHost = QString::fromLatin1("mail");
	m_uOGMPort = 25; // we should use getservent here.. soon

	configChanged();
}

QString topKCMEmail::quickHelp() const
{
	return i18n("<h1>e-mail</h1> This module allows you to enter basic e-mail"
	            " information for the current user. The information here is used,"
	            " among other things, for sending bug reports to the KDE developers"
	            " when you use the bug report dialog.<p>"
	            " Note that e-mail programs like KMail and Empath offer many more"
	            " features, but they provide their own configuration facilities.");
}

void topKCMEmail::selectEmailClient()
{
	QString client = KFileDialog::getOpenFileName(QString::null, "*", this);

	QFileInfo *clientInfo = new QFileInfo(client);
	if (clientInfo->exists() && clientInfo->isExecutable() && clientInfo->filePath().contains(' ') == 0)
		txtEMailClient->setText(client);
}

void topKCMEmail::profileChanged(const QString &s)
{
	save(); // Save our changes...
	load(s);
}

void topKCMEmail::slotComboChanged(const QString &name)
{
	if (m_bChanged) {
		if (KMessageBox::warningYesNo(this, i18n("Do you wish to discard changes to the current profile?")) == KMessageBox::No) {
			if (KMessageBox::warningYesNo(this, i18n("Do you wish to save changes to the current profile?")) == KMessageBox::Yes) {
				save();
			} else {
				int keep=-1;
				for (int i=0; i < cmbCurProfile->count(); i++) {
					if (cmbCurProfile->text(i) == pSettings->currentProfileName()) {
						keep=i; break;
					}
				}
				if (keep != -1)
					cmbCurProfile->setCurrentItem(keep);
				return;
			}
		}
	}
	load(name);
}

void topKCMEmail::slotICMSettings()
{
	if (!radICMLocal->isChecked()) {
		UserNameDlg *ud = new UserNameDlg(this, i18n("Incoming Mail Retrieval Settings"));
		ud->txtUsername->setText(m_sICMUsername);
		ud->txtPass->setText(m_sICMPassword);
		ud->txtHost->setText(m_sICMHost);
		if (ud->exec() == QDialog::Accepted) {
			m_sICMUsername = ud->txtUsername->text();
			m_bICMSecure = ud->chkTLS->isChecked();
			m_sICMPassword = ud->txtPass->text();
			m_sICMHost = ud->txtHost->text();
			m_uICMPort = ud->txtPort->text().toUInt();
			configChanged();
		}
		delete ud;
		return;
	} else {
		KFileDialog *kd = new KFileDialog(QString::null, "*", this, "kd", true);
		kd->setMode(static_cast<KFile::Mode>( KFile::File | KFile::Directory | KFile::ExistingOnly | KFile::LocalOnly));
		kd->setCaption(i18n("Choose incoming mailbox path"));
		kd->exec();
		if (!kd->selectedFile().isEmpty()) {
			QFileInfo *clientInfo = new QFileInfo(kd->selectedFile());
			if (clientInfo->exists())
				m_sICMPath=kd->selectedFile();
		}
		delete kd;
		return;
	}
}

void topKCMEmail::slotOGMSettings()
{
	if (!radOGMLocal->isChecked()) {
		UserNameDlg *ud = new UserNameDlg(this, i18n("Outgoing Mail Retrieval Settings"));
		ud->txtUsername->setText(m_sOGMUsername);
		ud->txtPass->setText(m_sOGMPassword);
		ud->txtHost->setText(m_sOGMHost);
		if (ud->exec() == QDialog::Accepted) {
			m_bOGMSecure = ud->chkTLS->isChecked();
			m_sOGMUsername = ud->txtUsername->text();
			m_sOGMPassword = ud->txtPass->text();
			m_sOGMHost = ud->txtHost->text();
			m_uOGMPort = ud->txtPort->text().toUInt();

			configChanged();
		}
		delete ud;
		return;
	} else {
		QString command = KFileDialog::getOpenFileName(QString::null, "*", this, i18n("Choose outgoing mailer"));
		QFileInfo *clientInfo = new QFileInfo(command);
		if (clientInfo->exists() && clientInfo->isExecutable() && clientInfo->filePath().contains(' ') == 0)
			m_sOGMCommand=command;
	}
}

extern "C"
{
	KCModule *create_email(QWidget *parent, const char *name) {
		KGlobal::locale()->insertCatalogue("kcmemail");
		return new topKCMEmail(parent, name);
	};
}

#include "email.moc"
