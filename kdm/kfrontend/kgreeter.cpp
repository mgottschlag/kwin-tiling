/*

Greeter widget for kdm

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

#include "kgreeter.h"
#include "kconsole.h"
#include "kdmconfig.h"
#include "kdmclock.h"
#include "kdm_greet.h"
#include "themer/kdmthemer.h"
#include "themer/kdmitem.h"
#include "themer/kdmlabel.h"

#include <KColorScheme>
#include <KConfigGroup>
#include <klocale.h>
#include <kseparator.h>
#include <kstandarddirs.h>
#include <kstringhandler.h>

#include <QAction>
#include <QBuffer>
#include <QDir>
#include <QFile>
#include <QImageReader>
#include <QKeyEvent>
#include <QLabel>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMenu>
#include <QMovie>
#include <QPainter>
#include <QPushButton>
#include <QShortcut>
#include <QStyle>

#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <pwd.h>
#include <grp.h>

#include <X11/Xlib.h>
#include <fixx11h.h>

class UserListView : public QListWidget {
  public:
    UserListView(QWidget *parent = 0)
        : QListWidget(parent)
        , cachedSizeHint(-1, 0)
    {
        setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Ignored);
        setUniformItemSizes(true);
        setVerticalScrollMode(ScrollPerPixel);
        setIconSize(QSize(48, 48));
        setAlternatingRowColors(true);
    }

    mutable QSize cachedSizeHint;

  protected:
    virtual QSize sizeHint() const
    {
        if (!cachedSizeHint.isValid()) {
            ensurePolished();
            QStyleOptionViewItem vo(viewOptions());
            QAbstractListModel *md(static_cast<QAbstractListModel *>(model()));
            uint maxw = 0, h = 0;
            for (int i = 0, rc = md->rowCount(); i < rc; i++) {
                QSize sh = itemDelegate()->sizeHint(vo, md->index(i));
                uint thisw = sh.width();
                if (thisw > maxw)
                    maxw = thisw;
                h += sh.height();
            }
            cachedSizeHint.setWidth(
                style()->pixelMetric(QStyle::PM_ScrollBarExtent) +
                (frameWidth() +
                 (style()->styleHint(QStyle::SH_ScrollView_FrameOnlyAroundContents) ?
                  style()->pixelMetric(QStyle::PM_DefaultFrameWidth) : 0)) * 2 +
                maxw);
            cachedSizeHint.setHeight(frameWidth() * 2 + h);
        }
        return cachedSizeHint;
    }

    virtual void keyPressEvent(QKeyEvent *event)
    {
        switch (event->key()) {
        case Qt::Key_Enter:
        case Qt::Key_Return:
            if (currentItem())
                emit itemDoubleClicked(currentItem());
            event->accept();
            break;
        case Qt::Key_Space:
            if (currentItem())
                emit itemClicked(currentItem());
            event->accept();
            break;
        default:
            QListWidget::keyPressEvent(event);
            break;
        }
    }

    virtual void mousePressEvent(QMouseEvent *event)
    {
        m_suppressClick = false;
        QListWidget::mousePressEvent(event);
    }

    virtual void mouseReleaseEvent(QMouseEvent *event)
    {
        if (m_suppressClick)
            m_suppressClick = false;
        else
            QListWidget::mouseReleaseEvent(event);
    }

    virtual void mouseDoubleClickEvent(QMouseEvent *event)
    {
        m_suppressClick = true;
        QListWidget::mouseDoubleClickEvent(event);
    }

  private:
    bool m_suppressClick;
};

class UserListViewItem : public QListWidgetItem {
  public:
    UserListViewItem(UserListView *parent, const QString &text,
                     const QPixmap &pixmap, const QString &username)
        : QListWidgetItem(parent)
        , login(username)
    {
        setIcon(pixmap);
        setText(text);
        parent->cachedSizeHint.setWidth(-1);
    }

    QString login;
};


int KGreeter::curPlugin = -1;
PluginList KGreeter::pluginList;

KGreeter::KGreeter(bool framed)
    : inherited(framed)
    , dName(dname)
    , userView(0)
    , userList(0)
    , nNormals(0)
    , nSpecials(0)
    , curPrev(0)
    , prevValid(true)
    , needLoad(false)
{
    stsGroup = new KConfigGroup(KSharedConfig::openConfig(_stsFile),
                                "PrevUser");

    if (_userList) {
        userView = new UserListView(this);
        connect(userView, SIGNAL(itemClicked(QListWidgetItem*)),
                SLOT(slotUserClicked(QListWidgetItem*)));
        connect(userView, SIGNAL(itemDoubleClicked(QListWidgetItem*)),
                SLOT(accept()));
    }
    if (_userCompletion)
        userList = new QStringList;
    if (userView || userList)
        insertUsers();

    sessMenu = new QMenu(this);
    connect(sessMenu, SIGNAL(triggered(QAction*)),
            SLOT(slotSessionSelected()));

    sessGroup = new QActionGroup(this);
    insertSessions();

    if (curPlugin < 0) {
        curPlugin = 0;
        pluginList = KGVerify::init(_pluginsLogin);
    }
}

KGreeter::~KGreeter()
{
    hide();
    delete userList;
    delete verify;
    delete stsGroup;
}

#define FILE_LIMIT_ICON 20
#define FILE_LIMIT_IMAGE 200
#define PIXEL_LIMIT_ICON 100
#define PIXEL_LIMIT_IMAGE 300

// replace this with a simple !access(..., X_OK) once we run with non-root real uid
static bool
dirAccessible(const char *dir)
{
    struct stat st;

    if (stat(dir, &st))
        return false;
    return (st.st_mode & S_IXOTH) != 0;
}

static bool
loadFace(QByteArray &fn, QImage &p, const QByteArray &pp, bool complain = false)
{
    int fd, ico;
    if ((fd = open(fn.data(), O_RDONLY | O_NONBLOCK)) < 0) {
        if (errno != ENOENT) {
            if (pp.isEmpty() || dirAccessible(pp.data()))
                (complain ? logError : logInfo)
                    ("Cannot load %s: %m\n", fn.data());
            return false;
        }
        fn.chop(5);
        if ((fd = open(fn.data(), O_RDONLY | O_NONBLOCK)) < 0) {
            if ((complain || errno != ENOENT) &&
                (pp.isEmpty() || dirAccessible(pp.data())))
                (complain ? logError : logInfo)
                    ("Cannot load %s: %m\n", fn.data());
            return false;
        }
        ico = 0;
    } else {
        ico = 1;
    }
    QFile f;
    f.open(fd, QFile::ReadOnly);
    int fs = f.size();
    if (fs > (ico ? FILE_LIMIT_ICON : FILE_LIMIT_IMAGE) * 1000) {
        logWarn("%s exceeds file size limit (%dkB)\n",
                fn.data(), ico ? FILE_LIMIT_ICON : FILE_LIMIT_IMAGE);
        return false;
    }
    QByteArray fc = f.read(fs);
    ::close(fd);
    QBuffer buf(&fc);
    buf.open(QBuffer::ReadOnly);
    QImageReader ir(&buf);
    QSize sz = ir.size();
    int lim = ico ? PIXEL_LIMIT_ICON : PIXEL_LIMIT_IMAGE;
    if (sz.width() > lim || sz.height() > lim) {
        logWarn("%s exceeds image dimension limit (%dx%d)\n",
                fn.data(), lim, lim);
        return false;
    }
    sz.scale(48, 48, Qt::KeepAspectRatio);
    ir.setScaledSize(sz);
    p = ir.read();
    if (p.isNull()) {
        logInfo("%s is no valid image\n", fn.data());
        return false;
    }
    if (p.width() < 48) {
        QImage np(48, p.height(), QImage::Format_ARGB32);
        np.fill(0);
        QPainter pnt(&np);
        pnt.drawImage((48 - p.width()) / 2, 0, p);
        p = np;
    }
    return true;
}

void
KGreeter::insertUser(const QImage &default_pix,
                     const QString &username, struct passwd *ps)
{
    if (userList)
        userList->append(username);
    if (!userView)
        return;

    int dp = 0, nd = 0;
    if (_faceSource == FACE_USER_ONLY ||
        _faceSource == FACE_PREFER_USER)
        dp = 1;
    if (_faceSource != FACE_USER_ONLY &&
        _faceSource != FACE_ADMIN_ONLY)
        nd = 1;
    QImage p;
    do {
        dp ^= 1;
        QByteArray pp, fn;
        if (!dp) {
            fn = pp = QByteArray(ps->pw_dir);
            fn += '/';
        } else {
            fn = QFile::encodeName(_faceDir);
            fn += '/';
            fn += ps->pw_name;
        }
        fn += ".face.icon";
        if (loadFace(fn, p, pp))
            goto gotit;
    } while (--nd >= 0);
    p = default_pix;
  gotit:
    QString realname = KStringHandler::from8Bit(ps->pw_gecos);
    realname.truncate(realname.indexOf(',') & (~0U >> 1));
    if (realname.isEmpty() || realname == username) {
        new UserListViewItem(userView, username, QPixmap::fromImage(p), username);
    } else {
        realname.append("\n").append(username);
        new UserListViewItem(userView, realname, QPixmap::fromImage(p), username);
    }
}

class UserList {
  public:
    UserList(char **in);
    bool hasUser(const char *str) const { return users.contains(str); }
    bool hasGroup(gid_t gid) const { return groups.contains(gid); }
    bool hasGroups() const { return !groups.isEmpty(); }

  private:
    QSet<gid_t> groups;
    QSet<QByteArray> users;
};

UserList::UserList(char **in)
{
    struct group *grp;

    for (; *in; in++)
        if (**in == '@') {
            if ((grp = getgrnam(*in + 1))) {
                for (; *grp->gr_mem; grp->gr_mem++)
                    users.insert(*grp->gr_mem);
                groups.insert(grp->gr_gid);
            }
        } else {
            users.insert(*in);
        }
}

void
KGreeter::insertUsers()
{
    struct passwd *ps;

    if (!getuid()) {
        if (!(ps = getpwnam("nobody")))
            return;
        if (setegid(ps->pw_gid))
            return;
        if (seteuid(ps->pw_uid)) {
            setegid(0);
            return;
        }
    }

    QImage default_pix;
    if (userView) {
        QByteArray fn = QFile::encodeName(_faceDir) + "/.default.face.icon";
        if (!loadFace(fn, default_pix, QByteArray(), true)) {
            default_pix = QImage(48, 48, QImage::Format_ARGB32);
            default_pix.fill(0);
        }
    }
    if (_showUsers == SHOW_ALL) {
        UserList noUsers(_noUsers);
        QSet<QString> dupes;
        for (setpwent(); (ps = getpwent()) != 0;) {
            if (*ps->pw_dir && *ps->pw_shell &&
                (ps->pw_uid >= (unsigned)_lowUserId ||
                 (!ps->pw_uid && _showRoot)) &&
                ps->pw_uid <= (unsigned)_highUserId &&
                !noUsers.hasUser(ps->pw_name) &&
                !noUsers.hasGroup(ps->pw_gid))
            {
                QString username(QFile::decodeName(ps->pw_name));
                if (!dupes.contains(username)) {
                    dupes.insert(username);
                    insertUser(default_pix, username, ps);
                }
            }
        }
    } else {
        UserList users(_users);
        if (users.hasGroups()) {
            QSet<QString> dupes;
            for (setpwent(); (ps = getpwent()) != 0;) {
                if (*ps->pw_dir && *ps->pw_shell &&
                    (ps->pw_uid >= (unsigned)_lowUserId ||
                     (!ps->pw_uid && _showRoot)) &&
                    ps->pw_uid <= (unsigned)_highUserId &&
                    (users.hasUser(ps->pw_name) ||
                     users.hasGroup(ps->pw_gid)))
                {
                    QString username(QFile::decodeName(ps->pw_name));
                    if (!dupes.contains(username)) {
                        dupes.insert(username);
                        insertUser(default_pix, username, ps);
                    }
                }
            }
        } else {
            for (int i = 0; _users[i]; i++)
                if ((ps = getpwnam(_users[i])) && (ps->pw_uid || _showRoot))
                    insertUser(default_pix, QFile::decodeName(_users[i]), ps);
        }
    }
    endpwent();
    endgrent();
    if (_sortUsers) {
        if (userView)
            userView->sortItems();
        if (userList)
            userList->sort();
    }

    if (!getuid()) {
        seteuid(0);
        setegid(0);
    }
}

void
KGreeter::putSession(const QString &type, const QString &name, bool hid, const char *exe)
{
    int prio = exe ? (!strcmp(exe, "default") ? 0 :
                      !strcmp(exe, "custom") ? 1 :
                      !strcmp(exe, "failsafe") ? 3 : 2) : 2;
    for (int i = 0; i < sessionTypes.size(); i++)
        if (sessionTypes[i].type == type) {
            sessionTypes[i].prio = prio;
            return;
        }
    sessionTypes.append(SessType(name, type, hid, prio));
}

void
KGreeter::insertSessions()
{
    for (char **dit = _sessionsDirs; *dit; ++dit)
        foreach (QString ent, QDir(*dit).entryList())
            if (ent.endsWith(".desktop")) {
                KConfigGroup dsk(
                    KSharedConfig::openConfig(
                        QString(*dit).append('/').append(ent)),
                    "Desktop Entry");
                putSession(ent.left(ent.length() - 8),
                            dsk.readEntry("Name"),
                            (dsk.readEntry("Hidden", false) ||
                             (dsk.hasKey("TryExec") &&
                              KStandardDirs::findExe(
                                  dsk.readEntry("TryExec")).isEmpty())),
                            dsk.readEntry("Exec").toLatin1());
            }
    putSession("default", i18nc("@item:inlistbox session type", "Default"), false, "default");
    putSession("custom", i18nc("@item:inlistbox session type", "Custom"), false, "custom");
    putSession("failsafe", i18nc("@item:inlistbox session type", "Failsafe"), false, "failsafe");
    qSort(sessionTypes);
    for (int i = 0; i < sessionTypes.size() && !sessionTypes[i].hid; i++) {
        sessionTypes[i].action = sessGroup->addAction(sessionTypes[i].name);
        sessionTypes[i].action->setData(i);
        sessionTypes[i].action->setCheckable(true);
        switch (sessionTypes[i].prio) {
        case 0: case 1: nSpecials++; break;
        case 2: nNormals++; break;
        }
    }
    sessMenu->addActions(sessGroup->actions());
}

void
KGreeter::slotUserEntered()
{
    struct passwd *pw;

    if (userView) {
        if ((pw = getpwnam(curUser.toLocal8Bit().data()))) {
            QString theUser = QString::fromLocal8Bit(pw->pw_name);
            for (int i = 0, rc = userView->model()->rowCount(); i < rc; i++) {
                UserListViewItem *item =
                    static_cast<UserListViewItem *>(userView->item(i));
                if (item->login == theUser) {
                    userView->setCurrentItem(item);
                    goto oke;
                }
            }
        }
        userView->clearSelection();
    }
  oke:
    if (isVisible())
        slotLoadPrevWM();
    else
        QTimer::singleShot(0, this, SLOT(slotLoadPrevWM()));
}

void
KGreeter::slotUserClicked(QListWidgetItem *item)
{
    if (item) {
        curUser = ((UserListViewItem *)item)->login;
        verify->setUser(curUser);
        slotLoadPrevWM();
    }
}

void
KGreeter::slotSessionSelected()
{
    verify->gplugChanged();
}

void
KGreeter::reject()
{
    verify->reject();
}

void
KGreeter::accept()
{
    verify->accept();
}

void // private
KGreeter::setPrevWM(QAction *wm)
{
    if (curPrev != wm) {
        if (curPrev)
            curPrev->setText(sessionTypes[curPrev->data().toInt()].name);
        if (wm)
            wm->setText(i18nc("@item:inmenu session type",
                              "%1 (previous)",
                              sessionTypes[wm->data().toInt()].name));
        curPrev = wm;
    }
}

void
KGreeter::slotLoadPrevWM()
{
    int len, i, b;
    unsigned long crc, by;
    QByteArray name;
    char *sess;

    // XXX this should actually check for !CoreBusy - would it be safe?
    if (verify->coreState != KGVerify::CoreIdle) {
        needLoad = true;
        return;
    }
    needLoad = false;

    prevValid = true;
    name = curUser.toLocal8Bit();
    gSendInt(G_ReadDmrc);
    gSendStr(name.data());
    gRecvInt(); // ignore status code ...
    if ((len = name.length())) {
        gSendInt(G_GetDmrc);
        gSendStr("Session");
        sess = gRecvStr();
        if (!sess) { /* no such user */
            if (!userView && !userList) { // don't fake if user list shown
                prevValid = false;
                /* simple crc32 */
                for (crc = _forgingSeed, i = 0; i < len; i++) {
                    by = (crc & 255) ^ name[i];
                    for (b = 0; b < 8; b++)
                        by = (by >> 1) ^ (-(by & 1) & 0xedb88320);
                    crc = (crc >> 8) ^ by;
                }
                /* forge a session with this hash - default & custom more probable */
                /* XXX - this should do a statistical analysis of the real users */
#if 1
                setPrevWM(sessionTypes[crc % (nSpecials * 2 + nNormals) % (nSpecials + nNormals)].action);
#else
                i = crc % (nSpecials * 2 + nNormals);
                if (i < nNormals)
                    setPrevWM(sessionTypes[i + nSpecials].action);
                else
                    setPrevWM(sessionTypes[(i - nNormals) / 2].action);
#endif
                return;
            }
        } else {
            for (int i = 0; i < sessionTypes.count() && !sessionTypes[i].hid; i++)
                if (sessionTypes[i].type == sess) {
                    free(sess);
                    setPrevWM(sessionTypes[i].action);
                    return;
                }
            if (!sessGroup->checkedAction())
                KFMsgBox::box(this, sorrybox,
                              i18n("Your saved session type '%1' is not valid any more.\n"
                                   "Please select a new one, otherwise 'default' will be used.", sess));
            free(sess);
            prevValid = false;
        }
    }
    setPrevWM(0);
}

void // protected
KGreeter::pluginSetup()
{
    int field = 0;
    QString ent, pn(verify->pluginName()), dn(dName + '_' + pn);

    if (_preselUser != PRESEL_PREV)
        stsGroup->deleteEntry(verify->entitiesLocal() ? dName : dn, 0);
    if (_preselUser != PRESEL_NONE && verify->entityPresettable()) {
        if (verify->entitiesLocal())
            ent = _preselUser == PRESEL_PREV ?
                stsGroup->readEntry(dName, QString()) : _defaultUser;
        else
            ent = _preselUser == PRESEL_PREV ?
                stsGroup->readEntry(dn, QString()) :
                verify->getConf(0, (pn + ".DefaultEntity").toLatin1(), QVariant()).toString();
        field = verify->entitiesFielded() ?
            verify->getConf(0, (pn + ".FocusField").toLatin1(), QVariant(0)).toInt() :
            _focusPasswd;
    }
    verify->presetEntity(ent, field);
    if (userList)
        verify->loadUsers(*userList);
}

void
KGreeter::verifyPluginChanged(int id)
{
    curPlugin = id;
    pluginSetup();
}

void
KGreeter::verifyClear()
{
    curUser.clear();
    slotUserEntered();
    if (QAction *curSel = sessGroup->checkedAction())
        curSel->setChecked(false);
}

void
KGreeter::verifyOk()
{
    if (_preselUser == PRESEL_PREV && verify->entityPresettable())
        stsGroup->writeEntry(verify->entitiesLocal() ?
                                 dName :
                                 dName + '_' + verify->pluginName(),
                             verify->getEntity());
    if (QAction *curSel = sessGroup->checkedAction()) {
        gSendInt(G_PutDmrc);
        gSendStr("Session");
        gSendStr(sessionTypes[curSel->data().toInt()].type.toUtf8());
    } else if (!prevValid) {
        gSendInt(G_PutDmrc);
        gSendStr("Session");
        gSendStr("default");
    }
    done(ex_login);
}

void
KGreeter::verifyFailed()
{
    if (userView)
        userView->setEnabled(false);
    if (needLoad)
        slotLoadPrevWM();
}

void
KGreeter::verifyRetry()
{
    if (userView)
        userView->setEnabled(true);
}

void
KGreeter::verifySetUser(const QString &user)
{
    curUser = user;
    slotUserEntered();
}


KStdGreeter::KStdGreeter()
    : KGreeter()
    , clock(0)
    , pixLabel(0)
{
    QBoxLayout *main_box;
#ifdef WITH_KDM_XCONSOLE
    if (consoleView) {
        QBoxLayout *ex_box = new QVBoxLayout(this);
        main_box = new QHBoxLayout();
        ex_box->addLayout(main_box);
        ex_box->addWidget(consoleView);
    } else
#endif
    {
        main_box = new QHBoxLayout(this);
    }
    int rs = layout()->spacing();
    main_box->setSpacing(layout()->margin());

    if (userView)
        main_box->addWidget(userView);

    QBoxLayout *inner_box = new QVBoxLayout();
    main_box->addLayout(inner_box);
    inner_box->setSpacing(rs);

    if (!_authorized && _authComplain) {
        QLabel *complainLabel = new QLabel(
            i18n("Warning: this is an unsecured session"), this);
        complainLabel->setToolTip(
            i18n("This display requires no X authorization.\n"
                 "This means that anybody can connect to it,\n"
                 "open windows on it or intercept your input."));
        complainLabel->setAlignment(Qt::AlignCenter);
        complainLabel->setFont(*_failFont);
        QPalette p;
        p.setBrush(QPalette::WindowText,
            KColorScheme(QPalette::Active, KColorScheme::Window)
                .foreground(KColorScheme::NegativeText));
        complainLabel->setPalette(p);
        inner_box->addWidget(complainLabel);
    }
    if (!_greetString.isEmpty()) {
        QLabel *welcomeLabel = new QLabel(_greetString, this);
        welcomeLabel->setAlignment(Qt::AlignCenter);
        welcomeLabel->setFont(*_greetFont);
        inner_box->addWidget(welcomeLabel);
    }

    switch (_logoArea) {
    case LOGO_CLOCK:
        clock = new KdmClock(this);
        break;
    case LOGO_LOGO: {
        QMovie *movie = new QMovie(this);
        movie->setFileName(_logo);
        if (movie->isValid()) {
            movie->start();
            pixLabel = new QLabel(this);
            pixLabel->setMovie(movie);
            if (!movie->currentImage().hasAlphaChannel())
                pixLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);
            pixLabel->setIndent(0);
        } else {
            delete movie;
        }
        break; }
    }

    if (userView) {
        if (clock)
            inner_box->addWidget(clock, 0, Qt::AlignCenter);
        else if (pixLabel)
            inner_box->addWidget(pixLabel, 0, Qt::AlignCenter);
        inner_box->addSpacing(inner_box->spacing());
    } else {
        if (clock)
            main_box->addWidget(clock, 0, Qt::AlignCenter);
        else if (pixLabel)
            main_box->addWidget(pixLabel, 0, Qt::AlignCenter);
    }

    goButton = new QPushButton(i18nc("@action:button", "L&ogin"), this);
    goButton->setDefault(true);
    connect(goButton, SIGNAL(clicked()), SLOT(accept()));
    QPushButton *menuButton = new QPushButton(i18nc("@action:button", "&Menu"), this);
    //helpButton

    QWidget *prec;
    if (userView)
        prec = userView;
#ifdef WITH_KDM_XCONSOLE
    else if (consoleView)
        prec = consoleView;
#endif
    else
        prec = menuButton;
    KGStdVerify *sverify =
        new KGStdVerify(this, this, prec, QString(),
                        pluginList, KGreeterPlugin::Authenticate,
                        KGreeterPlugin::Login);
    inner_box->addLayout(sverify->getLayout());
    QMenu *plugMenu = sverify->getPlugMenu();
    sverify->selectPlugin(curPlugin);
    verify = sverify;

    inner_box->addWidget(new KSeparator(Qt::Horizontal, this));

    QBoxLayout *hbox2 = new QHBoxLayout();
    inner_box->addLayout(hbox2);
    hbox2->addWidget(goButton);
    hbox2->addStretch(1);
    hbox2->addWidget(menuButton);
    hbox2->addStretch(1);

    if (sessMenu->actions().count() > 1) {
        inserten(i18nc("@title:menu", "Session &Type"), Qt::ALT + Qt::Key_T, sessMenu);
        needSep = true;
    }

    if (plugMenu) {
        inserten(i18nc("@title:menu", "&Authentication Method"), Qt::ALT + Qt::Key_A, plugMenu);
        needSep = true;
    }

#ifdef XDMCP
    completeMenu(LOGIN_LOCAL_ONLY, ex_choose, i18nc("@action:inmenu", "&Remote Login"), Qt::ALT + Qt::Key_R);
#else
    completeMenu();
#endif

    if (optMenu)
        menuButton->setMenu(optMenu);
    else
        menuButton->hide();

    pluginSetup();

    verify->start();
}

void
KStdGreeter::pluginSetup()
{
    inherited::pluginSetup();
    if (userView) {
        if (verify->entitiesLocal() && verify->entityPresettable())
            userView->show();
        else
            userView->hide();
    }
    adjustGeometry();
    update();
}

void
KStdGreeter::verifyFailed()
{
    goButton->setEnabled(false);
    inherited::verifyFailed();
}

void
KStdGreeter::verifyRetry()
{
    goButton->setEnabled(true);
    inherited::verifyRetry();
}


KThemedGreeter::KThemedGreeter(KdmThemer *_themer)
    : KGreeter(true)
    , themer(_themer)
//    , clock(0)
{
    // We do all painting ourselves
    setAttribute(Qt::WA_NoSystemBackground, true);
    // Allow tracking the mouse position
    setMouseTracking(true);

    adjustGeometry();

    themer->setWidget(this);

    if (_allowThemeDebug)
        new QShortcut(QKeySequence(QLatin1String("Ctrl+Alt+D")),
                      this, SLOT(slotDebugToggled()));

    connect(themer, SIGNAL(activated(QString)),
            SLOT(slotThemeActivated(QString)));

    KdmItem *console_node = themer->findNode("xconsole"); // kdm ext
    KdmItem *console_rect = themer->findNode("xconsole-rect"); // kdm ext
    if (!console_rect)
        console_rect = console_node;
    userlist_node = themer->findNode("userlist");
    userlist_rect = themer->findNode("userlist-rect");
    if (!userlist_rect)
        userlist_rect = userlist_node;
    caps_warning = themer->findNode("caps-lock-warning");
    xauth_warning = themer->findNode("xauth-warning"); // kdm ext
    pam_error = themer->findNode("pam-error");
    KdmLabel *pam_error_label = qobject_cast<KdmLabel *>(pam_error);
    if (pam_error_label)
        pam_error_label->setText(i18n("Login failed"));
    timed_label = themer->findNode("timed-label");

    KdmItem *itm;
    if ((itm = themer->findNode("pam-message"))) // done via msgboxes
        itm->setVisible(false);
    if ((itm = themer->findNode("language_button"))) // not implemented yet
        itm->setVisible(false);

    if (console_node) {
#ifdef WITH_KDM_XCONSOLE
        if (consoleView)
            console_node->setWidget(consoleView);
        else
#endif
            console_rect->setVisible(false);
    }

    if (xauth_warning && (_authorized || !_authComplain))
        xauth_warning->setVisible(false);

//    if (!_greetString.isEmpty()) {
//    }
//    clock = new KdmClock(this, "clock");

    QWidget *prec;
    if (userView)
        prec = userView;
#ifdef WITH_KDM_XCONSOLE
    else if (consoleView)
        prec = consoleView;
#endif
    else
        prec = 0;
    KGThemedVerify *tverify =
        new KGThemedVerify(this, themer, this, prec, QString(),
                           pluginList, KGreeterPlugin::Authenticate,
                           KGreeterPlugin::Login);
    QMenu *plugMenu = tverify->getPlugMenu();
    tverify->selectPlugin(curPlugin);
    verify = tverify;

    if ((session_button = themer->findNode("session_button"))) {
        if (sessMenu->actions().count() <= 1) {
            session_button->setVisible(false);
            session_button = 0;
        }
    } else {
        if (sessMenu->actions().count() > 1) {
            inserten(i18nc("@title:menu", "Session &Type"), Qt::ALT + Qt::Key_T, sessMenu);
            needSep = true;
        }
    }

    if (plugMenu) {
        inserten(i18nc("@title:menu", "&Authentication Method"), Qt::ALT + Qt::Key_A, plugMenu);
        needSep = true;
    }

#ifdef XDMCP
    completeMenu(LOGIN_LOCAL_ONLY, ex_choose, i18nc("@action:inmenu", "&Remote Login"), Qt::ALT + Qt::Key_R);
#else
    completeMenu();
#endif

    if ((system_button = themer->findNode("system_button"))) {
        if (optMenu)
            addAction(optMenu->menuAction());
        else
            system_button->setVisible(false);
    }

    pluginSetup();

    verify->start();
}

KThemedGreeter::~KThemedGreeter()
{
    themer->setWidget(0);
}

void
KThemedGreeter::slotDebugToggled()
{
    if ((debugLevel ^= DEBUG_THEMING))
        themer->slotNeedPlacement();
}

bool
KThemedGreeter::event(QEvent *e)
{
    if (themer)
        themer->widgetEvent(e);
    return inherited::event(e);
}

void
KThemedGreeter::pluginSetup()
{
    inherited::pluginSetup();

    if (userView && verify->entitiesLocal() && verify->entityPresettable() && userlist_node) {
        userlist_node->setWidget(userView);
        userlist_rect->setVisible(true);
    } else {
        if (userView)
            userView->hide();
        if (userlist_rect)
            userlist_rect->setVisible(false);
    }
}

#if 0
void
KThemedGreeter::verifyFailed()
{
//    goButton->setEnabled(false);
    inherited::verifyFailed();
}

void
KThemedGreeter::verifyRetry()
{
//    goButton->setEnabled(true);
    inherited::verifyRetry();
}
#endif

void
KThemedGreeter::updateStatus(bool fail, bool caps, int timedleft)
{
    if (pam_error)
        pam_error->setVisible(fail);
    if (caps_warning)
        caps_warning->setVisible(caps);
    if (timed_label) {
        if (timedleft) {
            if (timedleft != KdmLabel::timedDelay) {
                KdmLabel::timedDelay = timedleft;
                KdmLabel::timedUser = curUser;
                timed_label->setVisible(true);
                timed_label->update();
            }
        } else {
            KdmLabel::timedDelay = -1;
            timed_label->setVisible(false);
        }
    }
}

void
KThemedGreeter::slotThemeActivated(const QString &id)
{
    if (id == "login_button")
        accept();
    else if (id == "session_button")
        slotSessMenu();
    else if (id == "system_button")
        slotActionMenu();
}

void
KThemedGreeter::slotSessMenu()
{
    sessMenu->popup(mapToGlobal(session_button->rect().center()));
}

void
KThemedGreeter::slotActionMenu()
{
    if (system_button)
        optMenu->popup(mapToGlobal(system_button->rect().center()));
    else
        optMenu->popup(mapToGlobal(rect().center()));
}

void
KThemedGreeter::keyPressEvent(QKeyEvent *e)
{
    inherited::keyPressEvent(e);
    if (!(e->modifiers() & ~Qt::KeypadModifier) &&
        (e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter))
        accept();
}

#include "kgreeter.moc"
