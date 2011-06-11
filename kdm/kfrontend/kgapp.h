/*

Greeter module for xdm

Copyright (C) 1997, 1998 Steffen Hansen <hansen@kde.org>
Copyright (C) 2000-2003 Oswald Buddenhagen <ossi@kde.org>


This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/


#ifndef KGAPP_H
#define KGAPP_H

#include <QApplication>

class QTimerEvent;

class GreeterApp : public QApplication {
    Q_OBJECT
    typedef QApplication inherited;

  public:
    GreeterApp(int &argc, char **argv);
    void markBusy();
    void enableSendInteract() { sendInteract = true; }
    virtual bool x11EventFilter(XEvent *);

  public Q_SLOTS:
    void markReady();

  protected:
    virtual void timerEvent(QTimerEvent *);

  Q_SIGNALS:
    void activity();

  private:
    int pingInterval, pingTimerId;
    bool regrabPtr, regrabKbd, initalBusy, sendInteract;
    QPoint mouseStartPos, dialogStartPos;
    QWidget *dragWidget;
};

#endif /* KGAPP_H */
