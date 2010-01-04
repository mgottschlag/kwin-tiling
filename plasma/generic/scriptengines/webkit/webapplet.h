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
#ifndef WEBAPPLET_H
#define WEBAPPLET_H

#include <QUrl>

#include <Plasma/DataEngine>
#include <Plasma/AppletScript>

class QWebFrame;
class QWebPage;

namespace Plasma
{
    class WebView;
} // namespace Plasma

class WebApplet : public Plasma::AppletScript
{
    Q_OBJECT
public:
    WebApplet(QObject *parent, const QVariantList &args);
    ~WebApplet();

    bool init();

protected:
    Plasma::WebView *view() const;
    void paintInterface(QPainter *painter,
                                const QStyleOptionGraphicsItem *option,
                                const QRect &contentsRect);
    virtual QByteArray dataFor(const QString &str);
    QWebPage *page();
    bool loaded();

protected slots:
    void connectFrame(QWebFrame *);
    virtual void loadFinished(bool);
    virtual void initJsObjects();

private:
    class Private;
    Private *const d;
};

#endif
