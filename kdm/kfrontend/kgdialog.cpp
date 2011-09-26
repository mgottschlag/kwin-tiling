/*

Base class for various kdm greeter dialogs

Copyright (C) 1997, 1998 Steffen Hansen <hansen@kde.org>
Copyright (C) 2000-2004 Oswald Buddenhagen <ossi@kde.org>


This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/

#include "kgdialog.h"
#include "kgverify.h"
#include "kconsole.h"
#include "kdmshutdown.h"
#include "kdm_greet.h"
#include "utils.h"

#include <klocale.h>

#include <QAction>
#include <QMenu>

KGDialog::KGDialog(bool themed) : inherited(0, !themed)
{
#ifdef WITH_KDM_XCONSOLE
    consoleView = _showLog ? new KConsole(this) : 0;
#endif

    optMenu = 0;
    verify = 0;
}

void
#ifdef XDMCP
KGDialog::completeMenu(int _switchIf, int _switchCode, const QString &_switchMsg, int _switchAccel)
#else
KGDialog::completeMenu()
#endif
{
#ifdef HAVE_VTS
    if (_isLocal) {
        dpyMenu = new QMenu(this);
        inserten(i18n("Sw&itch User"), Qt::ALT + Qt::Key_I, dpyMenu);
        QAction *action = new QAction(this);
        action->setMenu(dpyMenu);
        action->setShortcut(Qt::ALT + Qt::CTRL + Qt::Key_Insert);
        connect(action, SIGNAL(triggered(bool)),
                SLOT(slotActivateMenu(bool)));
        addAction(action);
        connect(dpyMenu, SIGNAL(triggered(QAction*)),
                SLOT(slotDisplaySelected(QAction*)));
        connect(dpyMenu, SIGNAL(aboutToShow()),
                SLOT(slotPopulateDisplays()));
    }
#endif

    if (_allowClose || _isReserve)
        inserten(_isReserve ? i18n("Canc&el Session") :
                 _isLocal ? i18n("R&estart X Server") :
                            i18n("Clos&e Connection"),
                 Qt::ALT + Qt::Key_E, SLOT(slotExit()));

#ifdef XDMCP
    if (_isLocal && _loginMode != _switchIf) {
        switchCode = _switchCode;
        inserten(_switchMsg, _switchAccel, SLOT(slotSwitch()));
    }
#endif

    if (_hasConsole)
        inserten(i18n("Co&nsole Login"), Qt::ALT + Qt::Key_N, SLOT(slotConsole()));

    if (_allowShutdown != SHUT_NONE) {
        inserten(i18n("&Shutdown..."),  Qt::ALT + Qt::Key_S, SLOT(slotShutdown()));
        inserten(Qt::ALT + Qt::CTRL + Qt::Key_Delete, SLOT(slotShutdown()));
        inserten(Qt::SHIFT + Qt::ALT + Qt::CTRL + Qt::Key_PageUp, SLOT(slotShutdown()), SHUT_REBOOT);
        inserten(Qt::SHIFT + Qt::ALT + Qt::CTRL + Qt::Key_PageDown, SLOT(slotShutdown()), SHUT_HALT);
    }
}

void
KGDialog::ensureMenu()
{
    if (!optMenu) {
        optMenu = new QMenu(this);
        connect(optMenu, SIGNAL(triggered(QAction*)), SLOT(slotActivateMenu(QAction*)));
        needSep = false;
    } else if (needSep) {
        optMenu->addSeparator();
        needSep = false;
    }
}

void
KGDialog::inserten(const QKeySequence &shortcut, const char *member, int data)
{
    QAction *action = new QAction(this);
    action->setShortcut(shortcut);
    if (data != -1)
        action->setData(data);
    connect(action, SIGNAL(triggered()), member);
    addAction(action);
}

void
KGDialog::inserten(const QString &txt, const QKeySequence &shortcut, const char *member)
{
    ensureMenu();
    optMenu->addAction(txt, this, member, shortcut);
}

void
KGDialog::inserten(const QString &txt, const QKeySequence &shortcut, QMenu *cmnu)
{
    ensureMenu();
    QAction *action = optMenu->addMenu(cmnu);
    action->setShortcut(shortcut);
    action->setText(txt);
}

void
KGDialog::slotActivateMenu(QAction *action)
{
    QMenu *cmnu = action->menu();
    if (cmnu) {
        QSize sh(cmnu->sizeHint() / 2);
        cmnu->exec(geometry().center() - QPoint(sh.width(), sh.height()));
    }
}

void
KGDialog::slotActivateMenu(bool)
{
    slotActivateMenu(static_cast<QAction *>(sender()));
}

void
KGDialog::slotExit()
{
    if (verify)
        verify->abort();
    ::exit(_isReserve ? EX_RESERVE : EX_RESERVER_DPY);
}

void
KGDialog::slotSwitch()
{
#ifdef XDMCP
    done(switchCode);
#endif
}

void
KGDialog::slotConsole()
{
    QList<DpySpec> sess;
#ifdef HAVE_VTS
    sess = fetchSessions(0);
#endif
    if (verify)
        verify->suspend();
    int ret = KDMConfShutdown(-1, sess, SHUT_CONSOLE, 0).exec();
    if (verify)
        verify->resume();
    if (!ret)
        return;
    if (verify)
        verify->abort();
    gSet(1);
    gSendInt(G_Console);
    gSet(0);
}

void
KGDialog::slotShutdown()
{
    if (verify)
        verify->suspend();
    QAction *action = (QAction *)sender();
    if (action->data().isNull()) {
        if (_scheduledSd == SHUT_ALWAYS)
            KDMShutdown::scheduleShutdown(this);
        else
            KDMSlimShutdown(this).exec();
    } else
        KDMSlimShutdown::externShutdown(action->data().toInt(), 0, -1, false);
    if (verify)
        verify->resume();
}

void
KGDialog::slotDisplaySelected(QAction *action)
{
#ifdef HAVE_VTS
    gSet(1);
    gSendInt(G_Activate);
    gSendInt(action->data().toInt());
    gSet(0);
#else
    (void)action;
#endif
}

void
KGDialog::slotPopulateDisplays()
{
#ifdef HAVE_VTS
    dpyMenu->clear();
    QList<DpySpec> sessions = fetchSessions(lstPassive | lstTTY);
    QString user, loc;
    foreach (const DpySpec &sess, sessions) {
        decodeSession(sess, user, loc);
        QAction *action = dpyMenu->addAction(
            i18nc("session (location)", "%1 (%2)", user, loc));
        action->setData(sess.vt ? sess.vt : -1);
        action->setCheckable(true);
        if (!sess.vt)
            action->setEnabled(false);
        if (sess.flags & isSelf)
            action->setChecked(true);
    }
#endif
}

#include "kgdialog.moc"
