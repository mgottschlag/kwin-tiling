/*
 *   Copyright (C) 2007 Barış Metin <baris@pardus.org.tr>
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
#include <QFile>
#include <QHBoxLayout>
#include <QLabel>

#include <KIcon>
#include <KLocale>
#include <KStandardDirs>
#include <KProcess>
#include <KLocale>

#include "calculatorrunner.h"

CalculatorRunner::CalculatorRunner(QObject* parent, const QStringList& args)
    : Runner(parent),
      m_options(0)
{
    Q_UNUSED(args)
	
    setObjectName( i18n( "Calculator" ) );
    kDebug() << "CalculatorRunner initialized" << endl;
}

CalculatorRunner::~CalculatorRunner()
{
    delete m_options;
}

QAction* CalculatorRunner::accepts(const QString& term)
{
    kDebug() << "in CalculatorRunner::accepts" << endl;
    QString cmd = term.stripWhiteSpace();
    QAction *action = 0;
    
    if (!cmd.isEmpty() && (cmd[0].isNumber() || (cmd[0] == '(')) &&
	(QRegExp("[a-zA-Z\\]\\[]").search(cmd) == -1))
    {
	QString result = calculate(cmd);
	action = new QAction(KIcon("exec"),
			     i18n("Result: ") + result,
			     this);
    }

    return action;
}

bool CalculatorRunner::exec(const QString& term)
{
    Q_UNUSED(term)
    return true;
}

QString CalculatorRunner::calculate(const QString& term)
{
    QString result, cmd;
    const QString bc = KStandardDirs::findExe("bc");
    if ( !bc.isEmpty() )
	cmd = QString("echo %1 | %2").arg(KProcess::quote(QString("scale=8; ")+term), KProcess::quote(bc));
    else
	cmd = QString("echo $((%1))").arg(term);
    FILE *fs = popen(QFile::encodeName(cmd).data(), "r");
    if (fs)
    {
	{ // scope for QTextStream
	    QTextStream ts(fs, IO_ReadOnly);
	    result = ts.read().stripWhiteSpace();
	}
	pclose(fs);
    }
    return result;
}

#include "calculatorrunner.moc"
