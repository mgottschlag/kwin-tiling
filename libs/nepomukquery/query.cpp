/*
   This file is part of the Nepomuk KDE project.
   Copyright (C) 2008 Sebastian Trueg <trueg@kde.org>

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

#include "query.h"
#include "term.h"

#include <QtCore/QSharedData>
#include <QtCore/QDebug>


class Nepomuk::Search::Query::Private : public QSharedData
{
public:
    Private()
        : type( InvalidQuery ),
          limit( 0 ) {
    }

    Type type;
    Term term;
    QString sparqlQuery;
    int limit;

    QList<RequestProperty> requestProperties;
    QList<FolderLimit> folderLimits;
};


Nepomuk::Search::Query::Query()
    : d( new Private() )
{
}


Nepomuk::Search::Query::Query( const Query& other )
{
    d = other.d;
}


Nepomuk::Search::Query::Query( const Term& term )
    : d ( new Private() )
{
    d->type = PlainQuery;
    d->term = term;
}


Nepomuk::Search::Query::Query( const QString& sparqlQuery )
    : d ( new Private() )
{
    d->type = SPARQLQuery;
    d->sparqlQuery = sparqlQuery;
}


Nepomuk::Search::Query::~Query()
{
}


Nepomuk::Search::Query& Nepomuk::Search::Query::operator=( const Query& other )
{
    d = other.d;
    return *this;
}


Nepomuk::Search::Query::Type Nepomuk::Search::Query::type() const
{
    return d->type;
}


Nepomuk::Search::Term Nepomuk::Search::Query::term() const
{
    return d->term;
}


int Nepomuk::Search::Query::limit() const
{
    return d->limit;
}


QString Nepomuk::Search::Query::sparqlQuery() const
{
    return d->sparqlQuery;
}


void Nepomuk::Search::Query::setTerm( const Term& term )
{
    d->term = term;
    d->type = PlainQuery;
}


void Nepomuk::Search::Query::setLimit( int limit )
{
    d->limit = limit;
}


void Nepomuk::Search::Query::setSparqlQuery( const QString& qs )
{
    d->sparqlQuery = qs;
    d->term = Term();
    d->type = SPARQLQuery;
}


void Nepomuk::Search::Query::addRequestProperty( const QUrl& property, bool optional )
{
    d->requestProperties.append( qMakePair( property, optional ) );
}


void Nepomuk::Search::Query::clearRequestProperties()
{
    d->requestProperties.clear();
}


QList<Nepomuk::Search::Query::RequestProperty> Nepomuk::Search::Query::requestProperties() const
{
    return d->requestProperties;
}


namespace {
    template<typename T>
    bool compareQList( const QList<T>& rp1, const QList<T>& rp2 ) {
        // brute force
        foreach( const T& rp, rp1 ) {
            if ( !rp2.contains( rp ) ) {
                return false;
            }
        }
        foreach( const T& rp, rp2 ) {
            if ( !rp1.contains( rp ) ) {
                return false;
            }
        }
        return true;
    }
}

bool Nepomuk::Search::Query::operator==( const Query& other ) const
{
    if ( d->type == other.d->type &&
         d->limit == other.d->limit ) {
        if ( d->type == SPARQLQuery ) {
            return( d->sparqlQuery == other.d->sparqlQuery &&
                    compareQList( d->requestProperties, other.d->requestProperties ) &&
                    compareQList( d->folderLimits, other.d->folderLimits ) );
        }
        else {
            return( d->term == other.d->term &&
                    compareQList( d->requestProperties, other.d->requestProperties ) &&
                    compareQList( d->folderLimits, other.d->folderLimits ) );
        }
    }

    return false;
}


void Nepomuk::Search::Query::addFolderLimit( const QUrl& folder, bool include )
{
    d->folderLimits.append( qMakePair( folder, include ) );
}


void Nepomuk::Search::Query::clearFolderLimits()
{
    d->folderLimits.clear();
}


QList<Nepomuk::Search::Query::FolderLimit> Nepomuk::Search::Query::folderLimits() const
{
    return d->folderLimits;
}


QDebug operator<<( QDebug dbg, const Nepomuk::Search::Query& query )
{
    dbg << "(Query" << query.term() << query.limit() << ")";
    return dbg;
}


uint Nepomuk::Search::qHash( const Nepomuk::Search::Query& query )
{
    if ( query.type() == Nepomuk::Search::Query::SPARQLQuery )
        return qHash( query.sparqlQuery() );
    else
        return qHash( query.term() );
}
