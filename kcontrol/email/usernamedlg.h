/* vi: ts=8 sts=4 sw=4
 *
 * $Id$
 *
 * This file is part of the KDE project, module kcontrol.
 * Copyright (C) 1999-2001 by Alex Zepeda
 *
 * You can freely distribute this program under the GNU General Public
 * License. See the file "COPYING" for the exact licensing terms.
 *
 */

#ifndef USERNAMEDLG_H
#define USERNAMEDLG_H "$Id"

#include <kdialogbase.h>

class KLineEdit;
class QLabel;
class QCheckBox;

class UserNameDlg
	: public KDialogBase
{
public:
	UserNameDlg (QWidget *parent, const QString &caption);
	~UserNameDlg ();

	QCheckBox *chkTLS;
	KLineEdit *txtUsername, *txtPass, *txtHost, *txtPort;
	QLabel *lblUsername, *lblPass, *lblHost, *lblPort;
};

#endif
