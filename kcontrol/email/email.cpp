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
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qlayout.h>
#include <qvariant.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qfile.h>
#include <qfileinfo.h>

#include <iostream.h>
#include <kdebug.h>
#include <kcombobox.h>
#include <kdialog.h>
#include <kglobal.h>
#include <klocale.h>
#include <kfiledialog.h>
#include <kstddirs.h>
#include <kmessagebox.h>
#include <kemailsettings.h>

#include "email.h"

topKCMEmail::topKCMEmail (QWidget* parent,  const char* name)
    : KCModule (parent, name)
{
	m_bChanged=false;
	QVBoxLayout *topLayout = new QVBoxLayout(this, KDialog::marginHint(), KDialog::spacingHint());

	QHBoxLayout *layCurProfile = new QHBoxLayout(topLayout);

	lblCurrentProfile = new QLabel( this, "lblCurrentProfile" );
	lblCurrentProfile->setGeometry( QRect( 6, 6, 83, 22 ) ); 
	lblCurrentProfile->setText( i18n("Current Profile:" ) );
	lblCurrentProfile->setMinimumSize( QSize( 0, 0 ) );
	layCurProfile->addWidget(lblCurrentProfile);

	cmbCurProfile = new KComboBox( FALSE, this, "cmbCurProfile" );
	cmbCurProfile->setGeometry( QRect( 95, 6, 444, 22 ) ); 
	cmbCurProfile->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)3, (QSizePolicy::SizeType)0, cmbCurProfile->sizePolicy().hasHeightForWidth() ) );
	layCurProfile->addWidget(cmbCurProfile);

	QHBoxLayout *layCurProfile2 = new QHBoxLayout(topLayout);
	btnNewProfile = new QPushButton( this, "btnNewProfile" );
	btnNewProfile->setGeometry( QRect( 25, 35, 102, 26 ) ); 
	btnNewProfile->setText( i18n( "&New Profile.." ) );
	btnNewProfile->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
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
	lblFullName->setText("&Full Name:");
	lblFullName->setTextFormat( QLabel::PlainText );
	txtFullName = new QLineEdit( grpUserInfo, "txtFullName" );
	txtFullName->setGeometry( QRect( 122, 22, 406, 22 ) ); 
	connect(txtFullName, SIGNAL(textChanged(const QString&)), this, SLOT(configChanged()));
	lblFullName->setBuddy(txtFullName);

	lblOrganization = new QLabel( grpUserInfo, "lblOrganization" );
	lblOrganization->setGeometry( QRect( 7, 50, 107, 22 ) ); 
	lblOrganization->setText(i18n( "Or&ganization:" ) );
	lblOrganization->setTextFormat( QLabel::PlainText );
	txtOrganization = new QLineEdit( grpUserInfo, "txtOrganization" );
	txtOrganization->setGeometry( QRect( 122, 50, 406, 22 ) ); 
	connect(txtOrganization, SIGNAL(textChanged(const QString&)), this, SLOT(configChanged()));
	lblOrganization->setBuddy(txtOrganization);

	lblEMailAddr = new QLabel( grpUserInfo, "lblEMailAddr" );
	lblEMailAddr->setGeometry( QRect( 7, 78, 107, 22 ) ); 
	lblEMailAddr->setText( i18n( "&E-Mail Address:" ) );
	txtEMailAddr = new QLineEdit( grpUserInfo, "txtEMailAddr" );
	txtEMailAddr->setGeometry( QRect( 122, 78, 406, 22 ) ); 
	connect(txtEMailAddr, SIGNAL(textChanged(const QString&)), this, SLOT(configChanged()));
	lblEMailAddr->setBuddy(txtEMailAddr);

	lblReplyTo = new QLabel( grpUserInfo, "lblReplyTo" );
	lblReplyTo->setGeometry( QRect( 7, 106, 107, 22 ) ); 
	lblReplyTo->setText( i18n( "&Reply-To Address:" ) );
	txtReplyTo = new QLineEdit( grpUserInfo, "txtReplyTo" );
	txtReplyTo->setGeometry( QRect( 122, 106, 406, 22 ) ); 
	connect(txtReplyTo, SIGNAL(textChanged(const QString&)), this, SLOT(configChanged()));
	lblReplyTo->setBuddy(txtReplyTo);

	grpClient = new QGroupBox (2, Qt::Horizontal, this, "grpClient");
	grpClient->setGeometry( QRect( 5, 215, 535, 70 ) ); 
	grpClient->setTitle( i18n( "Preferred E-Mail Client" ) );
	grpClient->setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed));
	topLayout->addWidget(grpClient);

	txtEMailClient = new QLineEdit( grpClient, "txtEMailClient" );
	txtEMailClient->setGeometry( QRect( 5, 20, 410, 22 ) ); 
	connect(txtEMailClient, SIGNAL(textChanged(const QString&)), this, SLOT(configChanged()));

	btnBrowseClient = new QPushButton( grpClient, "btnBrowseClient" );
	btnBrowseClient->setGeometry( QRect( 420, 20, 102, 26 ) ); 
	btnBrowseClient->setText(i18n( "&Browse..." ) );

	chkRunTerminal = new QCheckBox( grpClient, "chkRunTerminal" );
	chkRunTerminal->setGeometry( QRect( 20, 45, 106, 19 ) ); 
	chkRunTerminal->setText(i18n( "Run in &terminal" ) );
	connect(chkRunTerminal, SIGNAL(clicked()), this, SLOT(configChanged()));

	grpIncoming = new QGroupBox(2, Qt::Horizontal, this, "grpIncoming");
	//grpIncoming->setGeometry( QRect( 5, 290, 535, 80 ) ); 
	grpIncoming->setTitle(i18n( "Incoming Mail" ) );
	grpIncoming->setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed));
	topLayout->addWidget(grpIncoming);

	grpICM = new QButtonGroup( grpIncoming, "grpICM" );
	grpICM->setTitle(i18n( "Mail retrieval type" ) );
	grpICM->setGeometry( QRect( 5, 20, 230, 50 ) ); 
	grpICM->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed));

	radIMAP = new QRadioButton( grpICM, "radIMAP" );
	radIMAP->setGeometry( QRect( 10, 20, 52, 19 ) ); 
	radIMAP->setText(i18n( "&IMAP" ) );
	connect(radIMAP, SIGNAL(clicked()), this, SLOT(configChanged()));

	radPOP = new QRadioButton( grpICM, "radPOP" );
	radPOP->setGeometry( QRect( 70, 20, 47, 19 ) ); 
	radPOP->setText(i18n( "&POP" ) );
	connect(radPOP, SIGNAL(clicked()), this, SLOT(configChanged()));

	radICMLocal = new QRadioButton( grpICM, "radICMLocal" );
	radICMLocal->setGeometry( QRect( 120, 20, 98, 19 ) ); 
	radICMLocal->setText(i18n( "Local &mailbox" ) );
	connect(radICMLocal, SIGNAL(clicked()), this, SLOT(configChanged()));

	btnICMSettings = new QPushButton( grpIncoming, "btnICMRemoteSettings" );
	btnICMSettings->setGeometry( QRect( 245, 45, 175, 26 ) ); 
	btnICMSettings->setText(i18n( "Incoming mailbox settings..." ) );
	btnICMSettings->setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed));

	grpOutgoing = new QGroupBox( 2, Qt::Horizontal, this, "grpOutgoing" );
	grpOutgoing->setGeometry( QRect( 5, 370, 535, 80 ) ); 
	grpOutgoing->setTitle( i18n( "Outgoing Mail Settings" ) );
	grpOutgoing->setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed));
	topLayout->addWidget(grpOutgoing);

	grpOGM = new QButtonGroup(grpOutgoing, "grpOGM" );
	grpOGM->setGeometry( QRect( 5, 20, 230, 50 ) ); 
	grpOGM->setTitle( i18n( "Mail Delivery Method" ) );
	grpOGM->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed));

	radSMTP = new QRadioButton( grpOGM, "radSMTP" );
	radSMTP->setGeometry( QRect( 10, 20, 55, 19 ) ); 
	radSMTP->setText( i18n("SMTP") );
	connect(radSMTP, SIGNAL(clicked()), this, SLOT(configChanged()));

	radOGMLocal = new QRadioButton( grpOGM, "radOGMLocal" );
	radOGMLocal->setGeometry( QRect( 75, 20, 103, 19 ) ); 
	radOGMLocal->setText( i18n("Local Delivery") );
	connect(radOGMLocal, SIGNAL(clicked()), this, SLOT(configChanged()));

	btnOGMSettings = new QPushButton( grpOutgoing, "btnOGMSMTPSettings" );
	btnOGMSettings->setGeometry( QRect( 245, 15, 175, 26 ) ); 
	btnOGMSettings->setText(i18n( "Outgoing mailbox settings..." ));
	btnOGMSettings->setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed));

	grpOGM->setGeometry( QRect( 5, 20, 230, 50 ) ); 
	grpICM->setGeometry( QRect( 5, 20, 230, 50 ) ); 


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
			grpICM->setButton(0);
		else if (intype == "pop3")
			grpICM->setButton(1);
		else if (intype == "localbox")
			grpICM->setButton(2);

		QString outtype = pSettings->getSetting(KEMailSettings::OutServerType);
		if (outtype == "smtp") {
			grpOGM->setButton(0);
		} else if (outtype == "local") {
			grpOGM->setButton(1);
		}

		txtEMailClient->setText(pSettings->getSetting(KEMailSettings::ClientProgram));
		chkRunTerminal->setChecked((pSettings->getSetting(KEMailSettings::ClientTerminal) == "true"));
		emit changed(false);
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
	grpICM->setButton(0);
	txtEMailClient->setText(QString::null);
	chkRunTerminal->setChecked(false);
	emit changed(false);
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
	QLineEdit *txtName = new QLineEdit(dlgAskName);
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

void topKCMEmail::configChanged()
{
        emit changed(true);
        m_bChanged=true;
}

void topKCMEmail::save()
{
	pSettings->setProfile(cmbCurProfile->text(cmbCurProfile->currentItem()));
	pSettings->setSetting(KEMailSettings::RealName, txtFullName->text());
	pSettings->setSetting(KEMailSettings::EmailAddress, txtEMailAddr->text());
	pSettings->setSetting(KEMailSettings::Organization, txtOrganization->text());
	pSettings->setSetting(KEMailSettings::ReplyToAddress, txtReplyTo->text());

	if (radIMAP->isChecked())
		pSettings->setSetting(KEMailSettings::InServerType, "imap4");
	else if (radPOP->isChecked())
		pSettings->setSetting(KEMailSettings::InServerType, "pop3");
	else if (radICMLocal->isChecked())
		pSettings->setSetting(KEMailSettings::InServerType, "localbox");

	if (radSMTP->isChecked()) {
		pSettings->setSetting(KEMailSettings::OutServerType, "smtp");
	} else if (radOGMLocal->isChecked()) {
		pSettings->setSetting(KEMailSettings::OutServerType, "local");
	}

	pSettings->setSetting(KEMailSettings::ClientProgram, txtEMailClient->text());
	pSettings->setSetting(KEMailSettings::ClientTerminal, (chkRunTerminal->isChecked()) ? "true" : "false");

	// insure proper permissions -- contains sensitive data
	QString cfgName(KGlobal::dirs()->findResource("config", "emaildefaults"));
	if (!cfgName.isEmpty())
		::chmod(cfgName.utf8(), 0600);

	emit changed(false);
	m_bChanged=false;
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

	emit changed(true);
	m_bChanged=true;
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

extern "C"
{
	KCModule *create_email(QWidget *parent, const char *name) {
		KGlobal::locale()->insertCatalogue("kcmemail");
		return new topKCMEmail(parent, name);
	};
}

#include "email.moc"
