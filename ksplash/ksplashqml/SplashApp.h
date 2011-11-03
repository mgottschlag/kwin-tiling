/*
 *   Copyright (C) 2010 Ivan Cukic <ivan.cukic(at)kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License version 2,
 *   or (at your option) any later version, as published by the Free
 *   Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef SPLASH_APP_H_
#define SPLASH_APP_H_

#include <QObject>
#include <QApplication>
#include <QBasicTimer>

#include <X11/Xlib.h>

class SplashWindow;

class SplashApp: public QApplication {

public:
    SplashApp(Display * display, int argc, char ** argv);
    ~SplashApp();

    Display * display() const;

    bool x11EventFilter(XEvent * xe);
    int x11ProcessEvent(XEvent * xe);

protected:
    void timerEvent(QTimerEvent * event);
    void setStage(int stage);

private:
    Display * m_display;
    int m_stage;
    Atom m_kde_splash_progress;
    QList<SplashWindow *> m_windows;
    bool m_testing;
    QBasicTimer m_timer;
};

#endif // SPLASH_APP_H_
