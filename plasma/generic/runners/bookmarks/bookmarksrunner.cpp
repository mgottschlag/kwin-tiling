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
#include <KMimeType>
#include <KMimeTypeTrader>
#include <KToolInvocation>
#include <KUrl>
#include <KStandardDirs>
#include <KDebug>
#include <KIO/Job>

BookmarksRunner::BookmarksRunner( QObject* parent, const QVariantList &args )
    : Plasma::AbstractRunner(parent, args)
{
    Q_UNUSED(args)
    setObjectName( QLatin1String("Bookmarks" ));
    m_icon = KIcon("bookmarks");
    m_bookmarkManager = KBookmarkManager::userBookmarksManager();
    m_browser = whichBrowser();
    addSyntax(Plasma::RunnerSyntax(":q:", i18n("Finds web browser bookmarks matching :q:.")));
    setDefaultSyntax(Plasma::RunnerSyntax(i18nc("list of all web browser bookmarks", "bookmarks"),
                                   i18n("List all web browser bookmarks")));

    connect(this, SIGNAL(prepare()), this, SLOT(prep()));
    connect(this, SIGNAL(teardown()), this, SLOT(down()));

    reloadConfiguration();
}

BookmarksRunner::~BookmarksRunner()
{
    if (!m_dbCacheFile.isEmpty()) {
        QFile db_CacheFile(m_dbCacheFile);
        if (db_CacheFile.exists()) {
            kDebug() << "Cache file was removed: " << db_CacheFile.remove();
        }
    }
}

void BookmarksRunner::reloadConfiguration()
{
    if (m_browser == Firefox) {
        if (QSqlDatabase::isDriverAvailable("QSQLITE")) {
            KConfigGroup grp = config();
            /* This allows the user to specify a profile database */
            m_dbFile = grp.readEntry<QString>("dbfile", "");
            if (m_dbFile.isEmpty() || QFile::exists(m_dbFile)) {
                //Try to get the right database file, the default profile is used
                KConfig firefoxProfile(QDir::homePath() + "/.mozilla/firefox/profiles.ini",
                                       KConfig::SimpleConfig);
                QStringList profilesList = firefoxProfile.groupList();
                profilesList = profilesList.filter(QRegExp("^Profile\\d+$"));
                int size = profilesList.size();

                QString profilePath;
                if (size == 1) {
                    // There is only 1 profile so we select it
                    KConfigGroup fGrp = firefoxProfile.group(profilesList.first());
                    profilePath = fGrp.readEntry("Path", "");
                } else {
                    // There are multiple profiles, find the default one
                    foreach(const QString & profileName, profilesList) {
                        KConfigGroup fGrp = firefoxProfile.group(profileName);
                        if (fGrp.readEntry<int>("Default", 0)) {
                            profilePath = fGrp.readEntry("Path", "");
                            break;
                        }
                    }
                }

                if (profilePath.isEmpty()) {
                    kDebug() << "No default firefox profile found";
                    m_db = QSqlDatabase();
                    return;
                }

                profilePath.prepend(QString("%1/.mozilla/firefox/").arg(QDir::homePath()));
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
}

void BookmarksRunner::prep()
{
    m_browser = whichBrowser();
    if (m_browser == Firefox) {
        if (m_db.isValid()) {
            if (m_dbCacheFile.isEmpty()) {
                m_dbCacheFile = KStandardDirs::locateLocal("cache", "") + "bookmarkrunnerfirefoxdbfile.sqlite";
            }

            // ### DO NOT USE KIO FROM RUNNER THREADS!
            // ### This looks like a local copy, so use QFile::copy instead.
            KIO::Job *job = KIO::file_copy(m_dbFile, m_dbCacheFile, -1,
                                           KIO::HideProgressInfo | KIO::Overwrite);
            connect(job, SIGNAL(result(KJob*)), this, SLOT(dbCopied(KJob*)));
        }
    } else if (m_browser == Opera) {
        // open bookmarks file
        QString operaBookmarksFilePath = QDir::homePath() + "/.opera/bookmarks.adr";
        QFile operaBookmarksFile(operaBookmarksFilePath);
        if (!operaBookmarksFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            kDebug() << "Could not open Operas Bookmark File " + operaBookmarksFilePath;
            return;
        }

        // check format
        QString firstLine = operaBookmarksFile.readLine();
        if (firstLine.compare("Opera Hotlist version 2.0\n")) {
            kDebug() << "Format of Opera Bookmarks File might have changed.";
        }
        operaBookmarksFile.readLine(); // skip options line ("Options: encoding = utf8, version=3")
        operaBookmarksFile.readLine(); // skip empty line

        // load contents
        QString contents = operaBookmarksFile.readAll();
        m_operaBookmarkEntries = contents.split("\n\n", QString::SkipEmptyParts);

        // close file
        operaBookmarksFile.close();
    }
}

void BookmarksRunner::dbCopied(KJob *)
{
    m_db.setDatabaseName(m_dbCacheFile);
    m_dbOK = m_db.open();
    kDebug() << "Database was opened: " << m_dbOK;
}

void BookmarksRunner::down()
{
    if (m_browser == Firefox) {
        if (m_db.isOpen()) {
            m_db.close();
            m_dbOK = false;
        }
    } else if (m_browser == Opera) {
        m_operaBookmarkEntries.clear();
    }
}

void BookmarksRunner::match(Plasma::RunnerContext &context)
{
    const QString term = context.query();
    if ((term.length() < 3) && (!context.singleRunnerQueryMode())) {
        return;
    }

    bool allBookmarks = term.compare(i18nc("list of all konqueror bookmarks", "bookmarks"),
                                     Qt::CaseInsensitive) == 0;
    switch (m_browser) {
        case Firefox:
            matchFirefoxBookmarks(context, allBookmarks, term);
            break;
        case Opera:
            matchOperaBookmarks(context, allBookmarks, term);
            break;
        case Default:
            matchKonquerorBookmarks(context, allBookmarks, term);
            break;
    }
}

KIcon BookmarksRunner::favicon(const KUrl &url)
{
    // query the favicons module
    const QString iconFile = KMimeType::favIconForUrl(url);

    if (iconFile.isEmpty()) {
        return m_icon;
    }

    return KIcon(iconFile);
}

void BookmarksRunner::matchOperaBookmarks(Plasma::RunnerContext& context, bool allBookmarks,
                                                        const QString& term)
{
    QList<Plasma::QueryMatch> matches;

    QLatin1String nameStart("\tNAME=");
    QLatin1String urlStart("\tURL=");
    QLatin1String descriptionStart("\tDESCRIPTION=");

    // search
    foreach (const QString & entry, m_operaBookmarkEntries) {
        if (!context.isValid()) {
            return;
        }

        QStringList entryLines = entry.split("\n");
        if (!entryLines.first().startsWith(QString("#URL"))) {
            continue; // skip folder entries
        }
        entryLines.pop_front();

        QString name;
        QString url;
        QString description;

        foreach (const QString & line, entryLines) {
            if (line.startsWith(nameStart)) {
                name = line.mid( QString(nameStart).length() ).simplified();
            } else if (line.startsWith(urlStart)) {
                url = line.mid( QString(urlStart).length() ).simplified();
            } else if (line.startsWith(descriptionStart)) {
                description = line.mid(QString(descriptionStart).length())
                              .simplified();
            }
        }

        if (url.simplified().isEmpty())
            continue; // skip useless entries

        Plasma::QueryMatch::Type type = Plasma::QueryMatch::NoMatch;
        qreal relevance = 0;

        if (name.compare(term, Qt::CaseInsensitive) == 0
             || description.compare(term, Qt::CaseInsensitive) == 0) {
            type = Plasma::QueryMatch::ExactMatch;
            relevance = 1.0;
        } else if (name.contains(term, Qt::CaseInsensitive)) {
            type = Plasma::QueryMatch::PossibleMatch;
            relevance = 0.45;
        } else if (description.contains(term, Qt::CaseInsensitive)) {
            type = Plasma::QueryMatch::PossibleMatch;
            relevance = 0.3;
        } else if (url.contains(term, Qt::CaseInsensitive)) {
            type = Plasma::QueryMatch::PossibleMatch;
            relevance = 0.2;
        } else if (allBookmarks) {
            type = Plasma::QueryMatch::PossibleMatch;
            relevance = 0.18;
        }

        if (type != Plasma::QueryMatch::NoMatch) {
            bool isNameEmpty = name.isEmpty();
            bool isDescriptionEmpty = description.isEmpty();

            Plasma::QueryMatch match(this);
            match.setType(type);
            match.setRelevance(relevance);
            match.setIcon(m_icon);

            // Try to set the following as text in this order: name, description, url
            match.setText( isNameEmpty
                           ?
                           (!isDescriptionEmpty ? description : url)
                           :
                           name );

            match.setData(url);
            matches << match;
        }
    }
    context.addMatches(term, matches);
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

        if (bookmark.isSeparator()) {
            bookmark = bookmarkGroup.next(bookmark);
            continue;
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
            Plasma::QueryMatch match(this);

            QIcon icon = favicon(bookmark.url());
            if (icon.isNull()) {
                match.setIcon(m_icon);
            } else {
                match.setIcon(icon);
            }

            match.setType(type);
            match.setRelevance(relevance);
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

    QList<Plasma::QueryMatch> matches;
    QString tmpTerm = term;
    QSqlQuery query;
    if (allBookmarks) {
        query = QSqlQuery("SELECT moz_bookmarks.fk, moz_bookmarks.title, moz_places.url," \
                    "moz_places.favicon_id FROM moz_bookmarks, moz_places WHERE " \
                    "moz_bookmarks.type = 1 AND moz_bookmarks.fk = moz_places.id");
    } else {
        const QString escapedTerm = tmpTerm.replace('\'', "\\'");
        query = QSqlQuery("SELECT moz_bookmarks.fk, moz_bookmarks.title, moz_places.url," \
                        "moz_places.favicon_id FROM moz_bookmarks, moz_places WHERE " \
                        "moz_bookmarks.type = 1 AND moz_bookmarks.fk = moz_places.id AND " \
                        "(moz_bookmarks.title LIKE  '%" + escapedTerm + "%' or moz_places.url LIKE '%"
                        + escapedTerm + "%')");
    }

    while (query.next() && context.isValid()) {
        const QString title = query.value(1).toString();
        const QUrl url = query.value(2).toString();
        //const int favicon_id = query.value(3).toInt();

        if (title.isEmpty() || url.isEmpty() || url.scheme().contains("place")) {
            //Don't use bookmarks with empty title, url or Firefox intern url
            kDebug() << "element was not added";
            continue;
        }

        Plasma::QueryMatch::Type type = Plasma::QueryMatch::NoMatch;
        qreal relevance = 0;

        if (title.compare(term, Qt::CaseInsensitive) == 0) {
            type = Plasma::QueryMatch::ExactMatch;
            relevance = 1.0;
        } else if (title.contains(term, Qt::CaseInsensitive)) {
            type = Plasma::QueryMatch::PossibleMatch;
            relevance = 0.45;
        } else if (url.toString().contains(term, Qt::CaseInsensitive)) {
            type = Plasma::QueryMatch::PossibleMatch;
            relevance = 0.2;
        } else if (allBookmarks) {
            type = Plasma::QueryMatch::PossibleMatch;
            relevance = 0.18;
        }

        Plasma::QueryMatch match(this);
        QIcon icon = favicon(url);
        if (icon.isNull()) {
            match.setIcon(m_icon);
        } else {
            match.setIcon(icon);
        }
        match.setText(title);
        match.setData(url);
        match.setType(type);
        match.setRelevance(relevance);
        matches << match;
    }
    context.addMatches(term, matches);
}

BookmarksRunner::Browser BookmarksRunner::whichBrowser()
{
    //HACK find the default browser
    KConfigGroup config(KSharedConfig::openConfig("kdeglobals"), QLatin1String("General") );
    QString exec = config.readPathEntry(QLatin1String("BrowserApplication"), QString());
    if (exec.isEmpty()) {
        KService::Ptr service = KMimeTypeTrader::self()->preferredService("text/html");
        if (service) {
            exec = service->exec();
        }
    }

    //kDebug() << exec;
    if (exec.contains("firefox", Qt::CaseInsensitive)) {
        return Firefox;
    } else if (exec.contains("opera", Qt::CaseInsensitive)) {
        return Opera;
    } else {
        return Default;
    }
}

void BookmarksRunner::run(const Plasma::RunnerContext &context, const Plasma::QueryMatch &action)
{
    Q_UNUSED(context);
    const QString term = action.data().toString();
    KUrl url = KUrl(term);

    //support urls like "kde.org" by transforming them to http://kde.org
    if (url.protocol().isEmpty()) {
        const int idx = term.indexOf('/');

        url.clear();
        url.setHost(term.left(idx));
        if (idx != -1) {
            //allow queries
            const int queryStart = term.indexOf('?', idx);
            int pathLength = -1;
            if ((queryStart > -1) && (idx < queryStart)) {
                pathLength = queryStart - idx;
                url.setQuery(term.mid(queryStart));
            }

            url.setPath(term.mid(idx, pathLength));
        }
        url.setProtocol("http");
    }

    //kDebug() << "BookmarksRunner::run opening: " << url.url();
    KToolInvocation::invokeBrowser(url.url());
}

QMimeData * BookmarksRunner::mimeDataForMatch(const Plasma::QueryMatch * match)
{
    QMimeData * result = new QMimeData();
    QList<QUrl> urls;
    urls << QUrl(match->data().toString());
    result->setUrls(urls);

    result->setText(match->data().toString());

    return result;
}

#include "bookmarksrunner.moc"
