/*
*   Copyright 2007 by Matt Broadstone <mbroadst@kde.org>
*   Copyright 2007 by Robert Knight <robertknight@gmail.com>
*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU Library General Public License version 2,
*   or (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details
*
*   You should have received a copy of the GNU Library General Public
*   License along with this program; if not, write to the
*   Free Software Foundation, Inc.,
*   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/
#include <QApplication>
#include <QDesktopWidget>
#include <QTimeLine>

#include <KWindowSystem>
#include <KDebug>

#include <plasma/corona.h>
#include <plasma/plasma.h>
#include <plasma/svg.h>
#include <plasma/widgets/layoutitem.h>
#include <plasma/widgets/boxlayout.h>
#include <plasma/widgets/layoutanimator.h>

#include "panel.h"
#include "panel.moc"

namespace Plasma
{

class PanelLayoutItem : public Plasma::LayoutItem
{
public:
    PanelLayoutItem(Panel* parent)
        : m_panel(parent)
    {
    }

    virtual Qt::Orientations expandingDirections() const 
    {
        return 0;
    }

    virtual QSizeF minimumSize() const { return geometry().size(); }
    virtual QSizeF maximumSize() const { return geometry().size(); }
    virtual QSizeF sizeHint() const { return geometry().size(); }

    virtual void setGeometry(const QRectF&) {}
    virtual QRectF geometry() const 
    {
        return m_panel->sceneRect();
    }

private:
    Panel* const m_panel;
};

class Panel::Private
{
public:
    Private(Panel* parent)
        : panel(parent),
          location(Plasma::BottomEdge),
          layout(0),
          scene(0),
          layoutItem(0),
          background(0)
    {}

    void createApplets();
    void updatePanelGeometry();

    Panel *const panel;
    Plasma::Location location;
    Plasma::BoxLayout *layout;
    Plasma::Corona *scene;
    PanelLayoutItem *layoutItem;
    Plasma::Svg *background;
};

Panel::Panel(QWidget *parent)
    : QGraphicsView(parent),
      d(new Private(this))
{
    // Graphics view setup
    setFrameStyle(QFrame::NoFrame);
    setAutoFillBackground(true);
   // setDragMode(QGraphicsView::RubberBandDrag);
    setCacheMode(QGraphicsView::CacheBackground);
    setInteractive(true);
    setAcceptDrops(true);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // Create layout to arrange applets
    d->layoutItem = new PanelLayoutItem(this);
    if (d->location == Plasma::BottomEdge || d->location == Plasma::TopEdge) {
        d->layout = new Plasma::BoxLayout(Plasma::BoxLayout::LeftToRight,d->layoutItem);
    }
    else {
        d->layout = new Plasma::BoxLayout(Plasma::BoxLayout::TopToBottom,d->layoutItem);
    }
    d->layout->setMargin(0);
    d->layout->setSpacing(0);

        // testing
        Plasma::LayoutAnimator *animator = new Plasma::LayoutAnimator(this);
        animator->setTimeLine( new QTimeLine(300,this) );
        animator->setEffect( Plasma::LayoutAnimator::StandardState , Plasma::LayoutAnimator::MoveEffect );
        d->layout->setAnimator(animator);

    // KWin setup
    KWindowSystem::setType(winId(), NET::Dock);
    KWindowSystem::setState(winId(), NET::Sticky | NET::StaysOnTop);

    // Highly questionable behavior used in kicker..
    // KWindowSystem::setOnAllDesktops(winId(), true);	
}

Panel::~Panel()
{
    delete d;
}


Plasma::Layout* Panel::layout() const
{
    return d->layout;
}

void Panel::setCorona(Plasma::Corona *corona)
{
    d->scene = corona;
    setScene(d->scene);
}

Plasma::Corona* Panel::corona() const
{
    return d->scene;
}

Plasma::Location Panel::location() const
{
    return d->location;	
}

void Panel::setLocation(Plasma::Location loc)
{
    d->location = loc;
    d->updatePanelGeometry();
}

void Panel::Private::updatePanelGeometry()
{
    // FIXME:Hardcoded 60 in as the height of the panel, change this later.
    QDesktopWidget *desktop = QApplication::desktop();
    int x=0, y=0, width=0, height=0;
    switch (location) {
    case Plasma::TopEdge:
    case Plasma::BottomEdge:
        height = 48;
        width = desktop->screenGeometry().width();
        break;

    case Plasma::LeftEdge:
    case Plasma::RightEdge:
    default:
        width = 48;
        height = desktop->screenGeometry().height();
        break;
    }

    switch (location) {
    case Plasma::TopEdge:
        y = 0;
        break;

    case Plasma::LeftEdge:
        x = 0;
        break;

    case Plasma::RightEdge:
        x = desktop->screenGeometry().width() - width;
        break;

    case Plasma::BottomEdge:
    default:
        y = desktop->screenGeometry().height() - height;
        break;
    }

    const QRectF newGeometry(0,0,width,height);
    panel->setGeometry(x,y,width,height);
    panel->setSceneRect(newGeometry);
    layout->setGeometry(newGeometry);
}

void Panel::drawBackground( QPainter *painter , const QRectF& rect )
{
    if (!d->background) {
        d->background = new Plasma::Svg("widgets/panel-background",this); 
    } 

    if (d->background->size() != rect.size().toSize()) {
        d->background->resize((int)rect.width(),(int)rect.height());
    }

    painter->setCompositionMode(QPainter::CompositionMode_Source);
    d->background->paint(painter,0,0);
}

} // Namespace
