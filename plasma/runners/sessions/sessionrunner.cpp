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

#include "sessionrunner.h"

#include <QWidget>
#include <QAction>

#include <KAction>
#include <KActionCollection>
#include <KAuthorized>
#include <KDebug>
#include <KLocale>
#include <KMessageBox>

#include "kworkspace/kworkspace.h"

#include "screensaver_interface.h"

SessionRunner::SessionRunner(QObject *parent, const QVariantList &args)
    : Plasma::AbstractRunner( parent )
{
    Q_UNUSED(args)

    setObjectName(i18n("Sessions"));
    setPriority(LowPriority);
}

SessionRunner::~SessionRunner()
{
}

void SessionRunner::match(Plasma::SearchContext *search)
{
    const QString term = search->searchTerm();
    QString user;

    if (term.size() < 3) {
        return;
    }

    //TODO: ugh, magic strings.
    bool listAll = (term == "SESSIONS");

    if (!listAll && term.startsWith("switch", Qt::CaseInsensitive)) {
        // interestingly, this means that if one wants to switch to a
        // session named "switch", they'd have to enter
        // switch switch. ha!
        user = term.right(term.size() - 6).trimmed();

        if (user.isEmpty()) {
            return;
        }
    }

    //kDebug() << "session switching to" << (listAll ? "all sessions" : term);

    QList<Plasma::SearchMatch*> matches;
    bool switchUser = listAll ||
                      term.compare("switch user", Qt::CaseInsensitive) == 0 ||
                      term.compare("new session", Qt::CaseInsensitive) == 0;
    if (switchUser &&
        KAuthorized::authorizeKAction("start_new_session") &&
        dm.isSwitchable() &&
        dm.numReserve() >= 0) {
        Plasma::SearchMatch *action = new Plasma::SearchMatch(this);
        action->setType(Plasma::SearchMatch::ExactMatch);
        action->setIcon(KIcon("system-switch-user"));
        action->setText(i18n("New Session"));

        matches << action;
    }

    // now add the active sessions
    SessList sessions;
    if (!dm.localSessions(sessions)) {
        return;
    }

    foreach (const SessEnt& session, sessions) {
        if (!session.vt || session.self) {
            continue;
        }

        QString name = KDisplayManager::sess2Str(session);
        Plasma::SearchMatch* action = 0;

        if (listAll) {
            action = new Plasma::SearchMatch(this);
            action->setType(Plasma::SearchMatch::ExactMatch);
            action->setRelevance(1);
        } else if (name.compare(user, Qt::CaseInsensitive) == 0) {
            // we need an elif branch here because we don't
            // want the last conditional to be checked if !listAll
            action = new Plasma::SearchMatch(this);
            action->setType(Plasma::SearchMatch::ExactMatch);
            action->setRelevance(1);
        } else if (name.contains(user, Qt::CaseInsensitive)) {
            action = new Plasma::SearchMatch(this);
        }

        if (action) {
            matches << action;
            action->setIcon(KIcon("user-identity"));
            action->setText(name);
            action->setData(session.session);
        }
    }

    Plasma::SearchMatch *match = 0;
    if (term.compare("logout", Qt::CaseInsensitive) == 0 ||
        term.compare("log out", Qt::CaseInsensitive) == 0) {
        match = new Plasma::SearchMatch(this);
        match->setText(i18n("Logout"));
        match->setIcon(KIcon("system-log-out"));
        match->setData(LogoutAction);
    } else if (term.compare("restart", Qt::CaseInsensitive) == 0) {
        match = new Plasma::SearchMatch(this);
        match->setText(i18n("Restart the computer"));
        match->setIcon(KIcon("system-restart"));
        match->setData(RestartAction);
    } else if (term.compare("shutdown", Qt::CaseInsensitive) == 0){
        match = new Plasma::SearchMatch(this);
        match->setText(i18n("Shutdown the computer"));
        match->setIcon(KIcon("system-shutdown"));
        match->setData(ShutdownAction);
    } else if (term.compare("lock", Qt::CaseInsensitive) == 0){
        match = new Plasma::SearchMatch(this);
        match->setText(i18n("Lock the screen"));
        match->setIcon(KIcon("system-lock-screen"));
        match->setData(LockAction);
    }

    if (match) {
        match->setType(Plasma::SearchMatch::ExactMatch);
        match->setRelevance(0.9);
        matches << match;
    }

    search->addMatches(term, matches);
}

void SessionRunner::exec(const Plasma::SearchContext *search, const Plasma::SearchMatch *action)
{
    Q_UNUSED(search);
    if (action->data().type() == QVariant::Int) {
        KWorkSpace::ShutdownType type = KWorkSpace::ShutdownTypeDefault;

        switch (action->data().toInt()) {
            case LogoutAction:
                type = KWorkSpace::ShutdownTypeNone;
                break;
            case RestartAction:
                type = KWorkSpace::ShutdownTypeReboot;
                break;
            case ShutdownAction:
                type = KWorkSpace::ShutdownTypeHalt;
                break;
            case LockAction:
                lock();
                return;
                break;
        }

        if (type != KWorkSpace::ShutdownTypeDefault) {
            KWorkSpace::ShutdownConfirm confirm = KWorkSpace::ShutdownConfirmDefault;
            KWorkSpace::requestShutDown(confirm, type);
            return;
        }
    }

    if (!action->data().toString().isEmpty()) {
        QString sessionName = action->text();

        SessList sessions;
        if (dm.localSessions(sessions)) {
            foreach (SessEnt session, sessions) {
                if (sessionName == KDisplayManager::sess2Str(session)) {
                    dm.lockSwitchVT(session.vt);
                    break;
                }
            }
        }

        return;
    }

    //TODO: this message is too verbose and too technical.
    int result = KMessageBox::warningContinueCancel(
            0,
            i18n("<p>You have chosen to open another desktop session.<br />"
                "The current session will be hidden "
                "and a new login screen will be displayed.<br />"
                "An F-key is assigned to each session; "
                "F%1 is usually assigned to the first session, "
                "F%2 to the second session and so on. "
                "You can switch between sessions by pressing "
                "Ctrl, Alt and the appropriate F-key at the same time. "
                "Additionally, the KDE Panel and Desktop menus have "
                "actions for switching between sessions.</p>",
                7, 8),
            i18n("Warning - New Session"),
            KGuiItem(i18n("&Start New Session"), "fork"),
            KStandardGuiItem::cancel(),
            ":confirmNewSession",
            KMessageBox::PlainCaption | KMessageBox::Notify);

    if (result == KMessageBox::Cancel) {
        return;
    }

    lock();
    dm.startReserve();
}

void SessionRunner::lock()
{
    QString interface("org.freedesktop.ScreenSaver");
    org::freedesktop::ScreenSaver screensaver(interface, "/ScreenSaver",
                                              QDBusConnection::sessionBus());
    if (screensaver.isValid()) {
        screensaver.Lock();
    }
}

#include "sessionrunner.moc"
