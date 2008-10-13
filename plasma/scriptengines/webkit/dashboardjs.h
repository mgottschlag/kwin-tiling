/*
Copyright (c) 2008 Beat Wolf <asraniel@fryx.ch>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
 */ 

#ifndef DASHBOARDJS_H
#define DASHBOARDJS_H

#include <QObject>
#include <QVariant>

#include <plasma/applet.h>

/**
  * Implements the Mac OS X Dashboard widget javascript functions.
  * This document was used to create this class:
  * http://developer.apple.com/documentation/AppleApplications/Reference/Dashboard_Ref/Dashboard_Ref.pdf
  */
class DashboardJs : public QObject
{
    Q_OBJECT
public:
    DashboardJs(QObject *parent=0, Plasma::Applet *applet = 0);

public slots:
    void hello(int test);

    /**
     * opens a certain application
     */
    void openApplication(QString name);
    
    /**
     * opens a URL. Does not open file urls by default.
     * TODO: find out what protocols dashboard widgets support. filter out the others
     */
    void openURL(QString name);

    /**
     * Returns the value assosiated with a certain key
     */
    QVariant preferenceForKey(QString key);

    void prepareForTransition(QString transition);

    void performTransition();

    void setCloseBoxOffset(int x, int y);

    /**
     * Saves a value to a key
     */
    void setPreferenceForKey(QString value, QString key);

    void system(QString command, QString handler);

private:
    //TODO: execute when needed
    QString onshow;
    QString onhide;
    QString onremove;

    QString onfocus;
    QString onblur;

    QString ondragstart;
    QString ondragstop;

    QString identifier;

    //my private stuff
    Plasma::Applet *applet;
};

#endif
