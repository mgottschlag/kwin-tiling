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

#include <QDesktopWidget>

#define TEST_STEP_INTERVAL 2000

SplashApp::SplashApp(Display * display, int argc, char ** argv)
    : QApplication(display, argc, argv),
      m_display(display), m_stage(0),
      m_testing(false)
{
    m_kde_splash_progress = XInternAtom(m_display, "_KDE_SPLASH_PROGRESS", False);
    m_testing = arguments().contains("--test");

    QDesktopWidget *desktop = QApplication::desktop();
    int numScreens = desktop->screenCount();

    for (int i = 0; i < numScreens; ++i) {
        SplashWindow *w = new SplashWindow(m_testing);
        w->setGeometry(desktop->availableGeometry(i));
        w->show();
        m_windows << w;
    }

    setStage(1);

    

    XSelectInput(display, DefaultRootWindow(display), SubstructureNotifyMask);

    if (m_testing) {
        m_timer.start(TEST_STEP_INTERVAL, this);
    }
}

SplashApp::~SplashApp()
{
    qDeleteAll(m_windows);
}

Display * SplashApp::display() const
{
    return m_display;
}

void SplashApp::timerEvent(QTimerEvent * event)
{
    if (event->timerId() == m_timer.timerId()) {
        m_timer.stop();

        setStage(m_stage + 1);

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

                int stage = -1;

                if (strcmp(message, "initial") == 0 && m_stage < 0)
                    stage = 0; // not actually used
                else if (strcmp(message, "kded") == 0 && m_stage < 1)
                    stage = 1;
                else if (strcmp(message, "confupdate") == 0 && m_stage < 2)
                    stage = 2;
                else if (strcmp(message, "kcminit") == 0 && m_stage < 3)
                    stage = 3;
                else if (strcmp(message, "ksmserver") == 0 && m_stage < 4)
                    stage = 4;
                else if (strcmp(message, "wm") == 0 && m_stage < 5)
                    stage = 5;
                else if (strcmp(message, "desktop") == 0 && m_stage < 6)
                    stage = 6;

                setStage(stage);
            }
    }
    return false;

}

int SplashApp::x11ProcessEvent(XEvent * xe)
{
    Q_UNUSED(xe)
    return 0;
}

void SplashApp::setStage(int stage)
{
    if (m_stage == 6) {
        QApplication::exit(EXIT_SUCCESS);
    }

    m_stage = stage;
    foreach (SplashWindow *w, m_windows) {
        w->setStage(stage);
    }
}

