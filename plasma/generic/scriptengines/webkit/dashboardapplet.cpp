/*
Copyright (c) 2007 Zack Rusin <zack@kde.org>
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
#include "dashboardapplet.h"

#include <QWebFrame>
#include <QFile>
#include <QByteArray>

#include <KGlobal>
#include <KStandardDirs>

#include <Plasma/WebView>
#include <Plasma/Applet>
#include <Plasma/Package>

#include "dashboardjs.h"

DashboardApplet::DashboardApplet(QObject *parent, const QVariantList &args)
    : WebApplet(parent, args)
{
}

DashboardApplet::~DashboardApplet()
{
}

bool DashboardApplet::init()
{
    applet()->setAspectRatioMode(Plasma::FixedSize);
    return WebApplet::init();
}

void DashboardApplet::loadFinished(bool success)
{
    WebApplet::loadFinished(success);
    if (success) {
        view()->resize(view()->mainFrame()->contentsSize());
        applet()->resize(view()->mainFrame()->contentsSize());
    }
}

void DashboardApplet::constraintsEvent(Plasma::Constraints constraints)
{
    if (constraints & Plasma::FormFactorConstraint) {
        applet()->setBackgroundHints(Plasma::Applet::NoBackground);
    }
}

void DashboardApplet::initJsObjects()
{
    QWebFrame *frame = qobject_cast<QWebFrame*>(sender());
    Q_ASSERT(frame);
    frame->addToJavaScriptWindowObject(QLatin1String("applet"), this);
    frame->addToJavaScriptWindowObject(QLatin1String("widget"), new DashboardJs(frame, this, applet()));
}

QByteArray DashboardApplet::dataFor(const QString &str)
{
    QFile f(str);
    f.open(QIODevice::ReadOnly);
    QByteArray data = f.readAll();
    f.close();

    //replace the apple javascript imports with the kde ones
    QString jsBaseDir = KGlobal::dirs()->findResourceDir("data","plasma/dashboard/button/genericButton.js") 
                            + "plasma/dashboard";

    data.replace("file:///System/Library/WidgetResources", jsBaseDir.toUtf8());
    data.replace("/System/Library/WidgetResources", jsBaseDir.toUtf8());

    return data;
}

#include "dashboardapplet.moc"
