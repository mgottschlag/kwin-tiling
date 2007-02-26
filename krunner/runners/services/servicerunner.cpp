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

#include <KLocale>
#include <KRun>
#include <KService>

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

bool ServiceRunner::accepts(const QString& term)
{
    KService::Ptr service = KService::serviceByName(term);
    return service && !service->exec().isEmpty();
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

#include "servicerunner.moc"
