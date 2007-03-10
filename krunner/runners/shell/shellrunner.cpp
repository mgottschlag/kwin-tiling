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
#include <QPushButton>

#include <KAuthorized>
#include <KIcon>
#include <KLocale>
#include <KRun>
#include <KStandardDirs>

#include "shellrunner.h"
#include "ui_shellOptions.h"

ShellRunner::ShellRunner( QObject* parent )
    : Plasma::Runner( parent ),
      m_options( 0 )
{
    setObjectName( i18n( "Command" ) );
    m_enabled = KAuthorized::authorizeKAction( "shell_access" );
}

ShellRunner::~ShellRunner()
{
    delete m_options;
    delete m_optionsUi;
}

QAction* ShellRunner::accepts(const QString& term)
{
    if ( !m_enabled ) {
        return 0;
    }

    QString executable = KStandardDirs::findExe(term);
    if ( !executable.isEmpty() ) {
        QAction* action = new QAction( KIcon( "exec" ), executable, this );
        return action;
    } else {
        return 0;
    }
}

bool ShellRunner::hasOptions()
{
    return true;
}

QWidget* ShellRunner::options()
{
    if ( !m_options ) {
        m_optionsUi = new Ui::shellOptions();
        m_options = new QWidget;
        m_optionsUi->setupUi( m_options );
    }

    return m_options;
}

bool ShellRunner::exec(const QString& command)
{
    if ( !m_enabled ) {
        return false;
    }

    return ( KRun::runCommand( command ) != 0 );
}

#include "shellrunner.moc"
