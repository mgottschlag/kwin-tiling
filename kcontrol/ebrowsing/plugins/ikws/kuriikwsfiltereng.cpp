/*  This file is part of the KDE project
    Copyright (C) 1999 Simon Hausmann <hausmann@kde.org>
    Copyright (C) 2000 Yves Arrouye <yves@realnames.com>
    Advanced web shortcuts:
    Copyright (C) 2001 Andreas Hochsteger <e9625392@student.tuwien.ac.at>

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

#include <qtextcodec.h>
#include <qregexp.h>

#include <kurl.h>
#include <kdebug.h>
#include <kprotocolinfo.h>
#include <kconfig.h>
#include <ksimpleconfig.h>
#include <kapplication.h>
#include <kstandarddirs.h>
#include <kstaticdeleter.h>

#include "kuriikwsfiltereng.h"
#include "searchprovider.h"

KStaticDeleter<KURISearchFilterEngine> kurisearchfilterengsd;
KURISearchFilterEngine *KURISearchFilterEngine::s_pSelf = 0L;

#define IKW_KEY         "Internet Keywords"
#define IKW_SUFFIX      " " IKW_KEY
#define IKW_REALNAMES	"RealNames"
#define PIDDBG kdDebug(7023) << "(" << getpid() << ") "
#define PDVAR(n,v) PIDDBG << n << " = '" << v << "'\n"

KURISearchFilterEngine::KURISearchFilterEngine()
{
    loadConfig();
}

QString KURISearchFilterEngine::searchQuery( const KURL &url ) const
{
    if ( m_bSearchKeywordsEnabled )
    {
        QString key, search = url.url();
        int pos = search.find(':');
        if ( pos > -1 )
            key = search.left(pos);

        if ( key.isEmpty() || KProtocolInfo::isKnownProtocol( key ) )
            return QString::null;

        SearchProvider *provider = SearchProvider::findByKey(key);

        if ( provider )
            return formatResult( provider->query(), provider->charset(),
                                 QString::null, search.mid(pos+1),
                                 url.isMalformed() );
    }
    return QString::null;
}


QString KURISearchFilterEngine::ikwsQuery( const KURL& url ) const
{
    if (m_bInternetKeywordsEnabled)
	{
	    QString key;
	    QString _url = url.url();

	    if( url.isMalformed() && _url[0] == '/' ) {
		key = QString::fromLatin1( "file" );
	    } else {
		key = url.protocol();
	    }

	    if( KProtocolInfo::isKnownProtocol(key) ) {
		return QString::null;
	    }

            SearchProvider *fallback = SearchProvider::findByDesktopName(m_searchFallback);
            if (fallback)
            {
                QString search = fallback->query();
                /*
                * As a special case, if there is a question mark
                * at the beginning of the query, we'll force the
                * use of the search fallback without going through
                * the Internet Keywords engine.
                *
                */

		PDVAR ("Fallback query", search);
                QRegExp question("^[ \t]*\\?[ \t]*");
		if (url.isMalformed() && _url.find(question) == 0) {
                        _url = _url.replace(question, "");
			QString fbq = formatResult (search, fallback->charset(), QString::null, _url, true);
			PDVAR ("fbq", fbq);
                        return fbq;
                } else {
			SubstMap map;
			QString qs = m_currInternetKeywordsEngine.m_strQueryWithSearch;
			QString fbq = formatResult (search, fallback->charset(), QString::null, _url, true);

			PDVAR ("qs", qs);
			PDVAR ("fbq", fbq);
			map.replace("wsc_fallback", fbq);
                        return formatResult (qs, fallback->charset(), QString::null, _url, url.isMalformed(), map);
                }
            }

	    return formatResult( m_currInternetKeywordsEngine.m_strQuery, m_currInternetKeywordsEngine.m_strCharset, QString::null, _url, url.isMalformed() );
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

QCString KURISearchFilterEngine::name() const
{
    return "kuriikwsfilter";
}

KURISearchFilterEngine* KURISearchFilterEngine::self()
{
    if (!s_pSelf)
	kurisearchfilterengsd.setObject( s_pSelf, new KURISearchFilterEngine );
    return s_pSelf;
}

QStringList KURISearchFilterEngine::modifySubstitutionMap(SubstMap& map, const QString& query) const
{	// Returns the number of query words
	QString userquery = query;

	// Do some pre-encoding, before we can start the work:
	{
		int start = 0;
		int pos = 0;
		QRegExp qsexpr("\\\"[^\\\"]*\\\"");

		// Temporary substitute spaces in quoted strings (" " -> "%20")
		// Needed to split user query into StringList correctly.
		while ((pos = qsexpr.search(userquery, start)) >= 0) {
			int i = 0;
			int n = 0;
			QString s = userquery.mid (pos, qsexpr.matchedLength());
			while ((i = s.find(" ")) != -1) {
				s = s.replace (i, 1, "%20");
				n++;
			}
			start = pos + qsexpr.matchedLength() + 2*n; // Move after last quote
			userquery = userquery.replace (pos, qsexpr.matchedLength(), s);
		}
	}

	// Split user query between spaces:
	QStringList l = QStringList::split(" ", userquery.simplifyWhiteSpace());

	// Back-substitute quoted strings (%20 -> " "):
	{
		int i = 0;
		while ((i = userquery.find("%20")) != -1)
			userquery = userquery.replace(i, 3, " ");
		for ( QStringList::Iterator it = l.begin(); it != l.end(); ++it )
			*it = (*it).replace(QRegExp("%20"), " ");
	}

	PIDDBG << "Generating substitution map:\n";
	// Generate substitution map from user query:
	for (unsigned int i=0; i<=l.count(); i++) {
		int j = 0;
		int pos = 0;
		QString v = "";
		QString nr = QString::number(i);

		if (i==0) { // Add whole user query (\{0}) to substitution map:
			v = userquery;
		} else { // Add partial user query items to substitution map:
			v = l[i-1];
		}

		// Back-substitute quoted strings (%20 -> " "):
		while ((j = v.find("%20")) != -1) v = v.replace(j, 3, " ");

		// Insert partial queries (referenced by \1 ... \n) to map:
		map.replace(QString::number(i), v);
		PDVAR ("  map['" + nr + "']", map[nr]);

		// Insert named references (referenced by \name) to map:
		j = 0;
		if ((i>0) && (pos = v.find("=")) > 0) {
			QString s = v.mid(pos + 1);
			QString k = v.left(pos);

			// Back-substitute references contained in references (e.g. '\refname' substitutes to 'thisquery=\0')
			while ((j = s.find("%5C")) != -1) s = s.replace(j, 3, "\\");
			map.replace(k, s);
			PDVAR ("  map['" + k + "']", map[k]);
		}
	}

	return l;
}

QString KURISearchFilterEngine::substituteQuery(const QString& url, SubstMap &map, const QString& userquery, const int encodingMib) const
{
	QString newurl = url;
	QStringList ql = modifySubstitutionMap (map, userquery);
	int count = ql.count();
	
	// Check, if old style '\1' is found and replace it with \{@} (compatibility mode):
	{
		int pos = -1;
		if ((pos = newurl.find("\\1")) >= 0) {
			PIDDBG << "WARNING: Using compatibility mode for newurl='" << newurl << "'. Please replace old style '\\1' with new style '\\{0}' in the query definition.\n";
			newurl = newurl.replace(pos, 2, "\\{@}");
		}
	}

	PIDDBG << "Substitute references:\n";
	// Substitute references (\{ref1,ref2,...}) with values from user query:
	{
		int pos = 0;
		QRegExp reflist("\\\\\\{[^\\}]+\\}");

		// Substitute reflists (\{ref1,ref2,...}):
		while ((pos = reflist.search(newurl, 0)) >= 0) {
			bool found = false;
			//bool rest = false;
			QString v = "";
			QString rlstring = newurl.mid(pos + 2, reflist.matchedLength() - 3);
			PDVAR ("  reference list", rlstring);
			if (rlstring == "@") {					// \{@} gets a special treatment later
				v = "\\@";
				found = true;
			}
			QStringList rl = QStringList::split(",", rlstring);	// Todo: strip whitespaces around commas
			unsigned int i = 0;
			while ((i<rl.count()) && !found) {
				QString rlitem = rl[i];
				QRegExp range("[0-9]*\\-[0-9]*");

				if (range.search(rlitem, 0) >= 0) {					// Substitute a range of keywords
					int pos = rlitem.find("-");
					int first = rlitem.left(pos).toInt();
					int last  = rlitem.right(rlitem.length()-pos-1).toInt();
					if (first == 0) first = 1;
					if (last  == 0) last = count;
					for (int i=first; i<=last; i++) {
						v += map[QString::number(i)] + " ";
						ql[i-1] = "";						// Remove used value from ql (needed for \{@}):
					}
					v = v.stripWhiteSpace();
					if (v != "") found = true;
					PDVAR ("    range", QString::number(first) + "-" + QString::number(last) + " => '" + v + "'");
					v = KURL::encode_string(v, encodingMib);
				} else if ((rlitem.left(1) == "\"") && (rlitem.right(1) == "\"")) {	// Use default string from query definition:
					found = true;
					QString s = rlitem.mid(1, rlitem.length() - 2);
					v = KURL::encode_string(s, encodingMib);
					PDVAR ("    default", s);
				} else if (map.contains(rlitem)) {					// Use value from substitution map:
					found = true;
					PDVAR ("    map['" + rlitem + "']", map[rlitem]);
					v = KURL::encode_string(map[rlitem], encodingMib);

					// Remove used value from ql (needed for \{@}):
					QString c = rlitem.left(1);
					if (c=="0") {							// It's a numeric reference to '0'
						for (QStringList::Iterator it = ql.begin(); it!=ql.end(); ++it) (*it) = "";
					} else if ((c>="0") && (c<="9")) {				// It's a numeric reference > '0'
						int n = rlitem.toInt();
						ql[n-1] = "";
					} else {							// It's a alphanumeric reference
						QStringList::Iterator it = ql.begin();
						while ((it != ql.end()) && ((rlitem + "=") != (*it).left(rlitem.length()+1))) ++it;
						if ((rlitem + "=") == (*it).left(rlitem.length()+1)) {
							(*it) = "";
						}
					}

					// Encode '+', otherwise it would be interpreted as space in the resulting url:
					int vpos = 0;
					while ((vpos = v.find('+')) != -1)
						v = v.replace (vpos, 1, "%2B");
				} else if (rlitem == "@") {
					v = "\\@";
					PDVAR ("    v", v);
				}
				i++;
			}

			newurl = newurl.replace(pos, reflist.matchedLength(), v);
		}

		{	// Special handling for \{@};

			PDVAR ("  newurl", newurl);
			// Generate list of unmatched strings:
			QString v = "";
			for (unsigned int i=0; i<ql.count(); i++) {
				v += " " + ql[i];
			}
			v = v.simplifyWhiteSpace();
			PDVAR ("    rest", v);
			v = KURL::encode_string(v, encodingMib);

			// Substitute \{@} with list of unmatched query strings
			int vpos = 0;
			while ((vpos = newurl.find("\\@")) != -1)
				newurl = newurl.replace (vpos, 2, v);
		}
	}

	return newurl;
}

QString KURISearchFilterEngine::formatResult( const QString& url,
                                              const QString& cset1,
                                              const QString& cset2,
                                              const QString& query,
                                              bool isMalformed ) const
{
	SubstMap map;
	return formatResult (url, cset1, cset2, query, isMalformed, map);
}

QString KURISearchFilterEngine::formatResult( const QString& url,
                                              const QString& cset1,
                                              const QString& cset2,
                                              const QString& query,
                                              bool /* isMalformed */,
                                              SubstMap& map ) const
{
	// Return nothing if userquery is empty:
	if (query.isEmpty()) return QString::null;

	// Debug info of map:
	if (!map.isEmpty()) {
		PIDDBG << "Got non-empty substitution map:\n";
		for(SubstMap::Iterator it = map.begin(); it != map.end(); ++it)
			PDVAR ("    map['" + it.key() + "']", it.data());
	}

	// Decode user query:
	QString userquery = KURL::decode_string(query);

	PDVAR ("user query", userquery);
	PDVAR ("query definition", url);

	// Create a codec for the desired encoding so that we can transcode the user's "url".
	QString cseta = cset1;
	if (cseta.isEmpty()) {
		cseta = "iso-8859-1";
	}
	QTextCodec *csetacodec = QTextCodec::codecForName(cseta.latin1());
	if (!csetacodec) {
		cseta = "iso-8859-1";
		csetacodec = QTextCodec::codecForName(cseta.latin1());
	}

	// Add charset indicator for the query to substitution map:
	map.replace("ikw_charset", cseta);

	// Add charset indicator for the fallback query to substitution map:
	QString csetb = cset2;
	if (csetb.isEmpty())
		csetb = "iso-8859-1";
	map.replace("wsc_charset", csetb);

	QString newurl = substituteQuery (url, map, userquery, csetacodec->mibEnum());

	PDVAR ("substituted query", newurl);

	return newurl;
}

void KURISearchFilterEngine::loadConfig()
{
    // Migrate from the old format,
    // this block should remain until we can assume "every"
    // user has upgraded to a KDE version that contains the
    // sycoca based search provider configuration (malte)
    {
        KSimpleConfig oldConfig(kapp->dirs()->saveLocation("config") + QString(name()) + "rc");
        oldConfig.setGroup("General");
        if (oldConfig.hasKey("SearchEngines"))
        {
            // User has an old config file in his local config dir
            kdDebug(7023) << "Migrating config file to .desktop files..." << endl;
            QString fallback = oldConfig.readEntry("InternetKeywordsSearchFallback");
            QStringList engines = oldConfig.readListEntry("SearchEngines");
            for (QStringList::ConstIterator it = engines.begin(); it != engines.end(); ++it)
            {
                if (!oldConfig.hasGroup(*it + " Search"))
                    continue;
                oldConfig.setGroup(*it + " Search");
                QString query = oldConfig.readEntry("Query");
                QStringList keys = oldConfig.readListEntry("Keys");
                QString charset = oldConfig.readEntry("Charset");
                oldConfig.deleteGroup(*it + " Search");
                QString name;
                for (QStringList::ConstIterator key = keys.begin(); key != keys.end(); ++key)
                {
                    // take the longest key as name for the .desktop file
                    if ((*key).length() > name.length())
                        name = *key;
                }
                if (*it == fallback)
                    fallback = name;
                SearchProvider *provider = SearchProvider::findByKey(name);
                if (provider)
                {
                    // If this entry has a corresponding global entry
                    // that comes with KDE's default configuration,
                    // compare both and if thei're equal, don't
                    // create a local copy
                    if (provider->name() == *it
                        && provider->query() == query
                        && provider->keys() == keys
                        && (provider->charset() == charset || (provider->charset().isEmpty() && charset.isEmpty())))
                    {
                        kdDebug(7023) << *it << " is unchanged, skipping" << endl;
                        continue;
                    }
                    delete provider;
                }
                KSimpleConfig desktop(kapp->dirs()->saveLocation("services", "searchproviders/") + name + ".desktop");
                desktop.setGroup("Desktop Entry");
                desktop.writeEntry("Type", "Service");
                desktop.writeEntry("ServiceTypes", "SearchProvider");
                desktop.writeEntry("Name", *it);
                desktop.writeEntry("Query", query);
                desktop.writeEntry("Keys", keys);
                desktop.writeEntry("Charset", charset);
                kdDebug(7023) << "Created searchproviders/" << name << ".desktop for " << *it << endl;
            }
            oldConfig.deleteEntry("SearchEngines", false);
            oldConfig.setGroup("General");
            oldConfig.writeEntry("InternetKeywordsSearchFallback", fallback);
            kdDebug(7023) << "...completed" << endl;
        }
    }

    PIDDBG << "Keywords Engine: Loading config..." << endl;
    // First empty any current config we have.
    m_lstInternetKeywordsEngine.clear();

    // Load the config.
    KConfig config( name() + "rc", false, false );
    QStringList engines;
    QString selIKWSEngine;
    config.setGroup( "General" );

    m_bInternetKeywordsEnabled = config.readBoolEntry("InternetKeywordsEnabled", true);
    selIKWSEngine = config.readEntry("InternetKeywordsSelectedEngine", IKW_REALNAMES);
    m_searchFallback = config.readEntry("InternetKeywordsSearchFallback");

    m_bVerbose = config.readBoolEntry("Verbose");
    m_bSearchKeywordsEnabled = config.readBoolEntry("SearchEngineShortcutsEnabled", true);

    PIDDBG << "Internet Keyword Enabled: " << m_bInternetKeywordsEnabled << endl;
    PIDDBG << "Selected IKWS Engine(s): " << selIKWSEngine << endl;
    PIDDBG << "Internet Keywords Fallback Search Engine: " << m_searchFallback << endl;

    engines = config.readListEntry("InternetKeywordsEngines");
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
	    e.m_strCharset = config.readEntry("Charset");
	    m_lstInternetKeywordsEngine.append(e);
	    if (selIKWSEngine == (e.m_strName)) {
		m_currInternetKeywordsEngine = e;
	    }
	}
    }

    IKWSEntry rn = ikwsEntryByName(IKW_REALNAMES);
    if (rn.m_strName.isEmpty())	{
	rn.m_strName = IKW_REALNAMES;
	rn.m_strQuery = QString::fromLatin1("http://navigation.realnames.com/resolver.dll?realname=\\1&charset=\\{ikw_charset}&responsecharset=\\{wsc_charset}&providerid=180");
	rn.m_strCharset = "utf-8";
	rn.m_strQueryWithSearch = QString::fromLatin1("http://navigation.realnames.com/resolver.dll?"
						      "action=navigation&realname=\\{0}&charset=\\{ikw_charset}&providerid=180&fallbackuri=\\{wsc_fallback}");
	if (rn.m_strName == selIKWSEngine)
	    m_currInternetKeywordsEngine = rn;

	m_lstInternetKeywordsEngine.append(rn);
    }
}

