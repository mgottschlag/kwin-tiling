/* This file is part of the KDE libraries

    Copyright (c) 2001  Martin R. Jones <mjones@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "kscreensaver.h"

#include <QPainter>
#include <QTimer>
#include <QtGui/QX11Info>
#include <QApplication>
#include <QDebug>
#include <krandom.h>

#ifdef Q_WS_X11
#include <X11/Xlib.h>
#endif

#ifdef Q_WS_WIN
#include <windows.h>
#endif

//-----------------------------------------------------------------------------

KScreenSaver::KScreenSaver( WId id ) : QWidget(), embeddedWidget(0)
{
    if ( id )
    {
        create( id, false, true );
    }
}

KScreenSaver::~KScreenSaver()
{
    destroy( false, false );
}

bool KScreenSaver::event(QEvent* e)
{
    bool r = QWidget::event(e);
    if (e->type() == QEvent::Polish)
        setAttribute(Qt::WA_StyledBackground, false);
    if ((e->type() == QEvent::Resize) && embeddedWidget)
    {
        embeddedWidget->resize( size() );
#ifdef Q_WS_WIN
        SetWindowPos(embeddedWidget->winId(), HWND_TOP, 0, 0, size().width(), size().height(), 0 );
#endif
    }
    return r;
}

void KScreenSaver::embed( QWidget *w )
{
    w->resize( size() );
    QApplication::sendPostedEvents();
#if defined(Q_WS_X11) //FIXME
    XReparentWindow(QX11Info::display(), w->winId(), winId(), 0, 0);
#elif defined(Q_WS_WIN)
    SetParent(w->winId(), winId());
    
    LONG style = GetWindowLong(w->winId(), GWL_STYLE);
    style &= ~(WS_CAPTION | WS_THICKFRAME | WS_MINIMIZE | WS_MAXIMIZE | WS_SYSMENU);
    SetWindowLong(w->winId(), GWL_STYLE, style);

    LONG exStyle = GetWindowLong(w->winId(), GWL_EXSTYLE);
    exStyle &= ~(WS_EX_DLGMODALFRAME | WS_EX_CLIENTEDGE | WS_EX_STATICEDGE);
    SetWindowLong(w->winId(), GWL_EXSTYLE, exStyle);
    
    SetWindowPos(w->winId(), HWND_TOP, 0, 0, size().width(), size().height(), SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
#endif
    w->setGeometry( 0, 0, width(), height() );
    embeddedWidget = w;
    QApplication::sendPostedEvents();
}

KScreenSaverInterface::~KScreenSaverInterface()
{
}

QDialog* KScreenSaverInterface::setup()
{
    return 0;
}

//============================================================================

class KBlankEffectPrivate
{
public:
    KBlankEffect::BlankEffect currentEffect;
    int effectProgress;
    QTimer *timer;
    QWidget *widget;
};

KBlankEffect::BlankEffect KBlankEffect::effects[] = {
    &KBlankEffect::blankNormal,
    &KBlankEffect::blankSweepRight,
    &KBlankEffect::blankSweepDown,
    &KBlankEffect::blankBlocks
};

KBlankEffect::KBlankEffect( QObject *parent ) : QObject( parent )
{
    d = new KBlankEffectPrivate;
    d->currentEffect = &KBlankEffect::blankNormal;
    d->effectProgress = 0;
    d->timer = new QTimer( this );
    connect( d->timer, SIGNAL(timeout()), this, SLOT(timeout()) );
}


KBlankEffect::~KBlankEffect()
{
    delete d;
}

void KBlankEffect::finished()
{
    d->timer->stop();
    d->effectProgress = 0;
    emit doneBlank();
}


void KBlankEffect::blank( QWidget *w, Effect effect )
{
    if ( !w ) {
        emit doneBlank();
        return;
    }

    if ( effect == Random )
        effect = (Effect)(KRandom::random() % MaximumEffects);

    d->effectProgress = 0;
    d->widget = w;
    d->currentEffect = effects[ (int)effect ];
    d->timer->start( 10 );
}

void KBlankEffect::timeout()
{
    (this->*d->currentEffect)();
}

void KBlankEffect::blankNormal()
{
    QPainter p( d->widget );
    p.fillRect( 0, 0, d->widget->width(), d->widget->height(), Qt::black );
    finished();
}


void KBlankEffect::blankSweepRight()
{
    QPainter p( d->widget );
    p.fillRect( d->effectProgress, 0, 50, d->widget->height(), Qt::black );
    qApp->flush();
    d->effectProgress += 50;
    if ( d->effectProgress >= d->widget->width() )
        finished();
}


void KBlankEffect::blankSweepDown()
{
    QPainter p( d->widget );
    p.fillRect( 0, d->effectProgress, d->widget->width(), 50, Qt::black );
    qApp->flush();
    d->effectProgress += 50;
    if ( d->effectProgress >= d->widget->height() )
        finished();
}


void KBlankEffect::blankBlocks()
{
    static int *block = 0;

    int bx = (d->widget->width()+63)/64;
    int by = (d->widget->height()+63)/64;

    if ( !d->effectProgress ) {
        block = new int [ bx*by ];

        for ( int i = 0; i < bx*by; i++ )
            block[i] = i;
        for ( int i = 0; i < bx*by; i++ ) {
            int swap = KRandom::random()%(bx*by);
            int tmp = block[i];
            block[i] = block[swap];
            block[swap] = tmp;
        }
    }

    QPainter p( d->widget );

    // erase a couple of blocks at a time, otherwise it looks too slow
    for ( int i = 0; i < 2 && d->effectProgress < bx*by; i++ ) {
        int x = block[d->effectProgress]%bx;
        int y = block[d->effectProgress]/bx;
        p.fillRect( x*64, y*64, 64, 64, Qt::black );
        d->effectProgress++;
    }

    qApp->flush();

    if ( d->effectProgress >= bx*by ) {
        delete[] block;
        finished();
    }
}

#include "kscreensaver.moc"
