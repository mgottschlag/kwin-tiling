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
#include <QAction>

#include <KAction>
#include <KActionCollection>
#include <KAuthorized>
#include <KLocale>
#include <KMessageBox>

#include <dmctl.h>

#include "krunnerapp.h"
#include "saverengine.h"
#include "sessionrunner.h"

SessionRunner::SessionRunner( QObject* parent )
    : Plasma::Runner( parent )
{
    setObjectName( i18n( "Sessions" ) );
}

SessionRunner::~SessionRunner()
{
}

QAction* SessionRunner::accepts(const QString& term)
{
    Q_UNUSED(term);
    return 0;
}

bool SessionRunner::exec(const QString& command)
{
    Q_UNUSED(command);
    return true;
}

void SessionRunner::fillMatches(KActionCollection* actions, const QString& term, int max, int offset)
{
    Q_UNUSED(max);
    Q_UNUSED(offset);
    //TODO: ugh, magic strings.
    if ( term != "SESSIONS") {
        return;
    }

    DM dm;

    if ( KAuthorized::authorizeKAction("start_new_session") &&
         dm.isSwitchable() &&
         dm.numReserve() >= 0 ) {
        QAction *action = actions->addAction( "newsession" );
        action->setIcon( KIcon("fork") );
        action->setText( i18n( "New Session" ) );
        connect( action, SIGNAL(triggered(bool)), SLOT(newSession()) );
    }

    // now add the active sessions
    SessList sessions;
    if ( !dm.localSessions( sessions ) ) {
        return;
    }

    foreach (const SessEnt& session, sessions) {
        if ( !session.vt || session.self ) {
            continue;
        }


        QAction* action = actions->addAction( DM::sess2Str( session ) );
        action->setIcon( KIcon( "user" ) );
        action->setText( DM::sess2Str( session ) );
    }
}

void SessionRunner::newSession()
{
    //TODO: this message is too verbose and too technical.
    int result = KMessageBox::warningContinueCancel(
            0,
            i18n("<p>You have chosen to open another desktop session.<br>"
                "The current session will be hidden "
                "and a new login screen will be displayed.<br>"
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
            ":confirmNewSession",
            KMessageBox::PlainCaption | KMessageBox::Notify);

    if ( result == KMessageBox::Cancel ) {
        return;
    }

    static_cast<KRunnerApp*>(qApp)->screensaver().Lock();
    DM().startReserve();
}

#include "sessionrunner.moc"
