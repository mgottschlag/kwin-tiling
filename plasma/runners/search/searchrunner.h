/*
 *   Copyright (C) 2006 Aaron Seigo <aseigo@kde.org>
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

#ifndef SEARCHRUNNER_H
#define SEARCHRUNNER_H

#include <KGenericFactory>

#include <plasma/abstractrunner.h>
#include <strigi/qtdbus/strigiclient.h>
#include <QtGui/QAction>

/**
 * This class runs the entered text through a search engine and returns the set
 * of results returned as possible programs, files, actions, etc to launch.
 */
class SearchRunner : public Plasma::AbstractRunner
{
    Q_OBJECT

    private:
        StrigiClient strigiclient;
        QString lastTerm;

    public:
        SearchRunner( QObject* parent, const QVariantList &args );
        ~SearchRunner();

        void exec(Plasma::SearchAction *action);
        virtual void match(Plasma::SearchContext *search);
};

K_EXPORT_PLASMA_RUNNER(searchrunner, SearchRunner)

#endif
