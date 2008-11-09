/***************************************************************************
 *   x11embedpainter.h                                                     *
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

#include "x11embedpainter.h"

#include <QtCore/QSet>
#include <QtCore/QTimer>

#include <KGlobal>


namespace SystemTray
{
namespace FDO
{


class X11EmbedPainter::Singleton
{
public:
    X11EmbedPainter instance;
};

K_GLOBAL_STATIC(X11EmbedPainter::Singleton, singleton)


X11EmbedPainter* X11EmbedPainter::self()
{
    return &singleton->instance;
}


class X11EmbedPainter::Private
{
public:
    QSet<X11EmbedContainer*> containers;
};


X11EmbedPainter::X11EmbedPainter()
    : d(new Private())
{
}


X11EmbedPainter::~X11EmbedPainter()
{
    delete d;
}


void X11EmbedPainter::updateContainer(X11EmbedContainer *container)
{
    if (d->containers.isEmpty()) {
        QTimer::singleShot(0, this, SLOT(performUpdates()));
    }

    d->containers.insert(container);
}


void X11EmbedPainter::performUpdates()
{
    QMultiHash<QWidget*, X11EmbedContainer*> containersByParent;

    foreach (X11EmbedContainer *container, d->containers) {
        QWidget *topWidget = container;
        while (topWidget->parentWidget()) {
            topWidget = topWidget->parentWidget();
        }
        containersByParent.insert(topWidget, container);
        container->setUpdatesEnabled(false);
    }

    foreach (QWidget *parent, containersByParent.uniqueKeys()) {
        QList<X11EmbedContainer*> containers = containersByParent.values(parent);
        containersByParent.remove(parent);

        QRect boundingRect;
        foreach (X11EmbedContainer *container, containers) {
            QRect rect = QRect(container->mapTo(parent, QPoint(0, 0)), container->size());
            if (boundingRect.isNull()) {
                boundingRect = rect;
            } else {
                boundingRect = boundingRect.united(rect);
            }
        }

        QPixmap background = QPixmap(parent->size());
        parent->render(&background, boundingRect.topLeft(), boundingRect);

        foreach (X11EmbedContainer *container, containers) {
            QRect rect = QRect(container->mapTo(parent, QPoint(0, 0)), container->size());
            container->setBackgroundPixmap(background.copy(rect));
        }
    }

    foreach (X11EmbedContainer *container, d->containers) {
        container->setUpdatesEnabled(true);
    }
}


}
}

#include "x11embedpainter.moc"
