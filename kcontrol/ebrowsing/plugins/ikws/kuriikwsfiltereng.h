/*  This file is part of the KDE project
    Copyright (C) 1999 Simon Hausmann <hausmann@kde.org>
    Internet Keywords support (C) 1999 Yves Arrouye <yves@realnames.com>
    Current maintainer Yves Arrouye <yves@realnames.com>

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

#ifndef __kurisearchfilterengine_h__
#define __kurisearchfilterengine_h__ $Id$

#include <qvaluelist.h>
#include <qstring.h>
#include <qstringlist.h>

class KURISearchFilterEngine {
public:
    KURISearchFilterEngine();

    static KURISearchFilterEngine *self();
    QCString name() const;

    struct SearchEntry {
	QString m_strName;
	QString m_strQuery;
	QStringList m_lstKeys;
    };

    struct NavEntry {
	QString m_strName;
	QString m_strQuery;
	QString m_strQueryWithSearch;
    };

    void insertSearchEngine(SearchEntry e);
    void removeSearchEngine(const QString & name);

    QValueList < SearchEntry > searchEngines() const {
	return m_lstSearchEngines;
    }
    QString searchQuery(const QString & key) const;
    QString navQuery() const;

    void setNavEnabled(bool);
    bool navEnabled() const;

    QString searchFallback() const;
    void setSearchFallback(const QString &name);

    SearchEntry searchEntryByName(const QString & name) const;
    NavEntry navEntryByName(const QString & name) const;

    bool verbose() const {
	return m_bVerbose;
    }

    void loadConfig();
    void saveConfig() const;

private:
    QValueList < SearchEntry > m_lstSearchEngines;

    bool m_bInternetKeywordsEnabled;
    QValueList < NavEntry > m_lstInternetKeywordsEngines;

    NavEntry m_currInternetKeywordsNavEngine;
    SearchEntry m_currInternetKeywordsSearchEngine;

    bool m_bVerbose;

    static KURISearchFilterEngine *s_pSelf;
};

#endif
