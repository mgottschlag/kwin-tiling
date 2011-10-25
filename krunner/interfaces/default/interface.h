/***************************************************************************
 *   Copyright 2006 by Aaron Seigo <aseigo@kde.org>                        *
 *   Copyright 2008 by Davide Bettio <davide.bettio@kdemail.net>           *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#ifndef INTERFACE_H
#define INTERFACE_H

#include <QMultiMap>
#include <QTimer>

// local includes
#include "krunnerdialog.h"
#include "resultitem.h"

class QGraphicsView;
class QGraphicsItem;
class QLabel;
class QToolButton;
class QHBoxLayout;
class QVBoxLayout;

class KCompletion;
class KrunnerHistoryComboBox;
class KLineEdit;

class ResultItem;
class ResultScene;

namespace Plasma
{
    class RunnerManager;
    class Svg;
}

class Interface : public KRunnerDialog
{
    Q_OBJECT

    public:
        explicit Interface(Plasma::RunnerManager *m_runnerManager, QWidget* parent = 0);
        ~Interface();

        void setConfigWidget(QWidget *w);
        bool eventFilter(QObject *obj, QEvent *event);

    public Q_SLOTS:
        void display(const QString& term = QString());
        void clearHistory();

    protected:
        void resizeEvent(QResizeEvent *);
        void hideEvent(QHideEvent *e);

    private slots:
        void fitWindow();
        void reenableHoverEvents();
        void run(ResultItem *item);
        void runDefaultResultItem();
        void queryTextEdited(const QString &query);
        void matchCountChanged(int count);
        void hideResultsArea();
        void themeUpdated();
        void resetInterface();
        void showHelp();
        void cleanupAfterConfigWidget();
        void configWidgetDestroyed();
        void searchTermSetFocus();
        void resetAndClose();
        void delayedQueryLaunch();
        void updateSystemActivityToolTip();

    private:
        void saveDialogSize(KConfigGroup &group);
        void restoreDialogSize(KConfigGroup &group);
        void centerOnScreen();
        void setStaticQueryMode(bool staticQuery);
        void resetResultsArea();

        QTimer m_hideResultsTimer;
        QTimer m_reenableHoverEventsTimer;

        QWidget *m_buttonContainer;
        QVBoxLayout* m_layout;
        QLabel *m_previousPage;
        QLabel *m_nextPage;
        QToolButton *m_configButton;
        QToolButton *m_activityButton;
        QToolButton *m_helpButton;
        QToolButton *m_closeButton;
        KrunnerHistoryComboBox* m_searchTerm;
        KCompletion *m_completion;
        QGraphicsView *m_resultsView;
        ResultScene *m_resultsScene;
        int m_minimumHeight;
        QSize m_defaultSize;
        QLabel *m_singleRunnerIcon;
        QLabel *m_singleRunnerDisplayName;
        KLineEdit *m_singleRunnerSearchTerm;
        QTimer m_delayedQueryTimer;
        SharedResultData m_resultData;
        bool m_delayedRun : 1;
        bool m_running : 1;
        bool m_queryRunning : 1;
};

#endif
