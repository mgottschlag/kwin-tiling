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
#include "webpage.h"

#include <qwebsettings.h>
#include <qdebug.h>

namespace Plasma
{

WebPage::WebPage(QObject *parent)
    : QWebPage(parent)
{
    settings()->setAttribute(QWebSettings::JavascriptCanOpenWindows, false);
    settings()->setAttribute(QWebSettings::JavascriptCanAccessClipboard, true);
    settings()->setAttribute(QWebSettings::LinksIncludedInFocusChain, true);
}

QWebPage * WebPage::createWindow(QWebPage::WebWindowType)
{
    Q_ASSERT(0);
    return 0;
}

void WebPage::javaScriptAlert(QWebFrame *frame, const QString& message)
{
    qDebug()<< "JS ALERT: "<< message;
}

void WebPage::javaScriptConsoleMessage(const QString& message, int lineNumber,
                                       const QString& sourceID)
{
    qDebug()<< "JS CONSOLE MESSAGE: line "<< lineNumber<<": " << message;
}

bool WebPage::javaScriptConfirm(QWebFrame *frame, const QString& msg)
{
    qDebug()<< "JS CONFIRM: "<< msg;
    return true;
}

bool WebPage::javaScriptPrompt(QWebFrame *frame, const QString& msg,
                               const QString& defaultValue, QString* result)
{
    qDebug()<<"JS PROMPT: "<< msg <<", default text: "<<defaultValue;
    *result = defaultValue;
    return true;
}

} // namespace Plasma

#include "webpage.moc"
