#ifndef USERNAMEDLG_H
#define USERNAMEDLG_H "$Id"

#include <kdialogbase.h>

class QLineEdit;
class QLabel;
class QCheckBox;

class UserNameDlg
	: public KDialogBase
{
public:
	UserNameDlg (QWidget *parent, const QString &caption);
	~UserNameDlg ();

	QCheckBox *chkTLS;
	QLineEdit *txtUsername, *txtPass;
	QLabel *lblUsername, *lblPass;
};

#endif
