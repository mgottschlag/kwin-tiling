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
    loadSyntaxes();
}

WebshortcutRunner::~WebshortcutRunner()
{
}

void WebshortcutRunner::readFiltersConfig()
{
    KConfig kuriconfig("kuriikwsfilterrc", KConfig::NoGlobals);
    KConfigGroup generalgroup(&kuriconfig, "General");
    m_delimiter = generalgroup.readEntry("KeywordDelimiter", QString(':'));

    // Make sure that the searchEngines cache, etc. is refreshed when the config file is changed.
    loadSyntaxes();
    //kDebug() << "keyworddelimiter is: " << delimiter;
}

void WebshortcutRunner::sycocaChanged(const QStringList &changes)
{
    if (changes.contains("services")) {
        loadSyntaxes();
    }
}

void WebshortcutRunner::loadSyntaxes()
{
    KConfig searchEngineConfig("kuriikwsfilterrc");
    KConfigGroup searchEngineGeneral(&searchEngineConfig, "General");
    QStringList enabledSearchEngines = searchEngineGeneral.readEntry("FavoriteSearchEngines", QStringList());
    enabledSearchEngines.removeAll(QString()); // Remove empty strings just to be sure (found one in my config).
    m_searchEngines.clear();
    QList<Plasma::RunnerSyntax> syns;

    const KService::List offers = serviceQuery("SearchProvider");
    if (!offers.isEmpty()) {
        foreach (const KService::Ptr &offer, offers) {
            // Check if offer (== search engine) is even enabled, else just skip it.
            if (!enabledSearchEngines.contains(offer->desktopEntryName())) {
                continue;
            }

            const QStringList keys = offer->property("Keys", QVariant::String).toString().split(",");
            if (keys.isEmpty()) {
                continue;
            }

            m_searchEngines.append(offer);

            Plasma::RunnerSyntax s(keys.at(0) + m_delimiter + ":q:",
                                   i18n("Opens \"%1\" in a web browser with the query :q:.", offer->name()));
            // If there's more than one shorthand for one search engine, try to
            // counter potential confusion by displaying one definite
            // shorthand as an example.
            if (keys.count() > 1) {
                QStringListIterator it(keys);
                it.next(); // skip the first key, we already have it!
                while (it.hasNext()) {
                    s.addExampleQuery(it.next() + m_delimiter + ":q:");
                }
            }

            syns << s;
        }
    }

    setSyntaxes(syns);
}

void WebshortcutRunner::resetState()
{
    m_lastFailedKey.clear();
    m_lastKey.clear();
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

    if (key != m_lastKey) {
        KService::Ptr matchingService;
        foreach (const KService::Ptr &offer, m_searchEngines) {
            if (offer->property("Keys", QVariant::String).toString().split(",").contains(key)) {
                matchingService = offer;
                break;
            }
        }

        if (!context.isValid()) {
            return;
        }

        if (matchingService.isNull()) {
            m_lastFailedKey = key;
            return;
        }

        m_lastKey = key;
        m_lastFailedKey.clear();
        m_lastServiceName = matchingService->name();

        const QString query = matchingService->property("Query").toString();
        m_match.setData(query);
        m_match.setId("WebShortcut:" + m_lastKey);

        if (matchingService->icon().isEmpty()) {
            m_match.setIcon(iconForUrl(query));
        } else {
            m_match.setIcon(KIcon(matchingService->icon()));
        }
    }

    QString actionText = i18n("Search %1 for %2", m_lastServiceName,
                              term.right(term.length() - delimIndex - 1));
    //kDebug() << "url is" << url << "!!!!!!!!!!!!!!!!!!!!!!!";

    m_match.setText(actionText);
    context.addMatch(term, m_match);
}

QString WebshortcutRunner::searchQuery(const QString &query, const QString &term)
{
    QString q(term);
    KUriFilter::self()->filterUri(q, QStringList("kurisearchfilter"));
    //kDebug() << "term" << term << "filtered to" << q;
    return q;
}

KIcon WebshortcutRunner::iconForUrl(const KUrl &url)
{
    // locate the favicon
    const QString iconFile = KMimeType::favIconForUrl(url);

    if (iconFile.isEmpty()) {
        return m_icon;
    }

    return KIcon(iconFile);
}

void WebshortcutRunner::run(const Plasma::RunnerContext &context, const Plasma::QueryMatch &match)
{
    QString location = searchQuery(match.data().toString(), context.query());

    if (!location.isEmpty()) {
        KToolInvocation::invokeBrowser(location);
    }
}

#include "webshortcutrunner.moc"
