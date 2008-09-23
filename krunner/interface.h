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

// pulls in definition for Window
#include <KSelectionWatcher>

// local includes
#include "krunnerdialog.h"

class QGraphicsView;
class QLabel;
class QToolButton;
class QHBoxLayout;
class QVBoxLayout;

class KHistoryComboBox;
class KCompletion;

class ResultItem;
class ResultScene;
class KRunnerConfigDialog;

namespace Plasma
{
    class Svg;
}

class Interface : public KRunnerDialog
{
    Q_OBJECT
    Q_CLASSINFO( "D-Bus Interface", "org.kde.krunner.Interface" )

    public:
        explicit Interface( QWidget* parent = 0 );
        ~Interface();

    public Q_SLOTS:
        // DBUS interface. if you change these methods, you MUST run:
        // qdbuscpp2xml interface.h -o org.kde.krunner.Interface.xml
        void display(const QString& term = QString());
        void displayWithClipboardContents();
        void switchUser();
        void clearHistory();

    protected:
        void resizeEvent(QResizeEvent *);
        void closeEvent(QCloseEvent* e);

    private slots:
        void setWidgetPalettes();
        void run(ResultItem *item);
        void runDefaultResultItem();
        void queryTextEdited(const QString &query);
        void showConfigDialog();
        void updateDescriptionLabel(ResultItem *item);
        void configCompleted();
        void matchCountChanged(int count);
        void hideResultsArea();
        void themeUpdated();
        void resetInterface();

    private:
        void centerOnScreen();
        void setStaticQueryMode(bool staticQuery);

        KRunnerConfigDialog *m_configDialog;
        QTimer m_hideResultsTimer;

        QWidget *m_buttonContainer;
        QVBoxLayout* m_layout;
        QHBoxLayout* m_statusLayout;
        QLabel *m_descriptionLabel;
        QLabel *m_previousPage;
        QLabel *m_nextPage;
        Plasma::Svg *m_iconSvg;
        QToolButton *m_configButton;
        QToolButton *m_activityButton;
        QToolButton *m_closeButton;
        KHistoryComboBox* m_searchTerm;
        KCompletion *m_completion;
        QGraphicsView *m_resultsView;
        QWidget *m_dividerLine;
        ResultScene *m_resultsScene;
        bool m_delayedRun;
        bool m_running;
        bool m_queryRunning;
};

#endif
