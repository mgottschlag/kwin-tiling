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

#include <QAction>
#include <QDBusInterface>
#include <QDBusReply>
#include <QLabel>
#include <QList>
#include <QStack>
#include <QWidget>

#include <KIcon>
#include <KBookmarkManager>
#include <KToolInvocation>
#include <KUrl>
#include <KStandardDirs>


BookmarksRunner::BookmarksRunner( QObject* parent, const QVariantList &args )
    : Plasma::AbstractRunner(parent, args)
{
    KGlobal::locale()->insertCatalog("krunner_bookmarksrunner");
    Q_UNUSED(args)
    setObjectName(i18n("Bookmarks"));
    m_icon = KIcon("bookmarks");
}

BookmarksRunner::~BookmarksRunner()
{
}

void BookmarksRunner::match(Plasma::SearchContext *search)
{
    const QString term = search->searchTerm();
    if (term.length() < 3) {
        return;
    }

    KBookmarkManager *bookmarkManager = KBookmarkManager::userBookmarksManager();
    KBookmarkGroup bookmarkGroup = bookmarkManager->root();

    QList<Plasma::SearchMatch*> matches;
    QStack<KBookmarkGroup> groups;

    KBookmark bookmark = bookmarkGroup.first();
    while (!bookmark.isNull()) {
        if (bookmark.isGroup()) { // descend
            //kDebug () << "descending into" << bookmark.text();
            groups.push(bookmarkGroup);
            bookmarkGroup = bookmark.toGroup();
            bookmark = bookmarkGroup.first();

            while (bookmark.isNull() && !groups.isEmpty()) {
                bookmark = bookmarkGroup;
                bookmarkGroup = groups.pop();
                bookmark = bookmarkGroup.next(bookmark);
            }

            continue;
        }

        Plasma::SearchMatch *match = 0;
        if (bookmark.text().toLower() == term.toLower()) {
            match = new Plasma::SearchMatch(this);
            match->setType(Plasma::SearchMatch::ExactMatch);
            match->setRelevance(1);
        } else if (bookmark.text().contains(term, Qt::CaseInsensitive)) {
            match = new Plasma::SearchMatch(this);
            match->setRelevance(0.9);
        } else if (bookmark.url().prettyUrl().contains(term, Qt::CaseInsensitive)) {
            match = new Plasma::SearchMatch(this);
            match->setRelevance(0.8);
        }

        if (match) {
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

            match->setIcon(m_icon);
            match->setText(bookmark.text());
            match->setData(bookmark.url().url());
            matches << match;
        }

        bookmark = bookmarkGroup.next(bookmark);
        while (bookmark.isNull() && !groups.isEmpty()) {
            bookmark = bookmarkGroup;
            bookmarkGroup = groups.pop();
            //kDebug() << "ascending from" << bookmark.text() << "to" << bookmarkGroup.text();
            bookmark = bookmarkGroup.next(bookmark);
        }
    }

    search->addMatches(term, matches);
}

KIcon BookmarksRunner::getFavicon(const KUrl &url)
{
    // query the favicons module
    QDBusInterface favicon("org.kde.kded", "/modules/favicons", "org.kde.FavIcon");
    QDBusReply<QString> reply = favicon.call("iconForUrl", url.url());

    if (!reply.isValid()) {
        return KIcon();
    }

    // locate the favicon
    QString iconFile = KGlobal::dirs()->findResource("cache",reply.value()+".png");
    if(iconFile.isNull()) {
        return KIcon();
    }

    KIcon icon = KIcon(iconFile);

    return icon;
}

void BookmarksRunner::exec(const Plasma::SearchContext *search, const Plasma::SearchMatch *action)
{
    KUrl url = (KUrl)action->data().toString();
    //kDebug() << "BookmarksRunner::exec opening: " << url.url();
    KToolInvocation::invokeBrowser(url.url());
}

#include "bookmarksrunner.moc"
