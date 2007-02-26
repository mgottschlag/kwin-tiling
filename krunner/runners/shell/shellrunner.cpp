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

#include <QWidget>

#include <KLocale>
#include <KRun>
#include <KStandardDirs>

#include "shellrunner.h"

ShellRunner::ShellRunner(QObject* parent)
    : Runner(parent),
      m_options(0)
{
    setObjectName( i18n( "Command" ) );
}

ShellRunner::~ShellRunner()
{
    delete m_options;
}

bool ShellRunner::accepts(const QString& term)
{
    return !KStandardDirs::findExe(term).isEmpty();
}

bool ShellRunner::hasOptions()
{
    return true;
}

QWidget* ShellRunner::options()
{
    if (!m_options)
    {
        // create options here
        m_options = new QWidget;
    }

    return m_options;
}

bool ShellRunner::exec(const QString& command)
{
    return (KRun::runCommand(command) != 0);
}

#include "shellrunner.moc"
