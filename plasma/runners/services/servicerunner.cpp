/*
 *   Copyright (C) 2006 Aaron Seigo <aseigo@kde.org>
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

#include "servicerunner.h"

#include <QWidget>
#include <KIcon>

#include <KDebug>
#include <KLocale>
#include <KRun>
#include <KService>
#include <KServiceTypeTrader>

ServiceRunner::ServiceRunner(QObject *parent, const QVariantList &args)
    : Plasma::AbstractRunner( parent )
{
    Q_UNUSED(args)

    setObjectName(i18n("Application"));
    setPriority(AbstractRunner::HighestPriority);
}

ServiceRunner::~ServiceRunner()
{
}

void ServiceRunner::match(Plasma::RunnerContext &context)
{
    const QString term = context.query();
    if (term.length() <  3) {
        return;
    }

    QMutexLocker lock(bigLock());
    QString query = QString("exist Exec and ('%1' =~ Name)").arg(term);
    KService::List services = KServiceTypeTrader::self()->query("Application", query);

    QList<Plasma::QueryMatch> matches;

    QHash<QString, bool> seen;
    if (!services.isEmpty()) {
        //kDebug() << service->name() << "is an exact match!" << service->storageId() << service->exec();
        KService::Ptr service = services.at(0);
        Plasma::QueryMatch match(this);
        match.setType(Plasma::QueryMatch::ExactMatch);
        setupAction(service, match);
        match.setRelevance(1);
        matches << match;
        seen[service->storageId()] = true;
        seen[service->exec()] = true;
    }

    query = QString("exist Exec and ('%1' ~subin Keywords or '%2' ~~ GenericName or '%3' ~~ Name)").arg(term, term, term);
    services = KServiceTypeTrader::self()->query("Application", query);

    //kDebug() << "got " << services.count() << " services from " << query;
    foreach (const KService::Ptr &service, services) {
        QString id = service->storageId();
        QString exec = service->exec();
        if (seen.contains(id) || seen.contains(exec)) {
            //kDebug() << "already seen" << id << exec;
            continue;
        }

        //kDebug() << "haven't seen" << id << "so processing now";
        seen[id] = true;
        seen[exec] = true;

        Plasma::QueryMatch match(this);
        match.setType(Plasma::QueryMatch::PossibleMatch);
        setupAction(service, match);
        qreal relevance(0.6);

        if (service->name().contains(term, Qt::CaseInsensitive)) {
            relevance = 0.8;
        } else if (service->genericName().contains(term, Qt::CaseInsensitive)) {
            relevance = 0.7;
        }

        if (service->categories().contains("KDE")) {
            if (id.startsWith("kde-")) {
                // This is an older version, let's disambiguate it
                QString subtext("KDE3");
                if (!match.subtext().isEmpty()) {
                    subtext.append(", " + match.subtext());
                }

                match.setSubtext(subtext);
            } else {
                relevance += .1;
            }
        }

        //kDebug() << service->name() << "is this relevant:" << relevance;
        match.setRelevance(relevance);
        matches << match;
    }

    context.addMatches(term, matches);
}

void ServiceRunner::run(const Plasma::RunnerContext &context, const Plasma::QueryMatch &match)
{
    Q_UNUSED(context);
    QMutexLocker lock(bigLock());
    KService::Ptr service = KService::serviceByStorageId(match.data().toString());
    if (service) {
        KRun::run(*service, KUrl::List(), 0);
    }
}

void ServiceRunner::setupAction(const KService::Ptr &service, Plasma::QueryMatch &match)
{
    const QString name = service->name();

    match.setText(name);
    match.setData(service->storageId());

    if (!service->genericName().isEmpty() && service->genericName() != name) {
        match.setSubtext(service->genericName());
    } else if (!service->comment().isEmpty()) {
        match.setSubtext(service->comment());
    }

    if (!service->icon().isEmpty()) {
        match.setIcon(KIcon(service->icon()));
    }
}

#include "servicerunner.moc"

