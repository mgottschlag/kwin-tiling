/* This file is part of the Nepomuk Project
   Copyright (c) 2008 Sebastian Trueg <trueg@kde.org>

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

#include "nepomuksearchrunner.h"
#include "queryclientwrapper.h"

#include "queryserviceclient.h"

#include <KRun>
#include <KDebug>

#include <Nepomuk/Resource>

#include <Soprano/Vocabulary/NAO>


Q_DECLARE_METATYPE(Nepomuk::Resource)

namespace {
    /**
     * Milliseconds to wait before issuing the next query.
     * This timeout is intended to prevent us from starting
     * a query after each key press by the user. So it should
     * roughly equal the time between key presses of an average user.
     */
    const int s_userActionTimeout = 400;
}


Nepomuk::SearchRunner::SearchRunner( QObject* parent, const QVariantList& args )
    : Plasma::AbstractRunner( parent, args )
{
    init();
}


Nepomuk::SearchRunner::SearchRunner( QObject* parent, const QString& serviceId )
    : Plasma::AbstractRunner( parent, serviceId )
{
    init();
}


void Nepomuk::SearchRunner::init()
{
    // we are pretty slow at times and use DBus calls
    setSpeed( SlowSpeed );

    // we are way less important than others, mostly because we are slow
    setPriority( LowPriority );

    m_matchCnt = 0;
}


Nepomuk::SearchRunner::~SearchRunner()
{
}


void Nepomuk::SearchRunner::match( Plasma::RunnerContext& context )
{
    kDebug() << &context << context.query();

    // This method needs to be thread-safe since KRunner does simply start new threads whenever
    // the query term changes.
    m_mutex.lock();

    // we do not want to restart a query on each key-press. That would result
    // in way too many queries for the rather sluggy Nepomuk query service
    // Thus, we use a little timeout to make sure we do not query too often

    // we remember the match count when starting to wait, so we do not start the query
    // in case another call deprecated us (KRunner will simply rerun this method in another
    // thread when the user input changes)
    int c = ++m_matchCnt;

    kDebug() << &context << "waiting for match cnt" << c;

    m_waiter.wait( &m_mutex, s_userActionTimeout );
    if( c != m_matchCnt ) {
        kDebug() << &context << "deprecated match cnt" << c;
        // we are no longer the latest call
        m_mutex.unlock();
        return;
    }

    kDebug() << &context << "running match cnt" << c;

    m_mutex.unlock();

    // no queries on very short strings
    if( Search::QueryServiceClient::serviceAvailable() &&
        context.query().count() >= 3 ) {
        QueryClientWrapper queryWrapper( this, &context );
        queryWrapper.runQuery();
        m_waiter.wakeAll();
    }
}


void Nepomuk::SearchRunner::run( const Plasma::RunnerContext&, const Plasma::QueryMatch& match )
{
    Nepomuk::Resource res = match.data().value<Nepomuk::Resource>();
    QUrl url;

    if( res.hasType( Soprano::Vocabulary::NAO::Tag() ) ) {
        url = QUrl( QString( "nepomuksearch:/hasTag:\"%1\"" ).arg( res.genericLabel() ) );
    }
    else {
        url = res.resourceUri();
    }
	
    (void)new KRun( url, 0 );
}

K_EXPORT_PLASMA_RUNNER(nepomuksearchrunner, Nepomuk::SearchRunner)

#include "nepomuksearchrunner.moc"
