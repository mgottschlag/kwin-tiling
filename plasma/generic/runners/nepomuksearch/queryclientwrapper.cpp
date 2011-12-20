/* This file is part of the Nepomuk Project
   Copyright (c) 2008-2009 Sebastian Trueg <trueg@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "queryclientwrapper.h"
#include "nepomuksearchrunner.h"

#include <nepomuk/nie.h>
#include <nepomuk/nfo.h>
#include <Nepomuk/File>
#include <Nepomuk/Variant>
#include <Nepomuk/Types/Class>
#include <Nepomuk/Query/QueryServiceClient>
#include <Nepomuk/Query/Result>
#include <Nepomuk/Query/Query>
#include <Nepomuk/Query/QueryParser>

#include <KIcon>
#include <KDebug>
#include <KMimeType>

#include <Plasma/QueryMatch>
#include <Plasma/RunnerContext>
#include <Plasma/AbstractRunner>

#include <QtCore/QTimer>
#include <QtCore/QMutex>


static const int s_maxResults = 10;

Nepomuk::QueryClientWrapper::QueryClientWrapper(SearchRunner* runner, Plasma::RunnerContext* context)
    : QObject(),
      m_runner(runner),
      m_runnerContext(context)
{
    // initialize the query client
    m_queryServiceClient = new Nepomuk::Query::QueryServiceClient(this);
    connect(m_queryServiceClient, SIGNAL(newEntries(QList<Nepomuk::Query::Result>)),
             this, SLOT(slotNewEntries(QList<Nepomuk::Query::Result>)));
}


Nepomuk::QueryClientWrapper::~QueryClientWrapper()
{
}


void Nepomuk::QueryClientWrapper::runQuery()
{
    //kDebug() << m_runnerContext->query();

    // add a timeout in case something goes wrong (no user wants to wait more than 30 seconds)
    QTimer::singleShot(30000, m_queryServiceClient, SLOT(close()));

    Query::Query q = Query::QueryParser::parseQuery(m_runnerContext->query());
    q.setLimit(s_maxResults);
    m_queryServiceClient->blockingQuery(q);

    //kDebug() << m_runnerContext->query() << "done";
}


namespace {
qreal normalizeScore(double score) {
    // no search result is ever a perfect match, NEVER. And mostly, when typing a URL
    // the users wants to open that url instead of using the search result. Thus, all
    // search results need to have a lower score than URLs which can drop to 0.5
    // And in the end, for 10 results, the score is not that important at the moment.
    // This can be improved in the future.
    // We go the easy way here and simply cut the score at 0.4
    return qMin(0.4, score);
}
}

void Nepomuk::QueryClientWrapper::slotNewEntries(const QList<Nepomuk::Query::Result>& results)
{
    QList<Plasma::QueryMatch> matches;
    foreach(const Query::Result& result, results) {
        Plasma::QueryMatch match(m_runner);
        match.setType(Plasma::QueryMatch::PossibleMatch);
        match.setRelevance(normalizeScore(result.score()));

        Nepomuk::Resource res = result.resource();

        QString type;
        QString iconName;

        KMimeType::Ptr mimetype;
        if (res.hasProperty(Nepomuk::Vocabulary::NIE::mimeType())) {
            mimetype = KMimeType::mimeType(res.property(Nepomuk::Vocabulary::NIE::mimeType()).toString());
        }
        if (!mimetype && res.isFile() && res.toFile().url().isLocalFile()) {
            const KUrl url(res.toFile().url());
            mimetype = KMimeType::findByUrl(url);
        }

        if (mimetype) {
            type = mimetype->comment();
            iconName = mimetype->iconName();
        }

        if (type.isEmpty() ) {
            type = Nepomuk::Types::Class(res.resourceType()).label();
            iconName = res.genericIcon();
        }

        match.setText(res.genericLabel());
        match.setSubtext(type);
        match.setIcon(KIcon(iconName.isEmpty() ? QString::fromLatin1("nepomuk") : iconName));

        match.setData(qVariantFromValue(res));
        match.setId(KUrl(res.resourceUri()).url());
        matches << match;
    }
    m_runnerContext->addMatches(m_runnerContext->query(), matches);
}

#include "queryclientwrapper.moc"
