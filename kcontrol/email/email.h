/* vi: ts=8 sts=4 sw=4
 *
 * $Id$
 *
 * This file is part of the KDE project, module kcontrol.
 * Copyright (C) 1999-2001 by Alex Zepeda and Daniel Molkentin
 *
 * You can freely distribute this program under the GNU General Public
 * License. See the file "COPYING" for the exact licensing terms.
 *
 */

#ifndef KCMEMAIL_H
#define KCMEMAIL_H

#include <qvariant.h>
#include <qdialog.h>

#include <kcmodule.h>

#include "emailbase.h"

class QButtonGroup;
class QCheckBox;
class QGroupBox;
class QLabel;
class KLineEdit;
class QPushButton;
class QRadioButton;
class QVBox;

class KComboBox;
class KEMailSettings;

class topKCMEmail
	: public KCModule
{
	Q_OBJECT

public:
	topKCMEmail (QWidget *parent = 0, const char *name = 0);
	~topKCMEmail ();

	KCMEmailBase *m_email;

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
