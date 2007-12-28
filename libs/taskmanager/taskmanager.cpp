/*****************************************************************

Copyright (c) 2000 Matthias Elter <elter@kde.org>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

******************************************************************/

#include "taskmanager.h"

#include <QApplication>
#include <QCursor>
#include <QImage>
#include <QTimer>
#include <QX11Info>
#include <QDesktopWidget>
#include <QPixmap>
#include <QList>

#include <kapplication.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klocale.h>
// #include <kpixmapio.h>
#include <k3staticdeleter.h>
#include <kwindowsystem.h>
#include <netwm.h>

#include <config-workspace.h>
#include <config-X11.h>

#if defined(HAVE_XCOMPOSITE) && \
    defined(HAVE_XRENDER) && \
    defined(HAVE_XFIXES)
#include <X11/Xlib.h>
#include <X11/extensions/Xcomposite.h>
#include <X11/extensions/Xfixes.h>
#include <X11/extensions/Xrender.h>
#include <fixx11h.h>
#if XCOMPOSITE_VERSION >= 00200 && \
    XFIXES_VERSION >= 20000 && \
    (RENDER_MAJOR > 0 || RENDER_MINOR >= 6)
#define THUMBNAILING_POSSIBLE
#endif
#endif


TaskManager* TaskManager::m_self = 0;
static K3StaticDeleter<TaskManager> staticTaskManagerDeleter;
uint TaskManager::m_xCompositeEnabled = 0;

TaskManager* TaskManager::self()
{
    if (!m_self)
    {
        staticTaskManagerDeleter.setObject(m_self, new TaskManager());
    }
    return m_self;
}

class TaskManager::Private
{
public:
    Private()
        : active(0),
          startupInfo(0),
          trackGeometry(false)
    {
    }

    Task::TaskPtr active;
    KStartupInfo* startupInfo;
    Task::Dict tasksByWId;
    Startup::List startups;
    WindowList skiptaskbarWindows;
    bool trackGeometry;
};

TaskManager::TaskManager()
    : QObject(),
      d(new Private)
{
    KGlobal::locale()->insertCatalog("libtaskmanager");
    connect(KWindowSystem::self(), SIGNAL(windowAdded(WId)),
            this,       SLOT(windowAdded(WId)));
    connect(KWindowSystem::self(), SIGNAL(windowRemoved(WId)),
            this,       SLOT(windowRemoved(WId)));
    connect(KWindowSystem::self(), SIGNAL(activeWindowChanged(WId)),
            this,       SLOT(activeWindowChanged(WId)));
    connect(KWindowSystem::self(), SIGNAL(currentDesktopChanged(int)),
            this,       SLOT(currentDesktopChanged(int)));
    connect(KWindowSystem::self(), SIGNAL(windowChanged(WId,unsigned int)),
            this,       SLOT(windowChanged(WId,unsigned int)));

    // register existing windows
    const QList<WId> windows = KWindowSystem::windows();
    QList<WId>::ConstIterator end(windows.end());
    for (QList<WId>::ConstIterator it = windows.begin(); it != end; ++it)
    {
        windowAdded(*it);
    }

    // set active window
    WId win = KWindowSystem::activeWindow();
    activeWindowChanged(win);
    configure_startup();
}

TaskManager::~TaskManager()
{
    KGlobal::locale()->removeCatalog("libtaskmanager");
    delete d;
}

void TaskManager::configure_startup()
{
    KConfig _c( "klaunchrc" );
    KConfigGroup c(&_c, "FeedbackStyle");
    if (!c.readEntry("TaskbarButton", true))
        return;
    d->startupInfo = new KStartupInfo( KStartupInfo::CleanOnCantDetect, this );
    connect( d->startupInfo,
        SIGNAL( gotNewStartup( const KStartupInfoId&, const KStartupInfoData& )),
        SLOT( gotNewStartup( const KStartupInfoId&, const KStartupInfoData& )));
    connect( d->startupInfo,
        SIGNAL( gotStartupChange( const KStartupInfoId&, const KStartupInfoData& )),
        SLOT( gotStartupChange( const KStartupInfoId&, const KStartupInfoData& )));
    connect( d->startupInfo,
        SIGNAL( gotRemoveStartup( const KStartupInfoId&, const KStartupInfoData& )),
        SLOT( killStartup( const KStartupInfoId& )));
    c=KConfigGroup(&_c, "TaskbarButtonSettings");
    d->startupInfo->setTimeout( c.readEntry( "Timeout", 30 ));
}

#ifdef THUMBNAILING_POSSIBLE
void TaskManager::setXCompositeEnabled(bool state)
{
    Display *dpy = QX11Info::display();

    if (!state)
    {
        if (!--m_xCompositeEnabled)
        {
            // unredirecting windows
            for (int i = 0; i < ScreenCount(dpy); i++)
            {
                XCompositeUnredirectSubwindows(dpy, RootWindow(dpy, i),
                                                CompositeRedirectAutomatic);
            }
        }
        return;
    }

    if (m_xCompositeEnabled)
    {
        // we don't unlearn riding bike ;)
        m_xCompositeEnabled++;
        return;
    }

    // XComposite extension check
    int event_base, error_base;
    if (!XCompositeQueryExtension(dpy, &event_base, &error_base))
    {
        return;
    }

    int major = 0, minor = 99; // The highest version we support
    XCompositeQueryVersion(dpy, &major, &minor);

    // We use XCompositeNameWindowPixmap(), i.e.  we need at least
    // version 0.2.
    if (major == 0 && minor < 2)
    {
        return;
    }

    // XRender extension check
    if (!XRenderQueryExtension(dpy, &event_base, &error_base))
    {
        return;
    }

    major = 0, minor = 99; // The highest version we support
    XRenderQueryVersion(dpy, &major, &minor);

    // We use SetPictureTransform() and SetPictureFilter(), i.e. we
    // need at least version 0.6.
    if (major == 0 && minor < 6)
    {
        return;
    }

    // XFixes extension check
    if (!XFixesQueryExtension(dpy, &event_base, &error_base))
    {
        return;
    }

    major = 3, minor = 99; // The highest version we support
    XFixesQueryVersion(dpy, &major, &minor);

    // We use Region objects, i.e. we need at least version 2.0.
    if (major < 2)
    {
        return;
    }

    // if we get here, we've got usable extensions
    m_xCompositeEnabled++;

    // redirecting windows to backing pixmaps
    for (int i = 0; i < ScreenCount(dpy); i++)
    {
        XCompositeRedirectSubwindows(dpy, RootWindow(dpy, i),
                                     CompositeRedirectAutomatic);
    }

    Task::Dict::iterator itEnd = d->tasksByWId.end();
    for (Task::Dict::iterator it = d->tasksByWId.begin(); it != itEnd; ++it)
    {
        it.value()->updateWindowPixmap();
    }
}
#else // THUMBNAILING_POSSIBLE
void TaskManager::setXCompositeEnabled(bool)
{
}
#endif // !THUMBNAILING_POSSIBLE

Task::TaskPtr TaskManager::findTask(WId w)
{
    // TODO: might be able to be made more efficient if
    // we check to see if w is a transient first?
    // profiling would say whether this is worth the effort

    Task::Dict::iterator it = d->tasksByWId.begin();
    Task::Dict::iterator itEnd = d->tasksByWId.end();

    for (; it != itEnd; ++it)
    {
        if (it.key() == w || it.value()->hasTransient(w))
        {
            return it.value();
        }
    }

    return Task::TaskPtr();
}

Task::TaskPtr TaskManager::findTask(int desktop, const QPoint& p)
{
    QList<WId> list = KWindowSystem::stackingOrder();

    Task::TaskPtr task;
    int currentIndex = -1;
    Task::Dict::iterator itEnd = d->tasksByWId.end();
    for (Task::Dict::iterator it = d->tasksByWId.begin(); it != itEnd; ++it)
    {
        Task::TaskPtr t = it.value();
        if (!t->isOnAllDesktops() && t->desktop() != desktop)
        {
            continue;
        }

        if (t->isIconified() || t->isShaded())
        {
            continue;
        }

        if (t->geometry().contains(p))
        {
            int index = list.indexOf(t->window());
            if (index > currentIndex)
            {
                currentIndex = index;
                task = t;
            }
        }
    }

    return task;
}

void TaskManager::windowAdded(WId w )
{
    NETWinInfo info(QX11Info::display(), w, QX11Info::appRootWindow(),
                    NET::WMWindowType | NET::WMPid | NET::WMState);

    // ignore NET::Tool and other special window types
    NET::WindowType wType =
        info.windowType( NET::NormalMask | NET::DesktopMask | NET::DockMask |
                         NET::ToolbarMask | NET::MenuMask | NET::DialogMask |
                         NET::OverrideMask | NET::TopMenuMask |
                         NET::UtilityMask | NET::SplashMask );

    if (wType != NET::Normal &&
        wType != NET::Override &&
        wType != NET::Unknown &&
        wType != NET::Dialog &&
        wType != NET::Utility)
    {
        return;
    }

    // ignore windows that want to be ignored by the taskbar
    if ((info.state() & NET::SkipTaskbar) != 0)
    {
        d->skiptaskbarWindows.push_front( w ); // remember them though
        return;
    }

    Window transient_for_tmp;
    if (XGetTransientForHint( QX11Info::display(), (Window) w, &transient_for_tmp ))
    {
        WId transient_for = (WId) transient_for_tmp;

        // check if it's transient for a skiptaskbar window
        if( d->skiptaskbarWindows.contains( transient_for ))
            return;

        // lets see if this is a transient for an existing task
        if( transient_for != QX11Info::appRootWindow()
            && transient_for != 0
            && wType != NET::Utility )
        {
            Task::TaskPtr t = findTask(transient_for);
            if (t)
            {
                if (t->window() != w)
                {
                    t->addTransient(w, info);
                    // kDebug() << "TM: Transient " << w << " added for Task: " << t->window();
                }
                return;
            }
        }
    }

    Task::TaskPtr t( new Task( w, 0 ) );
    d->tasksByWId[w] = t;

    if (d->startupInfo) {
        KStartupInfoId startupInfoId;
        // checkStartup modifies startupInfoId
        d->startupInfo->checkStartup(w, startupInfoId);
        foreach (Startup::StartupPtr startup, d->startups) {
            if (startup->id() == startupInfoId) {
                startup->addWindowMatch(w);
            }
        }
    }

    // kDebug() << "TM: Task added for WId: " << w;

    emit taskAdded(t);
}

void TaskManager::windowRemoved(WId w)
{
    d->skiptaskbarWindows.removeAll(w);

    // find task
    Task::TaskPtr t = findTask(w);
    if (!t)
    {
        return;
    }

    if (t->window() == w)
    {
        d->tasksByWId.remove(w);
        emit taskRemoved(t);

        if (t == d->active)
        {
            d->active = 0;
        }

        //kDebug() << "TM: Task for WId " << w << " removed.";
    }
    else
    {
        t->removeTransient(w);
        //kDebug() << "TM: Transient " << w << " for Task " << t->window() << " removed.";
    }
}

void TaskManager::windowChanged(WId w, unsigned int dirty)
{
    if (dirty & NET::WMState)
    {
        NETWinInfo info (QX11Info::display(), w, QX11Info::appRootWindow(),
                         NET::WMState | NET::XAWMState);
        if (info.state() & NET::SkipTaskbar)
        {
            windowRemoved(w);
            d->skiptaskbarWindows.push_front(w);
            return;
        }
        else
        {
            d->skiptaskbarWindows.removeAll(w);
            if (info.mappingState() != NET::Withdrawn && !findTask(w))
            {
                // skipTaskBar state was removed and the window is still
                // mapped, so add this window
                windowAdded( w );
            }
        }
    }

    // check if any state we are interested in is marked dirty
    if (!(dirty & (NET::WMVisibleName |NET::WMName |
                   NET::WMState | NET::WMIcon |
                   NET::XAWMState | NET::WMDesktop) ||
          (d->trackGeometry && dirty & NET::WMGeometry)))
    {
        return;
    }

    // find task
    Task::TaskPtr t = findTask(w);
    if (!t)
    {
        return;
    }

    //kDebug() << "TaskManager::windowChanged " << w << " " << dirty;

    if (dirty & NET::WMState)
    {
        t->updateDemandsAttentionState(w);
    }

    // refresh icon pixmap if necessary
    if (dirty & NET::WMIcon)
    {
        t->refreshIcon();
        dirty ^= NET::WMIcon;
    }

    if (dirty)
    {
        // only refresh this stuff if we have other changes besides icons
        t->refresh(dirty);
    }

    if (dirty & (NET::WMDesktop | NET::WMState | NET::XAWMState))
    {
        // moved to different desktop or is on all or change in iconification/withdrawnnes
        emit windowChanged(t);

        if (m_xCompositeEnabled && dirty & NET::WMState)
        {
            // update on restoring a minimized window
            updateWindowPixmap(w);
        }

    }
    else if (dirty & NET::WMGeometry)
    {
        emit windowChangedGeometry(t);

        if (m_xCompositeEnabled)
        {
            // update on size changes, not on task drags
            updateWindowPixmap(w);
        }

    }
}

void TaskManager::updateWindowPixmap(WId w)
{
    if (!m_xCompositeEnabled)
    {
        return;
    }

    Task::TaskPtr task = findTask(w);
    if (task)
    {
        task->updateWindowPixmap();
    }
}

void TaskManager::activeWindowChanged(WId w )
{
    //kDebug() << "TaskManager::activeWindowChanged";

    Task::TaskPtr t = findTask( w );
    if (!t) {
        if (d->active) {
            d->active->setActive(false);
            d->active = 0;
        }
    }
    else {
        if (d->active)
            d->active->setActive(false);

        d->active = t;
        d->active->setActive(true);
    }
}

void TaskManager::currentDesktopChanged(int desktop)
{
    emit desktopChanged(desktop);
}

void TaskManager::gotNewStartup( const KStartupInfoId& id, const KStartupInfoData& data )
{
    Startup::StartupPtr s( new Startup( id, data, 0 ) );
    d->startups.append(s);

    emit startupAdded(s);
}

void TaskManager::gotStartupChange( const KStartupInfoId& id, const KStartupInfoData& data )
{
    Startup::List::iterator itEnd = d->startups.end();
    for (Startup::List::iterator sIt = d->startups.begin(); sIt != itEnd; ++sIt)
    {
        if ((*sIt)->id() == id)
        {
            (*sIt)->update(data);
            return;
        }
    }
}

void TaskManager::killStartup( const KStartupInfoId& id )
{
    Startup::List::iterator sIt = d->startups.begin();
    Startup::List::iterator itEnd = d->startups.end();
    Startup::StartupPtr s;
    for (; sIt != itEnd; ++sIt)
    {
        if ((*sIt)->id() == id)
        {
            s = *sIt;
            break;
        }
    }

    if (!s)
    {
        return;
    }

    d->startups.erase(sIt);
    emit startupRemoved(s);
}

void TaskManager::killStartup(Startup::StartupPtr s)
{
    if (!s)
    {
        return;
    }

    Startup::List::iterator sIt = d->startups.begin();
    Startup::List::iterator itEnd = d->startups.end();
    for (; sIt != itEnd; ++sIt)
    {
        if ((*sIt) == s)
        {
            d->startups.erase(sIt);
            break;
        }
    }

    emit startupRemoved(s);
}

QString TaskManager::desktopName(int desk) const
{
    return KWindowSystem::desktopName(desk);
}

Task::Dict TaskManager::tasks() const
{
    return d->tasksByWId;
}

Startup::List TaskManager::startups() const
{
    return d->startups;
}

int TaskManager::numberOfDesktops() const
{
    return KWindowSystem::numberOfDesktops();
}

bool TaskManager::isOnTop(const Task* task)
{
    if (!task)
    {
        return false;
    }

    QList<WId> list = KWindowSystem::stackingOrder();
    QList<WId>::const_iterator begin(list.constBegin());
    QList<WId>::const_iterator it = list.begin() + (list.size() - 1);
    do
    {
        Task::Dict::iterator taskItEnd = d->tasksByWId.end();
        for (Task::Dict::iterator taskIt = d->tasksByWId.begin();
             taskIt != taskItEnd; ++taskIt)
        {
            Task::TaskPtr t = taskIt.value();
            if ((*it) == t->window())
            {
                if (t == task)
                {
                    return true;
                }

                if (!t->isIconified() &&
                    (t->isAlwaysOnTop() == task->isAlwaysOnTop()))
                {
                    return false;
                }

                break;
            }
        }
    } while (it-- != begin);

    return false;
}

void TaskManager::trackGeometry()
{
    d->trackGeometry = true;
}

bool TaskManager::isOnScreen(int screen, const WId wid)
{
    if (screen == -1)
    {
        return true;
    }

    KWindowInfo wi = KWindowSystem::windowInfo(wid, NET::WMFrameExtents);

    // for window decos that fudge a bit and claim to extend beyond the
    // edge of the screen, we just contract a bit.
    QRect window = wi.frameGeometry();
    QRect desktop = QApplication::desktop()->screenGeometry(screen);
    desktop.adjust(5, 5, -5, -5);
    return window.intersects(desktop);
}

bool TaskManager::xCompositeEnabled()
{
    return m_xCompositeEnabled != 0;
}

class Task::Private
{
public:
    Private(WId w)
     : active(false),
       win(w),
       frameId(w),
       info(KWindowSystem::windowInfo(w,
            NET::WMState | NET::XAWMState | NET::WMDesktop |
            NET::WMVisibleName | NET::WMGeometry,
            NET::WM2AllowedActions)),
       lastWidth(0),
       lastHeight(0),
       lastResize(false),
       lastIcon(),
       thumbSize(0.2),
       thumb(),
       grab()
    {
    }

    bool active;
    WId win;
    WId frameId;
    QPixmap pixmap;
    KWindowInfo info;
    WindowList transients;
    WindowList transientsDemandingAttention;

    int lastWidth;
    int lastHeight;
    bool lastResize;
    QPixmap lastIcon;

    double thumbSize;
    QPixmap thumb;
    QPixmap grab;
    QRect iconGeometry;

    Pixmap windowPixmap;
};

Task::Task(WId w, QObject *parent, const char *name)
  : QObject(parent),
    d(new Private(w))
{
    setObjectName( name );

    // try to load icon via net_wm
    d->pixmap = KWindowSystem::icon(d->win, 16, 16, true);

    // try to guess the icon from the classhint
    if (d->pixmap.isNull())
    {
        KIconLoader::global()->loadIcon(className().toLower(),
                                                    KIconLoader::Small,
                                                    KIconLoader::Small,
                                                    KIconLoader::DefaultState,
                                                    QStringList(), 0, true);
    }

    // load the icon for X applications
    if (d->pixmap.isNull())
    {
        d->pixmap = SmallIcon("xorg");
    }

#ifdef THUMBNAILING_POSSIBLE
    d->windowPixmap = 0;
    findWindowFrameId();

    if (TaskManager::xCompositeEnabled())
    {
        updateWindowPixmap();
    }
#endif // THUMBNAILING_POSSIBLE
}

Task::~Task()
{
#ifdef THUMBNAILING_POSSIBLE
    if (d->windowPixmap)
    {
        XFreePixmap(QX11Info::display(), d->windowPixmap);
    }
#endif // THUMBNAILING_POSSIBLE
}

// Task::findWindowFrameId()
// Code was copied from Kompose.
// Copyright (C) 2004 Hans Oischinger
// Permission granted on 2005-04-27.
void Task::findWindowFrameId()
{
#ifdef THUMBNAILING_POSSIBLE
    Window targetWin, parent, root;
    Window *children;
    uint nchildren;

    targetWin = d->win;
    for (;;)
    {
        if (!XQueryTree(QX11Info::display(), targetWin, &root,
                        &parent, &children, &nchildren))
        {
            break;
        }

        if (children)
        {
            XFree(children); // it's a list, that's deallocated!
        }

        if (!parent || parent == root)
        {
            break;
        }
        else
        {
            targetWin = parent;
        }
    }

    d->frameId = targetWin;
#endif // THUMBNAILING_POSSIBLE
}

void Task::refreshIcon()
{
    // try to load icon via net_wm
    d->pixmap = KWindowSystem::icon(d->win, 16, 16, true);

    // try to guess the icon from the classhint
    if(d->pixmap.isNull())
    {
        KIconLoader::global()->loadIcon(className().toLower(),
                                                    KIconLoader::Small,
                                                    KIconLoader::Small,
                                                    KIconLoader::DefaultState,
                                                    QStringList(), 0, true);
    }

    // load the icon for X applications
    if (d->pixmap.isNull())
    {
        d->pixmap = SmallIcon("xorg");
    }

    d->lastIcon = QPixmap();
    emit iconChanged();
}

void Task::refresh(unsigned int dirty)
{
    QString name = visibleName();
    d->info = KWindowSystem::windowInfo(d->win,
        NET::WMState | NET::XAWMState | NET::WMDesktop | NET::WMVisibleName | NET::WMGeometry,
        NET::WM2AllowedActions);

    if (dirty != NET::WMName || name != visibleName())
    {
        emit changed();
    }
}

void Task::setActive(bool a)
{
    d->active = a;
    emit changed();
    if ( a )
      emit activated();
    else
      emit deactivated();
}

double Task::thumbnailSize() const { return d->thumbSize; }


void Task::setThumbnailSize( double size )
{
    d->thumbSize = size;
}

bool Task::hasThumbnail() const
{
    return !d->thumb.isNull();
}

QPixmap Task::thumbnail() const
{
    return d->thumb;
}

bool Task::isMaximized() const
{
    return d->info.valid() && (d->info.state() & NET::Max);
}

bool Task::isMinimized() const
{
    return d->info.valid() && d->info.isMinimized();
}

bool Task::isIconified() const
{
    return d->info.valid() && d->info.isMinimized();
}

bool Task::isAlwaysOnTop() const
{
    return d->info.valid() && (d->info.state() & NET::StaysOnTop);
}

bool Task::isKeptBelowOthers() const
{
    return d->info.valid() && (d->info.state() & NET::KeepBelow);
}

bool Task::isFullScreen() const
{
    return d->info.valid() && (d->info.state() & NET::FullScreen);
}

bool Task::isShaded() const
{
    return d->info.valid() && (d->info.state() & NET::Shaded);
}

bool Task::isOnCurrentDesktop() const
{
    return d->info.valid() && d->info.isOnCurrentDesktop();
}

bool Task::isOnAllDesktops() const
{
    return d->info.valid() && d->info.onAllDesktops();
}

bool Task::isActive() const
{
    return d->active;
}

bool Task::isOnTop() const
{
    return TaskManager::self()->isOnTop(this);
}

bool Task::isModified() const
{
  static QString modStr = QString::fromUtf8("[") +
                          i18n("modified") +
                          QString::fromUtf8("]");
  int modStrPos = d->info.visibleName().indexOf(modStr);

  return ( modStrPos != -1 );
}

int Task::desktop() const
{
    return d->info.desktop();
}

bool Task::demandsAttention() const
{
    return (d->info.valid() && (d->info.state() & NET::DemandsAttention)) ||
            d->transientsDemandingAttention.count() > 0;
}

bool Task::isOnScreen( int screen ) const
{
    return TaskManager::isOnScreen( screen, d->win );
}

bool Task::showInTaskbar() const
{
    return d->info.state() ^ NET::SkipTaskbar;
}

bool Task::showInPager() const
{
    return d->info.state() ^ NET::SkipPager;
}

QRect Task::geometry() const
{
    return d->info.geometry();
}

void Task::updateDemandsAttentionState( WId w )
{
    if (window() != w)
    {
        // 'w' is a transient for this task
        NETWinInfo i( QX11Info::display(), w, QX11Info::appRootWindow(), NET::WMState );
        if(i.state() & NET::DemandsAttention)
        {
            if (!d->transientsDemandingAttention.contains(w))
            {
                d->transientsDemandingAttention.append(w);
            }
        }
        else
        {
            d->transientsDemandingAttention.removeAll( w );
        }
    }
}

void Task::addTransient( WId w, const NETWinInfo& info )
{
    d->transients.append(w);
    if (info.state() & NET::DemandsAttention)
    {
        d->transientsDemandingAttention.append(w);
        emit changed();
    }
}

void Task::removeTransient(WId w)
{
    d->transients.removeAll(w);
    d->transientsDemandingAttention.removeAll(w);
}

bool Task::hasTransient(WId w) const
{
    return d->transients.indexOf(w) != -1;
}

WId Task::window() const
{
    return d->win;
}

KWindowInfo Task::info() const
{
    return d->info;
}

QString Task::visibleName() const
{
    return d->info.visibleName();
}

QString Task::visibleNameWithState() const
{
    return d->info.visibleNameWithState();
}

QString Task::name() const
{
    return d->info.name();
}

QString Task::className()
{
    XClassHint hint;
    if(XGetClassHint(QX11Info::display(), d->win, &hint)) {
        QString nh( hint.res_name );
        XFree( hint.res_name );
        XFree( hint.res_class );
        return nh;
    }
    return QString();
}

QString Task::classClass()
{
    XClassHint hint;
    if(XGetClassHint(QX11Info::display(), d->win, &hint)) {
        QString ch( hint.res_class );
        XFree( hint.res_name );
        XFree( hint.res_class );
        return ch;
    }
    return QString();
}

QPixmap Task::icon( int width, int height, bool allowResize )
{
  if ( (width == d->lastWidth)
       && (height == d->lastHeight)
       && (allowResize == d->lastResize )
       && (!d->lastIcon.isNull()) )
    return d->lastIcon;

  QPixmap newIcon = KWindowSystem::icon( d->win, width, height, allowResize );
  if ( !newIcon.isNull() ) {
    d->lastIcon = newIcon;
    d->lastWidth = width;
    d->lastHeight = height;
    d->lastResize = allowResize;
  }

  return newIcon;
}

WindowList Task::transients() const
{
    return d->transients;
}

QPixmap Task::pixmap() const
{
    return d->pixmap;
}

QPixmap Task::bestIcon( int size, bool &isStaticIcon )
{
  QPixmap pixmap;
  isStaticIcon = false;

  switch( size ) {
  case KIconLoader::SizeSmall:
    {
      pixmap = icon( 16, 16, true  );

      // Icon of last resort
      if( pixmap.isNull() ) {
        pixmap = KIconLoader::global()->loadIcon( "xorg",
                                                  KIconLoader::NoGroup,
                                                  KIconLoader::SizeSmall );
        isStaticIcon = true;
      }
    }
    break;
  case KIconLoader::SizeMedium:
    {
      //
      // Try 34x34 first for KDE 2.1 icons with shadows, if we don't
      // get one then try 32x32.
      //
      pixmap = icon( 34, 34, false  );

      if ( (( pixmap.width() != 34 ) || ( pixmap.height() != 34 )) &&
           (( pixmap.width() != 32 ) || ( pixmap.height() != 32 )) )
      {
        pixmap = icon( 32, 32, true  );
      }

      // Icon of last resort
      if( pixmap.isNull() ) {
        pixmap = KIconLoader::global()->loadIcon( "xorg",
                            KIconLoader::NoGroup,
                            KIconLoader::SizeMedium );
        isStaticIcon = true;
      }
    }
    break;
  case KIconLoader::SizeLarge:
    {
      // If there's a 48x48 icon in the hints then use it
      pixmap = icon( size, size, false  );

      // If not, try to get one from the classname
      if ( pixmap.isNull() || pixmap.width() != size || pixmap.height() != size ) {
        pixmap = KIconLoader::global()->loadIcon( className(),
                            KIconLoader::NoGroup,
                            size,
                            KIconLoader::DefaultState,
                            QStringList(), 0L,
                            true );
        isStaticIcon = true;
      }

      // If we still don't have an icon then scale the one in the hints
      if ( pixmap.isNull() || ( pixmap.width() != size ) || ( pixmap.height() != size ) ) {
        pixmap = icon( size, size, true  );
        isStaticIcon = false;
      }

      // Icon of last resort
      if( pixmap.isNull() ) {
        pixmap = KIconLoader::global()->loadIcon( "xorg",
                                                  KIconLoader::NoGroup,
                                                  size );
        isStaticIcon = true;
      }
    }
  }

  return pixmap;
}

bool Task::idMatch( const QString& id1, const QString& id2 )
{
  if ( id1.isEmpty() || id2.isEmpty() )
    return false;

  if ( id1.contains( id2 ) > 0 )
    return true;

  if ( id2.contains( id1 ) > 0 )
    return true;

  return false;
}

void Task::move()
{
    bool on_current = d->info.isOnCurrentDesktop();

    if (!on_current)
    {
        KWindowSystem::setCurrentDesktop(d->info.desktop());
        KWindowSystem::forceActiveWindow(d->win);
    }

    if (d->info.isMinimized())
    {
        KWindowSystem::unminimizeWindow(d->win);
    }

    QRect geom = d->info.geometry();
    QCursor::setPos(geom.center());

    NETRootInfo ri(QX11Info::display(), NET::WMMoveResize);
    ri.moveResizeRequest(d->win, geom.center().x(),
                         geom.center().y(), NET::Move);
}

void Task::resize()
{
    bool on_current = d->info.isOnCurrentDesktop();

    if (!on_current)
    {
        KWindowSystem::setCurrentDesktop(d->info.desktop());
        KWindowSystem::forceActiveWindow(d->win);
    }

    if (d->info.isMinimized())
    {
        KWindowSystem::unminimizeWindow(d->win);
    }

    QRect geom = d->info.geometry();
    QCursor::setPos(geom.bottomRight());

    NETRootInfo ri(QX11Info::display(), NET::WMMoveResize);
    ri.moveResizeRequest(d->win, geom.bottomRight().x(),
                         geom.bottomRight().y(), NET::BottomRight);
}

void Task::setMaximized(bool maximize)
{
    KWindowInfo info = KWindowSystem::windowInfo(d->win, NET::WMState | NET::XAWMState | NET::WMDesktop);
    bool on_current = info.isOnCurrentDesktop();

    if (!on_current)
    {
        KWindowSystem::setCurrentDesktop(info.desktop());
    }

    if (info.isMinimized())
    {
        KWindowSystem::unminimizeWindow(d->win);
    }

    NETWinInfo ni(QX11Info::display(), d->win, QX11Info::appRootWindow(), NET::WMState);

    if (maximize)
    {
        ni.setState(NET::Max, NET::Max);
    }
    else
    {
        ni.setState(0, NET::Max);
    }

    if (!on_current)
    {
        KWindowSystem::forceActiveWindow(d->win);
    }
}

void Task::toggleMaximized()
{
    setMaximized(!isMaximized());
}

void Task::restore()
{
    KWindowInfo info = KWindowSystem::windowInfo(d->win, NET::WMState | NET::XAWMState | NET::WMDesktop);
    bool on_current = info.isOnCurrentDesktop();

    if (!on_current)
    {
        KWindowSystem::setCurrentDesktop(info.desktop());
    }

    if( info.isMinimized())
    {
        KWindowSystem::unminimizeWindow(d->win);
    }

    NETWinInfo ni(QX11Info::display(), d->win, QX11Info::appRootWindow(), NET::WMState);
    ni.setState(0, NET::Max);

    if (!on_current)
    {
        KWindowSystem::forceActiveWindow( d->win );
    }
}

void Task::setIconified(bool iconify)
{
    if (iconify)
    {
        KWindowSystem::minimizeWindow(d->win);
    }
    else
    {
        KWindowInfo info = KWindowSystem::windowInfo(d->win, NET::WMState | NET::XAWMState | NET::WMDesktop);
        bool on_current = info.isOnCurrentDesktop();

        if (!on_current)
        {
            KWindowSystem::setCurrentDesktop(info.desktop());
        }

        KWindowSystem::unminimizeWindow(d->win);

        if (!on_current)
        {
            KWindowSystem::forceActiveWindow(d->win);
        }
    }
}

void Task::toggleIconified()
{
    setIconified(!isIconified());
}

void Task::close()
{
    NETRootInfo ri( QX11Info::display(), NET::CloseWindow );
    ri.closeWindowRequest( d->win );
}

void Task::raise()
{
//    kDebug(1210) << "Task::raise(): " << name();
    KWindowSystem::raiseWindow( d->win );
}

void Task::lower()
{
//    kDebug(1210) << "Task::lower(): " << name();
    KWindowSystem::lowerWindow( d->win );
}

void Task::activate()
{
//    kDebug(1210) << "Task::activate():" << name();
    WId w = d->win;
    if (d->transientsDemandingAttention.count() > 0)
    {
        w = d->transientsDemandingAttention.last();
    }
    KWindowSystem::forceActiveWindow( w );
}

void Task::activateRaiseOrIconify()
{
    if (!isActive() || isIconified())
    {
        activate();
    }
    else if (!isOnTop())
    {
       raise();
    }
    else
    {
       setIconified(true);
    }
}

void Task::toDesktop(int desk)
{
    NETWinInfo ni(QX11Info::display(), d->win, QX11Info::appRootWindow(), NET::WMDesktop);
    if (desk == 0)
    {
        if (d->info.valid() && d->info.onAllDesktops())
        {
            ni.setDesktop(KWindowSystem::currentDesktop());
            KWindowSystem::forceActiveWindow(d->win);
        }
        else
        {
            ni.setDesktop(NETWinInfo::OnAllDesktops);
        }

        return;
    }
    ni.setDesktop(desk);
    if(desk == KWindowSystem::currentDesktop())
        KWindowSystem::forceActiveWindow(d->win);
}

void Task::toCurrentDesktop()
{
    toDesktop(KWindowSystem::currentDesktop());
}

void Task::setAlwaysOnTop(bool stay)
{
    NETWinInfo ni( QX11Info::display(), d->win, QX11Info::appRootWindow(), NET::WMState);
    if(stay)
        ni.setState( NET::StaysOnTop, NET::StaysOnTop );
    else
        ni.setState( 0, NET::StaysOnTop );
}

void Task::toggleAlwaysOnTop()
{
    setAlwaysOnTop( !isAlwaysOnTop() );
}

void Task::setKeptBelowOthers(bool below)
{
    NETWinInfo ni(QX11Info::display(), d->win, QX11Info::appRootWindow(), NET::WMState);

    if (below)
    {
        ni.setState(NET::KeepBelow, NET::KeepBelow);
    }
    else
    {
        ni.setState(0, NET::KeepBelow);
    }
}

void Task::toggleKeptBelowOthers()
{
    setKeptBelowOthers(!isKeptBelowOthers());
}

void Task::setFullScreen(bool fullscreen)
{
    NETWinInfo ni(QX11Info::display(), d->win, QX11Info::appRootWindow(), NET::WMState);

    if (fullscreen)
    {
        ni.setState(NET::FullScreen, NET::FullScreen);
    }
    else
    {
        ni.setState(0, NET::FullScreen);
    }
}

void Task::toggleFullScreen()
{
    setFullScreen(!isFullScreen());
}

void Task::setShaded(bool shade)
{
    NETWinInfo ni( QX11Info::display(), d->win, QX11Info::appRootWindow(), NET::WMState);
    if(shade)
        ni.setState( NET::Shaded, NET::Shaded );
    else
        ni.setState( 0, NET::Shaded );
}

void Task::toggleShaded()
{
    setShaded( !isShaded() );
}

void Task::publishIconGeometry(QRect rect)
{
    if (rect == d->iconGeometry)
    {
        return;
    }

    d->iconGeometry = rect;
    NETWinInfo ni(QX11Info::display(), d->win, QX11Info::appRootWindow(), 0);
    NETRect r;

    if (rect.isValid())
    {
        r.pos.x = rect.x();
        r.pos.y = rect.y();
        r.size.width = rect.width();
        r.size.height = rect.height();
    }
    ni.setIconGeometry(r);
}

void Task::updateThumbnail()
{
    if ( !d->info.valid() ||
            !isOnCurrentDesktop() ||
            !isActive() ||
            !d->grab.isNull() ) // We're already processing one...
    {
        return;
    }

    //
    // We do this as a two stage process to remove the delay caused
    // by the thumbnail generation. This makes things much smoother
    // on slower machines.
    //
    QWidget *rootWin = qApp->desktop();
    QRect geom = d->info.geometry();
    d->grab = QPixmap::grabWindow(rootWin->winId(),
                                geom.x(), geom.y(),
                                geom.width(), geom.height());

    if (!d->grab.isNull())
    {
       QTimer::singleShot(200, this, SLOT(generateThumbnail()));
    }
}

void Task::generateThumbnail()
{
   if ( d->grab.isNull() )
      return;

   double width = d->grab.width();
   double height = d->grab.height();
   width = width * d->thumbSize;
   height = height * d->thumbSize;

   d->thumb = d->grab.scaled( qRound(width), qRound(height), Qt::IgnoreAspectRatio, Qt::SmoothTransformation );
   d->grab = QPixmap(); // Makes grab a null image.

   emit thumbnailChanged();
}

#ifdef THUMBNAILING_POSSIBLE
QPixmap Task::thumbnail(int maxDimension)
{
    if (!TaskManager::xCompositeEnabled() || !d->windowPixmap)
    {
        return QPixmap();
    }

    Display *dpy = QX11Info::display();

    XWindowAttributes winAttr;
    XGetWindowAttributes(dpy, d->frameId, &winAttr);
    XRenderPictFormat *format = XRenderFindVisualFormat(dpy, winAttr.visual);

    XRenderPictureAttributes picAttr;
    picAttr.subwindow_mode = IncludeInferiors; // Don't clip child widgets

    Picture picture = XRenderCreatePicture(dpy, d->windowPixmap, format,
                                           CPSubwindowMode, &picAttr);

    // Get shaped windows handled correctly.
    XserverRegion region = XFixesCreateRegionFromWindow(dpy, d->frameId,
                                                        WindowRegionBounding);
    XFixesSetPictureClipRegion(dpy, picture, 0, 0, region);
    XFixesDestroyRegion(dpy, region);

    double factor;
    if (winAttr.width > winAttr.height)
    {
        factor = (double)maxDimension / (double)winAttr.width;
    }
    else
    {
        factor = (double)maxDimension / (double)winAttr.height;
    }
    int thumbnailWidth = (int)(winAttr.width * factor);
    int thumbnailHeight = (int)(winAttr.height * factor);

    QPixmap thumbnail(thumbnailWidth, thumbnailHeight);
    thumbnail.fill(QApplication::palette().color(QPalette::Active,QPalette::Background));

#if 0 // QImage::smoothScale() scaling
    QPixmap full(winAttr.width, winAttr.height);
    full.fill(QApplication::palette().active().background());

    bool hasAlpha = format->type == PictTypeDirect && format->direct.alphaMask;

    XRenderComposite(dpy,
                     hasAlpha ? PictOpOver : PictOpSrc,
                     picture, // src
                     None, // mask
                     full.x11RenderHandle(), // dst
                     0, 0, // src offset
                     0, 0, // mask offset
                     0, 0, // dst offset
                     winAttr.width, winAttr.height);

    KPixmapIO io;
    QImage image = io.toImage(full);
    thumbnail = io.convertToPixmap(image.smoothScale(thumbnailWidth,
                                                     thumbnailHeight));
#else // XRENDER scaling
    // Scaling matrix
    XTransform transformation = {{
        { XDoubleToFixed(1), XDoubleToFixed(0), XDoubleToFixed(     0) },
        { XDoubleToFixed(0), XDoubleToFixed(1), XDoubleToFixed(     0) },
        { XDoubleToFixed(0), XDoubleToFixed(0), XDoubleToFixed(factor) }
    }};

    XRenderSetPictureTransform(dpy, picture, &transformation);
    XRenderSetPictureFilter(dpy, picture, FilterBest, 0, 0);

    XRenderComposite(QX11Info::display(),
                     PictOpOver, // we're filtering, alpha values are probable
                     picture, // src
                     None, // mask
                     thumbnail.x11PictureHandle(), // dst
                     0, 0, // src offset
                     0, 0, // mask offset
                     0, 0, // dst offset
                     thumbnailWidth, thumbnailHeight);
#endif
    XRenderFreePicture(dpy, picture);

    return thumbnail;
}
#else // THUMBNAILING_POSSIBLE
QPixmap Task::thumbnail(int /* maxDimension */)
{
    return QPixmap();
}
#endif // THUMBNAILING_POSSIBLE

void Task::updateWindowPixmap()
{
#ifdef THUMBNAILING_POSSIBLE
    if (!TaskManager::xCompositeEnabled() || !isOnCurrentDesktop() ||
        isMinimized())
    {
        return;
    }

    Display *dpy = QX11Info::display();

    if (d->windowPixmap)
    {
        XFreePixmap(dpy, d->windowPixmap);
    }

    d->windowPixmap = XCompositeNameWindowPixmap(dpy, d->frameId);
#endif // THUMBNAILING_POSSIBLE
}

class Startup::Private
{
public:
    Private(const KStartupInfoId& id, const KStartupInfoData& data)
        : id(id), data(data)
    {
    }

    KStartupInfoId id;
    KStartupInfoData data;
    QSet<WId> windowMatches;
};

Startup::Startup(const KStartupInfoId& id, const KStartupInfoData& data,
                 QObject * parent, const char *name)
    : QObject(parent),
      d(new Private(id, data))
{
    setObjectName( name );
}

Startup::~Startup()
{
    delete d;
}

QString Startup::text() const
{
    return d->data.findName();
}

QString Startup::bin() const
{
    return d->data.bin();
}

QString Startup::icon() const
{
    return d->data.findIcon();
}

void Startup::update(const KStartupInfoData& data)
{
    d->data.update(data);
    emit changed();
}

KStartupInfoId Startup::id() const
{
    return d->id;
}

void Startup::addWindowMatch(WId window)
{
    d->windowMatches.insert(window);
}

bool Startup::matchesWindow(WId window) const
{
    return d->windowMatches.contains(window);
}

int TaskManager::currentDesktop() const
{
    return KWindowSystem::currentDesktop();
}

TaskDrag::TaskDrag(const Task::List& tasks, QWidget* source)
  : QDrag(source),
    d(0)
{
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);

    stream.setVersion(QDataStream::Qt_3_1);

    Task::List::const_iterator itEnd = tasks.constEnd();
    for (Task::List::const_iterator it = tasks.constBegin(); it != itEnd; ++it)
    {
        stream << (quint32)(*it)->window();
    }

    QMimeData* mimeData = new QMimeData();
    mimeData->setData("taskbar/task", data);
    setMimeData(mimeData);
}

TaskDrag::~TaskDrag()
{
}

bool TaskDrag::canDecode(const QMimeData* e)
{
    return e->hasFormat("taskbar/task");
}

Task::List TaskDrag::decode( const QMimeData* e )
{
    QByteArray data(e->data("taskbar/task"));
    Task::List tasks;

    if (data.size())
    {
        QDataStream stream(data);
        while (!stream.atEnd())
        {
            quint32 id;
            stream >> id;
            if (Task::TaskPtr task = TaskManager::self()->findTask(id))
            {
                tasks.append(task);
            }
        }
    }

    return tasks;
}

#include "taskmanager.moc"
