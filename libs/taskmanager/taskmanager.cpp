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

#include <kglobal.h>
#include <klocale.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <kwinmodule.h>
#include <netwm.h>

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "taskmanager.h"
#include "taskmanager.moc"

template class QList<Task>;
KWinModule* kwin_module = 0;

TaskManager::TaskManager(QObject *parent, const char *name)
    : QObject(parent, name), _active(0)
{
    // create and connect kwin module
    kwin_module = new KWinModule(this);

    connect(kwin_module, SIGNAL(windowAdded(WId)), SLOT(windowAdded(WId)));
    connect(kwin_module, SIGNAL(windowRemoved(WId)), SLOT(windowRemoved(WId)));
    connect(kwin_module, SIGNAL(activeWindowChanged(WId)), SLOT(activeWindowChanged(WId)));
    connect(kwin_module, SIGNAL(currentDesktopChanged(int)), SLOT(currentDesktopChanged(int)));
    connect(kwin_module, SIGNAL(windowChanged(WId,unsigned int)), SLOT(windowChanged(WId,unsigned int)));

    // register existing windows
    const QValueList<WId> windows = kwin_module->windows();
    for (QValueList<WId>::ConstIterator it = windows.begin(); it != windows.end(); ++it )
	windowAdded(*it);

    // application startup notification
    connectDCOPSignal(0, 0, "clientDied(pid_t)", "clientDied(pid_t)", false);
    connectDCOPSignal(0, 0, "clientStarted(QString,QString,pid_t,QString,bool)",
                      "clientStarted(QString,QString,pid_t,QString,bool)", false);
}

TaskManager::~TaskManager()
{
}

Task* TaskManager::findTask(WId w)
{
    for (Task* t = _tasks.first(); t != 0; t = _tasks.next())
        if (t->window() == w  || t->hasTransient(w))
            return t;
    return 0;
}

void TaskManager::windowAdded(WId w )
{
    NETWinInfo info (qt_xdisplay(),  w, qt_xrootwin(),
		     NET::WMWindowType | NET::WMPid | NET::WMState );

    // ignore NET::Tool and other special window types
    if (info.windowType() != NET::Normal
        && info.windowType() != NET::Override
        && info.windowType() != NET::Unknown)
	return;

    // ignore windows that want to be ignored by the taskbar
    if ((info.state() & NET::SkipTaskbar) != 0)
	return;

    // lets see if this is a transient for an existing task
    Window transient_for;
    if (XGetTransientForHint( qt_xdisplay(), (Window) w, &transient_for )
        && (WId) transient_for != qt_xrootwin()
        && transient_for != 0 ) {

        Task* t = findTask((WId) transient_for);
	if (t) {
	    if (t->window() != w) {
		t->addTransient(w);
                kdDebug() << "TM: Transient " << w << " added for Task: " << t->window() << endl;
            }
	    return;
	}
    }

    // Now do app-starting-notification stuff before we give the window
    // a taskbar button.

    // Strategy:
    //
    // Is this a NET_WM compliant app ?
    // Yes -> kill relevant app-starting button
    // No  -> Is the WM_CLASS.res_name for this app used by any existing
    //        app-starting buttons ?
    //        Yes -> kill relevant button.
    //        No  -> kill all non-NET_WM-compliant app-starting buttons.

    pid_t pid = info.pid();
    bool hasPid = (pid != 0);

    if (hasPid)
        killStartup(pid);
    else {

        // Hard - this app is not NET_WM compliant

        XClassHint hint;
        Status ok = XGetClassHint(qt_xdisplay(), w, &hint);

        bool found = false;

        if (ok != 0) { // We managed to read the class hint

            QString resName   (hint.res_name);
            QString resClass  (hint.res_class);

            for(Startup* s = _startups.first(); s != 0; s = _startups.next()) {

                if (s->compliant()) // Ignore the compliant ones
                    continue;

                if (s->bin() == resName || (s->bin().lower() == resClass.lower())) {
                    // Found it !
                    found = true;
                    _startups.removeRef(s);
                    delete s;
                    break;
                }
            }
        }

        if (!found) {

            // Build a list of all non-compliant buttons.
            QValueList<pid_t> buttonsToKill;

            for(Startup* s = _startups.first(); s != 0; s = _startups.next()) {
                if (!s->compliant())
                    buttonsToKill << s->pid();
            }

            // Kill all non-compliant buttons.
            QValueList<pid_t>::Iterator killit(buttonsToKill.begin());
            for (; killit != buttonsToKill.end(); ++killit)
                killStartup(*killit);
        }
    }

    Task* t = new Task(w, this);
    _tasks.append(t);

    kdDebug() << "TM: Task added for WId: " << w << endl;

    emit changed();
}

void TaskManager::windowRemoved(WId w )
{
    // find task
    Task* t = findTask(w);
    if (!t) return;

    if (t->window() == w) {
        _tasks.removeRef(t);
        if(t == _active) _active = 0;
        delete t;
        kdDebug() << "TM: Task for WId " << w << " removed." << endl;
    }
    else {
        t->removeTransient( w );
        kdDebug() << "TM: Transient " << w << " for Task " << t->window() << " removed." << endl;
    }
    emit changed();
}

void TaskManager::windowChanged(WId w, unsigned int dirty)
{
    // check if any state we are interested in is marked dirty
    if(!(dirty & (NET::WMVisibleName|NET::WMName|NET::WMState|NET::WMIcon|NET::XAWMState|NET::WMDesktop)) )
        return;

    // find task
    Task* t = findTask( w );
    if (!t) return;

    t->refresh();

    // refresh icon pixmap if necessary
    if (dirty & NET::WMIcon)
        t->refresh(true);

    if(dirty & NET::WMDesktop)
        emit changed();
}

void TaskManager::activeWindowChanged(WId w )
{
    Task* t = findTask( w );
    if (!t) {
        if (_active) {
            _active->setActive(false);
            _active->refresh();
            _active = 0;
        }
    }
    else {
        if (_active) {
            _active->setActive(false);
            _active->refresh();
        }

        _active = t;
        _active->setActive(true);
        _active->refresh();
    }
}

void TaskManager::currentDesktopChanged(int desktop)
{
    emit desktopChanged(desktop);
}

void TaskManager::clientStarted(QString name, QString icon, pid_t pid, QString bin, bool compliant)
{
    if ((long)pid == 0) return;
    kdDebug(1210) << "clientStarted(" << name << ", " << icon << ", " << (long)pid << "d)" << endl;

    Startup * s = new Startup(name, icon, pid, bin, compliant, this);
    _startups.append(s);

    connect(s, SIGNAL(killMe(pid_t)), SLOT(killStartup(pid_t)));
    emit changed();
}

void TaskManager::clientDied(pid_t pid)
{
    if ((long)pid != 0)
	killStartup(pid);
    emit changed();
}

void TaskManager::killStartup(pid_t pid)
{
    Startup* s = 0;
    for(s = _startups.first(); s != 0; s = _startups.next()) {
        if (s->pid() == pid)
            break;
    }
    if (s == 0) return;

    _startups.removeRef(s);
    delete s;
}

Task::Task(WId win, QObject * parent, const char *name)
    : QObject(parent, name), _active(false), _win(win)
{
    refresh(true);
}

Task::~Task()
{
}

void Task::refresh(bool icon)
{
    if (icon) {
        _pixmap = KWin::icon(_win, 16, 16, true);
        if(_pixmap.isNull())
            _pixmap = SmallIcon("xapp");
    }
    emit changed();
}

bool Task::maximized() const
{
    return(_info.state & NET::Max);
}

bool Task::iconified() const
{
    return (_info.mappingState == NET::Iconic);
}

bool Task::staysOnTop() const
{
    return (_info.state & NET::StaysOnTop);
}

bool Task::onCurrentDesktop() const
{
    return (_info.onAllDesktops || _info.desktop == kwin_module->currentDesktop());
}

bool Task::active() const
{
    return _active;
}

void Task::maximize()
{
    NETWinInfo ni( qt_xdisplay(),  _win, qt_xrootwin(), NET::WMState);
    ni.setState( NET::Max, NET::Max );
}

void Task::restore()
{
    NETWinInfo ni( qt_xdisplay(),  _win, qt_xrootwin(), NET::WMState);
    ni.setState( 0, NET::Max );
}

void Task::iconify()
{
    XIconifyWindow( qt_xdisplay(), _win, qt_xscreen() );
}

void Task::deiconify()
{
    KWin::setActiveWindow( _win );
}

void Task::close()
{
    NETRootInfo ri( qt_xdisplay(),  NET::CloseWindow );
    ri.closeWindowRequest( _win );
}

Startup::Startup(const QString& text, const QString& /*icon*/, pid_t pid,
                 const QString& bin, bool compliant, QObject * parent, const char *name)
    : QObject(parent, name), _bin(bin), _text(text), _pid(pid),_compliant(compliant)
{
    // go away after 20s if we weren't removed before.
    startTimer(20000);
}

Startup::~Startup()
{

}

void Startup::timerEvent(QTimerEvent *)
{
    killTimers();
    emit(killMe(_pid));
}
