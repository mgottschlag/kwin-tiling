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
    setIgnoredTypes(Plasma::RunnerContext::Directory | Plasma::RunnerContext::File | 
                    Plasma::RunnerContext::NetworkLocation);
}

SessionRunner::~SessionRunner()
{
}


  
Plasma::QueryMatch* SessionRunner::matchCommands(const QString& term)
{
      Plasma::QueryMatch *match = 0;
      if (term.compare(i18nc("log out command","logout"), Qt::CaseInsensitive) == 0 ||
          term.compare(i18n("log out"), Qt::CaseInsensitive) == 0) {
          match = new Plasma::QueryMatch(this);
          match->setText(i18nc("log out command","Logout"));
          match->setIcon(KIcon("system-log-out"));
          match->setData(LogoutAction);
      } else if (term.compare(i18nc("restart computer command", "restart"), Qt::CaseInsensitive) == 0 ||
                 term.compare(i18nc("restart computer command", "reboot"), Qt::CaseInsensitive) == 0) {
          match = new Plasma::QueryMatch(this);
          match->setText(i18n("Restart the computer"));
          match->setIcon(KIcon("system-restart"));
          match->setData(RestartAction);
      } else if (term.compare(i18nc("shutdown computer command","shutdown"), Qt::CaseInsensitive) == 0) {
          match = new Plasma::QueryMatch(this);
          match->setText(i18n("Shutdown the computer"));
          match->setIcon(KIcon("system-shutdown"));
          match->setData(ShutdownAction);
      } else if (term.compare(i18nc("lock screen command","lock"), Qt::CaseInsensitive) == 0) {
          match = new Plasma::QueryMatch(this);
          match->setText(i18n("Lock the screen"));
          match->setIcon(KIcon("system-lock-screen"));
          match->setData(LockAction);
      }

      if (match) {
          match->setType(Plasma::QueryMatch::ExactMatch);
          match->setRelevance(0.9);
      }
      return match;
  }


void SessionRunner::match(Plasma::RunnerContext *search)
{
    const QString term = search->query();
    QString user;
    bool matchUser = false;

    QList<Plasma::QueryMatch*> matches;

    if (term.size() < 3) {
        return;
    }
    // first compare with SESSIONS. this must *NOT* be translated (i18n)
    // as it is used as an internal command trigger (e.g. via d-bus),
    // not as a user supplied query. and yes, "Ugh, magic strings"
    bool listAll = (term == "SESSIONS");

    if (!listAll) {
        //no luck, try switch user command
        if (term.startsWith(i18nc("switch user command","switch"), Qt::CaseInsensitive)) {
            // interestingly, this means that if one wants to switch to a
            // session named "switch", they'd have to enter
            // switch switch. ha!

            // we don't know the size of 'switch' translated to your language, do we?
            QStringList words = term.split(" ");
            int switchCmdSize = words.at(0).size();

            user = term.right(term.size() - switchCmdSize).trimmed();
            matchUser = true;
            // can't match anything below, since it's just "switch"
           if (matchUser && user.isEmpty()) {       
                return;
           }
        } else {
            // we know it's not SESSION or "switch <something>", so let's
            // try some other possibilities
            Plasma::QueryMatch *commandMatch;
            commandMatch = matchCommands(term);
            if (commandMatch) {
                matches << commandMatch;
            }
        }
    }

    //kDebug() << "session switching to" << (listAll ? "all sessions" : term);

    bool switchUser = listAll ||
                      term.compare(i18n("switch user"), Qt::CaseInsensitive) == 0 ||
                      term.compare(i18n("new session"), Qt::CaseInsensitive) == 0;

    if (switchUser &&
        KAuthorized::authorizeKAction("start_new_session") &&
        dm.isSwitchable() &&
        dm.numReserve() >= 0) {
        Plasma::QueryMatch *action = new Plasma::QueryMatch(this);
        action->setType(Plasma::QueryMatch::ExactMatch);
        action->setIcon(KIcon("system-switch-user"));
        action->setText(i18n("New Session"));

        matches << action;
    }

    // now add the active sessions
    if (listAll || matchUser) {
        SessList sessions;
        foreach (const SessEnt& session, sessions) {
            if (!session.vt || session.self) {
                continue;
            }

            QString name = KDisplayManager::sess2Str(session);
            Plasma::QueryMatch* match = 0;

            if (listAll) {
                match = new Plasma::QueryMatch(this);
                match->setType(Plasma::QueryMatch::ExactMatch);
                match->setRelevance(1);
            } else if (matchUser) {
                if (name.compare(user, Qt::CaseInsensitive) == 0) {
                    // we need an elif branch here because we don't
                    // want the last conditional to be checked if !listAll
                    match = new Plasma::QueryMatch(this);
                    match->setType(Plasma::QueryMatch::ExactMatch);
                    match->setRelevance(1);
                } else if (name.contains(user, Qt::CaseInsensitive)) {
                    match = new Plasma::QueryMatch(this);
                }
            }

            if (match) {
                matches << match;
                match->setIcon(KIcon("user-identity"));
                match->setText(name);
                match->setData(session.session);
            }
        }
    }
    search->addMatches(term, matches);
}

void SessionRunner::run(const Plasma::RunnerContext *search, const Plasma::QueryMatch *action)
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
            foreach (const SessEnt &session, sessions) {
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
