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

#include <unistd.h>

#include <kurl.h>
#include <kdebug.h>
#include <ksimpleconfig.h>
#include <kprotocolinfo.h>

#include "kuriikwsfiltereng.h"

unsigned long KURISearchFilterEngine::s_refCnt = 0;
KURISearchFilterEngine *KURISearchFilterEngine::s_pSelf = 0L;

#define SEARCH_SUFFIX	" " "Search"
#define IKW_KEY         "Internet Keywords"
#define IKW_SUFFIX      " " IKW_KEY
#define IKW_REALNAMES	"RealNames"

KURISearchFilterEngine::KURISearchFilterEngine()
{
    loadConfig();
}

void KURISearchFilterEngine::insertSearchEngine(SearchEntry e)
{
    QValueList<SearchEntry>::Iterator it = m_lstSearchEngine.begin();
    QValueList<SearchEntry>::Iterator end = m_lstSearchEngine.end();
    for (; it != end; ++it)
	{
	    if ((*it).m_strName == e.m_strName)
		{
		    m_lstSearchEngine.remove(it);
		    break;
		}
	}
    m_lstSearchEngine.append(e);
}

void KURISearchFilterEngine::removeSearchEngine(const QString &name)
{
    QValueList<SearchEntry>::Iterator it = m_lstSearchEngine.begin();
    QValueList<SearchEntry>::Iterator end = m_lstSearchEngine.end();
    for (; it != end; ++it)
	{
	    if ((*it).m_strName == name)
		{
		    m_lstSearchEngine.remove(it);
		    break;
		}
	}
}

KURISearchFilterEngine::SearchEntry KURISearchFilterEngine::searchEntryByName(const QString &name) const
{
    QValueList<SearchEntry>::ConstIterator it = m_lstSearchEngine.begin();
    QValueList<SearchEntry>::ConstIterator end = m_lstSearchEngine.end();
    for (; it != end; ++it)
	{
	    if ((*it).m_strName == name)
		return *it;
	}
    return SearchEntry();
}

QString KURISearchFilterEngine::searchQuery( const KURL &url ) const
{
    if( m_bSearchKeywordsEnabled )
	{
	    QString key;

	    // NOTE: We simply do not use KURL::protocol() here
	    // because it would mean that the search engine short-
	    // cuts would have to be restricted to a sub-set of
	    // latin1 character sets, namely alpha-numeric characters
	    // a '+' and a '-', when they really do not have to be.
	    QString _url = url.url();
	    int pos = _url.find(':');
	    if (pos >= 0)
		key = _url.left(pos);

	    // Do not touch known protocols or search for empty keys ;)
	    if( KProtocolInfo::isKnownProtocol( key ) || key.isEmpty() )
		return QString::null;

	    QValueList<SearchEntry>::ConstIterator it = m_lstSearchEngine.begin();
	    QValueList<SearchEntry>::ConstIterator end = m_lstSearchEngine.end();
	    for (; it != end; ++it)
		{
		    if ((*it).m_lstKeys.contains(key))
			return formatResult((*it).m_strQuery, _url.mid(pos+1), url.isMalformed() );
		}
	}
    return QString::null;
}


QString KURISearchFilterEngine::ikwsQuery( const KURL& url ) const
{
    if (m_bInternetKeywordsEnabled)
	{
	    QString key;
	    QString _url = url.url();
	    if( url.isMalformed() && _url[0] == '/' )
		key = QString::fromLatin1( "file" );
	    else
		key = url.protocol();

	    if( KProtocolInfo::isKnownProtocol(key) )
		return QString::null;

	    QString search = m_currSearchKeywordsEngine.m_strQuery;
	    if (!search.isEmpty())
		{
		    int pct = m_currInternetKeywordsEngine.m_strQueryWithSearch.find("\\|");
		    if (pct >= 0)
			{
			    search = KURL::encode_string( search );
			    QString res = m_currInternetKeywordsEngine.m_strQueryWithSearch;
			    return formatResult( res.replace(pct, 2, search), _url, url.isMalformed() );
			}
		}
	    return formatResult( m_currInternetKeywordsEngine.m_strQuery, _url, url.isMalformed() );
	}
    return QString::null;
}

KURISearchFilterEngine::IKWSEntry KURISearchFilterEngine::ikwsEntryByName(const QString &name) const
{
    QValueList<IKWSEntry>::ConstIterator it = m_lstInternetKeywordsEngine.begin();
    QValueList<IKWSEntry>::ConstIterator end = m_lstInternetKeywordsEngine.end();
    for (; it != end; ++it)
	{
	    if ((*it).m_strName == name)
		return *it;
	}
    return IKWSEntry();
}

void KURISearchFilterEngine::setInternetKeywordsEnabled(bool flag)
{
    m_bInternetKeywordsEnabled = flag;
}

void KURISearchFilterEngine::setSearchKeywordsEnabled(bool flag)
{
    m_bSearchKeywordsEnabled = flag;
}

bool KURISearchFilterEngine::isInternetKeywordsEnabled() const
{
    return m_bInternetKeywordsEnabled;
}

bool KURISearchFilterEngine::isSearchKeywordsEnabled() const
{
    return m_bSearchKeywordsEnabled;
}

QString KURISearchFilterEngine::searchFallback() const
{
    return m_currSearchKeywordsEngine.m_strName;
}

void KURISearchFilterEngine::setSearchFallback(const QString &name)
{
    m_currSearchKeywordsEngine = searchEntryByName( name );
}

QCString KURISearchFilterEngine::name() const
{
    return "kuriikwsfilter";
}

void KURISearchFilterEngine::incRef()
{
    s_refCnt++;
}

void KURISearchFilterEngine::decRef()
{
    s_refCnt--;
    if ( s_refCnt == 0 && s_pSelf )
	{
	    delete s_pSelf;
	    s_pSelf = 0;
	}
}

KURISearchFilterEngine* KURISearchFilterEngine::self()
{
    if (!s_pSelf)
	{
	    if ( s_refCnt == 0 )
  	        s_refCnt++; //someone forgot to call incRef
	    s_pSelf = new KURISearchFilterEngine;
	}
    return s_pSelf;
}

QString KURISearchFilterEngine::formatResult( const QString& query, const QString& url, bool isMalformed ) const
{
    // Substitute the variable part we find in the query.
    if (!query.isEmpty())
	{
	    QString newurl = query;
	    int pct;
	    // Always use utf-8, since it is guaranteed that this
	    // will be understood.
	    if ((pct = newurl.find("\\2")) >= 0)
		newurl = newurl.replace(pct, 2, "utf-8");

	    QString userquery = url;
	    int space_pos;
	    while( (space_pos=userquery.find(' ')) != -1 )
		userquery=userquery.replace( space_pos, 1, "+" );

	    if( isMalformed )
		userquery = KURL::encode_string(userquery);

	    if ((pct = newurl.find("\\1")) >= 0)
		newurl = newurl.replace(pct, 2, userquery);

	    if ( m_bVerbose )
		kdDebug(7023) << "(" << getpid() << ") filtered " << url << " to " << newurl << "\n";

	    return newurl;
	}

    return QString::null;
}

void KURISearchFilterEngine::loadConfig()
{
    bool oldConfigFormat = false;
    kdDebug(7023) << "(" << getpid() << ") Keywords Engine: Loading config..." << endl;
    // First empty any current config we have.
    m_lstSearchEngine.clear();
    m_lstInternetKeywordsEngine.clear();

    // Load the config.
    KConfig config( name() + "rc", false, false );
    QStringList engines;
    QString selIKWSEngine, selIKWSFallback;
    config.setGroup( "General" );

    if( !config.hasKey("InternetKeywordsEnabled") && config.hasGroup(IKW_KEY)) {
	// Read the old settings
	kdDebug(7023) << "(" << getpid() << ") Config file has the OLD format..." << endl;
	oldConfigFormat = true;
	config.setGroup(IKW_KEY);
	m_bInternetKeywordsEnabled = config.readBoolEntry("NavEnabled", true);
	selIKWSEngine = config.readEntry("NavSelectedEngine", IKW_REALNAMES);
	selIKWSFallback = config.readEntry("NavSearchFallback");
	engines = config.readListEntry("NavEngines");
    } else {
	kdDebug(7023) << "(" << getpid() << ") Config file has the NEW format..." << endl;
	m_bInternetKeywordsEnabled = config.readBoolEntry("InternetKeywordsEnabled", true);
	selIKWSEngine = config.readEntry("InternetKeywordsSelectedEngine", IKW_REALNAMES);
	selIKWSFallback = config.readEntry("InternetKeywordsSearchFallback");
	engines = config.readListEntry("InternetKeywordsEngines");
    }

    kdDebug(7023) << "(" << getpid() << ") Internet Keyword Enabled: " << m_bInternetKeywordsEnabled << endl;
    kdDebug(7023) << "(" << getpid() << ") Selected IKWS Engine(s): " << selIKWSEngine << endl;
    kdDebug(7023) << "(" << getpid() << ") Internet Keywords Fallback Search Engine: " << selIKWSFallback << endl;

    QStringList::ConstIterator gIt = engines.begin();
    QStringList::ConstIterator gEnd = engines.end();
    for (; gIt != gEnd; ++gIt) {
	QString grpName = *gIt + IKW_SUFFIX;
	if (config.hasGroup(grpName)) {
	    config.setGroup( grpName );
	    IKWSEntry e;
	    e.m_strName = *gIt;
	    e.m_strQuery = config.readEntry("Query");
	    e.m_strQueryWithSearch = config.readEntry("QueryWithSearch");
	    m_lstInternetKeywordsEngine.append(e);
	    if (selIKWSEngine == (oldConfigFormat ? grpName : e.m_strName)) {
		m_currInternetKeywordsEngine = e;
	    }
	}
    }

    IKWSEntry rn = ikwsEntryByName(IKW_REALNAMES);
    if (rn.m_strName.isEmpty())	{
	rn.m_strName = IKW_REALNAMES;
	rn.m_strQuery = QString::fromLatin1("http://navigation.realnames.com/resolver.dll?realname=\\1&charset=\\2&providerid=180");
	rn.m_strQueryWithSearch = QString::fromLatin1("http://navigation.realnames.com/resolver.dll?"
						      "action=navigation&realname=\\1&charset=\\2&providerid=180&fallbackuri=\\|");
	if (rn.m_strName == selIKWSEngine)
	    m_currInternetKeywordsEngine = rn;
	
	m_lstInternetKeywordsEngine.append(rn);
    }

    // Load the Search engines

    config.setGroup("General");
    m_bVerbose = config.readBoolEntry("Verbose");
    engines = config.readListEntry("SearchEngines");
    m_bSearchKeywordsEnabled = config.readBoolEntry("SearchEngineShortcutsEnabled", true);

    kdDebug(7023) << "(" << getpid() << ") Search Engine Keywords Enabled: " << m_bSearchKeywordsEnabled << endl;
    kdDebug(7023) << "(" << getpid() << ") Number of search engine keywords found: " << engines.count() << endl;

    gIt = engines.begin();
    gEnd = engines.end();
    for (; gIt != gEnd; ++gIt) {
	QString grpName = *gIt + SEARCH_SUFFIX;
	if (!config.hasGroup(grpName)) {
	    grpName = *gIt;
	} else {
	    grpName = *gIt + SEARCH_SUFFIX;
	}

	kdDebug(7023) << "(" << getpid() << ") Search Engine Group name: " << grpName << " (for engine: " << *gIt << ")" << endl;
	
	config.setGroup( grpName );
	SearchEntry e;
	e.m_strName = *gIt;
	e.m_lstKeys = config.readListEntry("Keys");
	e.m_strQuery = config.readEntry("Query");
	m_lstSearchEngine.append(e);
	if (selIKWSFallback == (oldConfigFormat ? grpName : e.m_strName)) {
	    m_currSearchKeywordsEngine = e;
	}
    }
}

void KURISearchFilterEngine::saveConfig() const
{
    kdDebug(7023) << "(" << getpid() << ") Keywords Engine: Saving config..." << endl;

    KSimpleConfig config(name() + "rc");
    QStringList search_engines, ikws_engines;

    // Remove the OLD group [Internet Keywords].
    // Instead all generic info that has to do with
    // shortcuts will be saved under [General].
    if( config.hasGroup(IKW_KEY) )
	config.deleteGroup( IKW_KEY );

    // DUMP OUT THE SEARCH ENGINE INFO	
    QValueList<SearchEntry>::ConstIterator it = m_lstSearchEngine.begin();
    QValueList<SearchEntry>::ConstIterator end = m_lstSearchEngine.end();
    for (; it != end; ++it)
	{
	    search_engines.append((*it).m_strName);
	    config.setGroup((*it).m_strName + SEARCH_SUFFIX);
	    config.writeEntry("Keys", (*it).m_lstKeys);
	    config.writeEntry("Query", (*it).m_strQuery);
	}
	
    // DUMP OUT THE INTERNET KEYWORD INFO
    QValueList<IKWSEntry>::ConstIterator nit = m_lstInternetKeywordsEngine.begin();
    QValueList<IKWSEntry>::ConstIterator nend = m_lstInternetKeywordsEngine.end();
    for (; nit != nend; ++nit)
	{
	    ikws_engines.append((*nit).m_strName);
	    config.setGroup((*nit).m_strName + IKW_SUFFIX);
	    config.writeEntry("Query", (*nit).m_strQuery);
	    if (!(*nit).m_strQueryWithSearch.isEmpty())
		config.writeEntry("QueryWithSearch", (*nit).m_strQueryWithSearch);
	}

    config.setGroup("General");
    config.writeEntry("InternetKeywordsEnabled", m_bInternetKeywordsEnabled);
    config.writeEntry("InternetKeywordsEngines", ikws_engines);
    config.writeEntry("InternetKeywordsSelectedEngine", m_currInternetKeywordsEngine.m_strName);
    config.writeEntry("InternetKeywordsSearchFallback", m_currSearchKeywordsEngine.m_strName);
    config.writeEntry("SearchEngineShortcutsEnabled",m_bSearchKeywordsEnabled );
    config.writeEntry("SearchEngines", search_engines);
    if (m_bVerbose)
	config.writeEntry("Verbose", m_bVerbose);

    config.sync(); // Dump out the general config stuff
}
