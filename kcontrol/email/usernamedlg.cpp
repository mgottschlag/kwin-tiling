// $Id$

#include <qlineedit.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qlayout.h>
#include <qwhatsthis.h>

#include <klocale.h>

#include "usernamedlg.h"

UserNameDlg::UserNameDlg (QWidget *parent, const QString &caption)
	: KDialogBase (parent, caption.latin1(), true, caption, KDialogBase::Ok|KDialogBase::Cancel)
{
	QString wtstr;
	QWidget *top = new QWidget(this, "TOPWIDGET");
	setMainWidget(top);

	QVBoxLayout *topLayout = new QVBoxLayout(top, KDialog::marginHint(), KDialog::spacingHint());
	topLayout->setMargin(0);

	QHBoxLayout *layUsername = new QHBoxLayout(topLayout);
	layUsername->setMargin(0);
	lblUsername = new QLabel(top, "lblUsername");
	lblUsername->setText(i18n("&Username:"));
	lblUsername->setSizePolicy( QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed) );
	txtUsername = new QLineEdit(top, "txtUsername");
	txtUsername->setSizePolicy( QSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed) );
	lblUsername->setBuddy(txtUsername);
	layUsername->addWidget(lblUsername);
	layUsername->addWidget(txtUsername);
	wtstr = i18n(	"The user name you use to login to your e-mail server (sometimes just called \"login\")."
			" Your e-mail provider should have supplied this information. Your login name is often (but"
			" not always) identical to the part of your email address that comes before the \"@\".");
	QWhatsThis::add(lblUsername, wtstr);
	QWhatsThis::add(txtUsername, wtstr);

	QHBoxLayout *layPass = new QHBoxLayout(topLayout);
	layPass->setMargin(0);
	lblPass = new QLabel(top, "lblPass");
	lblPass->setText(i18n("&Password:"));
	lblPass->setSizePolicy( QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed) );
	txtPass = new QLineEdit(top, "txtPass");
	txtPass->setSizePolicy( QSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed) );
	lblPass->setBuddy(txtPass);
	layPass->addWidget(lblPass);
	layPass->addWidget(txtPass);

	chkTLS = new QCheckBox(top, "chkTLS");
	chkTLS->setChecked(false);
	chkTLS->setText(i18n("Use secure transport layer if available"));
	topLayout->addWidget(chkTLS);

}

UserNameDlg::~UserNameDlg ()
{
}
