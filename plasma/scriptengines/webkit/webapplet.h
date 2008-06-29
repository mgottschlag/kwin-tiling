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

#include <plasma/scripting/appletscript.h>

class QWebFrame;

namespace Plasma
{
    class WebContent;
} // namespace Plasma

class WebApplet : public Plasma::AppletScript
{
    Q_OBJECT
public:
    WebApplet(QObject *parent, const QVariantList &args);
    ~WebApplet();
    bool init();

public slots:
    void load(const QUrl &url);
    void setHtml(const QByteArray &html, const QUrl &baseUrl = QUrl());
    void loadHtml(const QUrl &baseUrl = QUrl());

protected:
    Plasma::WebContent *view() const;
    void paintInterface(QPainter *painter,
                                const QStyleOptionGraphicsItem *option,
                                const QRect &contentsRect);
    void constraintsEvent(Plasma::Constraints constraints);

private slots:
    void loadFinished(bool);
    void connectFrame(QWebFrame *);
    void initJsObjects();

private:
    class Private;
    Private *const d;
};

#endif
