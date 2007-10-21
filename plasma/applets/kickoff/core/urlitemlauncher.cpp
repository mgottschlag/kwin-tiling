/*  
    Copyright 2007 Robert Knight <robertknight@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

// Own
#include "core/urlitemlauncher.h"

// Qt
#include <QFileInfo>
#include <QHash>
#include <QModelIndex>
#include <QUrl>
#include <QtDebug>

// KDE
#include <KRun>

// Local
#include "core/models.h"

using namespace Kickoff;

class HandlerInfo
{
public:
    HandlerInfo() : type(UrlItemLauncher::ProtocolHandler) , handler(0) {}
    UrlItemLauncher::HandlerType type;
    UrlItemHandler *handler;
};

class GenericItemHandler : public UrlItemHandler
{
public:
    virtual bool openUrl(const QUrl& url) const 
    {
        new KRun(url,0);
        return true; 
    } 
};

class UrlItemLauncher::Private
{
public:
    static QHash<QString,HandlerInfo> globalHandlers;
    static GenericItemHandler genericHandler;
};

QHash<QString,HandlerInfo> UrlItemLauncher::Private::globalHandlers;
GenericItemHandler UrlItemLauncher::Private::genericHandler;

UrlItemLauncher::UrlItemLauncher(QObject *parent)
    : QObject(parent)
    , d(new Private)
{
}
UrlItemLauncher::~UrlItemLauncher()
{
    delete d;
}
bool UrlItemLauncher::openItem(const QModelIndex& index)
{
    QUrl url(index.data(UrlRole).value<QString>());
    if (url.isEmpty()) {
        qDebug() << "Item" << index.data(Qt::DisplayRole) << "has to URL to open.";
        return false;
    }

    qDebug() << "Opening item with URL" << url;

    HandlerInfo protocolHandler = Private::globalHandlers[url.scheme()];
    if (protocolHandler.type == ProtocolHandler && protocolHandler.handler != 0) {
        return protocolHandler.handler->openUrl(url);
    }

    QString extension = QFileInfo(url.path()).completeSuffix();
    HandlerInfo extensionHandler = Private::globalHandlers[extension];
    if (extensionHandler.type == ExtensionHandler && extensionHandler.handler != 0) {
            return extensionHandler.handler->openUrl(url); 
    }

    return Private::genericHandler.openUrl(url);
}
void UrlItemLauncher::addGlobalHandler(HandlerType type,const QString& name,UrlItemHandler *handler)
{
    HandlerInfo info;
    info.type = type;
    info.handler = handler;
    Private::globalHandlers.insert(name,info);
}


#include "urlitemlauncher.moc"
