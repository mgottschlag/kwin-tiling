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
    setIgnoredTypes(Plasma::RunnerContext::Directory | Plasma::RunnerContext::File | 
                         Plasma::RunnerContext::NetworkLocation | Plasma::RunnerContext::Executable |
                         Plasma::RunnerContext::ShellCommand);
}

CalculatorRunner::~CalculatorRunner()
{
}

void CalculatorRunner::powSubstitutions(QString& cmd)
{
     if (cmd.contains("e+", Qt::CaseInsensitive)) {
         cmd=cmd.replace("e+", "^", Qt::CaseInsensitive);
     }

     if (cmd.contains("e-", Qt::CaseInsensitive)) {
         cmd=cmd.replace("e-", "^-", Qt::CaseInsensitive);
     }

    // the below code is scary mainly because we have to honor priority
   // honor decimal numbers and parenthesis. 
    if (cmd.contains('^')){
        int where = cmd.indexOf('^');
        cmd = cmd.replace('^', ',');
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
                if (((next < '9' ) && (next > '0')) || next == decimalSymbol) {
                    preIndex--;
                    continue;
                }
            }
            if (count == 0) {
                break;
            }
            preIndex--;
        }

       //go forwards looking for the end of the number or expression
        count = 0;
        while (postIndex != cmd.size() - 1) { 
            QChar current=cmd.at(postIndex);
            QChar next=cmd.at(postIndex + 1);
            if (current == '(') {
                count++;
            } else if (current == ')') {
                count--;
            } else {
                if (((next < '9' ) && (next > '0')) || next == decimalSymbol) {
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
        bool ok;
        int pos = 0;
        QString hex;

        for (int i = 0; i < cmd.size(); i++) {
            hex.clear();
            pos = cmd.indexOf("0x", pos);

            for (int q = 0; q < cmd.size(); q++) {//find end of hex number
                QChar current = cmd[pos+q+2];
                if (((current <= '9' ) && (current >= '0')) || ((current <= 'F' ) && (current >= 'A'))) { //Check if valid hex sign
                    hex[q] = current;
                } else {
                    break;
                }
            }
            cmd = cmd.replace("0x" + hex,QString::number(hex.toInt(&ok,16))); //replace hex with decimal
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
    cmd = cmd.trimmed().replace(" ", "");

    if (cmd.length() < 4) {
        return;
    }
    bool toHex = cmd.startsWith("hex=");

    if (!toHex && (cmd[0] != '=')) {
        return;
    }

    cmd = cmd.remove(0, cmd.indexOf('=')+1);

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
