#ifndef KCMEMAIL_H
#define KCMEMAIL_H "$Id$"

#include <qvariant.h>
#include <qdialog.h>

#include <kcmodule.h>

class QButtonGroup;
class QCheckBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QRadioButton;

class KComboBox;
class KEMailSettings;

class topKCMEmail
	: public KCModule
{ 
    Q_OBJECT

public:
	topKCMEmail (QWidget* parent = 0, const char* name = 0);
	~topKCMEmail ();

	void load(const QString & = QString::null);
	void save();
	void defaults();
	//int buttons();
	QString quickHelp() const;

public slots:
	void configChanged();
	void configChanged(bool);
	void selectEmailClient();
	void profileChanged(const QString &);

protected:
	QGroupBox* grpClient;

	QLineEdit* txtEMailClient;
	QPushButton* btnBrowseClient;
	QCheckBox* chkRunTerminal;

	QGroupBox* grpIncoming;
	QButtonGroup* grpICM;
	QRadioButton* radIMAP, *radPOP, *radICMLocal;
	QPushButton *btnICMSettings;


	QGroupBox* grpOutgoing;
	QPushButton *btnOGMSettings;
	QButtonGroup *grpOGM;
	QRadioButton *radSMTP, *radOGMLocal;

	QPushButton *btnNewProfile;
	QLabel *lblCurrentProfile;
	KComboBox *cmbCurProfile;

	QGroupBox *grpUserInfo;
	QLabel *lblFullName, *lblOrganization, *lblEMailAddr, *lblReplyTo;
	QLineEdit *txtFullName, *txtOrganization, *txtEMailAddr, *txtReplyTo;

protected slots:
	void slotComboChanged(const QString &);
	void slotNewProfile();
	void slotICMSettings();
	void slotOGMSettings();

protected:
	void clearData();
	KEMailSettings *pSettings;	
	QString m_sICMPassword, m_sICMUsername;
	QString m_sOGMPassword, m_sOGMUsername;
	bool m_bChanged;
};

#endif // TOPKCMEMAIL_H
