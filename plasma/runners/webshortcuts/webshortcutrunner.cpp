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

#include "webshortcutrunner.h"

#include <QAction>
#include <QStringList>
#include <QDBusInterface>
#include <QDBusReply>

#include <KDebug>
#include <KRun>
#include <KLocale>
#include <KMimeType>
#include <KStandardDirs>
#include <KToolInvocation>
#include <KUrl>

WebshortcutRunner::WebshortcutRunner(QObject *parent, const QVariantList& args)
    : Plasma::AbstractRunner(parent, args)
{
    KGlobal::locale()->insertCatalog("krunner_webshortcutsrunner");
    Q_UNUSED(args);
    // set the name shown after the result in krunner window
    setObjectName(i18n("Web Shortcut"));
    // query ktrader for all available searchproviders and preload the default icon
    m_offers = serviceQuery("SearchProvider");
    m_icon = KIcon("internet-web-browser");
    setIgnoredTypes(Plasma::RunnerContext::FileSystem);
}

WebshortcutRunner::~WebshortcutRunner()
{
}

void WebshortcutRunner::match(Plasma::RunnerContext &context)
{
    const QString term = context.query();
    const char separator = ':';

    if (term.length() < 3 || !term.contains(separator)) {
        return;
    }

    kDebug() << "checking with" << term;

    QMutexLocker lock(bigLock());
    foreach (const KService::Ptr &service, m_offers) {
        //TODO: how about getting the keys for the localized sites?
        foreach (QString key, service->property("Keys").toStringList()) {
            // FIXME? should we look for the used separator from the konqi's settings?
            key = key.toLower() + separator;
            if (term.size() > key.size() &&
                term.startsWith(key, Qt::CaseInsensitive)) {
                QString actionText = i18n("Search %1 for %2",service->name(),
                                            term.right(term.length() - term.indexOf(':') - 1));
                QString url = getSearchQuery(service->property("Query").toString(), term);
                //kDebug() << "url is" << url << "!!!!!!!!!!!!!!!!!!!!!!!";

                Plasma::QueryMatch match(this);
                match.setType(Plasma::QueryMatch::ExactMatch);
                match.setText(actionText);
                match.setData(service->property("Query").toString());
                match.setRelevance(0.9);

                // let's try if we can get a proper icon from the favicon cache
                KIcon icon = getFavicon(url);
                if (icon.isNull()){
                    match.setIcon(m_icon);
                } else {
                    match.setIcon(icon);
                }

                context.addMatch(term, match);
                return;
            }
        }
    }
}

QString WebshortcutRunner::getSearchQuery(const QString &query, const QString &term)
{
    // FIXME delimiter check like for above?
    QStringList tempList = term.split(':');
    if (tempList.count() > 1) {
        QString searchWord(tempList[1]);
        QString finalQuery(query);
        // FIXME? currently only basic searches are supported
        finalQuery.replace("\\{@}", searchWord);
        KUrl url(finalQuery);
        return url.url();
    }

    return QString();
}

KIcon WebshortcutRunner::getFavicon(const KUrl &url) {
    // query the favicons module
    QDBusInterface favicon("org.kde.kded", "/modules/favicons", "org.kde.FavIcon");
    QDBusReply<QString> reply = favicon.call("iconForUrl", url.url());

    if (!reply.isValid()) {
        return KIcon();
    }

    // locate the favicon
    QString iconFile = KGlobal::dirs()->findResource("cache", reply.value()+".png");

    if (iconFile.isNull()) {
        return KIcon();
    }
    KIcon icon = KIcon(iconFile);

    return icon;
}

void WebshortcutRunner::run(const Plasma::RunnerContext &context, const Plasma::QueryMatch &match)
{
    QString location = getSearchQuery(match.data().toString(), context.query());

    if (!location.isEmpty()) {
        KToolInvocation::invokeBrowser(location);
    }
}

#include "webshortcutrunner.moc"
