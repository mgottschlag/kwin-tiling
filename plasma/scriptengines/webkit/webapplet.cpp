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
#include "webapplet.h"

#include "webpage.h"
#include "plasmajs.h"

#include <QDebug>
#include <QPainter>
#include <QWebView>
#include <QWebFrame>
#include <QWebPage>

#include <plasma/applet.h>
#include <plasma/widgets/webcontent.h>

using namespace Plasma;

class WebApplet::Private
{
public:
    void init(WebApplet *q)
    {
        loaded = false;

        Plasma::Applet *applet = q->applet();
        applet->resize(150, 150);

        page = new Plasma::WebContent(applet);
        page->setPage(new WebPage(page));
        QObject::connect(page, SIGNAL(loadFinished(bool)),
                         q, SLOT(loadFinished(bool)));
        QObject::connect(page->page(), SIGNAL(frameCreated(QWebFrame *)),
                         q, SLOT(connectFrame(QWebFrame *)));
        q->connectFrame(page->mainFrame());

        page->mainFrame()->setScrollBarPolicy(Qt::Horizontal, Qt::ScrollBarAlwaysOff);
        page->mainFrame()->setScrollBarPolicy(Qt::Vertical, Qt::ScrollBarAlwaysOff);
    }

    Plasma::WebContent *page;
    bool loaded;
};

WebApplet::WebApplet(QObject *parent, const QVariantList &args)
    : AppletScript(parent),
      d(new Private)
{
    Q_UNUSED(args)
    d->page = 0;
}

WebApplet::~WebApplet()
{
    delete d;
}

bool WebApplet::init()
{
    d->init(this);
    return true;
}

void WebApplet::paintInterface(QPainter *painter,
                               const QStyleOptionGraphicsItem *,
                               const QRect & contentsRect)
{
    //painter->save();
    /*QPalette pal = painter->palette();
    pal.setBrush(QPalette::Background, Qt::transparent);
    painter.setPalette(pal);*/
    //painter->restore();
}

void WebApplet::load(const QUrl &url)
{
    kDebug() << "Loading" << url;
    d->page->setUrl(url);

    //done to make sure we have very little layout space for
    //html which will mean that the returned contents-size will be
    //the minimum size for the widget.
    //d->->resize(10, 10);
}

Plasma::WebContent* WebApplet::view() const
{
    return d->page;
}

void WebApplet::loadFinished(bool success)
{
    kDebug() << success;
    if (success) {
        QSize newSize = d->page->mainFrame()->contentsSize();
        applet()->setGeometry(QRectF(QPoint(), newSize));
    }
}

void WebApplet::constraintsEvent(Plasma::Constraints constraints)
{
    if (d->page && constraints & Plasma::SizeConstraint) {
        d->page->resize(size());
    }
}

void WebApplet::connectFrame(QWebFrame *frame)
{
    connect(frame, SIGNAL(javaScriptWindowObjectCleared()),
            this, SLOT(initJsObjects()));
}

void WebApplet::initJsObjects()
{
    QWebFrame *frame = qobject_cast<QWebFrame*>(sender());
    Q_ASSERT(frame);
    frame->addToJavaScriptWindowObject(QLatin1String("applet"), this);
    frame->addToJavaScriptWindowObject(QLatin1String("plasma"), new PlasmaJs(this));
}

void WebApplet::setHtml(const QByteArray &html, const QUrl &baseUrl)
{
    //done to make sure we have very little layout space for
    //html which will mean that the returned contents-size will be
    //the minimum size for the widget.
    //d->webView->resize(10, 10);

    kDebug() << "loading" << baseUrl;
    d->page->mainFrame()->setHtml(html, baseUrl);
}

void WebApplet::loadHtml(const QUrl &url)
{
    //done to make sure we have very little layout space for
    //html which will mean that the returned contents-size will be
    //the minimum size for the widget.
    //d->webView->resize(10, 10);

    kDebug() << "loading" << url;
    d->page->mainFrame()->load(url);
}

#include "webapplet.moc"
