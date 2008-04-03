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

#ifndef CALCULATORRUNNER_H
#define CALCULATORRUNNER_H

#include <KGenericFactory>

#include <plasma/abstractrunner.h>

class QWidget;

/**
 * This class evaluates the basic expressions given in the interface.
 */
class CalculatorRunner : public Plasma::AbstractRunner
{
    Q_OBJECT

    public:
        CalculatorRunner(QObject* parent, const QVariantList &args);
        ~CalculatorRunner();

        void match(Plasma::SearchContext *context);

    private:
        QString calculate(const QString& term);
        void userFriendlySubstitutions(QString& cmd);
        void powSubstitutions(QString& cmd);
};

K_EXPORT_PLASMA_RUNNER(calculatorrunner, CalculatorRunner)

#endif
