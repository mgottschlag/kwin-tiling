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

#include <KActionCollection>
#include <KIcon>
#include <KLocale>

#include "searchrunner.h"

SearchRunner::SearchRunner( QObject* parent, const QStringList& args )
    : Plasma::AbstractRunner( parent )
{
    Q_UNUSED( args );
    setObjectName( i18n( "Search" ) );
}

SearchRunner::~SearchRunner()
{
}

QAction* SearchRunner::accepts( const QString& term )
{
    Q_UNUSED( term )

    // this should probably always turn down the term
    // and only act on actions provided via fillMatches
    return 0;
}

bool SearchRunner::exec( const QString& command )
{
    Q_UNUSED( command )

    return true;
}

void SearchRunner::fillMatches( KActionCollection* matches,
                                const QString& term,
                                int max, int offset )
{
    Q_UNUSED( term )
    Q_UNUSED( max )
    Q_UNUSED( offset )

    //TODO: actually ask strigi for results. for now, we just return a static set
    //      in reality, we probably want to make this async and use matchesUpdated
    QAction* action = matches->addAction( i18n( "Konsole" ) );
    action->setIcon( KIcon( "konsole" ) );
    action->setText( i18n( "Konsole" ) );
    connect( action, SIGNAL(triggered()), SLOT(launchKonsole()) );
}

#include <KRun>
void SearchRunner::launchKonsole()
{
    KRun::runCommand("konsole");
}

#include "searchrunner.moc"
