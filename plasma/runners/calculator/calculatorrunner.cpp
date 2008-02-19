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

#include <QHBoxLayout>
#include <QLabel>
#include <QWidget>
#include <QScriptEngine>

#include <KIcon>

CalculatorRunner::CalculatorRunner( QObject* parent, const QVariantList &args )
    : Plasma::AbstractRunner(parent, args)
{
    KGlobal::locale()->insertCatalog("krunner_calculatorrunner");
    Q_UNUSED(args)

    setObjectName(i18n("Calculator"));
}

CalculatorRunner::~CalculatorRunner()
{
}

void CalculatorRunner::match(Plasma::SearchContext *search)
{
    const QString term = search->searchTerm();
    QString cmd = term;

    if (cmd.length() < 3 || cmd[0] != '=') {
        return;
    }

    cmd = cmd.remove(0, 1).trimmed();

    if (cmd.isEmpty()) {
        return;
    }

    cmd.replace(QRegExp("([a-zA-Z]+)"), "Math.\\1");
    QString result = calculate(cmd);

    if (!result.isEmpty() && result != cmd) {
        Plasma::SearchMatch *match = new Plasma::SearchMatch(this);
        match->setType(Plasma::SearchMatch::InformationalMatch);
        match->setIcon(KIcon("accessories-calculator"));
        match->setText(QString("%1 = %2").arg(cmd, result));
        match->setData("= " + result);
        search->addMatch(term, match);
    }
}

QString CalculatorRunner::calculate( const QString& term )
{
    //kDebug() << "calculating" << term;
    QScriptEngine eng;
    QScriptValue result = eng.evaluate(term);

    if (result.isError()) {
        return QString();
    }

    return result.toString();
}

#include "calculatorrunner.moc"
