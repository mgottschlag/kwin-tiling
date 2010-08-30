/*
 *   Copyright (C) 2007 Teemu Rytilahti <tpr@iki.fi>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "webshortcutrunner.h"


#include <KDebug>
#include <KLocale>
#include <KMimeType>
#include <KServiceTypeTrader>
#include <KStandardDirs>
#include <KSycoca>
#include <KToolInvocation>
#include <KUrl>
#include <KUriFilter>

WebshortcutRunner::WebshortcutRunner(QObject *parent, const QVariantList& args)
    : Plasma::AbstractRunner(parent, args),
      m_match(this)
{
    Q_UNUSED(args);
    setObjectName("Web Shortcut");
    setIgnoredTypes(Plasma::RunnerContext::Directory | Plasma::RunnerContext::File | Plasma::RunnerContext::Executable);

    m_icon = KIcon("internet-web-browser");

    m_match.setType(Plasma::QueryMatch::ExactMatch);
    m_match.setRelevance(0.9);

    m_watch.addFile(KGlobal::dirs()->locateLocal("config", "kuriikwsfilterrc"));
    connect(&m_watch, SIGNAL(dirty(QString)), this, SLOT(readFiltersConfig()));
    connect(&m_watch, SIGNAL(created(QString)), this, SLOT(readFiltersConfig()));
    connect(&m_watch, SIGNAL(deleted(QString)), this, SLOT(readFiltersConfig()));

    connect(KSycoca::self(), SIGNAL(databaseChanged(QStringList)), this,
            SLOT(sycocaChanged(QStringList)));
    connect(this, SIGNAL(teardown()), this, SLOT(resetState()));

    readFiltersConfig();
}

WebshortcutRunner::~WebshortcutRunner()
{
}

void WebshortcutRunner::readFiltersConfig()
{
    // Make sure that the searchEngines cache, etc. is refreshed when the config file is changed.
    loadSyntaxes();
}

void WebshortcutRunner::sycocaChanged(const QStringList &changes)
{
    if (changes.contains("services")) {
        loadSyntaxes();
    }
}

void WebshortcutRunner::loadSyntaxes()
{
    KUriFilterData filterData;
    QStringList filters;
    filters << "kuriikwsfilter";
    KUriFilter::self()->filterUri(filterData, filters);
    m_delimiter = filterData.searchTermSeparator();
    //kDebug() << "keyword delimiter is: " << m_delimiter << filterData.preferredSearchProviders();

    QList<Plasma::RunnerSyntax> syns;
    foreach (const QString &provider, filterData.preferredSearchProviders()) {
        //kDebug() << "checking out" << provider;
        Plasma::RunnerSyntax s(filterData.queryForPreferredSearchProvider(provider), /*":q:",*/
                              i18n("Opens \"%1\" in a web browser with the query :q:.", provider));
        /*
        // counter potential confusion by displaying one definite
        // shorthand as an example.
        if (keys.count() > 1) {
            QStringListIterator it(keys);
            it.next(); // skip the first key, we already have it!
            while (it.hasNext()) {
                s.addExampleQuery(it.next() + m_delimiter + ":q:");
            }
        }
        */
        syns << s;
    }

    setSyntaxes(syns);
}

void WebshortcutRunner::resetState()
{
    m_lastFailedKey.clear();
    m_lastServiceName.clear();
}

void WebshortcutRunner::match(Plasma::RunnerContext &context)
{
    const QString term = context.query();

    if (term.length() < 3 || !term.contains(m_delimiter)) {
        return;
    }

    //kDebug() << "checking with" << term;

    int delimIndex = term.indexOf(m_delimiter);
    if (delimIndex == term.length() - 1) {
        return;
    }

    QString key = term.left(delimIndex);

    if (key == m_lastFailedKey) {
        // we already know it's going to suck ;)
        return;
    }

    KUriFilterData filterData(term);
    QStringList filters;
    filters << "kuriikwsfilter";

    if (!KUriFilter::self()->filterUri(filterData, filters)) {
        m_lastFailedKey = key;
        return;
    }

    if (!context.isValid()) {
        //kDebug() << "invalid context";
        return;
    }

    m_lastFailedKey.clear();

    m_match.setData(filterData.uri());
    m_match.setId("WebShortcut:" + key);

    m_match.setIcon(KIcon(filterData.iconName()));

    QString actionText = i18n("Search %1 for %2", filterData.searchProvider(), filterData.searchTerm());
    //kDebug() << "url is" << url << "!!!!!!!!!!!!!!!!!!!!!!!";

    m_match.setText(actionText);
    context.addMatch(term, m_match);
}

void WebshortcutRunner::run(const Plasma::RunnerContext &, const Plasma::QueryMatch &match)
{
    const QString location = match.data().toString();

    if (!location.isEmpty()) {
        KToolInvocation::invokeBrowser(location);
    }
}

#include "webshortcutrunner.moc"
