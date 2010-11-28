/***************************************************************************
 *   Copyright 2008 by Dario Freddi <drf@kdemod.ath.cx>                    *
 *   Copyright 2008 by Sebastian Kügler <sebas@kde.org>                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#include "PowerDevilRunner.h"

#include <QDBusInterface>
#include <QDBusReply>
#include <QtDBus/QDBusConnectionInterface>

#include <KIcon>
#include <KLocale>
#include <KDebug>
#include <KDirWatch>
#include <KStandardDirs>
#include <KRun>

#include <Solid/PowerManagement>

PowerDevilRunner::PowerDevilRunner(QObject *parent, const QVariantList &args)
        : Plasma::AbstractRunner(parent),
          m_dbus(QDBusConnection::sessionBus()),
          m_shortestCommand(1000)
{
    Q_UNUSED(args)

    setObjectName( QLatin1String("PowerDevil" ));
    setIgnoredTypes(Plasma::RunnerContext::Directory | Plasma::RunnerContext::File |
                    Plasma::RunnerContext::NetworkLocation | Plasma::RunnerContext::Help);
    updateStatus();
    initUpdateTriggers();

    /* Let's define all the words here. m_words contains all the words that
     * will eventually trigger a match in the runner.
     */

    QStringList commands;
    commands << i18nc("Note this is a KRunner keyword", "power profile")
             << i18nc("Note this is a KRunner keyword", "suspend")
             << i18nc("Note this is a KRunner keyword", "sleep")
             << i18nc("Note this is a KRunner keyword", "hibernate")
             << i18nc("Note this is a KRunner keyword", "to disk")
             << i18nc("Note this is a KRunner keyword", "to ram")
             << i18nc("Note this is a KRunner keyword", "screen brightness")
             << i18nc("Note this is a KRunner keyword", "dim screen");

    foreach (const QString &command, commands) {
        if (command.length() < m_shortestCommand) {
            m_shortestCommand = command.length();
        }
    }
}

void PowerDevilRunner::updateSyntaxes()
{
    QList<Plasma::RunnerSyntax> syntaxes;
    syntaxes.append(Plasma::RunnerSyntax(i18nc("Note this is a KRunner keyword", "power profile"),
                     i18n("Lists all power profiles and allows them to be activated")));
    syntaxes.append(Plasma::RunnerSyntax(i18nc("Note this is a KRunner keyword", "suspend"),
                     i18n("Lists system suspend (e.g. sleep, hibernate) options "
                          "and allows them to be activated")));

    QSet< Solid::PowerManagement::SleepState > states = Solid::PowerManagement::supportedSleepStates();

    if (states.contains(Solid::PowerManagement::SuspendState)) {
        Plasma::RunnerSyntax sleepSyntax(i18nc("Note this is a KRunner keyword", "sleep"),
                                         i18n("Suspends the system to RAM"));
        sleepSyntax.addExampleQuery(i18nc("Note this is a KRunner keyword", "to ram"));
        syntaxes.append(sleepSyntax);
    }

    if (states.contains(Solid::PowerManagement::HibernateState)) {
        Plasma::RunnerSyntax hibernateSyntax(i18nc("Note this is a KRunner keyword", "hibernate"),
                                         i18n("Suspends the system to disk"));
        hibernateSyntax.addExampleQuery(i18nc("Note this is a KRunner keyword", "to disk"));
        syntaxes.append(hibernateSyntax);
    }

    Plasma::RunnerSyntax brightnessSyntax(i18nc("Note this is a KRunner keyword", "screen brightness"),
                            // xgettext:no-c-format
                            i18n("Lists screen brightness options or sets it to the brightness defined by :q:; "
                                 "e.g. screen brightness 50 would dim the screen to 50% maximum brightness"));
    brightnessSyntax.addExampleQuery(i18nc("Note this is a KRunner keyword", "dim screen"));
    syntaxes.append(brightnessSyntax);
    setSyntaxes(syntaxes);
}

PowerDevilRunner::~PowerDevilRunner()
{
}

void PowerDevilRunner::initUpdateTriggers()
{
    // Also receive updates triggered through the DBus
    if (m_dbus.interface()->isServiceRegistered("org.kde.Solid.PowerManagement")) {
        if (!m_dbus.connect("org.kde.Solid.PowerManagement",
                            "/org/kde/Solid/PowerManagement",
                            "org.kde.Solid.PowerManagement",
                            "profileChanged", this, SLOT(updateStatus()))) {
            kDebug() << "error!";
        }
        if (!m_dbus.connect("org.kde.Solid.PowerManagement",
                            "/org/kde/Solid/PowerManagement",
                            "org.kde.Solid.PowerManagement",
                            "configurationReloaded", this, SLOT(updateStatus()))) {
            kDebug() << "error!";
        }
    }
}

void PowerDevilRunner::updateStatus()
{
    // Profiles and their icons
    {
        KSharedConfigPtr profilesConfig = KSharedConfig::openConfig("powerdevil2profilesrc", KConfig::SimpleConfig);
        m_availableProfiles = profilesConfig->groupList();
        foreach(const QString &profile, m_availableProfiles) {
            KConfigGroup settings(profilesConfig, profile);
            if (settings.readEntry< QString >("icon", QString()).isEmpty()) {
                m_profileIcon[profile] = "preferences-system-power-management";
            } else {
                m_profileIcon[profile] = settings.readEntry< QString >("icon", QString());
            }
        }
    }

    updateSyntaxes();
}


bool PowerDevilRunner::parseQuery(const QString& query, const QList<QRegExp>& rxList, QString& parameter) const
{
    foreach (const QRegExp& rx, rxList) {
        if (rx.exactMatch(query)) {
             parameter = rx.cap(1).trimmed();
             return true;
        }
    }
    return false;
}

void PowerDevilRunner::match(Plasma::RunnerContext &context)
{
    const QString term = context.query();
    if (term.length() < m_shortestCommand) {
        return;
    }

    QList<Plasma::QueryMatch> matches;

    QString parameter;

    if (parseQuery(term,
                   QList<QRegExp>() << QRegExp(i18nc("Note this is a KRunner keyword; %1 is a parameter", "power profile %1", "(.*)"), Qt::CaseInsensitive)
                                    << QRegExp(i18nc("Note this is a KRunner keyword", "power profile"), Qt::CaseInsensitive),
                   parameter)) {
        foreach(const QString &profile, m_availableProfiles) {
            if (!parameter.isEmpty()) {
                if (!profile.startsWith(parameter, Qt::CaseInsensitive)) {
                    continue;
                }
            }
            Plasma::QueryMatch match(this);
            match.setType(Plasma::QueryMatch::ExactMatch);
            match.setIcon(KIcon(m_profileIcon[profile]));
            match.setText(i18n("Set Profile to '%1'", profile));
            match.setData(profile);
            match.setRelevance(1);
            match.setId("ProfileChange "+profile);
            matches.append(match);
        }
    } else if (parseQuery(term,
                          QList<QRegExp>() << QRegExp(i18nc("Note this is a KRunner keyword; %1 is a parameter", "screen brightness %1", "(.*)"), Qt::CaseInsensitive)
                                           << QRegExp(i18nc("Note this is a KRunner keyword", "screen brightness"), Qt::CaseInsensitive)
                                           << QRegExp(i18nc("Note this is a KRunner keyword; %1 is a parameter", "dim screen %1", "(.*)"), Qt::CaseInsensitive)
                                           << QRegExp(i18nc("Note this is a KRunner keyword", "dim screen"), Qt::CaseInsensitive),
                          parameter)) {
        if (!parameter.isEmpty()) {
            bool test;
            int b = parameter.toInt(&test);
            if (test) {
                int brightness = qBound(0, b, 100);
                Plasma::QueryMatch match(this);
                match.setType(Plasma::QueryMatch::ExactMatch);
                match.setIcon(KIcon("preferences-system-power-management"));
                match.setText(i18n("Set Brightness to %1", brightness));
                match.setData(brightness);
                match.setRelevance(1);
                match.setId("BrightnessChange");
                matches.append(match);
            }
        } else {
            Plasma::QueryMatch match1(this);
            match1.setType(Plasma::QueryMatch::ExactMatch);
            match1.setIcon(KIcon("preferences-system-power-management"));
            match1.setText(i18n("Dim screen totally"));
            match1.setRelevance(1);
            match1.setId("DimTotal");
            matches.append(match1);

            Plasma::QueryMatch match2(this);
            match2.setType(Plasma::QueryMatch::ExactMatch);
            match2.setIcon(KIcon("preferences-system-power-management"));
            match2.setText(i18n("Dim screen by half"));
            match2.setRelevance(1);
            match2.setId("DimHalf");
            matches.append(match2);

            Plasma::QueryMatch match3(this);
            match3.setType(Plasma::QueryMatch::ExactMatch);
            match3.setIcon(KIcon("video-display"));
            match3.setText(i18n("Turn off screen"));
            match3.setRelevance(1);
            match3.setId("TurnOffScreen");
            matches.append(match3);
        }
    } else if (term.startsWith(i18nc("Note this is a KRunner keyword", "suspend"))) {
        QSet< Solid::PowerManagement::SleepState > states = Solid::PowerManagement::supportedSleepStates();

        if (states.contains(Solid::PowerManagement::SuspendState)) {
            addSuspendMatch(Solid::PowerManagement::SuspendState, matches);
        }

        if (states.contains(Solid::PowerManagement::HibernateState)) {
            addSuspendMatch(Solid::PowerManagement::HibernateState, matches);
        }
    } else if (term.startsWith(i18nc("Note this is a KRunner keyword", "sleep")) ||
               term.startsWith(i18nc("Note this is a KRunner keyword", "to ram"))) {
        addSuspendMatch(Solid::PowerManagement::SuspendState, matches);
    } else if (term.startsWith(i18nc("Note this is a KRunner keyword", "hibernate")) ||
               term.startsWith(i18nc("Note this is a KRunner keyword", "to disk"))) {
        addSuspendMatch(Solid::PowerManagement::HibernateState, matches);
    }

    if (!matches.isEmpty()) {
        context.addMatches(term, matches);
    }
}

void PowerDevilRunner::addSuspendMatch(int value, QList<Plasma::QueryMatch> &matches)
{
    Plasma::QueryMatch match(this);
    match.setType(Plasma::QueryMatch::ExactMatch);

    switch ((Solid::PowerManagement::SleepState)value) {
        case Solid::PowerManagement::SuspendState:
        case Solid::PowerManagement::StandbyState:
            match.setIcon(KIcon("system-suspend"));
            match.setText(i18n("Suspend to RAM"));
            break;
        case Solid::PowerManagement::HibernateState:
            match.setIcon(KIcon("system-suspend-hibernate"));
            match.setText(i18n("Suspend to Disk"));
            break;
    }

    match.setData(value);
    match.setRelevance(1);
    match.setId("Suspend");
    matches.append(match);
}

void PowerDevilRunner::run(const Plasma::RunnerContext &context, const Plasma::QueryMatch &match)
{
    Q_UNUSED(context)

    QDBusInterface iface("org.kde.Solid.PowerManagement",
                         "/org/kde/Solid/PowerManagement",
                         "org.kde.Solid.PowerManagement", m_dbus);
    if (match.id().startsWith("PowerDevil_ProfileChange")) {
        iface.asyncCall("loadProfile", match.data().toString());
    } else if (match.id() == "PowerDevil_BrightnessChange") {
        iface.asyncCall("setBrightness", match.data().toInt());
    } else if (match.id() == "PowerDevil_DimTotal") {
        iface.asyncCall("setBrightness", 0);
    } else if (match.id() == "PowerDevil_DimHalf") {
        iface.asyncCall("setBrightness", -2);
    } else if (match.id() == "PowerDevil_TurnOffScreen") {
        // FIXME: Maybe this should be even removed
        // iface.asyncCall("turnOffScreen");
    } else if (match.id().startsWith("PowerDevil_Suspend")) {
        switch ((Solid::PowerManagement::SleepState)match.data().toInt()) {
            case Solid::PowerManagement::SuspendState:
            case Solid::PowerManagement::StandbyState:
                iface.asyncCall("suspendToRam");
                break;
            case Solid::PowerManagement::HibernateState:
                iface.asyncCall("suspendToDisk");
                break;
        }
    }
}

#include "PowerDevilRunner.moc"
