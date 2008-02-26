/*  
    Copyright 2007 Robert Knight <robertknight@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef SEARCHMODEL_H
#define SEARCHMODEL_H 

// Qt
#include "core/kickoffmodel.h"

namespace Kickoff 
{

class SearchResult
{
public:
    QString url;
    QString title;
    QString subTitle;
};
typedef QList<SearchResult> ResultList;

class SearchModel : public KickoffModel
{
Q_OBJECT

public:
    SearchModel(QObject *parent);
    virtual ~SearchModel();

public Q_SLOTS:
    void setQuery(const QString& query);

private Q_SLOTS:
    void resultsAvailable(const QStringList& results);
    void resultsAvailable(const ResultList& results);

private:
    class Private;
    Private * const d;
};

class SearchInterface : public QObject
{
Q_OBJECT

public:
    SearchInterface(QObject *parent);

    virtual QString name() const = 0;
    virtual void setQuery(const QString& query) = 0;

        
Q_SIGNALS:
    void resultsAvailable(const QStringList& results);
    void resultsAvailable(const ResultList& results);
};

class ApplicationSearch : public SearchInterface
{
Q_OBJECT

public:
    ApplicationSearch(QObject *parent);

    virtual QString name() const;
    virtual void setQuery(const QString& query);

private:
   QString mimeNameForQuery(const QString& query) const;
};

class WebSearch : public SearchInterface
{
Q_OBJECT

public:
    WebSearch(QObject *parent);
    virtual QString name() const;
    virtual void setQuery(const QString& query);
};

class IndexerSearch : public SearchInterface
{
Q_OBJECT

public:
    IndexerSearch(QObject *parent);
    virtual QString name() const;
    virtual void setQuery(const QString& query);
};

}

#endif // SEARCHMODEL_H

