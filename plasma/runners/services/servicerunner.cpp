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

QString formattedName( KService::Ptr service )
{
    QString name = service->name();

    if ( !service->genericName().isEmpty() ) {
        name = i18n( "%1 - %2", name, service->genericName() );
    } else if ( !service->comment().isEmpty() ) {
        name = i18n( "%1 - %2", name, service->comment() );
    }

    return name;
}

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

void ServiceRunner::match(Plasma::SearchContext *search)
{
    const QString term = search->searchTerm();
    if (term.length() <  3) {
        return;
    }

    QMutexLocker lock(bigLock());
    KService::Ptr service = KService::serviceByName(term);

    QList<Plasma::SearchMatch*> matches;

    QHash<QString, bool> seen;
    if (service && !service->exec().isEmpty()) {
        //kDebug() << service->name() << "is an exact match!";
        Plasma::SearchMatch *match = new Plasma::SearchMatch(this);
        match->setType(Plasma::SearchMatch::ExactMatch);
        setupAction(service, match);
        match->setRelevance(1);
        matches << match;
        seen[service->storageId()] = true;
        seen[service->exec()] = true;
    }

    QString query = QString("exist Exec and ('%1' ~in Keywords or '%2' ~~ GenericName or '%3' ~~ Name)").arg(term, term, term);
    const KService::List services = KServiceTypeTrader::self()->query("Application", query);

    //kDebug() << "got " << services.count() << " services from " << query;
    foreach (const KService::Ptr service, services) {
        QString id = service->storageId();
        QString exec = service->exec();
        if (seen.contains(id) || seen.contains(exec)) {
            //kDebug() << "already seen" << id << exec;
            continue;
        }

        //kDebug() << "haven't seen" << id << "so processing now";
        seen[id] = true;
        seen[exec] = true;

        Plasma::SearchMatch *match = new Plasma::SearchMatch(this);
        setupAction(service, match);
        qreal relevance(0.6);

        if (service->name().contains(term, Qt::CaseInsensitive)) {
            relevance = .8;
        } else if (service->genericName().contains(term, Qt::CaseInsensitive)) {
            relevance = .7;
        }

        if (service->categories().contains("KDE")) {
            relevance += .1;
        }

        //kDebug() << service->name() << "is this relevant:" << relevance;
        match->setRelevance(relevance);
        matches << match;
    }

    search->addMatches(term, matches);
}

void ServiceRunner::exec(const Plasma::SearchContext *search, const Plasma::SearchMatch *action)
{
    QMutexLocker lock(bigLock());
    KService::Ptr service = KService::serviceByStorageId(action->data().toString());
    if (service) {
        KRun::run(*service, KUrl::List(), 0);
    }
}

void ServiceRunner::setupAction(const KService::Ptr &service, Plasma::SearchMatch *action)
{
    action->setText(service->name());
    action->setData(service->storageId());

    if (!service->icon().isEmpty()) {
        action->setIcon(KIcon(service->icon()));
    }
}

#include "servicerunner.moc"

