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

#include <QTimer>
#include <QWidget>

// pulls in definition for Window
#include <KSelectionWatcher>

#include "runner.h"

class QFrame;
class QLabel;
class QListWidget;
class QListWidgetItem;
class QSvgRenderer;
class QVBoxLayout;

class KLineEdit;
class KPushButton;

class CollapsibleWidget;
class SearchMatch;

namespace Plasma
{
    class Theme;
}

class Interface : public QWidget
{
    Q_OBJECT
    Q_CLASSINFO( "D-Bus Interface", "org.kde.krunner.Interface" )

    public:
        explicit Interface( QWidget* parent = 0 );
        ~Interface();

    public Q_SLOTS:
        // DBUS interface. if you change these methods, you MUST run:
        // qdbuscpp2xml interface.h -o org.kde.krunner.Interface.xml
        void display( const QString& term = QString() );

    protected Q_SLOTS:
        void match( const QString& term );
        void themeChanged();
        void setWidgetPalettes();
        void updateMatches();
        void exec();
        void matchActivated( QListWidgetItem* );
        void fuzzySearch();
        void showOptions(bool show);
        void setDefaultItem( QListWidgetItem* );

    protected:
        void paintEvent( QPaintEvent *e );
        void resizeEvent( QResizeEvent *e );
        void hideEvent( QHideEvent* e );

    private:
        Plasma::Theme* m_theme;
        QSvgRenderer* m_bgRenderer;
        QPixmap m_renderedSvg;
        bool m_renderDirty;

        QTimer m_searchTimer;
        Plasma::Runner::List m_runners;

        QVBoxLayout* m_layout;
        QFrame* m_header;
        KLineEdit* m_searchTerm;
        QListWidget* m_matchList;
        QLabel* m_optionsLabel;
        QLabel* m_headerLabel;
        KPushButton* m_cancelButton;
        KPushButton* m_runButton;
        KPushButton* m_optionsButton;
        CollapsibleWidget* m_expander;

        SearchMatch* m_defaultMatch;
        QMap<Plasma::Runner*, SearchMatch*> m_matches;
        QList<SearchMatch*> m_searchMatches;
};

#endif
