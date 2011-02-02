/*
 *   Copyright (C) 2011 Aaron Seigo <aseigo@kde.org>
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

#include "activityrunner.h"

#include <KDebug>
#include <KIcon>
#include <KLocale>

ActivityRunner::ActivityRunner(QObject *parent, const QVariantList &args)
    : Plasma::AbstractRunner(parent, args),
      m_activities(0),
      m_keywordi18n(i18nc("KRunner keyword", "activity")),
      m_keyword("activity"),
      m_enabled(false)
{
    setObjectName(QLatin1String("Activities"));
    setIgnoredTypes(Plasma::RunnerContext::Directory | Plasma::RunnerContext::File |
                    Plasma::RunnerContext::NetworkLocation | Plasma::RunnerContext::Help);

    connect(this, SIGNAL(prepare()), this, SLOT(prep()));
    connect(this, SIGNAL(teardown()), this, SLOT(down()));

    serviceStatusChanged(KActivityConsumer::FullFunctionality);
}

void ActivityRunner::prep()
{
    if (!m_activities) {
        m_activities = new KActivityConsumer(this);
        connect(m_activities, SIGNAL(serviceStatusChanged(KActivityConsumer::ServiceStatus)),
                this, SLOT(serviceStatusChanged(KActivityConsumer::ServiceStatus)));
        serviceStatusChanged(m_activities->serviceStatus());
    }
}

void ActivityRunner::down()
{
    delete m_activities;
}

void ActivityRunner::serviceStatusChanged(KActivityConsumer::ServiceStatus status)
{
    const bool active = status != KActivityConsumer::NotRunning;
    if (m_enabled == active) {
        return;
    }

    m_enabled = active;
    QList<Plasma::RunnerSyntax> syntaxes;
    if (m_enabled) {
        syntaxes << Plasma::RunnerSyntax(m_keywordi18n, i18n("Lists all activities currently available to be run."))
                 << Plasma::RunnerSyntax(i18nc("KRunner keyword", "activity :q:"), i18n("Switches to activity :q:."));
    }
    setSyntaxes(syntaxes);
}

ActivityRunner::~ActivityRunner()
{
}

void ActivityRunner::match(Plasma::RunnerContext &context)
{
    if (!m_enabled) {
        return;
    }

    const QString term = context.query().trimmed();
    bool list = false;
    QString name;

    if (term.startsWith(m_keywordi18n, Qt::CaseInsensitive)) {
        if (term.size() == m_keywordi18n.size()) {
            list = true;
        } else {
            name = term.right(term.size() - m_keywordi18n.size()).trimmed();
            list = name.isEmpty();
        }
    } else if (term.startsWith(m_keyword, Qt::CaseInsensitive)) {
        if (term.size() == m_keyword.size()) {
            list = true;
        } else {
            name = term.right(term.size() - m_keyword.size()).trimmed();
            list = name.isEmpty();
        }
    } else {
        return;
    }

    QList<Plasma::QueryMatch> matches;
    QStringList activities = m_activities->listActivities();
    qSort(activities);

    if (!context.isValid()) {
        return;
    }

    if (list) {
        foreach (const QString &activity, activities) {
            addMatch(activity, matches);

            if (!context.isValid()) {
                return;
            }
        }
    } else {
        foreach (const QString &activity, activities) {
            if (activity.startsWith(name, Qt::CaseInsensitive)) {
                addMatch(activity, matches);
            }

            if (!context.isValid()) {
                return;
            }
        }
    }

    context.addMatches(context.query(), matches);
}

void ActivityRunner::addMatch(const QString &activity, QList<Plasma::QueryMatch> &matches)
{
    Plasma::QueryMatch match(this);
    KActivityInfo info(activity);
    match.setId("activity:" + activity);
    match.setType(Plasma::QueryMatch::ExactMatch);
    match.setIcon(info.icon().isEmpty() ? KIcon("preferences-activities") : KIcon(info.icon()));
    match.setText(i18n("Switch to \"%1\"", info.name()));
    match.setRelevance(0.7 + ((info.state() == KActivityInfo::Running ||
                               info.state() == KActivityInfo::Starting) ? 0.1 : 0));
    matches << match;
}

void ActivityRunner::run(const Plasma::RunnerContext &context, const Plasma::QueryMatch &match)
{
    if (!m_enabled) {
        return;
    }
}

#include "activityrunner.moc"
