/*
 *   Copyright (C) 2007 Barış Metin <baris@pardus.org.tr>
 *   Copyright (C) 2006 David Faure <faure@kde.org>
 *   Copyright (C) 2007 Richard Moore <rich@kde.org>
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

#include "calculatorrunner.h"

#include <QAction>
#include <QHBoxLayout>
#include <QLabel>
#include <QWidget>
#include <QScriptEngine>

#include <KIcon>

CalculatorRunner::CalculatorRunner( QObject* parent, const QVariantList &args )
    : Plasma::AbstractRunner( parent ),
      m_options( 0 )
{
    Q_UNUSED(args)

    setObjectName( i18n( "Calculator" ) );
}

CalculatorRunner::~CalculatorRunner()
{
    delete m_options;
}

QAction* CalculatorRunner::accepts( const QString& term )
{
    QString cmd = term.trimmed();
    QAction *action = 0;

    if ( !cmd.isEmpty() && 
         ( cmd[0].isNumber() || ( cmd[0] == '(') ) &&
         ( QRegExp("[a-zA-Z\\]\\[]").indexIn(cmd) == -1 ) ) {
        QString result = calculate(cmd);

        if ( !result.isEmpty() ) {
            action = new QAction(KIcon("accessories-calculator"),
                                 QString("%1 = %2").arg(term, result),
                                 this);
            action->setEnabled( false );
        }
    }

    return action;
}

bool CalculatorRunner::exec(QAction* action, const QString& term)
{
    Q_UNUSED(action)
    Q_UNUSED(term)
    return true;
}

QString CalculatorRunner::calculate( const QString& term )
{
    QScriptEngine eng;
    QScriptValue result = eng.evaluate( term );
    return result.toString();
}

#include "calculatorrunner.moc"
