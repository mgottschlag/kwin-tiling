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

#include "dashboardjs.h"
#include <kdebug.h>

#include <KRun>
#include <KConfigGroup>

#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QWidget>

#include <QGraphicsItem>
#include <QEvent>
#include <QGraphicsScene>

DashboardJs::DashboardJs(QWebFrame *frame, QObject *parent, Plasma::Applet *applet)
    : QObject(parent)
{
    m_applet = applet;
    m_frame = frame;
}

DashboardJs::~DashboardJs()
{
    if(m_frame){
        kDebug() << "deconstructor calles javascript: " << m_onremove;
        m_frame->evaluateJavaScript(m_onremove);
    }
}

void DashboardJs::openApplication(QString name)
{
    //STUB
    kDebug() << "not implemented: open application" << name;

    //WARNING: this might be dangerous and exploited (or not)
    /* create list of applications:
     * com.apple.ical
     * com.RealNetworks.RealPlayer
     */
}

void DashboardJs::openURL(QString name)
{
    //TODO: this could be dangerous(really?). filter valid urls
    new KRun(name, 0); 
}

QVariant DashboardJs::preferenceForKey(QString key)
{
    KConfigGroup conf = m_applet->config();

    if (!conf.hasKey(key)) {
      return QVariant();
    }

    return conf.readEntry(key,"");
}

void DashboardJs::prepareForTransition(QString transition)
{
    //STUB
    kDebug() << "not implemented: transition with name" << transition;
    //TODO:freeze widget drawing. possible?
    //not really needed for things to work, but would be prettier
}

void DashboardJs::performTransition()
{
    //STUB
    //TODO: enable widget drawing again, perform nice animation.
    kDebug() << "not implemented: perform transition";
    //not really needed for things to work, but would be prettier
}

void DashboardJs::setCloseBoxOffset(int x, int y)
{
    //STUB, not needed, plasma has its own close box
    kDebug() << "not implemented: close box offset" << x << y;
}

void DashboardJs::setPreferenceForKey(QString value, QString key)
{
    kDebug() << "save key" << key << value;
    KConfigGroup conf = m_applet->config();
    conf.writeEntry(key, value);
}

void DashboardJs::system(QString command, QString handler)
{
    //WARNING: this might be dangerous and exploited. 
    //STUB
    kDebug() << "not implemented: system command:" << command << handler;
}

//Property sets and gets
QString DashboardJs::identifier() const
{ 
    return QString::number(m_applet->id());
}

QString DashboardJs::onshow() const 
{
    return m_onshow;
}

void DashboardJs::setOnshow(const QString &value)
{
    m_onshow = value;
}

QString DashboardJs::onhide() const
{
    return m_onhide;
}

void DashboardJs::setOnhide(const QString &value)
{
    m_onhide = value;
}

QString DashboardJs::onremove() const
{
    return m_onremove;
}

void DashboardJs::setOnremove(const QString &value)
{
    m_onremove = value;
}


QString DashboardJs::ondragstart() const
{
    return m_ondragstart;
}

void DashboardJs::setOndragstart(const QString &value)
{
    m_ondragstart = value;
}

QString DashboardJs::ondragstop() const
{
    return m_ondragstop;
}

void DashboardJs::setOndragstop(const QString &value)
{
    m_ondragstop = value;
}
//only for testing purpose
//TODO: remove when not needed anymore
void DashboardJs::hello(int test)
{
    kDebug() << "hello world" << test;
}

