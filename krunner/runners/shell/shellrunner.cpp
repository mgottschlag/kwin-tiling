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

#include "shellrunner.h"

#include <QWidget>
#include <QAction>
#include <QPushButton>

#include <KAuthorized>
#include <KIcon>
#include <KLocale>
#include <KRun>
#include <KStandardDirs>

#include "ui_shellOptions.h"

ShellRunner::ShellRunner(QObject* parent)
    : Plasma::AbstractRunner(parent)
{
    setObjectName(i18n("Command"));
    setHasMatchOptions(true);
    setPriority(AbstractRunner::HighestPriority);
    m_enabled = KAuthorized::authorizeKAction("shell_access");
}

ShellRunner::~ShellRunner()
{
}

void ShellRunner::match(Plasma::SearchContext *search)
{
    if (!m_enabled) {
        return;
    }

    if (search->type() == Plasma::SearchContext::Executable ||
        search->type() == Plasma::SearchContext::ShellCommand)  {
        Plasma::SearchMatch* action = search->addExactMatch(this);
        action->setIcon(KIcon("system-run"));
        action->setText(i18n("Run %1", search->searchTerm()));
    }
}

void ShellRunner::createMatchOptions(QWidget* parent)
{
    Ui::shellOptions ui;
    ui.setupUi(parent);
}

void ShellRunner::exec(Plasma::SearchMatch* action)
{
    if (!m_enabled) {
        return;
    }
    KRun::runCommand(action->searchTerm(), NULL);
}

#include "shellrunner.moc"
