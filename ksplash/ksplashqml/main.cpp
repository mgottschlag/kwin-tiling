/*
 *   Copyright (C) 2011 Ivan Cukic <ivan.cukic(at)kde.org>
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


#include <QApplication>
#include <QDialog>
#include <QDebug>
#include <QTextBrowser>
#include <QVBoxLayout>

#include <iostream>
#include <X11/Xlib.h>

class Dialog: public QDialog {
    public:
        Dialog(QWidget * parent = 0)
            : QDialog(parent)
        {
            setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::SplashScreen);

            QVBoxLayout * layout = new QVBoxLayout(this);
            layout->addWidget(textKnown = new QTextBrowser(this));
            layout->addWidget(textUnknown = new QTextBrowser(this));

        }

        QTextBrowser * textKnown;
        QTextBrowser * textUnknown;

};

class KSplashQml: public QApplication {
public:
    int state;
    Display * dpy;
    Atom kde_splash_progress;

    KSplashQml(Display * display, int argc, char **argv):
        QApplication(display, argc, argv), dpy(display)
    {
        state = 1;
        kde_splash_progress = XInternAtom(dpy, "_KDE_SPLASH_PROGRESS", False);
    }

    bool x11EventFilter(XEvent * xe)
    {
        char * message;
        switch (xe->type) {
            case ClientMessage:
                message = xe->xclient.data.b;
                int oldstate = state;

                if (xe->xclient.message_type ==  kde_splash_progress) {
                    window->textKnown->setText(window->textKnown->toPlainText() + "\n #### ksplash message");
                }

                if (strcmp(message, "initial") == 0 && state < 0)
                    state = 0; // not actually used
                else if (strcmp(message, "kded") == 0 && state < 1)
                    state = 1;
                else if (strcmp(message, "confupdate") == 0 && state < 2)
                    state = 2;
                else if (strcmp(message, "kcminit") == 0 && state < 3)
                    state = 3;
                else if (strcmp(message, "ksmserver") == 0 && state < 4)
                    state = 4;
                else if (strcmp(message, "wm") == 0 && state < 5)
                    state = 5;
                else if (strcmp(message, "desktop") == 0 && state < 6)
                    state = 6;

                if (state != oldstate) {
                    qDebug() << "Caught ClientMessage XEvent" << message;
                    window->setWindowTitle(window->windowTitle() + "C ");
                    window->textKnown->setText(window->textKnown->toPlainText() + "\n" + message);
                } else {
                    window->textUnknown->setText(window->textUnknown->toPlainText() + message);
                }

                if (state == 6) {
                    QApplication::exit(EXIT_SUCCESS);
                }
        }
        return false;
    }

    int x11ProcessEvent(XEvent * xe)
    {
        return 0;
        // nothing here for the time being
    }

    Dialog * window;
};

int main(int argc, char **argv)
{
    // lets fork and all that...

    pid_t pid = fork();
    if (pid < -1) {
        perror("fork()");
        return -1;
    }

    if (pid != 0) {
        // this is the parent process, returning pid of the fork
        // printf("%d\n", pid);
        return 0;
    }

    // close stdin,stdout,stderr, otherwise startkde will block
    close(0);
    close(1);
    close(2);

    Display * display = XOpenDisplay(NULL);

    KSplashQml app(display, argc, argv);
    Dialog dialog;
    app.window = &dialog;

    int sw = WidthOfScreen(ScreenOfDisplay(display, DefaultScreen(display)));
    int sh = HeightOfScreen(ScreenOfDisplay(display, DefaultScreen(display)));
    dialog.resize(sw, sh);
    dialog.move(0, 0);

    dialog.show();
    XSelectInput(display, DefaultRootWindow(display), SubstructureNotifyMask);

    return app.exec();
}
