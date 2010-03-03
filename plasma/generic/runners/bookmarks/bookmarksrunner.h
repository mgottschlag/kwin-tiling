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

#ifndef BOOKMARKSRUNNER_H
#define BOOKMARKSRUNNER_H

#include <KIcon>
#include <QSqlDatabase>
#include <QMimeData>
#include <Plasma/AbstractRunner>


class KBookmark;
class KBookmarkManager;
class KJob;

/** This runner searchs for bookmarks in browsers like Konqueror, Firefox and Opera */
class BookmarksRunner : public Plasma::AbstractRunner
{
    Q_OBJECT

    public:
        BookmarksRunner(QObject* parent, const QVariantList &args);
        ~BookmarksRunner();

        void match(Plasma::RunnerContext &context);
        void run(const Plasma::RunnerContext &context, const Plasma::QueryMatch &action);
        void reloadConfiguration();

    private:
        /** Defines the browser to get the bookmarks from
          * Add support for a browser by
          * -adding the browser as part of this enum
          * -adding an if-statement with the enum part
          * -adding a method getBrowserBookmarks
          * -adding a statement to whichBrowser()
          * @see whichBrowser()
          */
        enum Browser {
                       Firefox, ///< the browser is Firefox
                       Opera, ///< the browser is Opera
                       Default ///< KDE bookmarks
                     };

        /** @returns the favicon for the url */
        KIcon favicon(const KUrl &url);

        /** Get the bookmarks from Firefox */
        void matchFirefoxBookmarks(Plasma::RunnerContext& context, bool allBookmarks,
                                                      const QString& term);
        /** Get the bookmarks from Konqueror */
        void matchKonquerorBookmarks(Plasma::RunnerContext& context, bool allBookmarks,
                                                        const QString& term);

        /** Get the bookmarks from Opera */
        void matchOperaBookmarks(Plasma::RunnerContext& context, bool allBookmarks,
                                                        const QString& term);

        /** @returns the browser to get the bookmarks from
          * @see Browser
          */
        Browser whichBrowser();

    private:
        KIcon m_icon;
        bool m_dbOK;
        Browser m_browser;
        QString m_dbFile;
        QString m_dbCacheFile;
        QSqlDatabase m_db;
        KBookmarkManager *m_bookmarkManager;
        QStringList m_operaBookmarkEntries;

    protected Q_SLOTS:
        QMimeData * mimeDataForMatch(const Plasma::QueryMatch *match);

    private Q_SLOTS:
        void prep();
        void down();
        void dbCopied(KJob *);
};

K_EXPORT_PLASMA_RUNNER(bookmarksrunner, BookmarksRunner)

#endif
