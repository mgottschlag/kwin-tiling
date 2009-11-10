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

#ifndef _NEPOMUK_QUERY_CLIENT_WRAPPER_H_
#define _NEPOMUK_QUERY_CLIENT_WRAPPER_H_

#include <QtCore/QObject>
#include <QtCore/QList>

#include <Nepomuk/Query/Result>

namespace Plasma {
    class RunnerContext;
}

namespace Nepomuk {

    class SearchRunner;
    namespace Query {
        class QueryServiceClient;
    }

    /**
     * Wrapper around the query client which performs a blocking
     * query to be used in a krunner.
     *
     * It creates matches using a provided context.
     */
    class QueryClientWrapper : public QObject
    {
        Q_OBJECT

    public:
        QueryClientWrapper( SearchRunner* runner, Plasma::RunnerContext* context );
        ~QueryClientWrapper();

        void runQuery();

    private Q_SLOTS:
        void slotNewEntries( const QList<Nepomuk::Query::Result>& );

    private:
        Query::QueryServiceClient* m_queryServiceClient;

        SearchRunner* m_runner;
        Plasma::RunnerContext* m_runnerContext;
    };
}

#endif
