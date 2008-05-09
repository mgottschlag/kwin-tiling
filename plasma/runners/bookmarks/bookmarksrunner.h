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

#include <KGenericFactory>
#include <KIcon>

#include <plasma/abstractrunner.h>

class QWidget;

class KBookmark;
class KBookmarkGroup;

class BookmarksRunner : public Plasma::AbstractRunner
{
    Q_OBJECT

    public:
        BookmarksRunner(QObject* parent, const QVariantList &args);
        ~BookmarksRunner();

        void match(Plasma::RunnerContext &context);
        void run(const Plasma::RunnerContext &context, const Plasma::QueryMatch &action);

    private:
        KIcon getFavicon(const KUrl &url);

    private:
        KIcon m_icon;
};

K_EXPORT_PLASMA_RUNNER(bookmarksrunner, BookmarksRunner)

#endif
