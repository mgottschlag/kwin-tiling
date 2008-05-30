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

#include "locationrunner.h"

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


LocationsRunner::LocationsRunner(QObject *parent, const QVariantList& args)
    : Plasma::AbstractRunner(parent, args)
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

static void processUrl(KUrl &url, const QString &term)
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

void LocationsRunner::match(Plasma::RunnerContext &context)
{
    QString term = context.query();
    Plasma::RunnerContext::Type type = context.type();

    if (type == Plasma::RunnerContext::Directory ||
        type == Plasma::RunnerContext::File) {
        Plasma::QueryMatch match(this);
        match.setType(Plasma::QueryMatch::ExactMatch);
        match.setText(i18n("Open %1", term));
        match.setIcon(KIcon("system-file-manager"));
        match.setRelevance(1);
        match.setType(Plasma::QueryMatch::ExactMatch);

        if (type == Plasma::RunnerContext::Directory) {
            match.setId("opendir");
        } else {
            match.setId("openfile");
        }
        context.addMatch(term, match);
    } else if (type == Plasma::RunnerContext::Help) {
        //kDebug() << "Locations matching because of" << type;
        Plasma::QueryMatch match(this);
        match.setType(Plasma::QueryMatch::ExactMatch);
        match.setText(i18n("Open %1", term));
        match.setIcon(KIcon("system-help"));
        match.setRelevance(1);
        match.setRelevance(1);
        match.setType(Plasma::QueryMatch::ExactMatch);
        match.setId("help");
        context.addMatch(term, match);
    } else if (type == Plasma::RunnerContext::NetworkLocation ||
               (type == Plasma::RunnerContext::UnknownType &&
                term.contains(QRegExp("^[a-zA-Z0-9]+\\.")))) {
        KUrl url(term);
        processUrl(url, term);
        QMutexLocker lock(bigLock());
        if (!KProtocolInfo::isKnownProtocol(url.protocol())) {
            return;
        }

        Plasma::QueryMatch match(this);
        match.setText(i18n("Go to %1", url.prettyUrl()));
        match.setIcon(KIcon(KProtocolInfo::icon(url.protocol())));
        match.setData(url.url());

        if (KProtocolInfo::isHelperProtocol(url.protocol())) {
            //kDebug() << "helper protocol" << url.protocol() <<"call external application" ; 
            match.setText(i18n("Launch with %1", KProtocolInfo::exec(url.protocol())));
        } else {
            //kDebug() << "protocol managed by browser" << url.protocol();
            match.setText(i18n("Go to %1", url.prettyUrl()));
        }

        if (type == Plasma::RunnerContext::UnknownType) {
            match.setId("openunknown");
            match.setRelevance(0);
            match.setType(Plasma::QueryMatch::PossibleMatch);
        } else {
            match.setId("opennetwork");
            match.setRelevance(1);
            match.setType(Plasma::QueryMatch::ExactMatch);
        }

        context.addMatch(term, match);
    }
}

void LocationsRunner::run(const Plasma::RunnerContext &context, const Plasma::QueryMatch &match)
{
    QString data = match.data().toString();
    const QString location = context.query();
    Plasma::RunnerContext::Type type = context.type();

    //kDebug() << "command: " << match.query();
    //kDebug() << "url: " << location << data;
    if (type == Plasma::RunnerContext::UnknownType) {
        KToolInvocation::invokeBrowser(location);
    } else if (type == Plasma::RunnerContext::NetworkLocation && data.startsWith("http://")) {
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
