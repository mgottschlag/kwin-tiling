/*

Shell for kdm conversation plugins

Copyright (C) 1997, 1998, 2000 Steffen Hansen <hansen@kde.org>
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

#include <config-workspace.h>

#include "kgverify.h"
#include "kdmconfig.h"
#include "kdm_greet.h"

#include "themer/kdmthemer.h"
#include "themer/kdmitem.h"

#include <KColorScheme>
#include <kguiitem.h>
#include <klibrary.h>
#include <klocale.h>
#include <kpushbutton.h>
#include <krandom.h>
#include <kseparator.h>
#include <KStandardGuiItem>

#include <QAction>
#include <QApplication>
#include <QEvent>
#include <QKeyEvent>
#include <QLabel>
#include <QMenu>
#include <QSocketNotifier>
#include <QX11Info>

#include <X11/Xlib.h> // for updateLockStatus()
#include <fixx11h.h> // ... and make eventFilter() work again

#define FULL_GREET_TO 40 // normal inactivity timeout
#define TIMED_GREET_TO 20 // inactivity timeout when persisting timed login
#define MIN_TIMED_TO 5 // minimal timed login delay
#define DEAD_TIMED_TO 2 // <enter> dead time after re-activating timed login
#define SECONDS 1000 // reduce to 100 to speed up testing

void KGVerifyHandler::verifyClear()
{
}

void KGVerifyHandler::updateStatus(bool, bool, int)
{
}

KGVerify::KGVerify(KGVerifyHandler *_handler,
                   QWidget *_parent, QWidget *_predecessor,
                   const QString &_fixedUser,
                   const PluginList &_pluginList,
                   KGreeterPlugin::Function _func,
                   KGreeterPlugin::Context _ctx)
    : inherited()
    , coreState(CoreIdle)
    , fixedEntity(_fixedUser)
    , pluginList(_pluginList)
    , handler(_handler)
    , parent(_parent)
    , predecessor(_predecessor)
    , plugMenu(0)
    , curPlugin(-1)
    , timedLeft(0)
    , func(_func)
    , ctx(_ctx)
    , enabled(true)
    , running(false)
    , suspended(false)
    , failed(false)
    , isClear(true)
{
    sockNot = new QSocketNotifier(rfd, QSocketNotifier::Read, this);
    sockNot->setEnabled(false);
    connect(sockNot, SIGNAL(activated(int)), SLOT(handleVerify()));

    connect(&timer, SIGNAL(timeout()), SLOT(slotTimeout()));
    connect(qApp, SIGNAL(activity()), SLOT(slotActivity()));

    _parent->installEventFilter(this);
}

KGVerify::~KGVerify()
{
    debug("delete %s\n", pName.data());
    delete greet;
}

QMenu *
KGVerify::getPlugMenu()
{
    // assert(coreState != CoreBusy);
    if (!plugMenu) {
        uint np = pluginList.count();
        if (np > 1) {
            plugMenu = new QMenu(parent);
            QActionGroup *plugGroup = new QActionGroup(parent);
            connect(plugMenu, SIGNAL(triggered(QAction*)),
                    SLOT(slotPluginSelected(QAction*)));
            for (uint i = 0; i < np; i++) {
                int pid = pluginList[i];
                greetPlugins[pid].action = plugGroup->addAction(
                    i18nc("@item:inmenu authentication method",
                          greetPlugins[pid].info->name));
                greetPlugins[pid].action->setData(i);
                greetPlugins[pid].action->setCheckable(true);
            }
            plugMenu->addActions(plugGroup->actions());
        }
    }
    return plugMenu;
}

bool // public
KGVerify::entitiesLocal() const
{
    return greetPlugins[pluginList[curPlugin]].info->flags & KGreeterPluginInfo::Local;
}

bool // public
KGVerify::entitiesFielded() const
{
    return greetPlugins[pluginList[curPlugin]].info->flags & KGreeterPluginInfo::Fielded;
}

bool // public
KGVerify::entityPresettable() const
{
    return greetPlugins[pluginList[curPlugin]].info->flags & KGreeterPluginInfo::Presettable;
}

bool // public
KGVerify::isClassic() const
{
    return !strcmp(greetPlugins[pluginList[curPlugin]].info->method, "classic");
}

QString // public
KGVerify::pluginName() const
{
    QString name(greetPlugins[pluginList[curPlugin]].library->fileName());
    uint st = name.lastIndexOf('/') + 1;
    uint en = name.indexOf('.', st);
    if (en - st > 7 && QString::fromRawData(name.unicode() + st, 7) == QLatin1String("kgreet_"))
        st += 7;
    return name.mid(st, en - st);
}

static void
setTabOrder(QWidget *&pred, const QObjectList &list)
{
    foreach (QObject *o, list)
        if (QWidget *w = qobject_cast<QWidget *>(o)) {
            if (w->focusPolicy() & Qt::TabFocus) {
                QWidget::setTabOrder(pred, w);
                pred = w;
            } else {
                setTabOrder(pred, o->children());
            }
        }
}

void // public
KGVerify::selectPlugin(int id)
{
    if (pluginList.isEmpty()) {
        msgBox(errorbox, i18n("No greeter widget plugin loaded. Check the configuration."));
        ::exit(EX_UNMANAGE_DPY);
    }
    curPlugin = id;
    if (plugMenu)
        greetPlugins[pluginList[id]].action->setChecked(true);
    pName = ("greet_" + pluginName()).toLatin1();
    debug("new %s\n", pName.data());
    greet = greetPlugins[pluginList[id]].info->create(this, parent, fixedEntity, func, ctx);
    if (QWidget *pred = predecessor)
        setTabOrder(pred, *(const QObjectList *)&greet->getWidgets());
    timeable = _autoLoginDelay && entityPresettable() && isClassic();
}

void // private slot
KGVerify::slotPluginSelected(QAction *action)
{
    if (failed)
        return;
    int id = action->data().toInt();
    if (id != curPlugin) {
        parent->setUpdatesEnabled(false);
        debug("delete %s\n", pName.data());
        delete greet;
        selectPlugin(id);
        handler->verifyPluginChanged(id);
        if (running)
            start();
        parent->setUpdatesEnabled(true);
    }
}

void // public
KGVerify::loadUsers(const QStringList &users)
{
    debug("%s->loadUsers(...)\n", pName.data());
    greet->loadUsers(users);
}

void // public
KGVerify::presetEntity(const QString &entity, int field)
{
    presEnt = entity;
    presFld = field;
}

bool // private
KGVerify::applyPreset()
{
    if (!presEnt.isEmpty()) {
        debug("%s->presetEntity(%\"s, %d)\n", pName.data(),
              qPrintable(presEnt), presFld);
        greet->presetEntity(presEnt, presFld);
        if (entitiesLocal()) {
            curUser = presEnt;
            pamUser.clear();
            handler->verifySetUser(presEnt);
        }
        return true;
    }
    return false;
}

bool // private
KGVerify::scheduleAutoLogin(bool initial)
{
    if (timeable) {
        debug("%s->presetEntity(%\"s, -1)\n", pName.data(),
              qPrintable(_autoLoginUser), -1);
        greet->presetEntity(_autoLoginUser, -1);
        curUser = _autoLoginUser;
        handler->verifySetUser(_autoLoginUser);
        timer.start(1000);
        if (initial) {
            timedLeft = _autoLoginDelay;
            deadTicks = 0;
        } else {
            timedLeft = qMax(_autoLoginDelay - TIMED_GREET_TO, MIN_TIMED_TO);
            deadTicks = DEAD_TIMED_TO;
        }
        updateStatus();
        sockNot->setEnabled(false);
        running = false;
        isClear = true;
        return true;
    }
    return false;
}

void // private
KGVerify::performAutoLogin()
{
//    timer.stop();
    gSendInt(G_AutoLogin);
    coreState = CoreBusy;
    sockNot->setEnabled(true);
}

QString // public
KGVerify::getEntity() const
{
    debug("%s->getEntity()\n", pName.data());
    QString ent = greet->getEntity();
    debug("  entity: %s\n", qPrintable(ent));
    return ent;
}

void
KGVerify::setUser(const QString &user)
{
    // assert(fixedEntity.isEmpty());
    curUser = user;
    debug("%s->setUser(%\"s)\n", pName.data(), qPrintable(user));
    greet->setUser(user);
    talkerEdits();
}

void
KGVerify::start()
{
    authTok = (func == KGreeterPlugin::ChAuthTok);
    if (func == KGreeterPlugin::Authenticate && ctx == KGreeterPlugin::Login) {
        if (scheduleAutoLogin(true)) {
            if (!_autoLoginAgain)
                _autoLoginDelay = 0, timeable = false;
            return;
        } else {
            applyPreset();
        }
        if (_isReserve)
            timer.start(FULL_GREET_TO * SECONDS);
    }
    sockNot->setEnabled(true);
    running = true;
    if (!(func == KGreeterPlugin::Authenticate ||
          ctx == KGreeterPlugin::ChangeTok ||
          ctx == KGreeterPlugin::ExChangeTok))
    {
        coreState = CoreBusy;
    }
    debug("%s->start()\n", pName.data());
    greet->start();
}

void
KGVerify::abort()
{
    debug("%s->abort()\n", pName.data());
    greet->abort();
    sockNot->setEnabled(false);
    running = false;
}

void
KGVerify::suspend()
{
    // assert(coreState != CoreBusy);
    if (running) {
        debug("%s->abort()\n", pName.data());
        greet->abort();
    }
    suspended = true;
    updateStatus();
    timer.suspend();
    sockNot->setEnabled(false);
}

void
KGVerify::resume()
{
    sockNot->setEnabled(true);
    timer.resume();
    suspended = false;
    updateLockStatus();
    if (running) {
        debug("%s->start()\n", pName.data());
        greet->start();
    } else if (delayed) {
        delayed = false;
        running = true;
        sockNot->setEnabled(true);
        debug("%s->start()\n", pName.data());
        greet->start();
    }
}

void // not a slot - called manually by greeter
KGVerify::accept()
{
    debug("%s->next()\n", pName.data());
    greet->next();
}

void // private
KGVerify::doReject(bool initial)
{
    // assert(coreState != CoreBusy);
    if (running) {
        debug("%s->abort()\n", pName.data());
        greet->abort();
    }
    handler->verifyClear();
    debug("%s->clear()\n", pName.data());
    greet->clear();
    curUser.clear();
    pamUser.clear();
    if (!scheduleAutoLogin(initial)) {
        isClear = !(isClear && applyPreset());
        if (running) {
            debug("%s->start()\n", pName.data());
            greet->start();
        }
        if (!failed)
            timer.stop();
    }
}

void // not a slot - called manually by greeter
KGVerify::reject()
{
    doReject(true);
}

void
KGVerify::setEnabled(bool on)
{
    debug("%s->setEnabled(%s)\n", pName.data(), on ? "true" : "false");
    greet->setEnabled(on);
    enabled = on;
    updateStatus();
}

void // private
KGVerify::slotTimeout()
{
    if (failed) {
        failed = false;
        updateStatus();
        debug("%s->revive()\n", pName.data());
        greet->revive();
        handler->verifyRetry();
        if (suspended) {
            delayed = true;
        } else {
            running = true;
            sockNot->setEnabled(true);
            debug("%s->start()\n", pName.data());
            greet->start();
            slotActivity();
            talkerEdits();
        }
    } else if (timedLeft) {
        deadTicks--;
        if (!--timedLeft)
            performAutoLogin();
        else
            timer.start(1000);
        updateStatus();
    } else if (_isReserve) {
        // assert(ctx == Login && running);
        abort();
        ::exit(EX_RESERVE);
    } else {
        // assert(ctx == Login);
        isClear = true;
        doReject(false);
    }
}

void
KGVerify::slotActivity()
{
    if (timedLeft) {
        // timed login countdown running. cancel and reschedule it.
        debug("%s->revive()\n", pName.data());
        greet->revive();
        debug("%s->start()\n", pName.data());
        greet->start();
        sockNot->setEnabled(true);
        running = true;
        timedLeft = 0;
        updateStatus();
        timer.start(TIMED_GREET_TO * SECONDS);
    } else if (timeable) {
        // timed login is possible and thus scheduled. reschedule it.
        timer.start(TIMED_GREET_TO * SECONDS);
    } else if (_isReserve) {
        timer.start(FULL_GREET_TO * SECONDS);
    }
}

void
KGVerify::talkerEdits()
{
    if (func == KGreeterPlugin::Authenticate &&
        ctx == KGreeterPlugin::Login)
    {
        isClear = false;
        if (!timeable)
            timer.start(FULL_GREET_TO * SECONDS);
    }
}


void // private static
KGVerify::vrfMsgBox(QWidget *parent, const QString &user,
                    QMessageBox::Icon type, const QString &mesg)
{
    KFMsgBox::box(parent, type, user.isEmpty() ?
                  mesg : i18n("Logging in %1...\n\n", user) + mesg);
}

static const char * const msgs[] = {
    I18N_NOOP("You are required to change your password immediately (password aged)."),
    I18N_NOOP("You are required to change your password immediately (root enforced)."),
    I18N_NOOP("You are not allowed to login at the moment."),
    I18N_NOOP("Home folder not available."),
    I18N_NOOP("Logins are not allowed at the moment.\nTry again later."),
    I18N_NOOP("Your login shell is not listed in /etc/shells."),
    I18N_NOOP("Root logins are not allowed."),
    I18N_NOOP("Your account has expired; please contact your system administrator.")
};

void // private static
KGVerify::vrfErrBox(QWidget *parent, const QString &user, const char *msg)
{
    QMessageBox::Icon icon;
    QString mesg;

    if (!msg) {
        mesg = i18n("A critical error occurred.\n"
                    "Please look at KDM's logfile(s) for more information\n"
                    "or contact your system administrator.");
        icon = errorbox;
    } else {
        mesg = QString::fromLocal8Bit(msg);
        QString mesg1 = mesg + '.';
        for (uint i = 0; i < as(msgs); i++)
            if (mesg1 == msgs[i]) {
                mesg = i18n(msgs[i]);
                break;
            }
        icon = sorrybox;
    }
    vrfMsgBox(parent, user, icon, mesg);
}

void // private static
KGVerify::vrfInfoBox(QWidget *parent, const QString &user, const char *msg)
{
    QString mesg = QString::fromLocal8Bit(msg);
    QRegExp rx("^Warning: your account will expire in (\\d+) day");
    if (rx.indexIn(mesg) >= 0) {
        int expire = rx.cap(1).toInt();
        mesg = expire ?
            i18np("Your account expires tomorrow.",
                  "Your account expires in %1 days.", expire) :
            i18n("Your account expires today.");
    } else {
        rx.setPattern("^Warning: your password will expire in (\\d+) day");
        if (rx.indexIn(mesg) >= 0) {
            int expire = rx.cap(1).toInt();
            mesg = expire ?
                i18np("Your password expires tomorrow.",
                      "Your password expires in %1 days.", expire) :
                i18n("Your password expires today.");
        }
    }
    vrfMsgBox(parent, user, infobox, mesg);
}

bool // public static
KGVerify::handleFailVerify(QWidget *parent, bool showUser)
{
    char *msg;
    QString user;

    debug("handleFailVerify ...\n");

    if (showUser) {
        msg = gRecvStr();
        user = QString::fromLocal8Bit(msg);
        free(msg);
    }

    for (;;) {
        int ret = gRecvInt();

        // non-terminal status
        switch (ret) {
        /* case V_PUT_USER: cannot happen - we are in "classic" mode */
        /* case V_PRE_OK: cannot happen - not in ChTok dialog */
        /* case V_CHTOK: cannot happen - called by non-interactive verify */
        case V_CHTOK_AUTH:
            debug(" V_CHTOK_AUTH\n");
            {
                QStringList pgs(_pluginsLogin);
                pgs += _pluginsShutdown;
                foreach (const QString& pg, pgs)
                    if (pg == "classic" || pg == "modern") {
                        pgs = QStringList(pg);
                        goto gotit;
                    } else if (pg == "generic") {
                        pgs = QStringList("modern");
                        goto gotit;
                    }
                pgs = QStringList("classic");
              gotit:
                KGChTok chtok(parent, user, init(pgs), 0,
                              KGreeterPlugin::AuthChAuthTok,
                              KGreeterPlugin::Login);
                return chtok.exec();
            }
        case V_MSG_ERR:
            debug(" V_MSG_ERR\n");
            msg = gRecvStr();
            debug("  message %\"s\n", msg);
            vrfErrBox(parent, user, msg);
            free(msg);
            gSendInt(0);
            continue;
        case V_MSG_INFO_AUTH: // should not happen
        case V_MSG_INFO:
            debug(" V_MSG_INFO\n");
            msg = gRecvStr();
            debug("  message %\"s\n", msg);
            vrfInfoBox(parent, user, msg);
            free(msg);
            gSendInt(0);
            continue;
        }

        // terminal status
        switch (ret) {
        case V_OK:
            debug(" V_OK\n");
            return true;
        case V_AUTH:
            debug(" V_AUTH\n");
            vrfMsgBox(parent, user, sorrybox, i18n("Authentication failed"));
            return false;
        case V_FAIL:
            debug(" V_FAIL\n");
            return false;
        default:
            logPanic("Unknown V_xxx code %d from core\n", ret);
        }
    }
}

void // private
KGVerify::handleVerify()
{
    debug("handleVerify ...\n");
    QString user;
    char *msg;
    int ret, echo, ndelay;
    KGreeterPlugin::Function nfunc;

    ret = gRecvInt();

    // requests
    coreState = CorePrompting;
    switch (ret) {
    case V_GET_TEXT:
        debug(" V_GET_TEXT\n");
        msg = gRecvStr();
        debug("  prompt %\"s\n", msg);
        echo = gRecvInt();
        debug("  echo = %d\n", echo);
        ndelay = gRecvInt();
        debug("  ndelay = %d\n%s->textPrompt(...)\n", ndelay, pName.data());
        greet->textPrompt(msg, echo, ndelay);
        free(msg);
        return;
    case V_GET_BINARY:
        debug(" V_GET_BINARY\n");
        msg = gRecvArr(&ret);
        debug("  %d bytes prompt\n", ret);
        ndelay = gRecvInt();
        debug("  ndelay = %d\n%s->binaryPrompt(...)\n", ndelay, pName.data());
        greet->binaryPrompt(msg, ndelay);
        free(msg);
        return;
    }

    // non-terminal status
    coreState = CoreBusy;
    switch (ret) {
    case V_PUT_USER:
        debug(" V_PUT_USER\n");
        msg = gRecvStr();
        curUser = pamUser = QString::fromLocal8Bit(msg);
        // greet needs this to be able to return something useful from
        // getEntity(). but the backend is still unable to tell a domain ...
        debug("  %s->setUser(%\"s)\n", pName.data(), qPrintable(user));
        greet->setUser(curUser);
        handler->verifySetUser(curUser);
        free(msg);
        return;
    case V_PRE_OK: // this is only for func == AuthChAuthTok
        debug(" V_PRE_OK\n");
        // With the "classic" method, the wrong user simply cannot be
        // authenticated, even with the generic plugin. Other methods
        // could do so, but this applies only to ctx == ChangeTok, which
        // is not implemented yet.
        authTok = true;
        debug("%s->succeeded()\n", pName.data());
        greet->succeeded();
        return;
    case V_MSG_ERR:
        debug(" V_MSG_ERR\n");
        timer.suspend();
        msg = gRecvStr();
        debug("  %s->textMessage(%\"s, true)\n", pName.data(), msg);
        if (!greet->textMessage(msg, true)) { // XXX little point in filtering
            debug("  message passed\n");
            vrfErrBox(parent, pamUser, msg);
        } else {
            debug("  message swallowed\n");
        }
        free(msg);
        gSendInt(0);
        timer.resume();
        return;
    case V_MSG_INFO_AUTH:
        debug(" V_MSG_INFO_AUTH\n");
        timer.suspend();
        msg = gRecvStr();
        debug("  %s->textMessage(%\"s, false)\n", pName.data(), msg);
        if (!greet->textMessage(msg, false)) {
            debug("  message passed\n");
            vrfInfoBox(parent, pamUser, msg);
        } else {
            debug("  message swallowed\n");
        }
        free(msg);
        gSendInt(0);
        timer.resume();
        return;
    case V_MSG_INFO:
        debug(" V_MSG_INFO\n");
        timer.suspend();
        msg = gRecvStr();
        debug("  display %\"s\n", msg);
        vrfInfoBox(parent, pamUser, msg);
        free(msg);
        gSendInt(0);
        timer.resume();
        return;
    }

    // terminal status
    coreState = CoreIdle;
    running = false;
    sockNot->setEnabled(false);
    timer.stop();

    // These codes are not really terminal as far as the core is concerned,
    // but the branches as a whole are.
    if (ret == V_CHTOK_AUTH) {
        debug(" V_CHTOK_AUTH\n");
        nfunc = KGreeterPlugin::AuthChAuthTok;
        user = curUser;
        goto dchtok;
    } else if (ret == V_CHTOK) {
        debug(" V_CHTOK\n");
        nfunc = KGreeterPlugin::ChAuthTok;
        user.clear();
      dchtok:
        debug("%s->succeeded()\n", pName.data());
        greet->succeeded();
        KGChTok chtok(parent, user, pluginList, curPlugin, nfunc, KGreeterPlugin::Login);
        if (!chtok.exec())
            goto retry;
        handler->verifyOk();
        return;
    }

    if (ret == V_OK) {
        debug(" V_OK\n");
        if (!fixedEntity.isEmpty()) {
            debug("  %s->getEntity()\n", pName.data());
            QString ent = greet->getEntity();
            debug("  entity %\"s\n", qPrintable(ent));
            if (ent != fixedEntity) {
                debug("%s->failed()\n", pName.data());
                greet->failed();
                msgBox(sorrybox,
                       i18n("Authenticated user (%1) does not match requested user (%2).\n",
                            ent, fixedEntity));
                goto retry;
            }
        }
        debug("%s->succeeded()\n", pName.data());
        greet->succeeded();
        handler->verifyOk();
        return;
    }

    debug("%s->failed()\n", pName.data());
    greet->failed();

    if (ret == V_AUTH) {
        debug(" V_AUTH\n");
        failed = true;
        updateStatus();
        handler->verifyFailed();
        timer.start(1500 + KRandom::random() / (RAND_MAX / 1000));
        return;
    }
    if (ret != V_FAIL)
        logPanic("Unknown V_xxx code %d from core\n", ret);
    debug(" V_FAIL\n");
  retry:
    debug("%s->revive()\n", pName.data());
    greet->revive();
    sockNot->setEnabled(true);
    running = true;
    debug("%s->start()\n", pName.data());
    greet->start();
    slotActivity();
    talkerEdits();
}

void
KGVerify::gplugReturnText(const char *text, int tag)
{
    debug("%s: gplugReturnText(%\"s, %d)\n", pName.data(),
          tag & V_IS_SECRET ? "<masked>" : text, tag);
    gSendStr(text);
    if (text) {
        gSendInt(tag);
        coreState = CoreBusy;
    } else {
        coreState = CoreIdle;
    }
}

void
KGVerify::gplugReturnBinary(const char *data)
{
    if (data) {
        unsigned const char *up = (unsigned const char *)data;
        int len = up[3] | (up[2] << 8) | (up[1] << 16) | (up[0] << 24);
        debug("%s: gplugReturnBinary(%d bytes)\n", pName.data(), len);
        gSendArr(len, data);
        coreState = CoreBusy;
    } else {
        debug("%s: gplugReturnBinary(NULL)\n", pName.data());
        gSendArr(0, 0);
        coreState = CoreIdle;
    }
}

void
KGVerify::gplugSetUser(const QString &user)
{
    debug("%s: gplugSetUser(%\"s)\n", pName.data(), qPrintable(user));
    curUser = user;
    handler->verifySetUser(user);
}

void
KGVerify::gplugStart()
{
    // XXX handle func != Authenticate
    if (coreState != CoreIdle)
        return;
    debug("%s: gplugStart()\n", pName.data());
    gSendInt(ctx == KGreeterPlugin::Shutdown ? G_VerifyRootOK : G_Verify);
    gSendStr(greetPlugins[pluginList[curPlugin]].info->method);
    coreState = CoreBusy;
}

void
KGVerify::gplugChanged()
{
    debug("%s: gplugChanged()\n", pName.data());
    if (parent->isActiveWindow())
        talkerEdits();
}

void
KGVerify::gplugActivity()
{
    debug("%s: gplugActivity()\n", pName.data());
    slotActivity();
}

void
KGVerify::gplugMsgBox(QMessageBox::Icon type, const QString &text)
{
    debug("%s: gplugMsgBox(%d, %\"s)\n", pName.data(), type, qPrintable(text));
    msgBox(type, text);
}

bool
KGVerify::eventFilter(QObject *o, QEvent *e)
{
    switch (e->type()) {
    case QEvent::KeyPress:
        if (timedLeft) {
            QKeyEvent *ke = (QKeyEvent *)e;
            if (ke->key() == Qt::Key_Return || ke->key() == Qt::Key_Enter) {
                if (deadTicks <= 0) {
                    timedLeft = 0;
                    performAutoLogin();
                }
                return true;
            }
        }
        /* fall through */
    case QEvent::KeyRelease:
        updateLockStatus();
        /* fall through */
    default:
        break;
    }
    return inherited::eventFilter(o, e);
}

void
KGVerify::updateLockStatus()
{
    unsigned int lmask;
    Window dummy1, dummy2;
    int dummy3, dummy4, dummy5, dummy6;
    XQueryPointer(QX11Info::display(), DefaultRootWindow(QX11Info::display()),
                  &dummy1, &dummy2, &dummy3, &dummy4, &dummy5, &dummy6,
                  &lmask);
    capsLocked = lmask & LockMask;
    updateStatus();
}

void
KGVerify::msgBox(QMessageBox::Icon typ, const QString &msg)
{
    timer.suspend();
    KFMsgBox::box(parent, typ, msg);
    timer.resume();
}


QVariant // public static
KGVerify::getConf(void *, const char *key, const QVariant &dflt)
{
    if (!qstrcmp(key, "EchoPasswd")) {
        return QVariant(_echoPasswd);
    } else {
        QString fkey = QString::fromLatin1(key) + '=';
        foreach (const QString& pgo, _pluginOptions)
            if (pgo.startsWith(fkey))
                return pgo.mid(fkey.length());
        return dflt;
    }
}

QVector<GreeterPluginHandle> KGVerify::greetPlugins;

PluginList
KGVerify::init(const QStringList &plugins)
{
    PluginList pluginList;

    foreach (const QString& pg, plugins) {
        GreeterPluginHandle plugin;
        KLibrary *lib = new KLibrary(pg[0] == '/' ? pg : "kgreet_" + pg);
        if (lib->fileName().isEmpty()) {
            logError("GreeterPlugin %s does not exist\n", qPrintable(pg));
            delete lib;
            continue;
        }
        uint i, np = greetPlugins.count();
        for (i = 0; i < np; i++)
            if (greetPlugins[i].library->fileName() == lib->fileName()) {
                delete lib;
                goto next;
            }
        if (!lib->load()) {
            logError("Cannot load GreeterPlugin %s (%s)\n",
                     qPrintable(pg), qPrintable(lib->fileName()));
            delete lib;
            continue;
        }
        plugin.library = lib;
        plugin.info = (KGreeterPluginInfo *)lib->resolveSymbol("kgreeterplugin_info");
        if (!plugin.info) {
            logError("GreeterPlugin %s (%s) is no valid greet widget plugin\n",
                     qPrintable(pg), qPrintable(lib->fileName()));
            lib->unload();
            delete lib;
            continue;
        }

        if (!plugin.info->init(QString(), getConf, 0)) {
            logError("GreeterPlugin %s (%s) refuses to serve\n",
                     qPrintable(pg), qPrintable(lib->fileName()));
            lib->unload();
            delete lib;
            continue;
        }
        debug("GreeterPlugin %s (%s) loaded\n", qPrintable(pg), plugin.info->name);
        plugin.action = 0;
        greetPlugins.append(plugin);
      next:
        pluginList.append(i);
    }
    return pluginList;
}

void
KGVerify::done()
{
    for (int i = 0; i < greetPlugins.count(); i++) {
        if (greetPlugins[i].info->done)
            greetPlugins[i].info->done();
        greetPlugins[i].library->unload();
    }
}


KGStdVerify::KGStdVerify(KGVerifyHandler *_handler, QWidget *_parent,
                         QWidget *_predecessor, const QString &_fixedUser,
                         const PluginList &_pluginList,
                         KGreeterPlugin::Function _func,
                         KGreeterPlugin::Context _ctx)
    : inherited(_handler, _parent, _predecessor, _fixedUser,
                _pluginList, _func, _ctx)
    , failedLabelState(0)
{
    grid = new QGridLayout;
    grid->setAlignment(Qt::AlignCenter);

    failedLabel = new QLabel(parent);
    failedLabel->setFont(*_failFont);
    grid->addWidget(failedLabel, 1, 0, Qt::AlignCenter);

    updateLockStatus();
}

KGStdVerify::~KGStdVerify()
{
}

bool
KGStdVerify::gplugHasNode(const QString &)
{
    return false;
}

void // public
KGStdVerify::selectPlugin(int id)
{
    inherited::selectPlugin(id);
    QWidget *w = greet->getWidgets().first();
    grid->addWidget(w, 0, 0);
    w->show();
}

void
KGStdVerify::updateStatus()
{
    int nfls;

    if (!enabled)
        nfls = 1;
    else if (failed)
        nfls = 2;
    else if (timedLeft)
        nfls = -timedLeft;
    else if (!suspended && capsLocked)
        nfls = 3;
    else
        nfls = 1;

    if (failedLabelState != nfls) {
        failedLabelState = nfls;
        QPalette p;
        if (nfls < 0) {
            failedLabel->setText(i18np("Automatic login in 1 second...",
                                       "Automatic login in %1 seconds...",
                                       timedLeft));
        } else {
            switch (nfls) {
            default:
                failedLabel->clear();
                break;
            case 3:
                p.setBrush(QPalette::WindowText,
                    KColorScheme(QPalette::Active, KColorScheme::Window)
                        .foreground(KColorScheme::NegativeText));
                failedLabel->setText(i18n("Warning: Caps Lock is on"));
                break;
            case 2:
                failedLabel->setText(authTok ?
                                         i18n("Change failed") :
                                         fixedEntity.isEmpty() ?
                                            i18n("Login failed") :
                                            i18n("Authentication failed"));
                break;
            }
        }
        failedLabel->setPalette(p);
    }
}

KGThemedVerify::KGThemedVerify(KGVerifyHandler *_handler,
                               KdmThemer *_themer,
                               QWidget *_parent, QWidget *_predecessor,
                               const QString &_fixedUser,
                               const PluginList &_pluginList,
                               KGreeterPlugin::Function _func,
                               KGreeterPlugin::Context _ctx)
    : inherited(_handler, _parent, _predecessor, _fixedUser,
                _pluginList, _func, _ctx)
    , themer(_themer)
{
    updateLockStatus();
}

KGThemedVerify::~KGThemedVerify()
{
}

bool
KGThemedVerify::gplugHasNode(const QString &id)
{
    return themer->findNode(id) != 0;
}

void // public
KGThemedVerify::selectPlugin(int id)
{
    if (curPlugin != -1)
        themer->setTypeVisible(QString("plugin-specific-").append(pluginName()), false);
    inherited::selectPlugin(id);
    themer->setTypeVisible(QString("plugin-specific-").append(pluginName()), true);
    QSet<QString> oldTypes = showTypes;
    showTypes.clear();
    foreach (QWidget *w, greet->getWidgets())
        if (KdmItem *n = themer->findNode(w->objectName())) {
            QString tn(QString("plugin-").append(w->objectName()));
            themer->setTypeVisible(tn, true);
            showTypes.insert(tn);
            oldTypes.remove(tn);
            n->setWidget(w);
        } else {
            msgBox(errorbox,
                   i18n("Theme not usable with authentication method '%1'.",
                        i18n(greetPlugins[pluginList[id]].info->name)));
            break;
        }
    foreach (const QString &t, oldTypes)
        themer->setTypeVisible(t, false);
}

void
KGThemedVerify::updateStatus()
{
    handler->updateStatus(enabled && failed,
                          enabled && !suspended && capsLocked,
                          timedLeft);
}


KGChTok::KGChTok(QWidget *_parent, const QString &user,
                 const PluginList &pluginList, int curPlugin,
                 KGreeterPlugin::Function func,
                 KGreeterPlugin::Context ctx)
    : inherited(_parent)
    , verify(0)
{
    QSizePolicy fp(QSizePolicy::Fixed, QSizePolicy::Fixed);
    okButton = new KPushButton(KStandardGuiItem::ok(), this);
    okButton->setSizePolicy(fp);
    okButton->setDefault(true);
    cancelButton = new KPushButton(KStandardGuiItem::cancel(), this);
    cancelButton->setSizePolicy(fp);

    verify = new KGStdVerify(this, this, cancelButton, user, pluginList, func, ctx);
    verify->selectPlugin(curPlugin);

    QVBoxLayout *box = new QVBoxLayout(this);

    box->addWidget(new QLabel(i18nc("@title:window",
                                    "<qt><b>Changing authentication token</b></qt>"),
                              this), 0, Qt::AlignHCenter | Qt::AlignTop);

    box->addLayout(verify->getLayout());

    box->addWidget(new KSeparator(Qt::Horizontal, this));

    QHBoxLayout *hlay = new QHBoxLayout();
    box->addLayout(hlay);
    hlay->addStretch(1);
    hlay->addWidget(okButton);
    hlay->addStretch(1);
    hlay->addWidget(cancelButton);
    hlay->addStretch(1);

    connect(okButton, SIGNAL(clicked()), SLOT(accept()));
    connect(cancelButton, SIGNAL(clicked()), SLOT(reject()));

    QTimer::singleShot(0, verify, SLOT(start()));
}

KGChTok::~KGChTok()
{
    hide();
    delete verify;
}

void
KGChTok::accept()
{
    verify->accept();
}

void
KGChTok::verifyPluginChanged(int)
{
    // cannot happen
}

void
KGChTok::verifyOk()
{
    inherited::accept();
}

void
KGChTok::verifyFailed()
{
    okButton->setEnabled(false);
    cancelButton->setEnabled(false);
}

void
KGChTok::verifyRetry()
{
    okButton->setEnabled(true);
    cancelButton->setEnabled(true);
}

void
KGChTok::verifySetUser(const QString &)
{
    // cannot happen
}


////// helper class, nuke when qtimer supports suspend()/resume()

QXTimer::QXTimer()
    : inherited(0)
    , left(-1)
{
    connect(&timer, SIGNAL(timeout()), SLOT(slotTimeout()));
}

void
QXTimer::start(int msec)
{
    left = msec;
    timer.setSingleShot(true);
    timer.start(left);
    gettimeofday(&stv, 0);
}

void
QXTimer::stop()
{
    timer.stop();
    left = -1;
}

void
QXTimer::suspend()
{
    if (timer.isActive()) {
        timer.stop();
        struct timeval tv;
        gettimeofday(&tv, 0);
        left -= (tv.tv_sec - stv.tv_sec) * 1000 + (tv.tv_usec - stv.tv_usec) / 1000;
        if (left < 0)
            left = 0;
    }
}

void
QXTimer::resume()
{
    if (left >= 0 && !timer.isActive()) {
        timer.setSingleShot(true);
        timer.start(left);
        gettimeofday(&stv, 0);
    }
}

void
QXTimer::slotTimeout()
{
    left = -1;
    emit timeout();
}


#include "kgverify.moc"
