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

#ifndef CONTEXTMENU_HEADER
#define CONTEXTMENU_HEADER

#include <QButtonGroup>
#include <plasma/containmentactions.h>

class ContextMenu : public Plasma::ContainmentActions
{
    Q_OBJECT
public:
    ContextMenu(QObject* parent, const QVariantList& args);
    ~ContextMenu();

    void init(const KConfigGroup&);

    void contextEvent(QEvent *event);
    QList<QAction*> contextualActions();
    QAction* action(const QString &name);

    QWidget* createConfigurationInterface(QWidget* parent);
    void configurationAccepted();
    void save(KConfigGroup &config);

public slots:
    void runCommand();
    void lockScreen();
    void startLogout();
    void logout();

private:
    QAction *m_runCommandAction;
    QAction *m_lockScreenAction;
    QAction *m_logoutAction;
    QAction *m_separator1;
    QAction *m_separator2;
    QAction *m_separator3;

    QList<QString> m_allActions;
    QList<bool> m_enabledActions;
    QButtonGroup *m_buttons;

};

K_EXPORT_PLASMA_CONTAINMENTACTIONS(contextmenu, ContextMenu)

#endif
