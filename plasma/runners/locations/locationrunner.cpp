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
#include <KProtocolInfo>

#include <kservicetypetrader.h>

#include "locationrunner.h"


LocationsRunner::LocationsRunner(QObject *parent, const QVariantList& args)
    : Plasma::AbstractRunner(parent, args),
      m_type(Plasma::RunnerContext::UnknownType)
{
    KGlobal::locale()->insertCatalog("krunner_locationsrunner");
    Q_UNUSED(args);
    // set the name shown after the result in krunner window
    setObjectName(i18n("Locations"));
    setIgnoredTypes(Plasma::RunnerContext::Executable | Plasma::RunnerContext::ShellCommand);
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

void LocationsRunner::match(Plasma::RunnerContext *search)
{
    QString term = search->query();
    m_type = search->type();
    Plasma::QueryMatch *action = 0;

    if (m_type == Plasma::RunnerContext::Directory ||
        m_type == Plasma::RunnerContext::File) {
        action = new Plasma::QueryMatch(this);
        action->setType(Plasma::QueryMatch::ExactMatch);
        action->setText(i18n("Open %1", term));
        action->setIcon(KIcon("system-file-manager"));
    } else if (m_type == Plasma::RunnerContext::Help) {
        //kDebug() << "Locations matching because of" << m_type;
        action = new Plasma::QueryMatch(this);
        action->setType(Plasma::QueryMatch::ExactMatch);
        action->setText(i18n("Open %1", term));
        action->setIcon(KIcon("system-help"));
        action->setRelevance(1);
    } else if (m_type == Plasma::RunnerContext::NetworkLocation ||
               m_type == Plasma::RunnerContext::UnknownType) {
        KUrl url(term);
        processUrl(url, term);
        QMutexLocker lock(bigLock());
        if (!KProtocolInfo::isKnownProtocol(url.protocol())) {
            return;
        }

        action = new Plasma::QueryMatch(this);
        action->setText(i18n("Go to %1", url.prettyUrl()));
        action->setIcon(KIcon(KProtocolInfo::icon(url.protocol())));
        action->setData(url.url());
 
        if (KProtocolInfo::isHelperProtocol(url.protocol())) {
            //kDebug() << "helper protocol" << url.protocol() <<"call external application" ; 
            action->setText(i18n("Launch with %1", KProtocolInfo::exec(url.protocol())));
        } else {
            //kDebug() << "protocol managed by browser" << url.protocol();
            action->setText(i18n("Go to %1", url.prettyUrl()));
        }
    }

    if (action) {
        action->setRelevance(1);
        action->setType(Plasma::QueryMatch::ExactMatch);
        search->addMatch(term, action);
    }
}

void LocationsRunner::run(const Plasma::RunnerContext *search, const Plasma::QueryMatch *action)
{
    QString data = action->data().toString();
    const QString location = search->query();

    //kDebug() << "command: " << action->query();
    //kDebug() << "url: " << location << data;
    if (m_type == Plasma::RunnerContext::UnknownType) {
        KToolInvocation::invokeBrowser(location);
    } else if (m_type == Plasma::RunnerContext::NetworkLocation && data.left(4) == "http") {
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
