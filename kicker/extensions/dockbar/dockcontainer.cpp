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

#include <QWidget>
#include <QToolTip>
#include <kwin.h>
#include <QValidator>
#include <kinputdialog.h>
#include <klocale.h>
#include <kmenu.h>
#include <kdebug.h>

#include "dockcontainer.h"
#include "dockcontainer.moc"

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <QX11Info>


DockContainer::DockContainer( QString command, QWidget *parent,
                              QString resname, QString resclass, bool undocked_style )
    : QFrame( parent,
              undocked_style ? Qt::WStyle_Customize |
              Qt::WStyle_StaysOnTop | Qt::WStyle_Tool |
              Qt::WStyle_NoBorder | Qt::WX11BypassWM : Qt::Widget ),
      _embeddedWinId(0),
      _command(command),
      _resName(resname),
      _resClass(resclass)
{
    setObjectName( resname );
    XSelectInput( QX11Info::display(), winId(),
		  KeyPressMask | KeyReleaseMask |
		  ButtonPressMask | ButtonReleaseMask |
		  KeymapStateMask |
		  ButtonMotionMask |
		  PointerMotionMask |
		  EnterWindowMask | LeaveWindowMask |
		  FocusChangeMask |
		  ExposureMask |
		  StructureNotifyMask |
		  SubstructureRedirectMask |
		  SubstructureNotifyMask );
    if (!undocked_style) {
        setFrameStyle(StyledPanel | Raised);
        setLineWidth(border());
        this->setToolTip( command);
    } else {
        setFrameStyle(StyledPanel | Plain);
        setLineWidth(1);
    }
    resize(sz(),sz());
}

void DockContainer::embed( WId id )
{
    if( id == _embeddedWinId || id == 0)
        return;
    QRect geom = KWin::windowInfo(id,NET::WMKDEFrameStrut).frameGeometry();

    // does the same as KWM::prepareForSwallowing()
	QX11Info info;
    XWithdrawWindow( QX11Info::display(), id, info.screen() );
    while( KWin::windowInfo(id, NET::XAWMState).mappingState() != NET::Withdrawn );

    XReparentWindow( QX11Info::display(), id, winId(), 0, 0 );

    // resize if window is bigger than frame
    if( (geom.width() > width()) ||
        (geom.height() > height()) )
        XResizeWindow( QX11Info::display(), id, width(), height() );
    else
        XMoveWindow(QX11Info::display(), id,
                    (sz() -  geom.width())/2 - border(),
                    (sz() - geom.height())/2 - border());
    XMapWindow( QX11Info::display(), id );
    XUngrabButton( QX11Info::display(), AnyButton, AnyModifier, winId() );

    _embeddedWinId = id;
}

void DockContainer::unembed()
{
    if( _embeddedWinId )
        XReparentWindow( QX11Info::display(), _embeddedWinId, QX11Info::appRootWindow(), 0, 0 );
}

void DockContainer::kill()
{
    if ( _embeddedWinId ) {
        XKillClient( QX11Info::display(), _embeddedWinId );
        _embeddedWinId = 0; // in case the window does not exist anymore..
    }
    else emit embeddedWindowDestroyed(this); /* enable killing of empty windows.. */
}

bool DockContainer::x11Event( XEvent *e )
{
    switch( e->type ) {
    case DestroyNotify:
	if( e->xdestroywindow.window == _embeddedWinId || _embeddedWinId == 0) {
	    _embeddedWinId = 0;
	    emit embeddedWindowDestroyed(this);
	}
	break;
    case UnmapNotify:
        if ( e->xunmap.window == _embeddedWinId ) {
            kDebug() << "Unmap Notify !!! I hate smart dockapps as wmpinboard " << command() << endl;
            _embeddedWinId = 0;
        }
        break;
    case ReparentNotify:
	if( _embeddedWinId &&
	    (e->xreparent.window == _embeddedWinId) &&
	    (e->xreparent.parent != winId()) ) {
	    _embeddedWinId = 0;
	}
	else if( e->xreparent.parent == winId() ) {
	    _embeddedWinId = e->xreparent.window;
	    embed( _embeddedWinId );
	}
	break;
    }

    return false;
}

void DockContainer::askNewCommand(bool bad_command)
{
    bool ok;
    QString title( i18n("Enter Command Line for Applet %1.%2", resName(), resClass()) );
    QString description( i18n("This applet does not behave correctly and the dockbar was unable to "
                              "find the command line necessary to launch it the next time KDE starts up") );
    QString cmd;

    /*
      I was not able to figure out why valgrind complains inside the getText call..
      (invalid read of size 1 in Xmb.. functions)
    */
    if (bad_command) {
        cmd = KInputDialog::getText( title, description,
                                     command(), &ok, this );
    } else {
        cmd = KInputDialog::getText( title, QString(),
                                     command(), &ok, this );
    }
    if (ok) { _command = cmd; emit settingsChanged(this); }
}

void DockContainer::popupMenu(QPoint p)
{
    KMenu *pm = new KMenu(this);
    QAction *kill  = pm->addAction( i18n("Kill This Applet"));
    QAction *change = pm->addAction( i18n("Change Command"));
    QAction *r = pm->exec(p);
        /* pm is destroyed now .. if it is destroyed later,
           there is a risk that kill() double-frees it */
    delete pm;
    
    if (r == kill)
	this->kill();
    if (r == change)
        askNewCommand(false);
}

int& DockContainer::sz() {
    static int _sz = 66;
    return _sz;
}

int& DockContainer::border() {
    static int _border = 1;
    return _border;
}
