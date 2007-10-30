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
      m_type(KUriFilterData::Unknown)
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

QAction *WebshortcutRunner::accepts(const QString& term) {
    QString searchTerm = term.toLower();

    KUriFilterData filter(term);
    bool filtered = KUriFilter::self()->filterUri(filter);
    m_type = filter.uriType();
    if (filtered &&
        (m_type == KUriFilterData::LocalDir ||
         m_type == KUriFilterData::NetProtocol ||
         m_type == KUriFilterData::Help)) {
        QAction *action = new QAction(QString("Open %1").arg(term), this);
        action->setIcon(m_icon);
        m_url = filter.uri();
        return action;
    }

    m_type = KUriFilterData::Unknown;
    foreach (KService::Ptr service, m_offers) {
        // hmm, how about getting the keys for the localized sites?
        foreach(QString key, service->property("Keys").toStringList()) {
            // FIXME? should we look for the used separator from the konqi's settings?
            key = key.toLower() + ":";
            if (searchTerm.size() > key.size() &&
                searchTerm.startsWith(key, Qt::CaseInsensitive)) {
                m_url = getSearchQuery(service->property("Query").toString(),searchTerm);
                QString actionText = QString("Search %1 for %2");
                actionText = actionText.arg(service->name(),
                                            searchTerm.right(searchTerm.length() - searchTerm.indexOf(':') - 1));

                QAction *action = new QAction(actionText, this);

                // let's try if we can get a proper icon from the favicon cache
                QIcon icon = getFavicon(m_url);
                if (icon.isNull()){
                    action->setIcon(m_icon);
                } else {
                    action->setIcon(icon);
                }

                return action;
            }
        }
    }

    return 0;
}

KUrl WebshortcutRunner::getSearchQuery(const QString &query, const QString &term) {
    // FIXME delimiter check like for above?
    QStringList tempList = term.split(":");
    if(tempList.count() > 0) {
        QString searchWord(tempList[1]);
        QString finalQuery(query);
        // FIXME? currently only basic searches are supported
        finalQuery.replace("\\{@}", searchWord);
        KUrl url(finalQuery);
        return url;
    }

    return KUrl();
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

bool WebshortcutRunner::exec(QAction* action, const QString& command) {
    //TODO: this should probably use the action to store the url rather than
    //      store it internally in m_url so as to support multiple actions
    //      in the future
    Q_UNUSED(action)
    Q_UNUSED(command)
//    kDebug() << "command: " << command;
//    kDebug() << "url: " << m_url.url();

    if (m_type == KUriFilterData::Unknown ||
        (m_type == KUriFilterData::NetProtocol &&
         m_url.protocol().left(4) == "http")) {
        KToolInvocation::invokeBrowser(m_url.url());
    } else {
    /*  (m_type == KUriFilterData::LocalDir ||
         m_type == KUriFilterData::NetProtocol ||
         m_type == KUriFilterData::Help) */
        new KRun(m_url, 0);
    }

    return true;
}

#include "webshortcutrunner.moc"
