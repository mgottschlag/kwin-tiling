/****************************************************************************
** $Id$
**
** Implementation of QXEmbed class
**
** Created :
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees with valid Qt Professional Edition licenses may distribute and
** use this file in accordance with the Qt Professional Edition License
** provided at sale or upon request.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#include <qapplication.h>
#include "qxembed.h"
#include <X11/Xlib.h>

#include <qcursor.h>

QXEmbed::QXEmbed(QWidget *parent, const char *name)
  : QWidget(parent, name)
{
    window = 0;
    setFocusPolicy(StrongFocus);

    //trick to create extraData();
    QCursor old = cursor();
    setCursor(Qt::blankCursor);
    setCursor(old);
}

QXEmbed::~QXEmbed()
{
    if ( topLevelWidget()->isActiveWindow() ) {
	XEvent e;
	e.type = FocusIn;
	e.xfocus.window = topLevelWidget()->winId();
	e.xfocus.mode = NotifyNormal;
	e.xfocus.detail = NotifyDetailNone;
	XSendEvent(qt_xdisplay(), topLevelWidget()->winId(), 0, FALSE, &e);
    }
}


void QXEmbed::resizeEvent(QResizeEvent*)
{
    if (window != 0)
	XResizeWindow(qt_xdisplay(), window, width(), height());
}

void QXEmbed::showEvent(QShowEvent*)
{
    if (window != 0)
	XMapRaised(qt_xdisplay(), window);

}

void QXEmbed::keyPressEvent( QKeyEvent *e )
{
    static Atom qu = 0;
    if (!qu)
	qu = XInternAtom(qt_xdisplay(), "QT_UNICODE_KEY_PRESS", False);

    if (!window)
	return;

    XEvent ev;
    QString text = e->text();
    int i = 1;
    int m = QMAX(1, text.length());
    do{
	bzero(&ev, sizeof(ev));
	ev.xclient.type = ClientMessage;
	ev.xclient.window = window;
	ev.xclient.message_type = qu;
	ev.xclient.format = 16;
	ev.xclient.data.s[0] = e->key();
	ev.xclient.data.s[1] = e->ascii();
	ev.xclient.data.s[2] = e->state();
	ev.xclient.data.s[3] = e->isAutoRepeat();
	ev.xclient.data.s[4] = !text.isEmpty()?1:e->count();
	ev.xclient.data.s[5] = !text.isEmpty()?text[i-1].row:QChar::null.row;
	ev.xclient.data.s[6] = !text.isEmpty()?text[i-1].cell:QChar::null.cell;
	ev.xclient.data.s[7] = i++;
	ev.xclient.data.s[8] = m;
	XSendEvent(qt_xdisplay(), window, FALSE, NoEventMask, &ev);
    } while ( i <= m);
}

void QXEmbed::keyReleaseEvent( QKeyEvent *e )
{
    static Atom qu = 0;
    if (!qu)
	qu = XInternAtom(qt_xdisplay(), "QT_UNICODE_KEY_RELEASE", False);

    if (!window)
	return;

    XEvent ev;
    QString text = e->text();
    int i = 1;
    int m = QMAX(1, text.length());
    do{
	bzero(&ev, sizeof(ev));
	ev.xclient.type = ClientMessage;
	ev.xclient.window = window;
	ev.xclient.message_type = qu;
	ev.xclient.format = 16;
	ev.xclient.data.s[0] = e->key();
	ev.xclient.data.s[1] = e->ascii();
	ev.xclient.data.s[2] = e->state();
	ev.xclient.data.s[3] = e->isAutoRepeat();
	ev.xclient.data.s[4] = !text.isEmpty()?1:e->count();
	ev.xclient.data.s[5] = !text.isEmpty()?text[i-1].row:QChar::null.row;
	ev.xclient.data.s[6] = !text.isEmpty()?text[i-1].cell:QChar::null.cell;
	ev.xclient.data.s[7] = i++;
	ev.xclient.data.s[8] = m;
	XSendEvent(qt_xdisplay(), window, FALSE, NoEventMask, &ev);
    } while ( i <= m);
}

void QXEmbed::focusInEvent( QFocusEvent * ){
    if (!window)
	return;
    sendFocusIn();
}

void QXEmbed::focusOutEvent( QFocusEvent * ){
    if (!window)
	return;
    sendFocusOut();
}

void QXEmbed::embed(WId w)
{
    static Atom qsw = 0;
    if (!qsw)
	qsw = XInternAtom(qt_xdisplay(), "QT_EMBEDDED_WINDOW", False);
    window = w;
    long a = 1;
    XChangeProperty(qt_xdisplay(), w,
		    qsw, qsw, 32, PropModeReplace,
		    (const unsigned char*)&a, 1);
    XReparentWindow(qt_xdisplay(), w, winId(), 0, 0);
    QApplication::syncX();
    XResizeWindow(qt_xdisplay(), w, width(), height());
    XMapWindow(qt_xdisplay(),w);
    extraData()->xDndProxy = w;

    if (this == qApp->focusWidget() )
	sendFocusIn();
    else
	sendFocusOut();
}


void QXEmbed::sendFocusIn()
{
    XEvent e;
    e.type = FocusIn;
    e.xfocus.window = window;
    e.xfocus.mode = NotifyNormal;
    e.xfocus.detail = NotifyDetailNone;
    XSendEvent(qt_xdisplay(), window, 0, FALSE, &e);

}

void QXEmbed::sendFocusOut()
{
    XEvent e;
    e.type = FocusOut;
    e.xfocus.window = window;
    e.xfocus.mode = NotifyNormal;
    e.xfocus.detail = NotifyDetailNone;
    XSendEvent(qt_xdisplay(), window, 0, FALSE, &e);
}


bool QXEmbed::focusNextPrevChild( bool /* next */ )
{
    return FALSE;
}

