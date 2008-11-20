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

#include <QtCore/QPointer>
#include <QtCore/QTimer>

#include <QtGui/QApplication>
#include <QtGui/QGraphicsView>

#include <plasma/theme.h>


namespace SystemTray
{
namespace FDO
{


class GraphicsWidget::Private
{
public:
    Private()
        : clientEmbedded(false)
    {
    }

    ~Private()
    {
        delete widget;
    }

    WId winId;
    bool clientEmbedded;
    QPointer<X11EmbedDelegate> widget;
};


GraphicsWidget::GraphicsWidget(WId winId)
    : d(new GraphicsWidget::Private())
{
    d->winId = winId;

    setMinimumSize(22, 22);
    setMaximumSize(22, 22);
    resize(22, 22);

    setCacheMode(QGraphicsItem::NoCache);

    connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()),
            this, SLOT(updateWidgetBackground()));
}


GraphicsWidget::~GraphicsWidget()
{
    delete d;
}


void GraphicsWidget::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *parentWidget)
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

    if (d->widget->parentWidget() != parentView) {
        kDebug() << "embedding into" << parentView->metaObject()->className() << "(" << d->winId << ")";
        d->widget->setParent(parentView);
    }

    QPoint pos = parentView->mapFromScene(scenePos());
    if (d->widget->pos() != pos) {
        d->widget->move(pos);
    }

    if (!d->widget->isVisible()) {
        d->widget->show();
    }

    if (parentView->childAt(pos) != d->widget->container()) {
        d->widget->raise();
    }
}

void GraphicsWidget::setupXEmbedDelegate()
{
    if (d->widget) {
        return;
    }

#if QT_VERSION < 0x040401
    const Qt::ApplicationAttribute attr = (Qt::ApplicationAttribute);
#else
    const Qt::ApplicationAttribute attr = Qt::AA_DontCreateNativeWidgetSiblings;
#endif
    if (!QApplication::testAttribute(attr)) {
        QApplication::setAttribute(attr);
    }

    d->widget = new X11EmbedDelegate();
    d->widget->setMinimumSize(22, 22);
    d->widget->setMaximumSize(22, 22);
    d->widget->resize(22, 22);

    connect(d->widget->container(), SIGNAL(clientIsEmbedded()),
            this, SLOT(handleClientEmbedded()));
    connect(d->widget->container(), SIGNAL(clientClosed()),
            this, SLOT(handleClientClosed()));
    connect(d->widget->container(), SIGNAL(error(QX11EmbedContainer::Error)),
            this, SLOT(handleClientError(QX11EmbedContainer::Error)));

    d->widget->container()->embedSystemTrayClient(d->winId);
}

void GraphicsWidget::updateWidgetBackground()
{
    if (!d->widget) {
        return;
    }

    QPalette palette = d->widget->palette();
    palette.setBrush(QPalette::Window, Plasma::Theme::defaultTheme()->color(Plasma::Theme::BackgroundColor));
    d->widget->setPalette(palette);
    d->widget->setBackgroundRole(QPalette::Window);
}


void GraphicsWidget::handleClientEmbedded()
{
    kDebug() << "client embedded (" << d->winId << ")";

    d->clientEmbedded = true;
    update();
}


void GraphicsWidget::handleClientClosed()
{
    emit clientClosed();
    kDebug() << "client closed (" << d->winId << ")";
}


void GraphicsWidget::handleClientError(QX11EmbedContainer::Error error)
{
    Q_UNUSED(error);

    kDebug() << "client error (" << d->winId << ")";
    emit clientClosed();
}


}
}


#include "fdographicswidget.moc"
