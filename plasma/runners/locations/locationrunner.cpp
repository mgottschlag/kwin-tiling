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
#include <QDBusInterface>
#include <QDBusReply>

#include <KDebug>
#include <KRun>
#include <KLocale>
#include <KMimeType>
#include <KShell>
#include <KToolInvocation>
#include <KUrl>
#include <KIcon>

#include <kservicetypetrader.h>

#include "locationrunner.h"


LocationsRunner::LocationsRunner(QObject *parent, const QVariantList& args)
    : Plasma::AbstractRunner(parent, args),
      m_type(Plasma::SearchContext::UnknownType)
{
    KGlobal::locale()->insertCatalog("krunner_locationsrunner");
    Q_UNUSED(args);
    // set the name shown after the result in krunner window
    setObjectName(i18n("Locations"));
}

LocationsRunner::~LocationsRunner()
{
}

void processUrl(KUrl &url, const QString &term)
{
    if (url.protocol().isEmpty()) {
        int idx = term.indexOf('/');
        url.clear();
        url.setHost(term.left(idx));
        if (idx != -1) {
            url.setPath(term.mid(idx));
        }
        url.setProtocol("http");
    }
}

void LocationsRunner::match(Plasma::SearchContext *search)
{
    QString term = search->searchTerm();
    m_type = search->type();
    Plasma::SearchMatch *action = 0;

    if (m_type == Plasma::SearchContext::Directory ||
        m_type == Plasma::SearchContext::File) {
        action = new Plasma::SearchMatch(this);
        action->setType(Plasma::SearchMatch::ExactMatch);
        action->setText(i18n("Open %1", term));
        action->setIcon(KIcon("system-file-manager"));
    } else if (m_type == Plasma::SearchContext::Help) {
        //kDebug() << "Locations matching because of" << m_type;
        action = new Plasma::SearchMatch(this);
        action->setType(Plasma::SearchMatch::ExactMatch);
        action->setText(i18n("Open %1", term));
        action->setIcon(KIcon("system-help"));
        action->setRelevance(1);
    } else if (m_type == Plasma::SearchContext::NetworkLocation) {
        KUrl url(term);
        processUrl(url, term);

        action = new Plasma::SearchMatch(this);
        action->setText(i18n("Go to %1", url.prettyUrl()));
        action->setIcon(KIcon("internet-web-browser"));
        action->setData(url.url());
    }

    if (action) {
        action->setRelevance(1);
        action->setType(Plasma::SearchMatch::ExactMatch);
        search->addMatch(term, action);
    }
}

void LocationsRunner::exec(const Plasma::SearchContext *search, const Plasma::SearchMatch *action)
{
    QString data = action->data().toString();
    const QString location = search->searchTerm();

    //kDebug() << "command: " << action->searchTerm();
    //kDebug() << "url: " << location << data;
    if (m_type == Plasma::SearchContext::UnknownType) {
        KToolInvocation::invokeBrowser(location);
    } else if (m_type == Plasma::SearchContext::NetworkLocation && data.left(4) == "http") {
        // the text may have changed while we were running, so we have to refresh
        // our content
        KUrl url(location);
        processUrl(url, location);
        KToolInvocation::invokeBrowser(url.url());
    } else {
        new KRun(KShell::tildeExpand(location), 0);
    }
}

#include "locationrunner.moc"
