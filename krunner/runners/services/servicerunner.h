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

#ifndef SERVICERUNNER_H
#define SERVICERUNNER_H

#include <KService>

#include "abstractrunner.h"

class QWidget;
class QAction;

/**
 * This class looks for matches in the set of .desktop files installed by
 * applications. This way the user can type exactly what they see in the
 * appications menu and have it start the appropriate app. Essentially anything
 * that KService knows about, this runner can launch
 */
class ServiceRunner : public Plasma::AbstractRunner
{
    Q_OBJECT

    public:
        explicit ServiceRunner(QObject* parent = 0);
        ~ServiceRunner();

        bool hasOptions();
        QWidget* options();

    protected:
        QAction* accepts(const QString& term);
        void fillMatches( KActionCollection* matches,
                          const QString& term,
                          int max, int offset );
        bool exec(const QString& command);

    private:
        QString formattedName( KService::Ptr );
        QWidget* m_options;
};

#endif
