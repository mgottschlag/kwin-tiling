/*****************************************************************

Copyright (c) 1996-2000 the kicker authors. See file AUTHORS.

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

#include <QLayout>
#include <QToolTip>
#include <QBoxLayout>
#include <QWheelEvent>
#include <QResizeEvent>

#include <karrowbutton.h>
#include <kglobal.h>
#include <klocale.h>
#include <kdebug.h>

#include "panner.h"
#include "panner.moc"

class Panner::Private
{
public:
    Private()
     : layout(0), luSB(0), rdSB(0)
    {}
    Qt::Orientation orient;
    QBoxLayout *layout;
    KArrowButton *luSB; // Left Scroll Button
    KArrowButton *rdSB; // Right Scroll Button
};

Panner::Panner( QWidget* parent, const char* /* name */ )
    : QScrollArea( parent ),
      d(new Private())
{
    KGlobal::locale()->insertCatalog("libkicker");

    setWidgetResizable( true );
    setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
    setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOff );

    // layout
    d->layout = new QBoxLayout(QBoxLayout::LeftToRight, this);
    d->layout->addWidget(viewport(), 1);
    d->layout->setMargin(0);

    setOrientation(Qt::Horizontal);
}

Panner::~Panner() 
{
    delete d;
}

Qt::Orientation Panner::orientation() const
{
    return d->orient;
}

void Panner::setOrientation(Qt::Orientation o)
{
    d->orient = o;

    if (orientation() == Qt::Horizontal)
    {
        if (d->luSB)
        {
            d->luSB->setArrowType(Qt::LeftArrow);
            d->rdSB->setArrowType(Qt::RightArrow);
            d->luSB->setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding));
            d->rdSB->setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding));
            d->luSB->setToolTip( i18n("Scroll left"));
            d->rdSB->setToolTip( i18n("Scroll right"));
        }
        d->layout->setDirection(QBoxLayout::LeftToRight);
    }
    else
    {
        if (d->luSB)
        {
            d->luSB->setArrowType(Qt::UpArrow);
            d->rdSB->setArrowType(Qt::DownArrow);
            d->luSB->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum));
            d->rdSB->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum));
            d->luSB->setToolTip( i18n("Scroll up"));
            d->rdSB->setToolTip( i18n("Scroll down"));
        }
        d->layout->setDirection(QBoxLayout::TopToBottom);
    }

    if (isVisible())
    {
        // we need to manually redo the layout if we are visible
        // otherwise let the toolkit decide when to do this
        d->layout->activate();
    }
}

void Panner::resizeEvent( QResizeEvent* e )
{
    QScrollArea::resizeEvent( e );
    updateScrollButtons();
}

void Panner::scrollRightDown()
{
    if(orientation() == Qt::Horizontal) // scroll right
    {
        scroll( 40, 0 );
    }
    else // scroll down
    {
        scroll( 0, 40 );
    }
}

void Panner::scrollLeftUp()
{
    if(orientation() == Qt::Horizontal) // scroll left
    {
        scroll( -40, 0 );
    }
    else // scroll up
    {
        scroll( 0, -40 );
    }
}

void Panner::createScrollButtons()
{
    if (d->luSB)
    {
        return;
    }

    // left/up scroll button
    d->luSB = new KArrowButton(this);
    d->luSB->installEventFilter(this);
    d->luSB->setAutoRepeat(true);
    d->luSB->setMinimumSize(12, 12);
    d->luSB->hide();
    d->layout->addWidget(d->luSB);
    connect(d->luSB, SIGNAL(clicked()), SLOT(scrollLeftUp()));

    // right/down scroll button
    d->rdSB = new KArrowButton(this);
    d->rdSB->installEventFilter(this);
    d->rdSB->setAutoRepeat(true);
    d->rdSB->setMinimumSize(12, 12);
    d->rdSB->hide();
    d->layout->addWidget(d->rdSB);
    connect(d->rdSB, SIGNAL(clicked()), SLOT(scrollRightDown()));

    // set up the buttons
    setOrientation(orientation());
}

void Panner::updateScrollButtons()
{
    int l, t, r, b;
    getContentsMargins( &l, &t, &r, &b );

    if ((orientation() == Qt::Horizontal && contentsRect().width() - 1 > width()) ||
        (orientation() == Qt::Vertical && contentsRect().height() - 1 > height()))
    {
        createScrollButtons();

        // since they buttons may be visible but of the wrong size
        // we need to do this every single time
        d->luSB->show();
        d->rdSB->show();

        if (orientation() == Qt::Horizontal)
        {
            setContentsMargins(l, t, d->luSB->width() + d->rdSB->width(), b);
        }
        else
        {
            setContentsMargins(l, t, r, d->luSB->height() + d->rdSB->height());
        }
    }
    else if (d->luSB && d->luSB->isVisibleTo(this))
    {
        d->luSB->hide();
        d->rdSB->hide();
        setContentsMargins(l, t, l, t);
    }
}

/*
void Panner::resizeContents( int w, int h )
{
    QScrollArea::resizeContents( w, h );
    updateScrollButtons();
}*/
