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

#include <KLocale>
#include <KRun>
#include <KActionCollection>
#include <KService>
#include <KServiceType>
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

ServiceAction::ServiceAction(KService::Ptr service, QObject* parent)
    : QAction(KIcon(service->icon()), formattedName(service), parent),
      m_service(service)
{
}

ServiceRunner::ServiceRunner( QObject* parent )
    : Plasma::AbstractRunner( parent )
{
    setObjectName( i18n( "Application" ) );
}

ServiceRunner::~ServiceRunner()
{
}

QAction* ServiceRunner::accepts(const QString& term)
{
    KService::Ptr service = KService::serviceByName(term);
    QAction* action = 0;

    if ( service && !service->exec().isEmpty() ) {
        action = new ServiceAction(service, this);
    }

    return action;
}

void ServiceRunner::fillMatches( KActionCollection* matches,
                                 const QString& term,
                                 int max, int offset )
{
    Q_UNUSED( max )
    Q_UNUSED( offset )

    QString query = QString("exist Exec and '%1' ~in Keywords and Name != '%2'").arg(term, term);

    const KService::List services = KServiceTypeTrader::self()->query( "Application", query );

    //kDebug() << "got " << services.count() << " services from " << query;

    foreach ( const KService::Ptr service, services ) {
        ServiceAction* action = new ServiceAction(service, matches);
        matches->addAction(service->name(), action);
        connect(action, SIGNAL(triggered()), SLOT(launchService()));
    }
}

bool ServiceRunner::exec(QAction* action, const QString& term)
{
    KService::Ptr service;
    ServiceAction* serviceAction = qobject_cast<ServiceAction*>(action);

    if (serviceAction) {
        service = serviceAction->m_service;
    }

    if (!service && !term.isEmpty()) {
        service = KService::serviceByName(term);
    }

    return service &&
           KRun::run(*service, KUrl::List(), 0) != 0;
}

void ServiceRunner::launchService()
{
    ServiceAction* action = qobject_cast<ServiceAction*>(sender());

    if (!action) {
        return;
    }

    KRun::run(*action->m_service, KUrl::List(), 0);
}

#include "servicerunner.moc"
