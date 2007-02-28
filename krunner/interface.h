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

#include <QWidget>

// pulls in definition for Window
#include <KSelectionWatcher>

#include "runner.h"

class QLabel;
class QListWidget;
class QSvgRenderer;

class KLineEdit;

namespace Plasma
{
    class Theme;
}

class Interface : public QWidget
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.krunner.Interface")

    public:
        explicit Interface(QWidget* parent = 0);
        ~Interface();

    public Q_SLOTS:
        // DBUS interface. if you change these methods, you MUST run:
        // qdbuscpp2xml interface.h -o org.kde.krunner.Interface.xml
        void display(const QString& term = QString());

    protected Q_SLOTS:
        void search(const QString& term);
        void checkForCompositionManager(Window);
        void themeChanged();
        void updateMatches();
        void exec();

    protected:
        void paintEvent( QPaintEvent *e );
        void resizeEvent( QResizeEvent *e );
        void hideEvent( QHideEvent* e );

    private:
        void loadRunners();

        bool m_haveCompositionManager;
        KSelectionWatcher* m_compositeWatcher;

        Plasma::Theme* m_theme;
        QSvgRenderer* m_bgRenderer;
        QPixmap m_renderedSvg;
        bool m_renderDirty;

        Runner::List m_runners;
        Runner* m_currentRunner;

        KLineEdit* m_searchTerm;
        QListWidget* m_actionsList;
        QWidget* m_matches;
        QLabel* m_optionsLabel;
};

#endif
