/***************************************************************************
 *   stacdialog.h                                                          *
 *   Copyright (C) 2010 Marco Martin <notmart@gmail.com>                   *
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

#ifndef STACKDIALOG_H
#define STACKDIALOG_H

#include <Plasma/Dialog>

#include <QPoint>

class QTimer;

namespace Plasma
{
    class Applet;
    class FrameSvg;
}

namespace SystemTray
{
    class NotificationStack;
}

class StackDialog : public Plasma::Dialog
{
    Q_OBJECT
public:
    StackDialog(QWidget *parent = 0, Qt::WindowFlags f = Qt::Window);
    ~StackDialog();

    void setNotificationStack(SystemTray::NotificationStack *stack);
    SystemTray::NotificationStack *notificartionStack() const;

    void setApplet(Plasma::Applet *applet);
    Plasma::Applet *applet() const;

    void setWindowToTile(QWidget *widget);
    QWidget *windowToTile() const;

protected:
    void adjustWindowToTilePos();

    void paintEvent(QPaintEvent *e);
    bool event(QEvent *event);
    void showEvent(QShowEvent *event);
    void hideEvent(QHideEvent *event);
    void enterEvent(QEvent *event);
    void leaveEvent(QEvent *event);
    void resizeEvent(QResizeEvent *event);
    void moveEvent(QMoveEvent *event);

private:
    Plasma::Applet *m_applet;
    QWidget *m_windowToTile;

    Plasma::FrameSvg *m_background;
    SystemTray::NotificationStack *m_notificationStack;
    QTimer *m_hideTimer;
    bool m_drawLeft;
    bool m_drawRight;
};

#endif
