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
#include <QHBoxLayout>
#include <QLabel>
#include <QWidget>
#include <QScriptEngine>
#include <QList>
#include <QDBusInterface>
#include <QDBusReply>

#include <KIcon>
#include <KBookmarkManager>
#include <KToolInvocation>
#include <KUrl>
#include <KStandardDirs>


BookmarksRunner::BookmarksRunner( QObject* parent, const QVariantList &args )
    : Plasma::AbstractRunner(parent)
{
    KGlobal::locale()->insertCatalog("krunner_bookmarksrunner");
    Q_UNUSED(args)
    setObjectName(i18n("Bookmarks"));
    m_icon = KIcon("bookmark");
}

BookmarksRunner::~BookmarksRunner()
{
}

void BookmarksRunner::match(Plasma::SearchContext *search)
{
    if (search->searchTerm().length() < 3) {
        return;
    }

    KBookmarkManager *bookmarkMgr = KBookmarkManager::userBookmarksManager();
    KBookmarkGroup bookmarkGrp = bookmarkMgr->root();

    QList<KBookmark> matchingBookmarks = searchBookmarks(bookmarkGrp, search->searchTerm());
    foreach (KBookmark bookmark, matchingBookmarks) {
        kDebug() << "Found bookmark: " << bookmark.text() << " (" << bookmark.url().prettyUrl() << ")";
        Plasma::SearchMatch *action = search->addPossibleMatch(this);

        QIcon icon = getFavicon(bookmark.url());
        if (icon.isNull()) {
            action->setIcon(m_icon);
        }
        else {
            action->setIcon(icon);
        }
        action->setText(bookmark.text());
        action->setData(bookmark.url().url());
    }
}

QList<KBookmark> BookmarksRunner::searchBookmarks(const KBookmarkGroup &bookmarkGrp, const QString &query)
{
    QList<KBookmark> matchingBookmarks;
    KBookmark currentBookmark = bookmarkGrp.first();
    while(!currentBookmark.isNull()) {
        if(currentBookmark.isGroup()) { //recurse
            matchingBookmarks += searchBookmarks(currentBookmark.toGroup(), query);
        } else {
            if(currentBookmark.text().contains(query, Qt::CaseInsensitive) || currentBookmark.url().prettyUrl().contains(query, Qt::CaseInsensitive)) {
                matchingBookmarks.append(currentBookmark);
            }
        }
        currentBookmark = bookmarkGrp.next(currentBookmark);
    }
    return matchingBookmarks;
}

KIcon BookmarksRunner::getFavicon(const KUrl &url) {
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

void BookmarksRunner::exec(Plasma::SearchMatch *action)
{
    KUrl url = (KUrl)action->data().toString();
    kDebug() << "BookmarksRunner::exec opening: " << url.url();
    KToolInvocation::invokeBrowser(url.url());
}

#include "bookmarksrunner.moc"
