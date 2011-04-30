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

#include "SplashWindow.h"
#include "SplashApp.h"

#define TEST_STEP_INTERVAL 2000

SplashApp::SplashApp(Display * display, int argc, char ** argv)
    : QApplication(display, argc, argv),
      m_display(display), m_state(0), m_window(0),
      m_testing(false)
{
    m_kde_splash_progress = XInternAtom(m_display, "_KDE_SPLASH_PROGRESS", False);
    m_window = new SplashWindow();

    setState(1);

    int sw = WidthOfScreen(ScreenOfDisplay(display, DefaultScreen(display)));
    int sh = HeightOfScreen(ScreenOfDisplay(display, DefaultScreen(display)));

    m_window->setGeometry(0, 0, sw, sh);
    m_window->show();

    XSelectInput(display, DefaultRootWindow(display), SubstructureNotifyMask);

    if (m_testing = arguments().contains("--test")) {
        m_timer.start(TEST_STEP_INTERVAL, this);
    }
}

SplashApp::~SplashApp() {
    delete m_window;
}

Display * SplashApp::display() const
{
    return m_display;
}

void SplashApp::timerEvent(QTimerEvent * event)
{
    if (event->timerId() == m_timer.timerId()) {
        m_timer.stop();

        setState(m_state + 1);

        m_timer.start(TEST_STEP_INTERVAL, this);
    }
}

bool SplashApp::x11EventFilter(XEvent * xe)
{
    char * message;
    switch (xe->type) {
        case ClientMessage:
            if (xe->xclient.message_type == m_kde_splash_progress) {
                message = xe->xclient.data.b;

                int state;

                if (strcmp(message, "initial") == 0 && m_state < 0)
                    state = 0; // not actually used
                else if (strcmp(message, "kded") == 0 && m_state < 1)
                    state = 1;
                else if (strcmp(message, "confupdate") == 0 && m_state < 2)
                    state = 2;
                else if (strcmp(message, "kcminit") == 0 && m_state < 3)
                    state = 3;
                else if (strcmp(message, "ksmserver") == 0 && m_state < 4)
                    state = 4;
                else if (strcmp(message, "wm") == 0 && m_state < 5)
                    state = 5;
                else if (strcmp(message, "desktop") == 0 && m_state < 6)
                    state = 6;

                setState(state);
            }
    }
    return false;

}

int SplashApp::x11ProcessEvent(XEvent * xe)
{
    return 0;
}

void SplashApp::setState(int state)
{
    if (m_state == 6) {
        QApplication::exit(EXIT_SUCCESS);
    }

    m_state = state;
    m_window->setState(state);
}

