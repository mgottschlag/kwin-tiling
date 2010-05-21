/*
 *   Copyright (C) 2007-2008 Ryan P. Bitanga <ryan.bitanga@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .
 */

#ifndef QS_DIALOG_H
#define QS_DIALOG_H

#include "krunnerdialog.h"

namespace Plasma
{
    class QueryMatch;
    class RunnerManager;
}

namespace QuickSand
{
    class QsMatchView;
    class MatchItem;
    class QueryMatchItem;
}

using QuickSand::MatchItem;

/**
 * This is an ugly test dialog
 * It's only real purpose is to test the MatchView widgets
 */
class QsDialog : public KRunnerDialog
{
    Q_OBJECT
    public:
        explicit QsDialog(Plasma::RunnerManager *runnerManager, QWidget *parent = 0);
        ~QsDialog();

        void display(const QString& term = QString());
        void clearHistory();
        void setConfigWidget(QWidget *w);

    private slots:
        void run(MatchItem *item);
        void launchQuery(const QString &query);
        void setMatches(const QList<Plasma::QueryMatch> &matches);
        void loadActions(MatchItem *item);
        void setAction(MatchItem *item);
        void configWidgetDestroyed();
        void cleanupAfterConfigWidget();
        //afiestas: We should move this to krunnerdialog imho (I just copyied it atm)
        void updateSystemActivityToolTip();
    private:
        void adjustInterface();

        bool m_newQuery;
        QLabel *m_singleRunnerIcon;
        QToolButton *m_configButton;
        QToolButton *m_activityButton;

        QMultiMap<QString, Plasma::QueryMatch> m_matches;
        QuickSand::QsMatchView *m_matchView;
        QuickSand::QsMatchView *m_actionView;
        QuickSand::QueryMatchItem *m_currentMatch;
};

#endif
