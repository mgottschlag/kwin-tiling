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
#include <KService>
#include <KShell>
#include <KToolInvocation>
#include <KUrl>
#include <KIcon>

#include <kservicetypetrader.h>

#include "locationrunner.h"


LocationsRunner::LocationsRunner(QObject *parent, const QVariantList& args)
    : Plasma::AbstractRunner(parent),
      m_type(Plasma::SearchContext::UnknownType)
{
    Q_UNUSED(args);
    // set the name shown after the result in krunner window
    setObjectName(i18n("Locations"));
}

LocationsRunner::~LocationsRunner()
{
}

void LocationsRunner::match(Plasma::SearchContext *search)
{
    QString term = search->searchTerm().toLower();
    m_type = search->type();

    if (m_type == Plasma::SearchContext::Directory ||
        m_type == Plasma::SearchContext::File) {
        Plasma::SearchMatch *action = search->addExactMatch(this);
        action->setText(i18n("Open %1", term));
        action->setIcon(KIcon("system-file-manager"));
        action->setRelevance(1);
        return;
    }

    if (m_type == Plasma::SearchContext::Help) {
        kDebug() << "Locations matching because of" << m_type;
        Plasma::SearchMatch *action = search->addExactMatch(this);
        action->setText(i18n("Open %1", term));
        action->setIcon(KIcon("system-help"));
        action->setRelevance(1);
        return;
    }

    if (m_type == Plasma::SearchContext::NetworkLocation) {
        Plasma::SearchMatch *action = search->addPossibleMatch(this);
        KUrl url(term);

        if (url.protocol().isEmpty()) {
            url.clear();
            url.setHost(term);
            url.setProtocol("http");
        }

        action->setText(i18n("Go to %1", url.prettyUrl()));
        action->setIcon(KIcon("internet-web-browser"));
        action->setData(url.url());
        return;
    }

}

void LocationsRunner::exec(Plasma::SearchMatch *action)
{
    QString location = action->data().toString();

    if (location.isEmpty()) {
        location = action->searchTerm();
    }

    //kDebug() << "command: " << action->term();
    //kDebug() << "url: " << location;
    if (m_type == Plasma::SearchContext::UnknownType ||
        (m_type == Plasma::SearchContext::NetworkLocation &&
         location.left(4) == "http")) {
        KToolInvocation::invokeBrowser(location);
    } else {
        new KRun(KShell::tildeExpand(location), 0);
    }
}

#include "locationrunner.moc"
