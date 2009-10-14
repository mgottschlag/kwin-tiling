/***************************************************************************
 *   fdographicswidget.cpp                                                 *
 *                                                                         *
 *   Copyright (C) 2008 Jason Stubbs <jasonbstubbs@gmail.com>              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#include "fdographicswidget.h"
#include "x11embeddelegate.h"
#include "x11embedcontainer.h"

#include <QtCore/QWeakPointer>
#include <QtCore/QTimer>

#include <QtGui/QApplication>
#include <QtGui/QGraphicsView>

#include <plasma/theme.h>


namespace SystemTray
{

class FdoGraphicsWidget::Private
{
public:
    Private()
        : clientEmbedded(false)
    {
    }

    ~Private()
    {
        delete widget.data();
    }

    WId winId;
    bool clientEmbedded;
    QWeakPointer<X11EmbedDelegate> widget;
};

FdoGraphicsWidget::FdoGraphicsWidget(WId winId, QGraphicsWidget *parent)
    : QGraphicsWidget(parent),
      d(new Private())
{
    d->winId = winId;

    setMinimumSize(22, 22);
    setMaximumSize(22, 22);
    resize(22, 22);

    setCacheMode(QGraphicsItem::NoCache);

    connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()),
            this, SLOT(updateWidgetBackground()));
    QTimer::singleShot(0, this, SLOT(setupXEmbedDelegate()));
}


FdoGraphicsWidget::~FdoGraphicsWidget()
{
    delete d;
}


void FdoGraphicsWidget::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *parentWidget)
{
    QGraphicsWidget::paint(painter, option, parentWidget);

    QGraphicsView *parentView = 0;
    foreach (QGraphicsView *view, scene()->views()) {
        if (view->isVisible() && view->sceneRect().intersects(sceneBoundingRect())) {
            parentView = view;
        }
    }

    if (!parentView) {
        return;
    }

    if (!d->widget) {
        QTimer::singleShot(0, this, SLOT(setupXEmbedDelegate()));
        return;
    } else if (!d->clientEmbedded) {
        return;
    }

    QWidget *widget = d->widget.data();
    if (widget->parentWidget() != parentView) {
        //kDebug() << "embedding into" << parentView->metaObject()->className() << "(" << d->winId << ")";
        widget->setParent(parentView);
    }

    QPoint pos = parentView->mapFromScene(scenePos());
    pos += parentView->viewport()->pos();
    if (widget->pos() != pos) {
        widget->move(pos);
    }

    if (!widget->isVisible()) {
        widget->show();
    }
}

void FdoGraphicsWidget::hideEvent(QHideEvent *event)
{
    Q_UNUSED(event);
    if (d->widget) {
        d->widget.data()->hide();
    }
}

void FdoGraphicsWidget::showEvent(QShowEvent *event)
{
    Q_UNUSED(event);
    if (d->widget) {
        d->widget.data()->show();
    }
}

void FdoGraphicsWidget::setupXEmbedDelegate()
{
    if (d->widget) {
        return;
    }

#if QT_VERSION < 0x040401
    const Qt::ApplicationAttribute attr = (Qt::ApplicationAttribute)4;
#else
    const Qt::ApplicationAttribute attr = Qt::AA_DontCreateNativeWidgetSiblings;
#endif
    if (!QApplication::testAttribute(attr)) {
        QApplication::setAttribute(attr);
    }

    X11EmbedDelegate *widget = new X11EmbedDelegate();
    widget->setMinimumSize(22, 22);
    widget->setMaximumSize(22, 22);
    widget->resize(22, 22);

    connect(widget->container(), SIGNAL(clientIsEmbedded()),
            this, SLOT(handleClientEmbedded()));
    connect(widget->container(), SIGNAL(clientClosed()),
            this, SLOT(handleClientClosed()));
    connect(widget->container(), SIGNAL(error(QX11EmbedContainer::Error)),
            this, SLOT(handleClientError(QX11EmbedContainer::Error)));

    widget->container()->embedSystemTrayClient(d->winId);
    d->widget = widget;
}

void FdoGraphicsWidget::updateWidgetBackground()
{
    X11EmbedDelegate *widget = d->widget.data();
    if (!widget) {
        return;
    }

    QPalette palette = widget->palette();
    palette.setBrush(QPalette::Window, Plasma::Theme::defaultTheme()->color(Plasma::Theme::BackgroundColor));
    widget->setPalette(palette);
    widget->setBackgroundRole(QPalette::Window);
}


void FdoGraphicsWidget::handleClientEmbedded()
{
    //kDebug() << "client embedded (" << d->winId << ")";
    d->clientEmbedded = true;
    update();
}


void FdoGraphicsWidget::handleClientClosed()
{
    emit clientClosed();
    //kDebug() << "client closed (" << d->winId << ")";
}


void FdoGraphicsWidget::handleClientError(QX11EmbedContainer::Error error)
{
    Q_UNUSED(error);

    //kDebug() << "client error (" << d->winId << ")";
    emit clientClosed();
}

}

#include "fdographicswidget.moc"
