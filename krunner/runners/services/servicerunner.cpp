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

#include <QWidget>
#include <QAction>
#include <KIcon>

#include <KLocale>
#include <KRun>
#include <KActionCollection>
#include <KService>
#include <KServiceType>
#include <KServiceTypeTrader>

#include "servicerunner.h"

ServiceRunner::ServiceRunner(QObject* parent)
    : Runner(parent),
      m_options(0)
{
    setObjectName( i18n( "Application" ) );
}

ServiceRunner::~ServiceRunner()
{
    delete m_options;
}

QAction* ServiceRunner::accepts(const QString& term)
{
    KService::Ptr service = KService::serviceByName(term);
    QAction* action = 0;

    if ( service && !service->exec().isEmpty() ) {
        action = new QAction( KIcon( service->icon() ),
                              formattedName( service ),
                              this );
    }

    return action;
}

void ServiceRunner::fillMatches( KActionCollection* matches,
                                 const QString& term,
                                 int max, int offset )
{
    Q_UNUSED( max )
    Q_UNUSED( offset )

    //TODO: this is horifically inneficient and doesn't really return very good matches anyways
    //      this is more of a proof-of-concept than anything
    QString query = QString( "exist Exec and '%1' in Keywords and Name != '%2')" ).arg( term, term );
    KServiceType::List serviceTypes = KServiceType::allServiceTypes();

    foreach ( const KServiceType::Ptr serviceType, serviceTypes ) {
        KService::List services = KServiceTypeTrader::self()->query( serviceType->name(),
                                                                     query );

        kDebug() << "got " << services.count() << " services from " << query << endl;

        foreach ( const KService::Ptr service, services ) {
            QAction* action = matches->addAction( service->name() );
            action->setIcon( KIcon( service->icon() ) );
            action->setText( formattedName( service ) );
            // TODO: connect the actions to ... something =)
            //connect( action, SIGNAL(triggered()), SLOT(launchKonsole()) );
        }
    }
}

bool ServiceRunner::hasOptions()
{
    return true;
}

QWidget* ServiceRunner::options()
{
    if (!m_options)
    {
        // create options here
        m_options = new QWidget;
    }

    return m_options;
}

bool ServiceRunner::exec(const QString& term)
{
    KService::Ptr service = KService::serviceByName(term);
    return service &&
           KRun::run(*service, KUrl::List(), 0) != 0;
}

QString ServiceRunner::formattedName( KService::Ptr service )
{
    QString name = service->name();

    if ( !service->genericName().isEmpty() ) {
        name = i18n( "%1 - %2", name, service->genericName() );
    } else if ( !service->comment().isEmpty() ) {
        name = i18n( "%1 - %2", name, service->comment() );
    }

    return name;
}

#include "servicerunner.moc"
