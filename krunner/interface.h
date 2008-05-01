/*
 *   Copyright (C) 2006 Aaron Seigo <aseigo@kde.org>
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

#ifndef INTERFACE_H
#define INTERFACE_H

// pulls in definition for Window
#include <KSelectionWatcher>

// local includes
#include "krunnerdialog.h"

class QLabel;
class QListWidget;
class QListWidgetItem;
class QVBoxLayout;

class KHistoryComboBox;
class KCompletion;
class KPushButton;
class KTitleWidget;

class CollapsibleWidget;

//this is Interface internal SearchMatch
class SearchMatch;

namespace Plasma {
    
    class RunnerManager;
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

    protected Q_SLOTS:
        void match();
        void updateMatches();
        void setWidgetPalettes();
        void run();
        void matchActivated( QListWidgetItem* );
        void showOptions(bool show);
        void setDefaultItem( QListWidgetItem* );

    protected:
        void closeEvent(QCloseEvent* e);

    private:
        void resetInterface();
        
        Plasma::RunnerManager* m_runnerManager;

        QVBoxLayout* m_layout;
        KTitleWidget* m_header;
        KHistoryComboBox* m_searchTerm;
        KCompletion *m_completion;
        QListWidget* m_matchList;
        QLabel* m_optionsLabel;
        KPushButton* m_cancelButton;
        KPushButton* m_runButton;
        KPushButton* m_optionsButton;
        CollapsibleWidget* m_expander;
        QWidget *m_optionsWidget;

        SearchMatch* m_defaultMatch;

        bool m_execQueued;
};

#endif
