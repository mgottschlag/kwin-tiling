/*
 *
 * $Id$
 *
 * This file is part of the KDE project, module kcontrol.
 * Copyright (C) 1999-2001 by Alex Zepeda
 *
 * You can freely distribute this program under the following terms:
 *
 * 1.) If you use this program in a product outside of KDE, you must 
 * credit the authors, and must mention "kcmemail".
 *
 * 2.) If this program is used in a product other than KDE, this license 
 * must be reproduced in its entirety in the documentation and/or some 
 * other highly visible location (e.g. startup splash screen)
 * 
 * 3.) Use of this product implies the following terms:
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

#include <qlabel.h>
#include <qcheckbox.h>
#include <qlayout.h>
#include <qwhatsthis.h>

#include <klineedit.h>
#include <klocale.h>

#include "usernamedlg.h"

UserNameDlg::UserNameDlg (QWidget *parent, const QString &caption)
	: KDialogBase (parent, "usernamedlg", true, caption, KDialogBase::Ok|KDialogBase::Cancel)
{
	QString wtstr;
	QWidget *top = new QWidget(this, "TOPWIDGET");
	setMainWidget(top);

	QVBoxLayout *topLayout = new QVBoxLayout(top, KDialog::marginHint(), KDialog::spacingHint());
	topLayout->setMargin(0);

	QHBoxLayout *layHost = new QHBoxLayout(topLayout);
	layHost->setMargin(0);
	lblHost = new QLabel(top, "lblHost");
	lblHost->setText(i18n("&Hostname:"));
	//lblHost->setSizePolicy( QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed) );
	txtHost = new KLineEdit(top, "txtHost");
	txtHost->setSizePolicy( QSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed) );
	lblHost->setBuddy(txtHost);
	layHost->addWidget(lblHost);
	layHost->addWidget(txtHost, 5);

	lblPort = new QLabel(top, "lblPort");
	lblPort->setText(i18n("&Port:"));
	//lblPort->setSizePolicy( QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed) );
	txtPort = new KLineEdit(top, "txtPort");
	txtPort->setSizePolicy( QSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed) );
	lblPort->setBuddy(txtPort);
	layHost->addWidget(lblPort);
	layHost->addWidget(txtPort, 2);

	QHBoxLayout *layUsername = new QHBoxLayout(topLayout);
	layUsername->setMargin(0);
	lblUsername = new QLabel(top, "lblUsername");
	lblUsername->setText(i18n("&Username:"));
	lblUsername->setSizePolicy( QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed) );
	txtUsername = new KLineEdit(top, "txtUsername");
	txtUsername->setSizePolicy( QSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed) );
	lblUsername->setBuddy(txtUsername);
	layUsername->addWidget(lblUsername);
	layUsername->addWidget(txtUsername);
	wtstr = i18n(	"The user name you use to login to your email server (sometimes just called \"login\")."
			" Your email provider should have supplied this information. Your login name is often (but"
			" not always) identical to the part of your email address that comes before the \"@\".");
	QWhatsThis::add(lblUsername, wtstr);
	QWhatsThis::add(txtUsername, wtstr);

	QHBoxLayout *layPass = new QHBoxLayout(topLayout);
	layPass->setMargin(0);
	lblPass = new QLabel(top, "lblPass");
	lblPass->setText(i18n("&Password:"));
	lblPass->setSizePolicy( QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed) );
	txtPass = new KLineEdit(top, "txtPass");
	txtPass->setSizePolicy( QSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed) );
	txtPass->setEchoMode(QLineEdit::Password);
	lblPass->setBuddy(txtPass);
	layPass->addWidget(lblPass);
	layPass->addWidget(txtPass);

	chkTLS = new QCheckBox(top, "chkTLS");
	chkTLS->setChecked(false);
	chkTLS->setText(i18n("Use &secure transport layer if available"));
	topLayout->addWidget(chkTLS);
	txtHost->setFocus();

}

UserNameDlg::~UserNameDlg ()
{
}
