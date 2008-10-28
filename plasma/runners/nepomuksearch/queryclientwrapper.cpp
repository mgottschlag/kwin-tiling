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

#include "queryclientwrapper.h"
#include "nepomuksearchrunner.h"
#include "queryserviceclient.h"
#include "result.h"

#include <Nepomuk/Resource>
#include <Nepomuk/Types/Class>

#include <Soprano/Vocabulary/Xesam>

#include <KIcon>
#include <KDebug>
#include <KMimeType>

#include <plasma/querymatch.h>
#include <plasma/runnercontext.h>

#include <QtCore/QTimer>


Q_DECLARE_METATYPE(Nepomuk::Resource)

Nepomuk::QueryClientWrapper::QueryClientWrapper( SearchRunner* runner, Plasma::RunnerContext* context )
    : QObject(),
      m_runner( runner ),
      m_runnerContext( context )
{
    // initialize the query client
    m_queryServiceClient = new Nepomuk::Search::QueryServiceClient( this );
    connect( m_queryServiceClient, SIGNAL(newEntries( const QList<Nepomuk::Search::Result>& )),
             this, SLOT(slotNewEntries( const QList<Nepomuk::Search::Result>& )) );
}


Nepomuk::QueryClientWrapper::~QueryClientWrapper()
{
}


void Nepomuk::QueryClientWrapper::runQuery()
{
    kDebug() << m_runnerContext->query();

    // add a timeout in case something goes wrong (no user wants to wait more than 30 seconds)
    QTimer::singleShot( 30000, m_queryServiceClient, SLOT(close()) );

    m_queryServiceClient->blockingQuery( m_runnerContext->query() );

    kDebug() << m_runnerContext->query() << "done";
}


void Nepomuk::QueryClientWrapper::slotNewEntries( const QList<Nepomuk::Search::Result>& results )
{
    foreach( const Search::Result& result, results ) {
        Plasma::QueryMatch match( m_runner );
        match.setType( Plasma::QueryMatch::PossibleMatch );
        match.setRelevance( result.score() );

        Nepomuk::Resource res( result.resourceUri() );

        QString type;
        if( res.hasType( Soprano::Vocabulary::Xesam::File() ) ||
            res.resourceUri().scheme() == "file" ) {
            type = KMimeType::findByUrl( res.resourceUri() )->comment();
        }
        else {
            type = Nepomuk::Types::Class( res.resourceType() ).label();
        }

        match.setText( i18nc( "@action file/resource to be opened from KRunner. %1 is the name and %2 the type",
                              "Open %1 (%2)",
                              res.genericLabel(),
                              type ) );
        QString s = res.genericIcon();
        match.setIcon( KIcon( s.isEmpty() ? QString("nepomuk") : s ) );

        match.setData( qVariantFromValue( res ) );

        m_runnerContext->addMatch( m_runnerContext->query(), match );
    }
}

#include "queryclientwrapper.moc"
