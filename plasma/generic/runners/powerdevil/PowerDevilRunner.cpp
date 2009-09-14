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

#include <KIcon>
#include <KLocale>
#include <KDebug>
#include <KDirWatch>
#include <KStandardDirs>
#include <KRun>

#include <solid/control/powermanager.h>

PowerDevilRunner::PowerDevilRunner(QObject *parent, const QVariantList &args)
        : Plasma::AbstractRunner(parent),
          m_dbus(QDBusConnection::sessionBus()),
          m_shortestCommand(1000)
{
    Q_UNUSED(args)

    setObjectName("PowerDevil");
    setIgnoredTypes(Plasma::RunnerContext::Directory | Plasma::RunnerContext::File |
                    Plasma::RunnerContext::NetworkLocation | Plasma::RunnerContext::Help);
    updateStatus();
    initUpdateTriggers();

    /* Let's define all the words here. m_words contains all the words that
     * will eventually trigger a match in the runner.
     */

    QStringList commands;
    commands << i18nc("Note this is a KRunner keyword", "power profile")
             << i18nc("Note this is a KRunner keyword", "power profile")
             << i18nc("Note this is a KRunner keyword", "suspend")
             << i18nc("Note this is a KRunner keyword", "sleep")
             << i18nc("Note this is a KRunner keyword", "hibernate")
             << i18nc("Note this is a KRunner keyword", "to disk")
             << i18nc("Note this is a KRunner keyword", "to ram")
             << i18nc("Note this is a KRunner keyword", "cpu policy")
             << i18nc("Note this is a KRunner keyword", "power governor")
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
    syntaxes.append(Plasma::RunnerSyntax(i18nc("Note this is a KRunner keyword", "power profile"),
                     i18n("Lists all power saving schemes and allows them to be activated")));
    syntaxes.append(Plasma::RunnerSyntax(i18nc("Note this is a KRunner keyword", "suspend"),
                     i18n("Lists system suspend (e.g. sleep, hibernate) options "
                          "and allows them to be activated")));

    if (m_synonyms.contains("sleep")) {
        Plasma::RunnerSyntax sleepSyntax(i18nc("Note this is a KRunner keyword", "sleep"),
                                         i18n("Suspends the system to RAM"));
        sleepSyntax.addExampleQuery(i18nc("Note this is a KRunner keyword", "to ram"));
        syntaxes.append(sleepSyntax);
    }

    if (m_synonyms.contains("hibernate")) {
        Plasma::RunnerSyntax hibernateSyntax(i18nc("Note this is a KRunner keyword", "hibernate"),
                                         i18n("Suspends the system to disk"));
        hibernateSyntax.addExampleQuery(i18nc("Note this is a KRunner keyword", "to disk"));
        syntaxes.append(hibernateSyntax);
    }

    Plasma::RunnerSyntax cpuFreqSyntax(i18nc("Note this is a KRunner keyword", "cpu policy"),
                         i18n("Lists all CPU frequency scaling policies and allows them to be activated"));
    cpuFreqSyntax.addExampleQuery(i18nc("Note this is a KRunner keyword", "power governor"));
    syntaxes.append(cpuFreqSyntax);

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

    // listen for changes to the profiles
    KDirWatch *profilesWatch = new KDirWatch(this);
    profilesWatch->addFile(KStandardDirs::locate("config", "powerdevilprofilesrc"));
    connect(profilesWatch,SIGNAL(dirty(QString)),this,SLOT(updateStatus()));
    connect(profilesWatch,SIGNAL(created(QString)),this,SLOT(updateStatus()));
    connect(profilesWatch,SIGNAL(deleted(QString)),this,SLOT(updateStatus()));

    // Also receive updates triggered through the DBus
    QStringList modules;
    QDBusInterface kdedInterface("org.kde.kded", "/kded", "org.kde.kded");
    QDBusReply<QStringList> reply = kdedInterface.call("loadedModules");

    if (!reply.isValid()) {
        return;
    }

    modules = reply.value();

    if (modules.contains("powerdevil")) {
        if (!m_dbus.connect("org.kde.kded", "/modules/powerdevil", "org.kde.PowerDevil",
                          "profileChanged", this, SLOT(updateStatus()))) {
            kDebug() << "error!";
        }
        if (!m_dbus.connect("org.kde.kded", "/modules/powerdevil", "org.kde.PowerDevil",
                          "stateChanged", this, SLOT(updateStatus()))) {
            kDebug() << "error!";
        }

        QDBusMessage msg = QDBusMessage::createMethodCall("org.kde.kded", "/modules/powerdevil",
                           "org.kde.PowerDevil", "streamData");
        m_dbus.call(msg);
    }
}

void PowerDevilRunner::updateStatus()
{
    // Governors
    {
        QDBusMessage msg = QDBusMessage::createMethodCall("org.kde.kded",
                            "/modules/powerdevil", "org.kde.PowerDevil", "getSupportedGovernors");
        QDBusReply<QVariantMap> govs = m_dbus.call(msg);
        m_supportedGovernors = govs.value().keys();
        foreach(const QString &governor, m_supportedGovernors) {
            m_governorData[governor] = govs.value()[governor].toInt();
        }
    }

    // Profiles and their icons
    {
        KConfig *profilesConfig = new KConfig("powerdevilprofilesrc", KConfig::SimpleConfig);
        m_availableProfiles = profilesConfig->groupList();
        foreach(const QString &profile, m_availableProfiles) {
            KConfigGroup *settings = new KConfigGroup(profilesConfig, profile);
            if (settings->readEntry("iconname").isEmpty()) {
                m_profileIcon[profile] = "preferences-system-power-management";
            } else {
                m_profileIcon[profile] = settings->readEntry("iconname");
            }
            delete settings;
        }
        delete profilesConfig;
    }

    // Schemes
    {
        QDBusMessage msg = QDBusMessage::createMethodCall("org.kde.kded",
                            "/modules/powerdevil", "org.kde.PowerDevil", "getSupportedSchemes");
        QDBusReply<QStringList> schemes = m_dbus.call(msg);
        m_supportedSchemes = schemes.value();
    }

    // Suspend
    {
        QDBusMessage msg = QDBusMessage::createMethodCall("org.kde.kded",
                                "/modules/powerdevil", "org.kde.PowerDevil", "getSupportedSuspendMethods");
        QDBusReply<QVariantMap> methods = m_dbus.call(msg);
        QMapIterator<QString, QVariant> it(methods);
        m_suspendMethods.clear();
        m_synonyms.clear();
        while (it.hasNext()) {
            it.next();
            const int value = it.value().toInt();
            m_suspendMethods[value] = it.key();
            if (it.key() == i18n("Suspend to RAM")) {
                m_synonyms[i18nc("Note this is a KRunner keyword", "sleep")] = value;
                m_synonyms[i18nc("Note this is a KRunner keyword", "to ram")] = value;
            } else  if (it.key() == i18n("Suspend to Disk")) {
                m_synonyms[i18nc("Note this is a KRunner keyword", "hibernate")] = value;
                m_synonyms[i18nc("Note this is a KRunner keyword", "to disk")] = value;
            }
        }
    }

    updateSyntaxes();
}

void PowerDevilRunner::match(Plasma::RunnerContext &context)
{
    const QString term = context.query().toLower();
    if (term.length() < m_shortestCommand) {
        return;
    }

    QList<Plasma::QueryMatch> matches;

    if (term.startsWith(i18nc("Note this is a KRunner keyword", "power profile"))) {
        foreach(const QString &profile, m_availableProfiles) {
            const QStringList strings = term.split(' ');
            if (strings.count() == 3) {
                if (!profile.startsWith(strings.at(2))) {
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
    } else if (term.startsWith(i18nc("Note this is a KRunner keyword", "cpu policy")) ||
               term.startsWith(i18nc("Note this is a KRunner keyword", "power governor"))) {
        foreach(const QString &ent, m_supportedGovernors) {
            const QStringList strings = term.split(' ');
            if (strings.count() == 3) {
                if (!ent.startsWith(strings.at(2))) {
                    continue;
                }
            }
            Plasma::QueryMatch match(this);
            match.setType(Plasma::QueryMatch::ExactMatch);

            switch (m_governorData[ent]) {
                case (int) Solid::Control::PowerManager::Performance:
                    match.setIcon(KIcon("preferences-system-performance"));
                    break;
                case (int) Solid::Control::PowerManager::OnDemand:
                    match.setIcon(KIcon("system-switch-user"));
                    break;
                case (int) Solid::Control::PowerManager::Conservative:
                    match.setIcon(KIcon("user-invisible"));
                    break;
                case (int) Solid::Control::PowerManager::Powersave:
                    match.setIcon(KIcon("preferences-system-power-management"));
                    break;
                case (int) Solid::Control::PowerManager::Userspace:
                    match.setIcon(KIcon("kuser"));
                    break;
                default:
                    match.setIcon(KIcon("preferences-system-power-management"));
                    break;
            }

            match.setText(i18n("Set CPU frequency scaling policy to '%1'", ent));
            match.setData(m_governorData[ent]);
            match.setRelevance(1);
            match.setId("GovernorChange "+ent);
            matches.append(match);
        }
    } else if (term.startsWith(i18nc("Note this is a KRunner keyword", "power scheme"))) {
        foreach(const QString &ent, m_supportedSchemes) {
            const QStringList strings = term.split(' ');
            if (strings.count() == 3) {
                if (!ent.startsWith(strings.at(2))) {
                    continue;
                }
            }

            Plasma::QueryMatch match(this);

            match.setType(Plasma::QueryMatch::ExactMatch);

            match.setIcon(KIcon("preferences-system-power-management"));
            match.setText(i18n("Set Powersaving Scheme to '%1'", ent));
            match.setData(ent);

            match.setRelevance(1);
            match.setId("SchemeChange "+ent);
            matches.append(match);
        }
    } else if (term.startsWith(i18nc("Note this is a KRunner keyword", "screen brightness")) ||
               term.startsWith(i18nc("Note this is a KRunner keyword", "dim screen"))) {
        const QStringList strings = term.split(' ');
        if (strings.count() == 3) {
            bool test;
            int b = strings.at(2).toInt(&test);
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
        } else if (strings.count() == 2) {
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
        QHashIterator<int, QString> it(m_suspendMethods);
        while (it.hasNext()) {
            it.next();
            addSuspendMatch(it.key(), matches);
        }
    } else if (m_synonyms.contains(term)) {
        addSuspendMatch(m_synonyms.value(term), matches);
    }

    if (!matches.isEmpty()) {
        context.addMatches(term, matches);
    }
}

void PowerDevilRunner::addSuspendMatch(int value, QList<Plasma::QueryMatch> &matches)
{
    Plasma::QueryMatch match(this);
    match.setType(Plasma::QueryMatch::ExactMatch);

    switch (value) {
        case 1:
        case 2:
            match.setIcon(KIcon("system-suspend"));
            break;
        case 4:
            match.setIcon(KIcon("system-suspend-hibernate"));
            break;
        default:
            match.setIcon(KIcon("preferences-system-power-management"));
            break;
    }

    match.setText(m_suspendMethods[value]);
    match.setData(value);
    match.setRelevance(1);
    match.setId("Suspend " + m_suspendMethods[value]);
    matches.append(match);
}

void PowerDevilRunner::run(const Plasma::RunnerContext &context, const Plasma::QueryMatch &match)
{
    Q_UNUSED(context)

        QDBusInterface iface("org.kde.kded", "/modules/powerdevil", "org.kde.PowerDevil", m_dbus);
    if (match.id().startsWith("PowerDevil_ProfileChange")) {
        iface.call("setProfile", match.data().toString());
    } else if (match.id().startsWith("PowerDevil_GovernorChange")) {
        iface.call("setGovernor", match.data().toInt());
    } else if (match.id().startsWith("PowerDevil_SchemeChange")) { 
        iface.call("setPowersavingScheme", match.data().toString());
    } else if (match.id() == "PowerDevil_BrightnessChange") {
        iface.call("setBrightness", match.data().toInt());
    } else if (match.id() == "PowerDevil_DimTotal") {
        iface.call("setBrightness", 0);
    } else if (match.id() == "PowerDevil_DimHalf") {
        iface.call("setBrightness", -2);
    } else if (match.id() == "PowerDevil_TurnOffScreen") {
        iface.call("turnOffScreen");
    } else if (match.id().startsWith("PowerDevil_Suspend")) {
        iface.call("suspend", match.data().toInt());
    }
}

#include "PowerDevilRunner.moc"
