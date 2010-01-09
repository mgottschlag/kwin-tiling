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

#include <QScriptEngine>

#include <KIcon>

CalculatorRunner::CalculatorRunner( QObject* parent, const QVariantList &args )
    : Plasma::AbstractRunner(parent, args)
{
    Q_UNUSED(args)

    setObjectName("Calculator");
    setIgnoredTypes(Plasma::RunnerContext::Directory | Plasma::RunnerContext::File |
                         Plasma::RunnerContext::NetworkLocation | Plasma::RunnerContext::Executable |
                         Plasma::RunnerContext::ShellCommand);

    QString description = i18n("Calculates the value of :q: when :q: is made up of numbers and "
                               "mathematical symbols such as +, -, /, * and ^.");
    addSyntax(Plasma::RunnerSyntax("=:q:", description));
    addSyntax(Plasma::RunnerSyntax(":q:=", description));
}

CalculatorRunner::~CalculatorRunner()
{
}

void CalculatorRunner::powSubstitutions(QString& cmd)
{
     if (cmd.contains("e+", Qt::CaseInsensitive)) {
         cmd=cmd.replace("e+", "*10^", Qt::CaseInsensitive);
     }

     if (cmd.contains("e-", Qt::CaseInsensitive)) {
         cmd=cmd.replace("e-", "*10^-", Qt::CaseInsensitive);
     }

    // the below code is scary mainly because we have to honor priority
    // honor decimal numbers and parenthesis.
    while (cmd.contains('^')) {
        int where = cmd.indexOf('^');
        cmd = cmd.replace(where, 1, ',');
        int preIndex = where - 1;
        int postIndex = where + 1;
        int count = 0;

        QChar decimalSymbol = KGlobal::locale()->decimalSymbol().at(0);
        //avoid out of range on weird commands
        preIndex = qMax(0, preIndex);
        postIndex = qMin(postIndex, cmd.length()-1);

        //go backwards looking for the beginning of the number or expression
        while (preIndex != 0) {
            QChar current = cmd.at(preIndex);
            QChar next = cmd.at(preIndex-1);
            //kDebug() << "index " << preIndex << " char " << current;
            if (current == ')') {
                count++;
            } else if (current == '(') {
                count--;
            } else {
                if (((next <= '9' ) && (next >= '0')) || next == decimalSymbol) {
                    preIndex--;
                    continue;
                }
            }
            if (count == 0) {
                //check for functions
                if (!((next <= 'z' ) && (next >= 'a'))) {
                    break;
                }
            }
            preIndex--;
        }

       //go forwards looking for the end of the number or expression
        count = 0;
        while (postIndex != cmd.size() - 1) {
            QChar current=cmd.at(postIndex);
            QChar next=cmd.at(postIndex + 1);

            //check for functions
            if ((count == 0) && (current <= 'z') && (current >= 'a')) {
                postIndex++;
                continue;
            }

            if (current == '(') {
                count++;
            } else if (current == ')') {
                count--;
            } else {
                if (((next <= '9' ) && (next >= '0')) || next == decimalSymbol) {
                    postIndex++;
                    continue;
                 }
            }
            if (count == 0) {
                break;
            }
            postIndex++;
        }

        preIndex = qMax(0, preIndex);
        postIndex = qMin(postIndex, cmd.length());

        cmd.insert(preIndex,"pow(");
        // +1 +4 == next position to the last number after we add 4 new characters pow(
        cmd.insert(postIndex + 1 + 4, ')');
        //kDebug() << "from" << preIndex << " to " << postIndex << " got: " << cmd;
    }
}

void CalculatorRunner::hexSubstitutions(QString& cmd)
{
    if (cmd.contains("0x")) {
        //Append +0 so that the calculator can serve also as a hex converter
        cmd.append("+0");
        bool ok;
        int pos = 0;
        QString hex;

        while (cmd.contains("0x")) {
            hex.clear();
            pos = cmd.indexOf("0x", pos);

            for (int q = 0; q < cmd.size(); q++) {//find end of hex number
                QChar current = cmd[pos+q+2];
                if (((current <= '9' ) && (current >= '0')) || ((current <= 'F' ) && (current >= 'A')) || ((current <= 'f' ) && (current >= 'a'))) { //Check if valid hex sign
                    hex[q] = current;
                } else {
                    break;
                }
            }
            cmd = cmd.replace(pos, 2+hex.length(), QString::number(hex.toInt(&ok,16))); //replace hex with decimal
        }
    }
}

void CalculatorRunner::userFriendlySubstitutions(QString& cmd)
{
    if (cmd.contains(KGlobal::locale()->decimalSymbol(), Qt::CaseInsensitive)) {
         cmd=cmd.replace(KGlobal::locale()->decimalSymbol(), ".", Qt::CaseInsensitive);
    }

    hexSubstitutions(cmd);
    powSubstitutions(cmd);

    if (cmd.contains(QRegExp("\\d+and\\d+"))) {
         cmd = cmd.replace(QRegExp("(\\d+)and(\\d+)"), "\\1&\\2");
    }
    if (cmd.contains(QRegExp("\\d+or\\d+"))) {
         cmd = cmd.replace(QRegExp("(\\d+)or(\\d+)"), "\\1|\\2");
    }
    if (cmd.contains(QRegExp("\\d+xor\\d+"))) {
         cmd = cmd.replace(QRegExp("(\\d+)xor(\\d+)"), "\\1^\\2");
    }
}


void CalculatorRunner::match(Plasma::RunnerContext &context)
{
    const QString term = context.query();
    QString cmd = term;

    //no meanless space between friendly guys: helps simplify code
    cmd = cmd.trimmed().replace(' ', "");

    if (cmd.length() < 4) {
        return;
    }
    if (cmd.toLower() == "universe" || cmd.toLower() == "life") {
        Plasma::QueryMatch match(this);
        match.setType(Plasma::QueryMatch::InformationalMatch);
        match.setIcon(KIcon("accessories-calculator"));
        match.setText("= 42");
        match.setId(QString());
        context.addMatch(term, match);
        return;
    }

    bool toHex = cmd.startsWith("hex=");
    bool startsWithEquals = !toHex && cmd[0] == '=';

    if (toHex || startsWithEquals) {
        cmd.remove(0, cmd.indexOf('=') + 1);
    } else if (cmd.endsWith('=')) {
        cmd.chop(1);
    } else {
        // we don't have an actionable equation here
        return;
    }

    if (cmd.isEmpty()) {
        return;
    }

    userFriendlySubstitutions(cmd);
    cmd.replace(QRegExp("([a-zA-Z]+)"), "Math.\\1"); //needed for accessing math funktions like sin(),....

    QString result = calculate(cmd);

    if (!result.isEmpty() && result != cmd) {
        if (toHex) {
            result = "0x" + QString::number(result.toInt(), 16).toUpper();
        }

        Plasma::QueryMatch match(this);
        match.setType(Plasma::QueryMatch::InformationalMatch);
        match.setIcon(KIcon("accessories-calculator"));
        match.setText(result);
        match.setData("= " + result);
        match.setId(QString());
        context.addMatch(term, match);
    }
}

QString CalculatorRunner::calculate(const QString& term)
{
    //kDebug() << "calculating" << term;
    QScriptEngine eng;
    QScriptValue result = eng.evaluate(" var result ="+term+"; result");

    if (result.isError()) {
        return QString();
    }

    QString resultString = result.toString();
    if (resultString.isEmpty()) {
        return QString();
    }

    if (!resultString.contains('.')) {
            return resultString;
        }

    //ECMAScript has issues with the last digit in simple rational computations
    //This script rounds off the last digit; see bug 167986
    QString roundedResultString = eng.evaluate("var exponent = 15-(1+Math.floor(Math.log(result)/Math.log(10)));\
                                                var order=Math.pow(10,exponent);\
                                                (order > 0? Math.round(result*order)/order : 0)").toString();

    roundedResultString.replace(".", KGlobal::locale()->decimalSymbol(), Qt::CaseInsensitive);

    return roundedResultString;

}

#include "calculatorrunner.moc"
