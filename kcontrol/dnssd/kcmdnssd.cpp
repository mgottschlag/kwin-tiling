/***************************************************************************
 *   Copyright (C) 2004 by Jakub Stachowski                                *
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
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include <qlayout.h>

#include <klocale.h>
#include <kglobal.h>
#include <kparts/genericfactory.h>
#include <kprocess.h>
#include <qgroupbox.h>
#include <qradiobutton.h>
#include <qtimer.h>

#include "kcmdnssd.h"
#include <dnssd/settings.h>
#include <dnssd/domainbrowser.h>
#include <kipc.h>

typedef KGenericFactory<KCMDnssd, QWidget> KCMDnssdFactory;
K_EXPORT_COMPONENT_FACTORY( kcm_kdnssd, KCMDnssdFactory("kcmkdnssd"))

KCMDnssd::KCMDnssd(QWidget *parent, const char *name, const QStringList&)
		: ConfigDialog(parent, name)
{
	setAboutData(new KAboutData(I18N_NOOP("kcm_kdnssd"),
	                            I18N_NOOP("ZeroConf configuration"),0,0,KAboutData::License_GPL,
	                            I18N_NOOP("(C) 2004, Jakub Stachowski")));
	setQuickHelp(i18n("Setup services browsing with ZeroConf"));
	addConfig(DNSSD::Configuration::self(),this);
	load();
	// why the hell is this needed?!
	QTimer::singleShot(0,this,SLOT(notchanged()));
}

void KCMDnssd::save()
{
	KCModule::save();
	KIPC::sendMessageAll((KIPC::Message)KIPCDomainsChanged);
}


void KCMDnssd::notchanged() 
{
	emit changed(false);
}

#include "kcmdnssd.moc"
