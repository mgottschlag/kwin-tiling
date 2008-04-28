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

#ifndef SHELLRUNNER_H
#define SHELLRUNNER_H

#include <plasma/abstractrunner.h>

class QWidget;
class QAction;

/**
 * This class runs programs using the literal name of the binary, much as one
 * would use at a shell prompt.
 */
class ShellRunner : public Plasma::AbstractRunner
{
    Q_OBJECT

    public:
        ShellRunner(QObject *parent, const QVariantList &args);
        ~ShellRunner();

        void createMatchOptions(QWidget* parent);
        void match(Plasma::SearchContext *search);
        void run(const Plasma::SearchContext *context, const Plasma::SearchMatch *action);


    public slots:
        void setRunInTerminal(bool inTerminal);

    private:
        bool m_enabled;
        bool m_inTerminal;
};

K_EXPORT_PLASMA_RUNNER(shell, ShellRunner)

#endif
