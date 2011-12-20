/* This file is part of the Nepomuk Project
   Copyright (c) 2008 Sebastian Trueg <trueg@kde.org>

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

#ifndef _NEPOMUK_SEARCH_RUNNER_H_
#define _NEPOMUK_SEARCH_RUNNER_H_

#include <Plasma/AbstractRunner>

#include <QtCore/QMutex>
#include <QtCore/QWaitCondition>
#include <QtCore/QList>

#include <Nepomuk/Query/Result>

class KFileItemActions;

class QAction;

namespace Nepomuk {

    class SearchRunner : public Plasma::AbstractRunner
    {
        Q_OBJECT

    public:
        SearchRunner( QObject* parent, const QVariantList& args );
        SearchRunner( QObject* parent, const QString& serviceId = QString() );
        ~SearchRunner();

        void match( Plasma::RunnerContext& context );
        void run( const Plasma::RunnerContext& context, const Plasma::QueryMatch& action );

        QList<QAction*> actionsForMatch(const Plasma::QueryMatch &match);

    protected Q_SLOTS:
        void init();
        QMimeData *mimeDataForMatch(const Plasma::QueryMatch *match);

    private:
        /**
         * Returns a list of all actions in the given QMenu
         * This method flattens the hierarchy of the menu by prefixing the
         * text of all actions in a submenu with the submenu title.
         *
         * @param menu the QMenu storing the actions
         * @param prefix text to display before the text of all actions in the menu
         * @param parent QObject to be passed as parent of all the actions in the list
         *
         * @since 4.4
         */
        QList<QAction*> actionsFromMenu(QMenu *menu, const QString &prefix = QString(), QObject *parent = 0);

        QMutex m_mutex;
        QWaitCondition m_waiter;

        KFileItemActions *m_actions;
        QList<QAction*> m_konqActions;
    };
}

#endif
