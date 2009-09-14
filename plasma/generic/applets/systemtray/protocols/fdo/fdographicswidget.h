/***************************************************************************
 *   fdographicswidget.h                                                   *
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

#ifndef FDOGRAPHICSWIDGET_H
#define FDOGRAPHICSWIDGET_H

#include <QtGui/QGraphicsWidget>
#include <QtGui/QX11EmbedContainer>


namespace SystemTray
{

class FdoGraphicsWidget : public QGraphicsWidget
{
    Q_OBJECT

public:
    FdoGraphicsWidget(WId winId, QGraphicsWidget *parent = 0);
    ~FdoGraphicsWidget();

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);

signals:
    void clientClosed();

protected:
    void showEvent(QShowEvent *event);
    void hideEvent(QHideEvent *event);

private slots:
    void setupXEmbedDelegate();
    void handleClientEmbedded();
    void handleClientClosed();
    void handleClientError(QX11EmbedContainer::Error);
    void updateWidgetBackground();

private:
    class Private;
    Private* const d;
};

}


#endif
