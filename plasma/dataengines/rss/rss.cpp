/*
 *   Copyright (C) 2007 Aaron Seigo <aseigo@kde.org>
 *   Copyright (C) 2007 Petri Damsten <damu@iki.fi>
 *   Copyright (C) 2008 Rob Scheepmaker <r.scheepmaker@student.utwente.nl>
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

//Own
#include "rss.h"

//KDE
#include <KDebug>
#include <KUrl>
#include <kstandarddirs.h>
#include <syndication/item.h>
#include <syndication/loader.h>
#include <syndication/image.h>

//Plasma
#include <plasma/dataengine.h>

//Qt
#include <QDateTime>
#include <QDBusReply>
#include <QDBusInterface>
#include <QTimer>
#include <QSignalMapper>

#define TIMEOUT 15000    //timeout before updating the source if not all feeds
                         //are fetched.
#define CACHE_TIMEOUT 60 //time in seconds before the cached feeds are marked
                         //as out of date.
#define MINIMUM_INTERVAL 60000
#define FAVICONINTERFACE "org.kde.FavIcon"

RssEngine::RssEngine(QObject* parent, const QVariantList& args)
    : Plasma::DataEngine(parent, args)
{
    Q_UNUSED(args)
    setMinimumPollingInterval(MINIMUM_INTERVAL);
    m_favIconsModule = new QDBusInterface("org.kde.kded", "/modules/favicons",
                                          FAVICONINTERFACE);
    m_signalMapper = new QSignalMapper(this);
    connect(m_favIconsModule, SIGNAL(iconChanged(bool,QString,QString)),
            this, SLOT(slotIconChanged(bool,QString,QString)));
    connect(m_signalMapper, SIGNAL(mapped(const QString &)),
            this, SLOT(timeout(const QString &)));
}

RssEngine::~RssEngine()
{
    delete m_favIconsModule;
}

bool RssEngine::updateSourceEvent(const QString &name)
{
    /* Plasmoids using this engine should be able to retrieve
     * multiple feeds at the same time, so we allow a comma
     * separated list of url's
     */
    // NOTE: A comma seperated list of feeds is not url compliant. Urls 
    // may and do contain commas see http://www.spiegel.de/schlagzeilen/rss/0,5291,,00.xml
    // I have changed it to something more not url compliant " " three dots
    // Otherwise take a list instead
    QStringList sources = name.split(" ", QString::SkipEmptyParts);

    foreach (const QString& source, sources) {
        // Let's first see if we've got a recent cached version of
        // the feed. This avoids 'large' amounts of unnecesarry network
        // traffic.
        if (QDateTime::currentDateTime() >
            m_feedTimes[source.toLower()].addSecs(CACHE_TIMEOUT)){
            kDebug() << "Cache from " << source <<
                        " older than 60 seconds, refreshing...";

            Syndication::Loader * loader = Syndication::Loader::create();
            connect(loader, SIGNAL(loadingComplete(Syndication::Loader*,
                                                   Syndication::FeedPtr,
                                                   Syndication::ErrorCode)),
                      this, SLOT(processRss(Syndication::Loader*,
                                            Syndication::FeedPtr,
                                            Syndication::ErrorCode)));

            m_feedMap.insert(loader, source);
            m_sourceMap.insert(loader, name);
            loader->loadFrom(source);
        } else {
            kDebug() << "Recent cached version of " << source <<
                        " found. Skipping...";

            // We might want to update the source:
            if (cachesUpToDate(name)) {
                updateFeeds(name, m_feedTitles[ source ] );
            }
        }
    }

    QTimer *timer = new QTimer(this);
    m_timerMap[name] = timer;
    timer->setSingleShot(true);
    m_signalMapper->setMapping(timer, name);

    connect(timer, SIGNAL(timeout()), m_signalMapper, SLOT(map()));

    timer->start(TIMEOUT);
    return true;
}

void RssEngine::slotIconChanged(bool isHost, const QString& hostOrURL,
                                             const QString& iconName)
{
    Q_UNUSED(isHost);
    QString iconFile = KGlobal::dirs()->findResource("cache",
                                                     iconName+".png");
    QString url = hostOrURL.toLower();

    m_feedIcons[url] = iconFile;
    QMap<QString, QVariant> map;

    for (int i = 0; i < m_feedItems[url].size(); i++) {
        map = m_feedItems[url].at(i).toMap();
        map["icon"] = iconFile;
        m_feedItems[url].replace(i, map);
    }

    //Are there sources ready to get updated now?
    foreach (const QString& source, m_sourceMap) {
        if (source.contains(url, Qt::CaseInsensitive) &&
            cachesUpToDate(source)) {
            kDebug() << "all caches from source " << source <<
                        " up to date, updating...";
            updateFeeds(source, m_feedTitles[ source ] );
        }
    }
}

void RssEngine::timeout(const QString & source)
{
    kDebug() << "timout fired, updating source";
    updateFeeds(source, m_feedTitles[ source ] );
    m_signalMapper->removeMappings(m_timerMap[source]);
}

bool RssEngine::sourceRequestEvent(const QString &name)
{
    setData(name, DataEngine::Data());
    updateSourceEvent(name);
    return true;
}

void RssEngine::processRss(Syndication::Loader* loader,
                           Syndication::FeedPtr feed,
                           Syndication::ErrorCode error)
{
    QString url = m_feedMap.take(loader);
    QString source = m_sourceMap.take(loader);
    QString title;
    bool iconRequested = false;
    KUrl u(url);

    if (error != Syndication::Success) {
        kDebug() << "Syndication did not work out... url = " << url;
        title = i18n("Syndication did not work out");
        setData(source, "title", i18n("Fetching feed failed."));
        setData(source, "link", url);
    } else {
        title = feed->title();
        QVariantList items;
        QString location;

        foreach (const Syndication::ItemPtr& item, feed->items()) {
            QMap<QString, QVariant> dataItem;

            dataItem["title"]       = item->title();
            dataItem["feed_title"]  = feed->title();
            dataItem["link"]        = item->link();
            dataItem["feed_url"]    = url;
            dataItem["description"] = item->description();
            dataItem["content"]     = item->content();
            dataItem["time"]        = (uint)item->datePublished();
            if (!m_feedIcons.contains(url.toLower()) && !iconRequested) {
                //lets request an icon, and only do this once per feed.
                location = iconLocation(u);
                if (location.isEmpty()) {
                    m_favIconsModule->call( "downloadHostIcon", u.url() );
                } else {
                    //the icon is already in cache, so call this slot manually.
                    slotIconChanged(false, u.url(), iconLocation(u));
                }
                iconRequested = true;
            }
            dataItem["icon"] = m_feedIcons[url.toLower()];

            items.append(dataItem);
        }
        m_feedItems[url.toLower()] = items;
        m_feedTimes[url.toLower()] = QDateTime::currentDateTime();
        m_feedTitles[url.toLower()] = title;

        // If we update the feeds every time a feed is fetched,
        // only the first update will actually update a connected
        // applet, which is actually sane, since plasma updates
        // only one time each interval. This means, however, that
        // we maybe want to delay updating the feeds untill either
        // timeout, or all feeds are up to date.
        if (cachesUpToDate(source)) {
            kDebug() << "all caches from source " << source
                     << " up to date, updating...";
            updateFeeds(source, title);
        } else {
            kDebug() << "not all caches from source " << source
                     << ", delaying update.";
        }
    }
}

void RssEngine::updateFeeds(const QString & source, const QString & title)
{
    /**
     * TODO: can this be improved? I'm calling mergeFeeds way too
     * often here...
     */
    QVariantList list = mergeFeeds(source);
    setData(source, "items", list);
    QStringList sources = source.split(" ", QString::SkipEmptyParts);
    if (sources.size() >  1) {
        setData(source, "title", i18np("1 RSS feed fetched",
                                       "%1 RSS feeds fetched", sources.size()));
    } else {
        setData(source, "title", title);
    }
}

bool RssEngine::cachesUpToDate(const QString & source) const
{
    QStringList sources = source.split(" ", QString::SkipEmptyParts);
    bool outOfDate = false;
    foreach (const QString &url, sources) {
        if (QDateTime::currentDateTime() >
            m_feedTimes[url.toLower()].addSecs(CACHE_TIMEOUT)){
            outOfDate = true;
        }
        if (!m_feedIcons.contains(url.toLower())) {
            outOfDate = true;
        }
    }
    return (!outOfDate);
}

bool compare(const QVariant &v1, const QVariant &v2)
{
     return v1.toMap()["time"].toUInt() > v2.toMap()["time"].toUInt();
}

QVariantList RssEngine::mergeFeeds(QString source) const
{
    QVariantList result;
    QStringList sources = source.split(" ", QString::SkipEmptyParts);

    foreach (const QString& feed, sources) {
        result += m_feedItems[feed.toLower()];
    }

    qSort(result.begin(), result.end(), compare);
    return result;
}

QString RssEngine::iconLocation(const KUrl & url) const
{
    QDBusReply<QString> reply = m_favIconsModule->call( "iconForUrl", url.url() );
    if (reply.isValid()) {
        QString result = reply;
        return result;
    }
    return QString();
}

#include "rss.moc"

