/*
Copyright (c) 2007 Zack Rusin <zack@kde.org>

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

#include <QGraphicsSceneMouseEvent>
#include <kstandarddirs.h>

#include <QApplication>
#include <QPainter>
#include <qdebug.h>
#include <QtNetwork>
#include <math.h>

#include <Plasma/Applet>
#include <Plasma/Package>

static inline QByteArray dataFor(const QString &str)
{
    QFile f(str);
    f.open(QIODevice::ReadOnly);
    QByteArray data = f.readAll();
    f.close();
    return data;
}

DashboardApplet::DashboardApplet(QObject *parent, const QVariantList &args)
    : WebApplet(parent, args)
{
}

DashboardApplet::~DashboardApplet()
{
}

bool DashboardApplet::init()
{
    WebApplet::init();
    //applet()->setHasConfigurationInterface(false);
    applet()->setAcceptsHoverEvents(true);
    //FIXME: setBackgroundHints is protected now
    //applet()->setBackgroundHints(Plasma::Applet::NoBackground);
    QString webpage = package()->filePath("webpage");
    //kDebug() << "webpage is at" << webpage;

    if (webpage.isEmpty()) {
        return false;
    }

    //kDebug() << QUrl(package()->path());
    setHtml(dataFor(webpage), QUrl(package()->path()));
    return true;
}

#include "dashboardapplet.moc"
