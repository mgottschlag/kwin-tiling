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

#include <plasma/contextaction.h>

class ContextMenu : public Plasma::ContextAction
{
    Q_OBJECT
public:
    ContextMenu(QObject* parent, const QVariantList& args);
    ~ContextMenu();

    void init(const KConfigGroup&);

    void contextEvent(QEvent *event);
    void contextEvent(QGraphicsSceneMouseEvent *event);
    void wheelEvent(QGraphicsSceneWheelEvent *event);
    QList<QAction*> contextualActions();

public slots:
    void updateImmutability(const Plasma::ImmutabilityType immutable);

    void runCommand();

    void lockScreen();
    void logout();

    void addPanel();
    void addPanel(const QString &plugin);

private:
    QMenu *m_addPanelsMenu;
    QAction *m_addPanelAction;
    QAction *m_runCommandAction;
    QAction *m_lockScreenAction;
    QAction *m_logoutAction;

};

K_EXPORT_PLASMA_CONTEXTACTION(contextmenu, ContextMenu)

#endif
