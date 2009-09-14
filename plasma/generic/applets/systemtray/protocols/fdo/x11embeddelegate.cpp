/***************************************************************************
 *   x11embeddelegate.cpp                                                  *
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

#include "x11embedcontainer.h"
#include "x11embeddelegate.h"

#include <QtCore/QEvent>


namespace SystemTray
{

class X11EmbedDelegate::Private
{
public:
    X11EmbedContainer *container;
};


X11EmbedDelegate::X11EmbedDelegate(QWidget *parent)
    : QWidget(parent),
      d(new Private())
{
    d->container = new X11EmbedContainer(this);
    d->container->move(0, 0);
    d->container->show();
}


X11EmbedDelegate::~X11EmbedDelegate()
{
    delete d;
}


void X11EmbedDelegate::setParent(QWidget *newParent)
{
    if (parent()) {
        parent()->removeEventFilter(this);
    }
    QWidget::setParent(newParent);
    if (newParent) {
        newParent->installEventFilter(this);
    }
}


void X11EmbedDelegate::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    d->container->resize(size());
}


X11EmbedContainer* X11EmbedDelegate::container()
{
    return d->container;
}


bool X11EmbedDelegate::eventFilter(QObject *watched, QEvent *event)
{
    bool ret = QWidget::eventFilter(watched, event);

    if (event->type() == QEvent::Hide) {
        setParent(0);
    }

    return ret;
}

}

#include "x11embeddelegate.moc"
