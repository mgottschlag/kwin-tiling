#ifndef _EMAILSETTINGS_H
#define _EMAILSETTINGS_H "$Id$"

#include <qstring.h>
#include <qstringlist.h>

class KEMailSettingsPrivate;

class KEMailSettings {
public:
	enum Setting {
		ClientProgram,
		ClientTerminal,
		RealName,
		EmailAddress,
		ReplyToAddress,
		Organization,
		OutServer,
		InServer,
		InServerLogin,
		InServerPass,
		InServerType
	};

	enum Extension {
		POP3,
		SMTP,
		OTHER
	};

	KEMailSettings();
	~KEMailSettings();
	QStringList profiles();
	QString currentProfileName();
	void setProfile (const QString &);
	QString defaultProfileName();
	QString getSetting(KEMailSettings::Setting s);
	void setSetting(KEMailSettings::Setting s, const QString &v);

	// Use this when trying to get at currently unimplemented settings
	// such as POP3 authentication methods, or mail specific TLS settings
	QString getExtendedSetting(KEMailSettings::Extension e, const QString &s );
	void setExtendedSetting(KEMailSettings::Extension e, const QString &s, const QString &v );

protected:
	KEMailSettingsPrivate *p;
};

#endif
