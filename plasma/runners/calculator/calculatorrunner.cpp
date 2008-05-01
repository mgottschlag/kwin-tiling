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

#include <plasma/searchcontext.h>


CalculatorRunner::CalculatorRunner( QObject* parent, const QVariantList &args )
    : Plasma::AbstractRunner(parent, args)
{
    KGlobal::locale()->insertCatalog("krunner_calculatorrunner");
    Q_UNUSED(args)

    setObjectName(i18n("Calculator"));
    setIgnoredTypes(Plasma::SearchContext::Directory | Plasma::SearchContext::File | 
                         Plasma::SearchContext::NetworkLocation | Plasma::SearchContext::Executable |
                         Plasma::SearchContext::ShellCommand);
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
        int where=cmd.indexOf('^');
        cmd=cmd.replace('^', ',');
        int preIndex=where-1;
        int postIndex=where+1;
        int count=0;

        QChar decimalSymbol=KGlobal::locale()->decimalSymbol().at(0);     
        //avoid out of range on weird commands 
        preIndex=qMax(0, preIndex);
        postIndex=qMin(postIndex, cmd.length()-1); 
      
        //go backwards looking for the end of the number or expresion
        while (preIndex!=0) {
            QChar current=cmd.at(preIndex);
            QChar next=cmd.at(preIndex-1);
            //kDebug() << "index " << preIndex << " char " << current;
            if (current == ')') {
                count++;
            } else if (current == '(') {
                count--;
            } else {
                if (((next <'9' ) && (next>'0')) || next==decimalSymbol) {
                    preIndex--;
                    continue;
                }
            }
            if (count==0) {
                break;
            }
            preIndex--;
        }   

       //go forwards looking for the end of the number or expresion
        count=0;
        while (postIndex!=cmd.size()-1) {
            QChar current=cmd.at(postIndex);
            QChar next=cmd.at(postIndex+1);
            if (current == '(') {
                count++;
            } else if (current == ')') {
                count--;
            } else {
                if (((next <'9' ) && (next>'0')) || next==decimalSymbol) {
                    postIndex++;
                    continue;
                 }
            }
            if (count==0) {
                break;
            }
            postIndex++;
        }   
       
        preIndex=qMax(0, preIndex);
        postIndex=qMin(postIndex, cmd.length()); 

        cmd.insert(preIndex,"pow(");
        // +1 +4 == next position to the last number after we add 4 new characters pow(
        cmd.insert(postIndex+1+4, ')'); 
        //kDebug() << "from" << preIndex << " to " << postIndex << " got: " << cmd;
    }
}

void CalculatorRunner::userFriendlySubstitutions(QString& cmd)
{
    if (cmd.contains(KGlobal::locale()->decimalSymbol(), Qt::CaseInsensitive)) {
         cmd=cmd.replace(KGlobal::locale()->decimalSymbol(), ".", Qt::CaseInsensitive);
    }

    //no meanless space between friendly guys: helps simplify below code
    cmd.replace(" ","");

    powSubstitutions(cmd);

    if (cmd.contains(QRegExp("\\d+and\\d+"))) {
         cmd=cmd.replace(QRegExp("(\\d+)and(\\d+)"), "\\1&\\2");
    }
    if (cmd.contains(QRegExp("\\d+or\\d+"))) {
         cmd=cmd.replace(QRegExp("(\\d+)or(\\d+)"), "\\1|\\2");
    }
    if (cmd.contains(QRegExp("\\d+xor\\d+"))) {
         cmd=cmd.replace(QRegExp("(\\d+)xor(\\d+)"), "\\1^\\2");
    }
}


void CalculatorRunner::match(Plasma::SearchContext *search)
{
    const QString term = search->searchTerm();
    QString cmd = term;

    if (cmd.length() < 4 || cmd[0] != '=') {
        return;
    }

    cmd = cmd.remove(0, 1).trimmed();

    if (cmd.isEmpty()) {
        return;
    }

    userFriendlySubstitutions(cmd);
 
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
