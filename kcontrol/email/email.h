#ifndef KCMEMAIL_H
#define KCMEMAIL_H "$Id$"

#include <qvariant.h>
#include <qdialog.h>
#include <qvbox.h>

#include <kcmodule.h>

class QButtonGroup;
class QCheckBox;
class QGroupBox;
class QLabel;
class KLineEdit;
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

        void load();
	void load(const QString &);
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
	QGroupBox *grpClient;
	KLineEdit *txtEMailClient;
	QPushButton *btnBrowseClient;
	QCheckBox *chkRunTerminal;

	QButtonGroup *grpIncoming;
	QVBox *grpICM;
	QRadioButton *radIMAP, *radPOP, *radICMLocal;
	QPushButton *btnICMSettings;

        QButtonGroup *grpOutgoing;
        QVBox *grpOGM;
	QPushButton *btnOGMSettings;
	QRadioButton *radSMTP, *radOGMLocal;

	QPushButton *btnNewProfile;
	QLabel *lblCurrentProfile;
	KComboBox *cmbCurProfile;

	QGroupBox *grpUserInfo;
	QLabel *lblFullName, *lblOrganization, *lblEMailAddr, *lblReplyTo;
	KLineEdit *txtFullName, *txtOrganization, *txtEMailAddr, *txtReplyTo;

protected slots:
	void slotComboChanged(const QString &);
	void slotNewProfile();
	void slotICMSettings();
	void slotOGMSettings();

protected:
	void clearData();
	KEMailSettings *pSettings;
	QString m_sICMPassword, m_sICMUsername, m_sICMPath, m_sICMHost;
	QString m_sOGMPassword, m_sOGMUsername, m_sOGMCommand, m_sOGMHost;
	unsigned int m_uOGMPort, m_uICMPort;
	bool m_bOGMSecure, m_bICMSecure;

	bool m_bChanged;
};

#endif // TOPKCMEMAIL_H
