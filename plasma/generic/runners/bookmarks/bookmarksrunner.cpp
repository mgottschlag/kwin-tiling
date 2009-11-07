/*
 *   Copyright 2007 Glenn Ergeerts <glenn.ergeerts@telenet.be>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
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

#include "bookmarksrunner.h"

#include <QDBusInterface>
#include <QDBusReply>
#include <QList>
#include <QStack>
#include <QSqlQuery>
#include <QDir>

#include <KIcon>
#include <KBookmarkManager>
#include <KToolInvocation>
#include <KUrl>
#include <KStandardDirs>
#include <KDebug>
#include <KIO/Job>

BookmarksRunner::BookmarksRunner( QObject* parent, const QVariantList &args )
    : Plasma::AbstractRunner(parent, args)
{
    Q_UNUSED(args)
    setObjectName("Bookmarks");
    m_icon = KIcon("bookmarks");
    m_bookmarkManager = KBookmarkManager::userBookmarksManager();
    m_browser = whichBrowser();
    m_dbCacheFile = KStandardDirs::locateLocal("cache", "") + "bookmarkrunnerfirefoxdbfile.sqlite";
    addSyntax(Plasma::RunnerSyntax(":q:", i18n("Finds web browser bookmarks matching :q:.")));
    addSyntax(Plasma::RunnerSyntax(i18nc("list of all web browser bookmarks", "bookmarks"),
                                   i18n("List all web browser bookmarks")));

    connect(this, SIGNAL(prepare()), this, SLOT(prep()));
    connect(this, SIGNAL(teardown()), this, SLOT(down()));

    reloadConfiguration();
}

BookmarksRunner::~BookmarksRunner()
{
}

void BookmarksRunner::reloadConfiguration()
{
    if (QSqlDatabase::isDriverAvailable("QSQLITE")) {
        KConfigGroup grp = config();
        /* This allows the user to specify a profile database */
        m_dbFile = grp.readEntry<QString>("dbfile", "");
        if (m_dbFile.isEmpty()) {//Try to get the right database file, the default profile is used
            KConfig firefoxProfile(QDir::homePath() + "/.mozilla/firefox/profiles.ini",
                                   KConfig::SimpleConfig);
            QString profilePath(QDir::homePath() + "/.mozilla/firefox/");
            for (int i = 0; i < 20; i++) {
                QString groupName = QString("Profile%1").arg(i);
                if (firefoxProfile.hasGroup(groupName)) {
                    KConfigGroup fGrp = firefoxProfile.group(groupName);
                    if (fGrp.readEntry<int>("Default", 0)) {
                        profilePath = profilePath + fGrp.readEntry("Path", QString());
                        break;
                    }
                }
            }
            m_dbFile = profilePath + "/places.sqlite";
            grp.writeEntry("dbfile", m_dbFile);
        }
        m_db = QSqlDatabase::addDatabase("QSQLITE");
        m_db.setHostName("localhost");
    } else {
        kDebug() << "SQLITE driver isn't available";
        m_db = QSqlDatabase();
    }
}

void BookmarksRunner::prep()
{
    m_browser = whichBrowser();
    if (m_browser == Firefox) {
        if (m_db.isValid()) {
            kDebug() << "Cache file was removed: " << QFile(m_dbCacheFile).remove();
            kDebug() << "Database was copyed: " << QFile(m_dbFile).copy(m_dbCacheFile);
            m_db.setDatabaseName(m_dbCacheFile);
            m_dbOK = m_db.open();
            kDebug() << "Database was opened: " << m_dbOK;
        }
    }
}

void BookmarksRunner::down()
{
    if (m_db.isOpen()) {
        m_db.close();
        m_dbOK = false;
        QFile db_CacheFile(m_dbCacheFile);
        if (db_CacheFile.exists()) {
            kDebug() << "Cache file was removed: " << db_CacheFile.remove();
        }
    }
}

void BookmarksRunner::match(Plasma::RunnerContext &context)
{
    const QString term = context.query();
    if (term.length() < 3) {
        return;
    }
    bool allBookmarks = term.compare(i18nc("list of all konqueror bookmarks", "bookmarks"),
                                     Qt::CaseInsensitive) == 0;
    if (m_browser == Konqueror) {
        matchKonquerorBookmarks(context, allBookmarks, term);
    } else if (m_browser == Firefox) {
        matchFirefoxBookmarks(context, allBookmarks, term);
    }
}

KIcon BookmarksRunner::favicon(const KUrl &url)
{
    // query the favicons module
    QDBusInterface favicon("org.kde.kded", "/modules/favicons", "org.kde.FavIcon");
    QDBusReply<QString> reply = favicon.call("iconForUrl", url.url());

    if (!reply.isValid()) {
        return KIcon();
    }

    // locate the favicon
    const QString iconFile = KGlobal::dirs()->findResource("cache",reply.value()+".png");
    if(iconFile.isNull()) {
        return KIcon();
    }

    KIcon icon = KIcon(iconFile);

    return icon;
}

void BookmarksRunner::matchKonquerorBookmarks(Plasma::RunnerContext& context, bool allBookmarks,
                                              const QString& term)
{
    KBookmarkGroup bookmarkGroup = m_bookmarkManager->root();

    QList<Plasma::QueryMatch> matches;
    QStack<KBookmarkGroup> groups;

    KBookmark bookmark = bookmarkGroup.first();
    while (!bookmark.isNull()) {
        if (!context.isValid()) {
            return;
        }

        if (bookmark.isGroup()) { // descend
            //kDebug () << "descending into" << bookmark.text();
            groups.push(bookmarkGroup);
            bookmarkGroup = bookmark.toGroup();
            bookmark = bookmarkGroup.first();

            while (bookmark.isNull() && !groups.isEmpty()) {
                if (!context.isValid()) {
                    return;
                }

                bookmark = bookmarkGroup;
                bookmarkGroup = groups.pop();
                bookmark = bookmarkGroup.next(bookmark);
            }

            continue;
        }

        Plasma::QueryMatch::Type type = Plasma::QueryMatch::NoMatch;
        qreal relevance = 0;

        const QString text = bookmark.text();
        const QString url = bookmark.url().prettyUrl();
        if (text.compare(term, Qt::CaseInsensitive) == 0) {
            type = Plasma::QueryMatch::ExactMatch;
            relevance = 1.0;
        } else if (text.contains(term, Qt::CaseInsensitive)) {
            type = Plasma::QueryMatch::PossibleMatch;
            relevance = 0.45;
        } else if (url.contains(term, Qt::CaseInsensitive)) {
            type = Plasma::QueryMatch::PossibleMatch;
            relevance = 0.2;
        } else if (allBookmarks) {
            type = Plasma::QueryMatch::PossibleMatch;
            relevance = 0.18;
        }

        if (type != Plasma::QueryMatch::NoMatch) {
            //kDebug() << "Found bookmark: " << bookmark.text() << " (" << bookmark.url().prettyUrl() << ")";
            // getting the favicon is too slow and can easily lead to starving the thread pool out
            /*
            QIcon icon = getFavicon(bookmark.url());
            if (icon.isNull()) {
                match->setIcon(m_icon);
            }
            else {
                match->setIcon(icon);
            }
            */
            Plasma::QueryMatch match(this);
            match.setType(type);
            match.setRelevance(relevance);
            match.setIcon(m_icon);
            match.setText(bookmark.text());
            match.setData(bookmark.url().url());
            matches << match;
        }

        bookmark = bookmarkGroup.next(bookmark);
        while (bookmark.isNull() && !groups.isEmpty()) {
            if (!context.isValid()) {
                return;
            }

            bookmark = bookmarkGroup;
            bookmarkGroup = groups.pop();
            //kDebug() << "ascending from" << bookmark.text() << "to" << bookmarkGroup.text();
            bookmark = bookmarkGroup.next(bookmark);
        }
    }
    context.addMatches(term, matches);
}

void BookmarksRunner::matchFirefoxBookmarks(Plasma::RunnerContext& context, bool allBookmarks, const QString& term)
{
    if (!m_dbOK) {
        return;
    }

    QList<int> fks;
    QHash<int, QString> titles; //index (fk), title
    QHash<int, QUrl> urls; //index, url (QUrl in order to go with QVariant)
    QHash<int, KIcon> icons; //index, icon
    QString tmpTerm = term;
    QSqlQuery query;
    if (allBookmarks) {
        query = QSqlQuery("SELECT moz_bookmarks.fk, moz_bookmarks.title, moz_places.url," \
                    "moz_places.favicon_id FROM moz_bookmarks, moz_places WHERE " \
                    "moz_bookmarks.type = 1 AND moz_bookmarks.fk = moz_places.id");
    } else {
        const QString escapedTerm = tmpTerm.replace("'", "\\'");
        query = QSqlQuery("SELECT moz_bookmarks.fk, moz_bookmarks.title, moz_places.url," \
                        "moz_places.favicon_id FROM moz_bookmarks, moz_places WHERE " \
                        "moz_bookmarks.type = 1 AND moz_bookmarks.fk = moz_places.id AND " \
                        "(moz_bookmarks.title LIKE  '%" + escapedTerm + "%' or moz_places.url LIKE '%"
                        + escapedTerm + "%')");
    }
    while (query.next()) {
        const QString title = query.value(1).toString();
        const QUrl url = query.value(2).toString();
        //const int favicon_id = query.value(3).toInt();

        int fk = query.value(0).toInt();
        fks << fk;
        titles.insert(fk, title);
        urls.insert(fk, url);
        icons.insert(fk, m_icon); //could be changed to use favicon
    }

    if (!context.isValid()) {
        return;
    }

    QList<Plasma::QueryMatch> matches;
    foreach (const int& fk, fks) {
        Plasma::QueryMatch match(this);
        match.setIcon(icons[fk]);
        match.setText(titles[fk]);
        match.setData(urls[fk]);
        matches << match;
    }
    context.addMatches(term, matches);
}

BookmarksRunner::Browser BookmarksRunner::whichBrowser()
{
    KConfigGroup config(KSharedConfig::openConfig("kdeglobals"), QLatin1String("General") );
    QString exec = config.readPathEntry( QLatin1String("BrowserApplication"), QString("") );
    kDebug() << exec;
    if (exec.contains("firefox", Qt::CaseInsensitive)) {
        return Firefox;
    } else if (exec.contains("konqueror", Qt::CaseInsensitive)) {
        return Konqueror;
    } else {
        return Default;
    }
}

void BookmarksRunner::run(const Plasma::RunnerContext &context, const Plasma::QueryMatch &action)
{
    Q_UNUSED(context);
    KUrl url = (KUrl)action.data().toString();
    //kDebug() << "BookmarksRunner::run opening: " << url.url();
    KToolInvocation::invokeBrowser(url.url());
}

#include "bookmarksrunner.moc"
