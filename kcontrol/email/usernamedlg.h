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
