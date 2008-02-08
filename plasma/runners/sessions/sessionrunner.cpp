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
#include <KLocale>
#include <KMessageBox>

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
    //TODO: ugh, magic strings.
    QString term = search->searchTerm();
    bool listAll = (term == "SESSIONS");


    if (listAll &&
        KAuthorized::authorizeKAction("start_new_session") &&
        dm.isSwitchable() &&
        dm.numReserve() >= 0) {
        Plasma::SearchMatch *action = search->addExactMatch(this);
        action->setIcon(KIcon("system-switch-user"));
        action->setText(i18n("New Session"));
    }

    // now add the active sessions
    SessList sessions;
    if (!dm.localSessions(sessions)) {
        return;
    }

    QList<Plasma::SearchMatch*> exact;
    QList<Plasma::SearchMatch*> possible;
    QList<Plasma::SearchMatch*> info;
    foreach (const SessEnt& session, sessions) {
        if (!session.vt || session.self) {
            continue;
        }

        QString name = DM::sess2Str(session);
        Plasma::SearchMatch* action = 0;

        if (listAll) {
            action = new Plasma::SearchMatch(search, this);
            exact.append(action);
        } else if (name == term) {
            // we need an elif branch here because we don't
            // want the last conditional to be checked if !listAll
            action = new Plasma::SearchMatch(search, this);
            exact.append(action);
        } else if (name.contains(term, Qt::CaseInsensitive)) {
            action = new Plasma::SearchMatch(search, this);
            possible.append(action);
        }

        if (action) {
            action->setIcon(KIcon("user-identity"));
            action->setText(name);
            action->setData(session.session);
        }
    }

    search->addMatches(term, exact, possible, info);
}

void SessionRunner::exec(Plasma::SearchMatch * action)
{
    if (!action->data().toString().isEmpty()) {
        QString sessionName = action->text();

        SessList sessions;
        if (dm.localSessions(sessions)) {
            foreach (SessEnt session, sessions) {
                if (sessionName == DM::sess2Str(session)) {
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

    QString interface("org.freedesktop.ScreenSaver");
    org::freedesktop::ScreenSaver screensaver(interface, "/ScreenSaver",
                                              QDBusConnection::sessionBus());
    if (screensaver.isValid()) {
        screensaver.Lock();
    }

    DM().startReserve();
}

#include "sessionrunner.moc"
