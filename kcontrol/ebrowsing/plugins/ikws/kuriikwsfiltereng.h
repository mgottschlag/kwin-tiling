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
#include <qstringlist.h>

class KURL;

class KURISearchFilterEngine
{

public:

    struct SearchEntry
    {
	  QString m_strName;
	  QString m_strQuery;
	  QStringList m_lstKeys;
    };

    struct IKWSEntry
    {
	  QString m_strName;
	  QString m_strQuery;
	  QString m_strQueryWithSearch;
    };

    KURISearchFilterEngine();
    ~KURISearchFilterEngine() {};

    QCString name() const;
    void insertSearchEngine(SearchEntry e);
    void removeSearchEngine(const QString & name);

    QValueList<SearchEntry> searchEngines() const { return m_lstSearchEngine; }
    QValueList<IKWSEntry> ikwsEngines() const { return m_lstInternetKeywordsEngine; }

    QString searchQuery( const KURL& ) const;
    QString ikwsQuery( const KURL& ) const;

    void setInternetKeywordsEnabled(bool);
    void setSearchKeywordsEnabled(bool);
    bool isInternetKeywordsEnabled() const;
    bool isSearchKeywordsEnabled() const;

    QString searchFallback() const;
    void setSearchFallback(const QString &name);

    SearchEntry searchEntryByName(const QString & name) const;
    IKWSEntry ikwsEntryByName(const QString & name) const;

    bool verbose() const { return m_bVerbose; }

    void loadConfig();
    void saveConfig() const;

    static void incRef();
    static void decRef();
    static KURISearchFilterEngine *self();

protected:
    QString formatResult( const QString&, const QString&, bool ) const;

private:
    bool m_bSearchKeywordsEnabled;
    QValueList<SearchEntry> m_lstSearchEngine;

    bool m_bInternetKeywordsEnabled;
    QValueList<IKWSEntry> m_lstInternetKeywordsEngine;

    IKWSEntry m_currInternetKeywordsEngine;
    SearchEntry m_currSearchKeywordsEngine;

    bool m_bVerbose;

    static KURISearchFilterEngine *s_pSelf;
    static unsigned long s_refCnt;
};

#endif
