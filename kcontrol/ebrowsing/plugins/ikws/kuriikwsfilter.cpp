/*  This file is part of the KDE project
    Copyright (C) 1999 Simon Hausmann <hausmann@kde.org>
    Copyright (C) 2000 Yves Arrouye <yves@realnames.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

*/

#include <qregexp.h>

#include <kdebug.h>
#include <klocale.h>
#include <kurl.h>
#include <kprotocolmanager.h>
#include <kinstance.h>
#include <kglobal.h>

#include <iostream.h>

#include "ikwsopts.h"
#include "kuriikwsfiltereng.h"
#include "kuriikwsfilter.h"

KInstance *KURISearchFilterFactory::s_instance = 0L;

KURISearchFilter::KURISearchFilter(QObject *parent, const char *name) : KURIFilterPlugin(parent, name ? name : "KURISearchFilter", "ikws", 1.0), DCOPObject("KURISearchFilterIface") {
}

KURISearchFilter::~KURISearchFilter() {
}

void KURISearchFilter::configure() {
    KURISearchFilterEngine::self()->loadConfig();
}

bool KURISearchFilter::filterURI(KURL &kurl) const {
    //QString url = kurl.isMalformed() ? kurl.malformedUrl() : kurl.url();
    QString url = kurl.url();

    if (KURISearchFilterEngine::self()->verbose()) {
	kDebugInfo("filtering %s", url.ascii());
    }

    // Is this URL a candidate for filtering?

    if (kurl.isMalformed() || !KProtocolManager::self().protocols().contains(kurl.protocol())) {
	QString query = QString::null;
	int pos = -1;

	// See if it's a searcher prefix. If not, use Internet Keywords
	// if we can. Note that we want a colon to match a searcher
	// prefix. Also note that other filterings like doing a DNS
	// lookup for a hostname should have been taken care of before.
	
	if (kurl.isMalformed()) {
	    query = KURISearchFilterEngine::self()->navQuery();
	} else {
	    pos = url.find(':');
	    if (pos >= 0) {
		QString key = url.left(pos);
		query = KURISearchFilterEngine::self()->searchQuery(key);
		if (query == QString::null) {
		    return false;
		}
	    }
	}
	
	// Substitute the variable part in the query we found.
	
	if (query != QString::null) {
	    QString newurl = query;
	    
	    int pct;
	    
	    // Always use utf-8, since it is guaranteed that this
	    // will be understood.
	    
	    if ((pct = newurl.find("\\2")) >= 0) {
		newurl = newurl.replace(pct, 2, "utf-8");
	    }
	    
	    QString userquery = url.mid(pos+1).replace(QRegExp(" "), "+").utf8();
	    if (kurl.isMalformed()) {
	        KURL::encode(userquery);
	    }
	    if ((pct = newurl.find("\\1")) >= 0) {
		newurl = newurl.replace(pct, 2, userquery);
	    }
	    
	    if (KURISearchFilterEngine::self()->verbose()) {
		kDebugInfo("filtered %s to %s\n", url.ascii(), newurl.ascii());
	    }
	    
	    kurl = newurl;
	    
	    return true;
	}
    }
    
    return false;
}

KCModule *KURISearchFilter::configModule(QWidget *parent, const char *name) const {
    return new InternetKeywordsOptions(parent, name);
}

QString KURISearchFilter::configName() const {
    return i18n("Internet &Keywords");
}

KURISearchFilterFactory::KURISearchFilterFactory(QObject *parent, const char *name) : KLibFactory(parent, name) {
    s_instance = new KInstance(KURISearchFilterEngine::self()->name());
}

KURISearchFilterFactory::~KURISearchFilterFactory() {
    delete s_instance;
}

QObject *KURISearchFilterFactory::create( QObject *parent, const char *, const char*, const QStringList & )
{
  return new KURISearchFilter( parent );
}

KInstance *KURISearchFilterFactory::instance() {
    return s_instance;
}

extern "C" {

void *init_libkuriikwsfilter() {
    return new KURISearchFilterFactory;
}

}

#include "kuriikwsfilter.moc"


