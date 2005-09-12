/***************************************************************************
 *   Copyright (C) 2004,2005 by Jakub Stachowski                           *
 *   qbast@go2.pl                                                          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Steet, Fifth Floor, Boston, MA  02110-1301, USA.             *
 ***************************************************************************/

#include <sys/stat.h>
#include <config.h>

#include <qlayout.h>
#include <qfile.h>
#include <q3groupbox.h>
#include <qradiobutton.h>
#include <qtimer.h>
#include <qtabwidget.h>
//Added by qt3to4:
#include <QTextStream>

#include <klocale.h>
#include <kglobal.h>
#include <kparts/genericfactory.h>
#include <kprocess.h>
#include <klineedit.h>
#include <kpassdlg.h>
#include <ksimpleconfig.h>

#include "kcmdnssd.h"
#include <dnssd/settings.h>
#include <dnssd/domainbrowser.h>
#include <kipc.h>

#define MDNSD_CONF "/etc/mdnsd.conf"
#define MDNSD_PID "/var/run/mdnsd.pid"

typedef KGenericFactory<KCMDnssd, QWidget> KCMDnssdFactory;
K_EXPORT_COMPONENT_FACTORY( kcm_kdnssd, KCMDnssdFactory("kcmkdnssd"))

KCMDnssd::KCMDnssd(QWidget *parent, const char *name, const QStringList&)
		: ConfigDialog(parent, name), m_wdchanged(false)
{
	setAboutData(new KAboutData(I18N_NOOP("kcm_kdnssd"),
	                            I18N_NOOP("ZeroConf configuration"),0,0,KAboutData::License_GPL,
	                            I18N_NOOP("(C) 2004,2005 Jakub Stachowski")));
	setQuickHelp(i18n("Setup services browsing with ZeroConf"));
	if (geteuid()!=0) tabs->removePage(tab_2); // normal user cannot change wide-area settings
	// show only global things in 'administrator mode' to prevent confusion
		else if (getenv("KDESU_USER")!=0) tabs->removePage(tab); 
	addConfig(DNSSD::Configuration::self(),this);
	// it is host-wide setting so it has to be in global config file
	domain = new KSimpleConfig( QLatin1String( KDE_CONFDIR "/kdnssdrc" ));
	domain->setGroup("publishing");
	load();
	connect(hostedit,SIGNAL(textChanged(const QString&)),this,SLOT(wdchanged()));
	connect(secretedit,SIGNAL(textChanged(const QString&)),this,SLOT(wdchanged()));
	connect(domainedit,SIGNAL(textChanged(const QString&)),this,SLOT(wdchanged()));
	if (DNSSD::Configuration::self()->publishDomain().isEmpty()) WANButton->setEnabled(false);
}

KCMDnssd::~KCMDnssd()
{
	delete domain;
}

void KCMDnssd::save()
{
	KCModule::save();
	if (geteuid()==0 && m_wdchanged) saveMdnsd(); 
	domain->setFileWriteMode(0644); // this should be readable for everyone
	domain->writeEntry("PublishDomain",domainedit->text());
	domain->sync();
	KIPC::sendMessageAll((KIPC::Message)KIPCDomainsChanged);
}

void KCMDnssd::load()
{
	KCModule::load();
	if (geteuid()==0) loadMdnsd();
}

// hack to work around not working isModified() for KPasswordEdit
void KCMDnssd::wdchanged()
{
	WANButton->setEnabled(!domainedit->text().isEmpty() && !hostedit->text().isEmpty());
	changed();
	m_wdchanged=true;
}

void KCMDnssd::loadMdnsd()
{
	QFile f(MDNSD_CONF);
	if (!f.open(QIODevice::ReadWrite)) return;
	QTextStream stream(&f);
	QString line;
	while (!stream.atEnd()) {
		line = stream.readLine();
		mdnsdLines.insert(line.section(' ',0,0,QString::SectionSkipEmpty),
			line.section(' ',1,-1,QString::SectionSkipEmpty));
		}
	if (!mdnsdLines["zone"].isNull()) domainedit->setText(mdnsdLines["zone"]);
	if (!mdnsdLines["hostname"].isNull()) hostedit->setText(mdnsdLines["hostname"]);
	if (!mdnsdLines["secret-64"].isNull()) secretedit->setText(mdnsdLines["secret-64"]);
}		
	
bool KCMDnssd::saveMdnsd()
{
	mdnsdLines["zone"]=domainedit->text();
	mdnsdLines["hostname"]=hostedit->text();
	if (!secretedit->text().isEmpty()) mdnsdLines["secret-64"]=QString(secretedit->password());
		else mdnsdLines.remove("secret-64");
	QFile f(MDNSD_CONF);
	bool newfile=!f.exists();
	if (!f.open(QIODevice::WriteOnly)) return false; 
	QTextStream stream(&f);
	for (QMap<QString,QString>::ConstIterator it=mdnsdLines.begin();it!=mdnsdLines.end();
		++it) stream << it.key() << " " << (*it) << "\n";
	f.close();
	// if it is new file, then make it only accessible for root as it can contain shared
	// secret for dns server. 
	if (newfile) chmod(MDNSD_CONF,0600); 
	f.setName(MDNSD_PID);
	if (!f.open(QIODevice::ReadOnly)) return true; // it is not running so no need to signal
	QString line;
	if ((line = f.readLine()).isNull()) return true;
	unsigned int pid = line.toUInt();
	if (pid==0) return true;           // not a pid
	kill(pid,SIGHUP);
	return true;
}
	
#include "kcmdnssd.moc"
