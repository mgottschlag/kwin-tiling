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
#include <KDebug>
#include <KIcon>
#include <KLocale>
#include <KRun>
#include <KStandardDirs>
#include <KToolInvocation>

#include "ui_shellOptions.h"

ShellRunner::ShellRunner(QObject *parent, const QVariantList &args)
    : Plasma::AbstractRunner(parent, args),
      m_inTerminal(false)
{
    Q_UNUSED(args)
    KGlobal::locale()->insertCatalog("krunner_shellrunner");

    setObjectName(i18n("Command"));
    setHasRunOptions(true);
    setPriority(AbstractRunner::HighestPriority);
    m_enabled = KAuthorized::authorizeKAction("shell_access");
    setIgnoredTypes(Plasma::RunnerContext::Directory | Plasma::RunnerContext::File | 
                    Plasma::RunnerContext::NetworkLocation | Plasma::RunnerContext::UnknownType | 
                    Plasma::RunnerContext::Help);
}

ShellRunner::~ShellRunner()
{
}

void ShellRunner::match(Plasma::RunnerContext &context)
{
    if (!m_enabled) {
        return;
    }

    if (context.type() == Plasma::RunnerContext::Executable ||
        context.type() == Plasma::RunnerContext::ShellCommand)  {
        const QString term = context.query();
        Plasma::QueryMatch match(this);
        match.setType(Plasma::QueryMatch::ExactMatch);
        match.setIcon(KIcon("system-run"));
        match.setText(i18n("Run %1", term));
        match.setRelevance(1);
        context.addMatch(term, match);
    }
}

void ShellRunner::createMatchOptions(QWidget* parent)
{
    m_inTerminal = false;
    Ui::shellOptions ui;
    ui.setupUi(parent);

    //TODO: connect rest of UI up too!
    connect(ui.cbRunInTerminal, SIGNAL(toggled(bool)), this, SLOT(setRunInTerminal(bool)));
}

void ShellRunner::setRunInTerminal(bool inTerminal)
{
    m_inTerminal = inTerminal;
}

void ShellRunner::run(const Plasma::RunnerContext &context, const Plasma::QueryMatch &match)
{
    QMutexLocker lock(bigLock());
    Q_UNUSED(match);
    if (!m_enabled) {
        return;
    }

    if (m_inTerminal) {
        KToolInvocation::invokeTerminal(context.query());

        // reset for the next run!
        m_inTerminal = false;
    } else {
        KRun::runCommand(context.query(), NULL);
    }
}

#include "shellrunner.moc"
