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

#include "runner.h"

class QWidget;
class QAction;

/**
 * This class runs programs using the literal name of the binary, much as one
 * would use at a shell prompt.
 */
class ShellRunner : public Runner
{
    Q_OBJECT

    typedef QList<Runner*> List;

    public:
        explicit ShellRunner(QObject* parent = 0);
        ~ShellRunner();

        bool hasOptions();
        QWidget* options();

    protected:
        QAction* accepts(const QString& term);
        bool exec(const QString& command);

    private:
        QWidget* m_options;
};

#endif
