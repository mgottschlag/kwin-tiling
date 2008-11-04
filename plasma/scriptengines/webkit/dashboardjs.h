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
#include <QGraphicsItem>

#include <QWebFrame>

#include <Plasma/Applet>

/**
  * Implements the Mac OS X Dashboard widget javascript functions.
  * This document was used to create this class:
  * http://developer.apple.com/documentation/AppleApplications/Reference/Dashboard_Ref/Dashboard_Ref.pdf
  */
class DashboardJs : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString identifier READ identifier)

    Q_PROPERTY(QString onshow READ onshow WRITE setOnshow)
    Q_PROPERTY(QString onhide READ onhide WRITE setOnhide)
    Q_PROPERTY(QString onremove READ onremove WRITE setOnremove)
    Q_PROPERTY(QString ondragstart READ ondragstart WRITE setOndragstart)
    Q_PROPERTY(QString ondragstop READ ondragstop WRITE setOndragstop)
    
public:
    DashboardJs(QWebFrame *frame, QObject *parent= 0, Plasma::Applet *applet = 0);
    ~DashboardJs();

    QString identifier() const;
    
    QString onshow() const;
    void setOnshow(const QString &onshow);

    QString onhide() const;
    void setOnhide(const QString &onhide);

    QString onremove() const;
    void setOnremove(const QString &onremove);
    
    QString ondragstart() const;
    void setOndragstart(const QString &ondragstart);

    QString ondragstop() const;
    void setOndragstop(const QString &ondragstop);

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
    void openURL(QString name); //ok

    /**
     * Returns the value associated with a certain key
     */
    QVariant preferenceForKey(QString key); //ok

    void prepareForTransition(QString transition);

    void performTransition();

    void setCloseBoxOffset(int x, int y); //not needed

    /**
     * Saves a value to a key
     */
    void setPreferenceForKey(QString value, QString key); //ok

    void system(QString command, QString handler); //cannot really be implemented
private:
    //TODO: execute when needed
    QString m_onshow; //has no equivalent in plasma, because always shown
    QString m_onhide; //has no equivalent in plasma, because always shown
    QString m_onremove; //ok

    QString m_ondragstart;
    QString m_ondragstop;

    //my private stuff
    Plasma::Applet *m_applet;
    QWebFrame *m_frame;
};

#endif
