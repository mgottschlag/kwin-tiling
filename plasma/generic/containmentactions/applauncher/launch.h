/*
 *   Copyright 2009 by Chani Armitage <chani@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
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

#ifndef SWITCHWINDOW_HEADER
#define SWITCHWINDOW_HEADER

#include <plasma/containmentactions.h>

class QAction;
class KMenu;

class AppLauncher : public Plasma::ContainmentActions
{
    Q_OBJECT
    public:
        AppLauncher(QObject* parent, const QVariantList& args);
        ~AppLauncher();

        void init(const KConfigGroup &config);

        void contextEvent(QEvent *event);
        QList<QAction*> contextualActions();
        //returns true if something (other than a separator) was successfully added
        bool addApp(QMenu *menu, const QString &source);

    public slots:
        void switchTo(QAction *action);

    protected:
        void makeMenu();

    private:
        KMenu *m_menu;
        QAction *m_action;
};

K_EXPORT_PLASMA_CONTAINMENTACTIONS(applauncher, AppLauncher)

#endif
