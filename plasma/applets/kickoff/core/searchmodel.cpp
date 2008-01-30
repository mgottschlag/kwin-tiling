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

// Own
#include "core/searchmodel.h"

#include "config-kickoff-applets.h" 
// Qt

// KDE
#include <KDebug>
#include <KMimeType>
#include <KServiceTypeTrader>
#ifdef HAVE_STRIGIDBUS
#include <strigi/qtdbus/strigiclient.h>
#endif
#include <solid/networking.h>

// Local
#include "core/models.h"

using namespace Kickoff;

class SearchModel::Private
{
public:
    Private(SearchModel *parent) : q(parent) {}

    void addItemForIface(SearchInterface *iface,QStandardItem *item)
    {
        int index = searchIfaces.indexOf(iface);
        Q_ASSERT(index >= 0);
        q->item(index)->appendRow(item);
    }
    void clear()
    {
        for (int i=0;i<q->rowCount();i++) {
            q->item(i)->removeRows(0,q->item(i)->rowCount());
        }
    }

    SearchModel * const q;
    QList<SearchInterface*> searchIfaces;
};

SearchModel::SearchModel(QObject *parent)
    : QStandardItemModel(parent)
    , d(new Private(this))
{
    d->searchIfaces << new ApplicationSearch(this);
    //d->searchIfaces << new IndexerSearch(this);
    d->searchIfaces << new WebSearch(this);

    foreach(SearchInterface *iface,d->searchIfaces) {
        QStandardItem *ifaceItem = new QStandardItem(iface->name());
        appendRow(ifaceItem);
        connect(iface,SIGNAL(resultsAvailable(QStringList)),
                this,SLOT(resultsAvailable(QStringList)));
        connect(iface,SIGNAL(resultsAvailable(ResultList)),
                this,SLOT(resultsAvailable(ResultList)));
    }
}
SearchModel::~SearchModel()
{
    delete d;
}
void SearchModel::resultsAvailable(const QStringList& results)
{
    SearchInterface *iface = qobject_cast<SearchInterface*>(sender());
    
    Q_ASSERT(iface);

    foreach(const QString& result,results) {
        //kDebug() << "Search hit from" << iface->name() << result;
        QStandardItem *resultItem = StandardItemFactory::createItemForUrl(result);
        d->addItemForIface(iface,resultItem);
    }
}
void SearchModel::resultsAvailable(const ResultList& results)
{
    SearchInterface *iface = qobject_cast<SearchInterface*>(sender());

    Q_ASSERT(iface);

    foreach(const SearchResult& result,results) {
        QStandardItem *item = StandardItemFactory::createItemForUrl(result.url);
        item->setData(result.title,Qt::DisplayRole);
        item->setData(result.subTitle,SubTitleRole);
        d->addItemForIface(iface,item);
    }
}
void SearchModel::setQuery(const QString& query)
{
    d->clear();

    if (query.isEmpty()) {
        return; 
    }

    foreach(SearchInterface *iface, d->searchIfaces) {
        iface->setQuery(query);
    }
}

SearchInterface::SearchInterface(QObject *parent)
    : QObject(parent)
{
}

ApplicationSearch::ApplicationSearch(QObject *parent)
    : SearchInterface(parent)
{
}

QString ApplicationSearch::name() const
{
    return i18n("Applications");
}

void ApplicationSearch::setQuery(const QString& query)
{
    //QString mimeName = mimeNameForQuery(query);
    QString traderQuery = QString("((exist GenericName) and ('%1' ~~ GenericName)) or ('%1' ~~ Name) or ((exist Keywords) and ('%1' ~in Keywords))"
                                  //" or ('%2' in MimeType)"
                                 )
                            .arg(query); //.arg(mimeName);
    KServiceTypeTrader *trader = KServiceTypeTrader::self();
    KService::List results = trader->query("Application",traderQuery);

    // If we have KDE 3 and KDE 4 versions of a service, return only the 
    // KDE 4 version
    QHash<QString,int> desktopNames;
    QSet<QString> execFields;
    for (int i=0;i<results.count();i++) {
        KService::Ptr service = results[i];

        int existingPos = desktopNames.value(service->name(),-1);
        KService::Ptr existing = existingPos < 0 ? KService::Ptr(0) : results[existingPos]; 

        if (!existing.isNull()) {
            if (isLaterVersion(existing,service)) {
                results[i] = 0; 
            } else if (isLaterVersion(service,existing)) {
                results[existingPos] = 0;
            } else {
                // do not show more than one entry which does the same thing when run
                // (ie. ignore entries that have an identical 'Exec' field to an existing
                // entry)
                if (execFields.contains(service->exec())) {
                    results[i] = 0;
                }
            }
        } else {
            desktopNames.insert(service->name(),i);
            execFields.insert(service->exec());
        }
    }

    QStringList pathResults;
    foreach(KService::Ptr service,results) {
        if (!service.isNull() && !service->noDisplay())  {
            pathResults << service->entryPath();
        }
    }
    emit resultsAvailable(pathResults);
}

QString ApplicationSearch::mimeNameForQuery(const QString& query) const
{
    KMimeType::Ptr type = KMimeType::findByPath('.'+query,0,true);
    if (type) {
        kDebug() << "Mime type name" << type->name();
        return type->name();
    }
    return QString();
}
WebSearch::WebSearch(QObject *parent)
    : SearchInterface(parent)
{
}
QString WebSearch::name() const
{
    return i18n("Web Searches");
}
void WebSearch::setQuery(const QString& query)
{
    ResultList results;
    SearchResult googleResult;
    googleResult.url = QString("http://www.google.com/search?q=%1").arg(query);
    googleResult.title = i18n("Search web for '%1'",query);
    results << googleResult; 
    emit resultsAvailable(results);
}
IndexerSearch::IndexerSearch(QObject *parent)
    : SearchInterface(parent)
{
}
QString IndexerSearch::name() const
{
    return i18n("Documents");
}
void IndexerSearch::setQuery(const QString& query)
{
#ifdef HAVE_STRIGIDBUS 
    static const StrigiClient searchClient;

    QList<QString> urls;
    QList<StrigiHit> hits = searchClient.getHits(query,10,0);
    foreach(const StrigiHit& hit,hits) {
        if (!hit.uri.isEmpty()) {
            urls << hit.uri;
        }
    }
    emit resultsAvailable(urls);
#endif
}

#include "searchmodel.moc"
