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

#include <qapplication.h>
#include <qcursor.h>
#include <qimage.h>
#include <qtimer.h>

#include <kconfig.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klocale.h>
// #include <kpixmapio.h>
#include <kstaticdeleter.h>
#include <kwinmodule.h>
#include <netwm.h>

#include "taskmanager.h"
#include "taskmanager.moc"

TaskManager* TaskManager::m_self = 0;
static KStaticDeleter<TaskManager> staticTaskManagerDeleter;
uint TaskManager::m_xCompositeEnabled = 0;

TaskManager* TaskManager::the()
{
    if (!m_self)
    {
        staticTaskManagerDeleter.setObject(m_self, new TaskManager());
    }
    return m_self;
}

TaskManager::TaskManager()
    : QObject(),
      _active(0),
      _startup_info(0),
      m_winModule(new KWinModule()),
      m_trackGeometry(false)
{
    KGlobal::locale()->insertCatalogue("libtaskmanager");
    connect(m_winModule, SIGNAL(windowAdded(WId)),
            this,        SLOT(windowAdded(WId)));
    connect(m_winModule, SIGNAL(windowRemoved(WId)),
            this,        SLOT(windowRemoved(WId)));
    connect(m_winModule, SIGNAL(activeWindowChanged(WId)),
            this,        SLOT(activeWindowChanged(WId)));
    connect(m_winModule, SIGNAL(currentDesktopChanged(int)),
            this,        SLOT(currentDesktopChanged(int)));
    connect(m_winModule, SIGNAL(windowChanged(WId,unsigned int)),
            this,        SLOT(windowChanged(WId,unsigned int)));

    // register existing windows
    const QValueList<WId> windows = m_winModule->windows();
    QValueList<WId>::ConstIterator end(windows.end());
    for (QValueList<WId>::ConstIterator it = windows.begin(); it != end; ++it)
    {
        windowAdded(*it);
    }

    // set active window
    WId win = m_winModule->activeWindow();
    activeWindowChanged(win);
    configure_startup();
}

TaskManager::~TaskManager()
{
    KGlobal::locale()->removeCatalogue("libtaskmanager");
}

void TaskManager::configure_startup()
{
    KConfig c("klaunchrc", true);
    c.setGroup("FeedbackStyle");
    if (!c.readBoolEntry("TaskbarButton", true))
        return;
    _startup_info = new KStartupInfo( KStartupInfo::CleanOnCantDetect, this );
    connect( _startup_info,
        SIGNAL( gotNewStartup( const KStartupInfoId&, const KStartupInfoData& )),
        SLOT( gotNewStartup( const KStartupInfoId&, const KStartupInfoData& )));
    connect( _startup_info,
        SIGNAL( gotStartupChange( const KStartupInfoId&, const KStartupInfoData& )),
        SLOT( gotStartupChange( const KStartupInfoId&, const KStartupInfoData& )));
    connect( _startup_info,
        SIGNAL( gotRemoveStartup( const KStartupInfoId&, const KStartupInfoData& )),
        SLOT( killStartup( const KStartupInfoId& )));
    c.setGroup( "TaskbarButtonSettings" );
    _startup_info->setTimeout( c.readUnsignedNumEntry( "Timeout", 30 ));
}

void TaskManager::setXCompositeEnabled(bool state)
{
#ifdef THUMBNAILING_POSSIBLE
    Display *dpy = QPaintDevice::x11AppDisplay();

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

    Task::Dict::iterator itEnd = m_tasksByWId.end();
    for (Task::Dict::iterator it = m_tasksByWId.begin(); it != itEnd; ++it)
    {
        it.data()->updateWindowPixmap();
    }
#endif // THUMBNAILING_POSSIBLE
}

Task::Ptr TaskManager::findTask(WId w)
{
    // TODO: might be able to be made more efficient if
    // we check to see if w is a transient first?
    // profiling would say whether this is worth the effort

    Task::Dict::iterator it = m_tasksByWId.begin();
    Task::Dict::iterator itEnd = m_tasksByWId.end();

    if (it != itEnd)
    {
        return it.data();
    }

    for (; it !=itEnd; ++it)
    {
        if (it.key() == w || it.data()->hasTransient(w))
        {
            return it.data();
        }
    }

    return 0;
}

Task::Ptr TaskManager::findTask(int desktop, const QPoint& p)
{
    QValueList<WId> list = winModule()->stackingOrder();

    Task::Ptr task = 0;
    int currentIndex = -1;
    Task::Dict::iterator itEnd = m_tasksByWId.end();
    for (Task::Dict::iterator it = m_tasksByWId.begin(); it != itEnd; ++it)
    {
        Task::Ptr t = it.data();
        if (!t->isOnAllDesktops() && t->desktop() != desktop)
        {
            continue;
        }

        if (t->geometry().contains(p))
        {
            int index = list.findIndex(t->window());
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
    NETWinInfo info(qt_xdisplay(),  w, qt_xrootwin(),
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
        _skiptaskbar_windows.push_front( w ); // remember them though
        return;
    }

    Window transient_for_tmp;
    if (XGetTransientForHint( qt_xdisplay(), (Window) w, &transient_for_tmp ))
    {
        WId transient_for = (WId) transient_for_tmp;

        // check if it's transient for a skiptaskbar window
        if( _skiptaskbar_windows.contains( transient_for ))
            return;

        // lets see if this is a transient for an existing task
        if( transient_for != qt_xrootwin()
            && transient_for != 0
            && wType != NET::Utility )
        {
            Task::Ptr t = findTask(transient_for);
            if (t)
            {
                if (t->window() != w)
                {
                    t->addTransient(w, info);
                    // kdDebug() << "TM: Transient " << w << " added for Task: " << t->window() << endl;
                }
                return;
            }
        }
    }

    Task::Ptr t = new Task(w, this);
    m_tasksByWId[w] = t;

    // kdDebug() << "TM: Task added for WId: " << w << endl;

    emit taskAdded(t);
}

void TaskManager::windowRemoved(WId w)
{
    _skiptaskbar_windows.remove(w);

    // find task
    Task::Ptr t = findTask(w);
    if (!t)
    {
        return;
    }

    if (t->window() == w)
    {
        m_tasksByWId.remove(w);
        emit taskRemoved(t);

        if (t == _active)
        {
            _active = 0;
        }

        //kdDebug() << "TM: Task for WId " << w << " removed." << endl;
    }
    else
    {
        t->removeTransient(w);
        //kdDebug() << "TM: Transient " << w << " for Task " << t->window() << " removed." << endl;
    }
}

void TaskManager::windowChanged(WId w, unsigned int dirty)
{
    if (dirty & NET::WMState)
    {
        NETWinInfo info (qt_xdisplay(),  w, qt_xrootwin(),
                         NET::WMState | NET::XAWMState);
        if (info.state() & NET::SkipTaskbar)
        {
            windowRemoved(w);
            _skiptaskbar_windows.push_front(w);
            return;
        }
        else
        {
            _skiptaskbar_windows.remove(w);
            if (info.mappingState() != NET::Withdrawn && !findTask(w))
            {
                // skipTaskBar state was removed and the window is still
                // mapped, so add this window
                windowAdded( w );
            }
        }
    }

    // check if any state we are interested in is marked dirty
    if (!(dirty & (NET::WMVisibleName | NET::WMVisibleIconName | NET::WMName |
                   NET::WMIconName | NET::WMState | NET::WMIcon |
                   NET::XAWMState | NET::WMDesktop) ||
          (m_trackGeometry && dirty & NET::WMGeometry)))
    {
        return;
    }

    // find task
    Task::Ptr t = findTask(w);
    if (!t)
    {
        return;
    }

    //kdDebug() << "TaskManager::windowChanged " << w << " " << dirty << endl;

    if (dirty & NET::WMState)
    {
        t->updateDemandsAttentionState(w);
    }

    // refresh icon pixmap if necessary
    if (dirty & NET::WMIcon)
    {
        t->refreshIcon();
    }

    if (dirty != NET::WMIcon)
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

    Task::Ptr task = findTask(w);
    if (task)
    {
        task->updateWindowPixmap();
    }
}

void TaskManager::activeWindowChanged(WId w )
{
    //kdDebug() << "TaskManager::activeWindowChanged" << endl;

    Task::Ptr t = findTask( w );
    if (!t) {
        if (_active) {
            _active->setActive(false);
            _active = 0;
        }
    }
    else {
        if (_active)
            _active->setActive(false);

        _active = t;
        _active->setActive(true);
    }
}

void TaskManager::currentDesktopChanged(int desktop)
{
    emit desktopChanged(desktop);
}

void TaskManager::gotNewStartup( const KStartupInfoId& id, const KStartupInfoData& data )
{
    Startup::Ptr s = new Startup( id, data, this );
    _startups.append(s);

    emit startupAdded(s);
}

void TaskManager::gotStartupChange( const KStartupInfoId& id, const KStartupInfoData& data )
{
    Startup::List::iterator itEnd = _startups.end();
    for (Startup::List::iterator sIt = _startups.begin(); sIt != itEnd; ++sIt)
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
    Startup::List::iterator itEnd = _startups.end();
    Startup::Ptr s = 0;
    for (Startup::List::iterator sIt = _startups.begin(); sIt != itEnd; ++sIt)
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

    _startups.remove(s);
    emit startupRemoved(s);
}

void TaskManager::killStartup(Startup::Ptr s)
{
    if (!s)
    {
        return;
    }

    _startups.remove(s);
    emit startupRemoved(s);
}

QString TaskManager::desktopName(int desk) const
{
    return m_winModule->desktopName(desk);
}

int TaskManager::numberOfDesktops() const
{
    return m_winModule->numberOfDesktops();
}

bool TaskManager::isOnTop(const Task* task)
{
    if (!task)
    {
        return false;
    }

    QValueList<WId>::ConstIterator begin(m_winModule->stackingOrder().constBegin());
    QValueList<WId>::ConstIterator it = m_winModule->stackingOrder().fromLast();
    do
    {
        Task::Dict::iterator taskItEnd = m_tasksByWId.end();
        for (Task::Dict::iterator taskIt = m_tasksByWId.begin();
             taskIt != taskItEnd; ++taskIt)
        {
            Task::Ptr t = taskIt.data();
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

bool TaskManager::isOnScreen(int screen, const WId wid)
{
    if (screen == -1)
    {
        return true;
    }

    KWin::WindowInfo wi = KWin::windowInfo(wid, NET::WMKDEFrameStrut);

    // for window decos that fudge a bit and claim to extend beyond the
    // edge of the screen, we just contract a bit.
    QRect window = wi.frameGeometry();
    QRect desktop = QApplication::desktop()->screenGeometry(screen);
    desktop.addCoords(5, 5, -5, -5);
    return window.intersects(desktop);
}

Task::Task(WId win, QObject *parent, const char *name)
  : QObject(parent, name),
    _active(false),
    _win(win),
    m_frameId(win),
    _info(KWin::windowInfo(_win, 0, NET::WM2AllowedActions)),
    _lastWidth(0),
    _lastHeight(0),
    _lastResize(false),
    _lastIcon(),
    _thumbSize(0.2),
    _thumb(),
    _grab()
{
    // try to load icon via net_wm
    _pixmap = KWin::icon(_win, 16, 16, true);

    // try to guess the icon from the classhint
    if(_pixmap.isNull())
    {
        KGlobal::iconLoader()->loadIcon(className().lower(),
                                                    KIcon::Small,
                                                    KIcon::Small,
                                                    KIcon::DefaultState,
                                                    0, true);
    }

    // load xapp icon
    if (_pixmap.isNull())
    {
        _pixmap = SmallIcon("kcmx");
    }

#ifdef THUMBNAILING_POSSIBLE
    m_windowPixmap = 0;
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
    if (m_windowPixmap)
    {
        XFreePixmap(QPaintDevice::x11AppDisplay(), m_windowPixmap);
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
    Window target_win, parent, root;
    Window *children;
    uint nchildren;

    target_win = _win;
    for (;;)
    {
        if (!XQueryTree(QPaintDevice::x11AppDisplay(), target_win, &root,
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
            target_win = parent;
        }
    }

    m_frameId = target_win;
#endif // THUMBNAILING_POSSIBLE
}

void Task::refreshIcon()
{
    // try to load icon via net_wm
    _pixmap = KWin::icon(_win, 16, 16, true);

    // try to guess the icon from the classhint
    if(_pixmap.isNull())
    {
        KGlobal::iconLoader()->loadIcon(className().lower(),
                                                    KIcon::Small,
                                                    KIcon::Small,
                                                    KIcon::DefaultState,
                                                    0, true);
    }

    // load xapp icon
    if (_pixmap.isNull())
    {
        _pixmap = SmallIcon("kcmx");
    }

    _lastIcon.resize(0,0);
    emit iconChanged();
}

void Task::refresh(unsigned int dirty)
{
    QString name = visibleIconicName();
    _info = KWin::windowInfo(_win, 0, NET::WM2AllowedActions);

    if (dirty != NET::WMName || name != visibleIconicName())
    {
        emit changed();
    }
}

void Task::setActive(bool a)
{
    _active = a;
    emit changed();
    if ( a )
      emit activated();
    else
      emit deactivated();
}

bool Task::isMaximized() const
{
    return _info.valid() && (_info.state() & NET::Max);
}

bool Task::isMinimized() const
{
    return _info.valid() && _info.isMinimized();
}

bool Task::isIconified() const
{
    return _info.valid() && _info.isMinimized();
}

bool Task::isAlwaysOnTop() const
{
    return _info.valid() && (_info.state() & NET::StaysOnTop);
}

bool Task::isKeptBelowOthers() const
{
    return _info.valid() && (_info.state() & NET::KeepBelow);
}

bool Task::isFullScreen() const
{
    return _info.valid() && (_info.state() & NET::FullScreen);
}

bool Task::isShaded() const
{
    return _info.valid() && (_info.state() & NET::Shaded);
}

bool Task::isOnCurrentDesktop() const
{
    return _info.valid() && _info.isOnCurrentDesktop();
}

bool Task::isOnAllDesktops() const
{
    return _info.valid() && _info.onAllDesktops();
}

bool Task::isActive() const
{
    return _active;
}

bool Task::isOnTop() const
{
    return TaskManager::the()->isOnTop(this);
}

bool Task::isModified() const
{
  static QString modStr = QString::fromUtf8("[") +
                          i18n("modified") +
                          QString::fromUtf8("]");
  int modStrPos = _info.visibleName().find(modStr);

  return ( modStrPos != -1 );
}

bool Task::demandsAttention() const
{
    return (_info.valid() && (_info.state() & NET::DemandsAttention)) ||
           _transients_demanding_attention.count() > 0;
}

bool Task::isOnScreen( int screen ) const
{
    return TaskManager::isOnScreen( screen, _win );
}

void Task::updateDemandsAttentionState( WId w )
{
    if (window() != w)
    {
        // 'w' is a transient for this task
        NETWinInfo i( qt_xdisplay(), w, qt_xrootwin(), NET::WMState );
        if(i.state() & NET::DemandsAttention)
        {
            if (!_transients_demanding_attention.contains(w))
            {
                _transients_demanding_attention.append(w);
            }
        }
        else
        {
            _transients_demanding_attention.remove( w );
        }
    }
}

void Task::addTransient( WId w, const NETWinInfo& info )
{
    _transients.append(w);
    if (info.state() & NET::DemandsAttention)
    {
        _transients_demanding_attention.append(w);
        emit changed();
    }
}

void Task::removeTransient(WId w)
{
    _transients.remove(w);
    _transients_demanding_attention.remove(w);
}

QString Task::className()
{
    XClassHint hint;
    if(XGetClassHint(qt_xdisplay(), _win, &hint)) {
        QString nh( hint.res_name );
        XFree( hint.res_name );
        XFree( hint.res_class );
        return nh;
    }
    return QString::null;
}

QString Task::classClass()
{
    XClassHint hint;
    if(XGetClassHint(qt_xdisplay(), _win, &hint)) {
        QString ch( hint.res_class );
        XFree( hint.res_name );
        XFree( hint.res_class );
        return ch;
    }
    return QString::null;
}

QPixmap Task::icon( int width, int height, bool allowResize )
{
  if ( (width == _lastWidth)
       && (height == _lastHeight)
       && (allowResize == _lastResize )
       && (!_lastIcon.isNull()) )
    return _lastIcon;

  QPixmap newIcon = KWin::icon( _win, width, height, allowResize );
  if ( !newIcon.isNull() ) {
    _lastIcon = newIcon;
    _lastWidth = width;
    _lastHeight = height;
    _lastResize = allowResize;
  }

  return newIcon;
}

QPixmap Task::bestIcon( int size, bool &isStaticIcon )
{
  QPixmap pixmap;
  isStaticIcon = false;

  switch( size ) {
  case KIcon::SizeSmall:
    {
      pixmap = icon( 16, 16, true  );

      // Icon of last resort
      if( pixmap.isNull() ) {
        pixmap = KGlobal::iconLoader()->loadIcon( "go",
                                                  KIcon::NoGroup,
                                                  KIcon::SizeSmall );
        isStaticIcon = true;
      }
    }
    break;
  case KIcon::SizeMedium:
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
        pixmap = KGlobal::iconLoader()->loadIcon( "go",
                            KIcon::NoGroup,
                            KIcon::SizeMedium );
        isStaticIcon = true;
      }
    }
    break;
  case KIcon::SizeLarge:
    {
      // If there's a 48x48 icon in the hints then use it
      pixmap = icon( size, size, false  );

      // If not, try to get one from the classname
      if ( pixmap.isNull() || pixmap.width() != size || pixmap.height() != size ) {
        pixmap = KGlobal::iconLoader()->loadIcon( className(),
                            KIcon::NoGroup,
                            size,
                            KIcon::DefaultState,
                            0L,
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
        pixmap = KGlobal::iconLoader()->loadIcon( "go",
                                                  KIcon::NoGroup,
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
    bool on_current = _info.isOnCurrentDesktop();

    if (!on_current)
    {
        KWin::setCurrentDesktop(_info.desktop());
        KWin::forceActiveWindow(_win);
    }

    if (_info.isMinimized())
    {
        KWin::deIconifyWindow(_win);
    }

    QRect geom = _info.geometry();
    QCursor::setPos(geom.center());

    NETRootInfo ri(qt_xdisplay(), NET::WMMoveResize);
    ri.moveResizeRequest(_win, geom.center().x(),
                         geom.center().y(), NET::Move);
}

void Task::resize()
{
    bool on_current = _info.isOnCurrentDesktop();

    if (!on_current)
    {
        KWin::setCurrentDesktop(_info.desktop());
        KWin::forceActiveWindow(_win);
    }

    if (_info.isMinimized())
    {
        KWin::deIconifyWindow(_win);
    }

    QRect geom = _info.geometry();
    QCursor::setPos(geom.bottomRight());

    NETRootInfo ri(qt_xdisplay(), NET::WMMoveResize);
    ri.moveResizeRequest(_win, geom.bottomRight().x(),
                         geom.bottomRight().y(), NET::BottomRight);
}

void Task::setMaximized(bool maximize)
{
    KWin::WindowInfo info = KWin::windowInfo(_win, NET::WMState | NET::XAWMState | NET::WMDesktop);
    bool on_current = info.isOnCurrentDesktop();

    if (!on_current)
    {
        KWin::setCurrentDesktop(info.desktop());
    }

    if (info.isMinimized())
    {
        KWin::deIconifyWindow(_win);
    }

    NETWinInfo ni(qt_xdisplay(), _win, qt_xrootwin(), NET::WMState);

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
        KWin::forceActiveWindow(_win);
    }
}

void Task::toggleMaximized()
{
    setMaximized(!isMaximized());
}

void Task::restore()
{
    KWin::WindowInfo info = KWin::windowInfo(_win, NET::WMState | NET::XAWMState | NET::WMDesktop);
    bool on_current = info.isOnCurrentDesktop();

    if (!on_current)
    {
        KWin::setCurrentDesktop(info.desktop());
    }

    if( info.isMinimized())
    {
        KWin::deIconifyWindow(_win);
    }

    NETWinInfo ni(qt_xdisplay(), _win, qt_xrootwin(), NET::WMState);
    ni.setState(0, NET::Max);

    if (!on_current)
    {
        KWin::forceActiveWindow( _win );
    }
}

void Task::setIconified(bool iconify)
{
    if (iconify)
    {
        KWin::iconifyWindow(_win);
    }
    else
    {
        KWin::WindowInfo info = KWin::windowInfo(_win, NET::WMState | NET::XAWMState | NET::WMDesktop);
        bool on_current = info.isOnCurrentDesktop();

        if (!on_current)
        {
            KWin::setCurrentDesktop(info.desktop());
        }

        KWin::deIconifyWindow(_win);

        if (!on_current)
        {
            KWin::forceActiveWindow(_win);
        }
    }
}

void Task::toggleIconified()
{
    setIconified(!isIconified());
}

void Task::close()
{
    NETRootInfo ri( qt_xdisplay(),  NET::CloseWindow );
    ri.closeWindowRequest( _win );
}

void Task::raise()
{
//    kdDebug(1210) << "Task::raise(): " << name() << endl;
    KWin::raiseWindow( _win );
}

void Task::lower()
{
//    kdDebug(1210) << "Task::lower(): " << name() << endl;
    KWin::lowerWindow( _win );
}

void Task::activate()
{
//    kdDebug(1210) << "Task::activate():" << name() << endl;
    WId w = _win;
    if (_transients_demanding_attention.count() > 0)
    {
        w = _transients_demanding_attention.last();
    }
    KWin::forceActiveWindow( w );
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
    NETWinInfo ni(qt_xdisplay(), _win, qt_xrootwin(), NET::WMDesktop);
    if (desk == 0)
    {
        if (_info.valid() && _info.onAllDesktops())
        {
            ni.setDesktop(TaskManager::the()->winModule()->currentDesktop());
            KWin::forceActiveWindow(_win);
        }
        else
        {
            ni.setDesktop(NETWinInfo::OnAllDesktops);
        }

        return;
    }
    ni.setDesktop(desk);
    if(desk == TaskManager::the()->winModule()->currentDesktop())
        KWin::forceActiveWindow(_win);
}

void Task::toCurrentDesktop()
{
    toDesktop(TaskManager::the()->winModule()->currentDesktop());
}

void Task::setAlwaysOnTop(bool stay)
{
    NETWinInfo ni( qt_xdisplay(),  _win, qt_xrootwin(), NET::WMState);
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
    NETWinInfo ni(qt_xdisplay(), _win, qt_xrootwin(), NET::WMState);

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
    NETWinInfo ni(qt_xdisplay(), _win, qt_xrootwin(), NET::WMState);

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
    NETWinInfo ni( qt_xdisplay(),  _win, qt_xrootwin(), NET::WMState);
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
    NETWinInfo ni( qt_xdisplay(),  _win, qt_xrootwin(), 0 );
    NETRect r;
    r.pos.x = rect.x();
    r.pos.y = rect.y();
    r.size.width = rect.width();
    r.size.height = rect.height();
    ni.setIconGeometry(r);
}

void Task::updateThumbnail()
{
    if ( !_info.valid() ||
            !isOnCurrentDesktop() ||
            !isActive() ||
            !_grab.isNull() ) // We're already processing one...
    {
        return;
    }

    //
    // We do this as a two stage process to remove the delay caused
    // by the thumbnail generation. This makes things much smoother
    // on slower machines.
    //
    QWidget *rootWin = qApp->desktop();
    QRect geom = _info.geometry();
    _grab = QPixmap::grabWindow(rootWin->winId(),
                                geom.x(), geom.y(),
                                geom.width(), geom.height());

    if (!_grab.isNull())
    {
       QTimer::singleShot(200, this, SLOT(generateThumbnail()));
    }
}

void Task::generateThumbnail()
{
   if ( _grab.isNull() )
      return;

   QImage img = _grab.convertToImage();

   double width = img.width();
   double height = img.height();
   width = width * _thumbSize;
   height = height * _thumbSize;

   img = img.smoothScale( qRound(width), qRound(height) );
   _thumb = img;
   _grab.resize( 0, 0 ); // Makes grab a null image.

   emit thumbnailChanged();
}

QPixmap Task::thumbnail(int maxDimension)
{
#ifdef THUMBNAILING_POSSIBLE
    if (!TaskManager::xCompositeEnabled() || !m_windowPixmap)
    {
        return QPixmap();
    }

    Display *dpy = QPaintDevice::x11AppDisplay();

    XWindowAttributes winAttr;
    XGetWindowAttributes(dpy, m_frameId, &winAttr);
    XRenderPictFormat *format = XRenderFindVisualFormat(dpy, winAttr.visual);

    XRenderPictureAttributes picAttr;
    picAttr.subwindow_mode = IncludeInferiors; // Don't clip child widgets

    Picture picture = XRenderCreatePicture(dpy, m_windowPixmap, format,
                                           CPSubwindowMode, &picAttr);

    // Get shaped windows handled correctly.
    XserverRegion region = XFixesCreateRegionFromWindow(dpy, m_frameId,
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
    thumbnail.fill(QApplication::palette().active().background());

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
    QImage image = io.convertToImage(full);
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

    XRenderComposite(QPaintDevice::x11AppDisplay(),
                     PictOpOver, // we're filtering, alpha values are probable
                     picture, // src
                     None, // mask
                     thumbnail.x11RenderHandle(), // dst
                     0, 0, // src offset
                     0, 0, // mask offset
                     0, 0, // dst offset
                     thumbnailWidth, thumbnailHeight);
#endif
    XRenderFreePicture(dpy, picture);

    return thumbnail;
#else // THUMBNAILING_POSSIBLE
    return QPixmap();
#endif // THUMBNAILING_POSSIBLE
}

void Task::updateWindowPixmap()
{
#ifdef THUMBNAILING_POSSIBLE
    if (!TaskManager::xCompositeEnabled() || !isOnCurrentDesktop() ||
        isMinimized())
    {
        return;
    }

    Display *dpy = QPaintDevice::x11AppDisplay();

    if (m_windowPixmap)
    {
        XFreePixmap(dpy, m_windowPixmap);
    }

    m_windowPixmap = XCompositeNameWindowPixmap(dpy, m_frameId);
#endif // THUMBNAILING_POSSIBLE
}

Startup::Startup(const KStartupInfoId& id, const KStartupInfoData& data,
                 QObject * parent, const char *name)
    : QObject(parent, name), _id(id), _data(data)
{
}

Startup::~Startup()
{
}

void Startup::update(const KStartupInfoData& data)
{
    _data.update(data);
    emit changed();
}

int TaskManager::currentDesktop() const
{
    return m_winModule->currentDesktop();
}

TaskDrag::TaskDrag(const Task::List& tasks, QWidget* source, const char* name)
  : QStoredDrag("taskbar/task", source, name)
{
    QByteArray data;
    QDataStream stream(data, IO_WriteOnly);

    Task::List::const_iterator itEnd = tasks.end();
    for (Task::List::const_iterator it = tasks.begin(); it != itEnd; ++it)
    {
        stream << (*it)->window();
    }

    setEncodedData(data);
}

TaskDrag::~TaskDrag()
{
}

bool TaskDrag::canDecode(const QMimeSource* e)
{
    return e->provides("taskbar/task");
}

Task::List TaskDrag::decode( const QMimeSource* e )
{
    QByteArray data(e->encodedData("taskbar/task"));
    Task::List tasks;

    if (data.size())
    {
        QDataStream stream(data, IO_ReadOnly);
        while (!stream.atEnd())
        {
            WId id;
            stream >> id;
            if (Task::Ptr task = TaskManager::the()->findTask(id))
            {
                tasks.append(task);
            }
        }
    }

    return tasks;
}

