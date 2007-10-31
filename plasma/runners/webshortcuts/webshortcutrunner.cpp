/*
 *   Copyright (C) 2007 Teemu Rytilahti <tpr@iki.fi>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <QAction>
#include <QStringList>
#include <QIcon>
#include <QDBusInterface>
#include <QDBusReply>

#include <KDebug>
#include <KRun>
#include <KIconLoader>
#include <KLocale>
#include <KMimeType>
#include <KService>
#include <KStandardDirs>
#include <KToolInvocation>
#include <KUrl>

#include <kservicetypetrader.h>

#include "webshortcutrunner.h"


WebshortcutRunner::WebshortcutRunner(QObject *parent, const QVariantList& args)
    : Plasma::AbstractRunner(parent),
      m_type(Plasma::SearchContext::UnknownType)
{
    Q_UNUSED(args);
    // set the name shown after the result in krunner window
    setObjectName(i18n("Locations"));
    // query ktrader for all available searchproviders and preload the default icon
    m_offers = KServiceTypeTrader::self()->query("SearchProvider");
    m_icon = QIcon(KIconLoader::global()->loadIcon("konqueror", KIconLoader::Small));
}

WebshortcutRunner::~WebshortcutRunner()
{
}

void WebshortcutRunner::match(Plasma::SearchContext *search)
{
    QString term = search->term().trimmed().toLower();
    m_type = search->type();

    foreach (KService::Ptr service, m_offers) {
        //TODO: how about getting the keys for the localized sites?
        foreach (QString key, service->property("Keys").toStringList()) {
            // FIXME? should we look for the used separator from the konqi's settings?
            key = key.toLower() + ":";
            if (term.size() > key.size() &&
                term.startsWith(key, Qt::CaseInsensitive)) {
                QString actionText = QString("Search %1 for %2");
                actionText = actionText.arg(service->name(),
                                            term.right(term.length() - term.indexOf(':') - 1));

                QAction *action = search->addExactMatch(this);
                action->setText(actionText);
                QString url = getSearchQuery(service->property("Query").toString(), term);
                //kDebug() << "url is" << url << "!!!!!!!!!!!!!!!!!!!!!!!";
                action->setData(url);

                // let's try if we can get a proper icon from the favicon cache
                QIcon icon = getFavicon(url);
                if (icon.isNull()){
                    action->setIcon(m_icon);
                } else {
                    action->setIcon(icon);
                }

                return;
            }
        }
    }

    if (m_type == Plasma::SearchContext::Directory ||
        m_type == Plasma::SearchContext::Help) {
        //kDebug() << "Locations matching because of" << m_type;
        QAction *action = search->addExactMatch(this);
        action->setText(i18n("Open %1", term));
        action->setIcon(m_icon);
        return;
    }

    if (m_type == Plasma::SearchContext::NetworkLocation) {
        QAction *action = search->addPossibleMatch(this);
        KUrl url(term);

        if (url.protocol().isEmpty()) {
            url.clear();
            url.setHost(term);
            url.setProtocol("http");
        }

        action->setText(i18n("Go to %1", url.prettyUrl()));
        action->setIcon(m_icon);
        action->setData(url.url());
        return;
    }

}

QString WebshortcutRunner::getSearchQuery(const QString &query, const QString &term)
{
    // FIXME delimiter check like for above?
    QStringList tempList = term.split(":");
    if(tempList.count() > 0) {
        QString searchWord(tempList[1]);
        QString finalQuery(query);
        // FIXME? currently only basic searches are supported
        finalQuery.replace("\\{@}", searchWord);
        KUrl url(finalQuery);
        return url.url();
    }

    return QString();
}

QIcon WebshortcutRunner::getFavicon(const KUrl &url) {
    // query the favicons module
    QDBusInterface favicon("org.kde.kded", "/modules/favicons", "org.kde.FavIcon");
    QDBusReply<QString> reply = favicon.call("iconForUrl", url.url());

    if (!reply.isValid()) {
        return QIcon();
    }

    // locate the favicon
    QString iconFile = KGlobal::dirs()->findResource("cache",reply.value()+".png");
    QIcon icon = QIcon(iconFile);

    if (!icon.isNull()) {
        return icon;
    }

    return QIcon();
}

void WebshortcutRunner::exec(Plasma::SearchAction *action)
{
    //TODO: this should probably use the action to store the url rather than
    //      store it internally in m_url so as to support multiple actions
    //      in the future
    QString location = action->data().toString();

    if (location.isEmpty()) {
        location = action->term();
    }

    //kDebug() << "command: " << action->term();
    //kDebug() << "url: " << location;
    if (m_type == Plasma::SearchContext::UnknownType ||
        (m_type == Plasma::SearchContext::NetworkLocation &&
         location.left(4) == "http")) {
        KToolInvocation::invokeBrowser(location);
    } else {
        new KRun(location, 0);
    }
}

#include "webshortcutrunner.moc"
