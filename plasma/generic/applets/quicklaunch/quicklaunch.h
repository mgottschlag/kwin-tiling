/***************************************************************************
 *   Copyright (C) 2008 by Lukas Appelhans <l.appelhans@gmx.de>            *
 *   Copyright (C) 2010 by Ingomar Wesp <ingomar@wesp.name>                *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 ***************************************************************************/
#ifndef QUICKLAUNCH_QUICKLAUNCH_H
#define QUICKLAUNCH_QUICKLAUNCH_H

#include "ui_quicklaunchConfig.h"

// Qt
#include <QtCore/QObject>

// Plasma
#include <Plasma/Applet>

class KUrl;

namespace Plasma
{
    class Dialog;
    class IconWidget;
}

class QEvent;
class QGraphicsLinearLayout;
class QGraphicsSceneContextMenuEvent;
class QPoint;

class KConfigGroup;

namespace Quicklaunch {

class IconGrid;

class Quicklaunch : public Plasma::Applet
{
    Q_OBJECT

public:
    Quicklaunch(QObject *parent, const QVariantList &args);
    ~Quicklaunch();

    void init();

    void createConfigurationInterface(KConfigDialog *parent);
    bool eventFilter(QObject *watched, QEvent *event);

protected:
    void constraintsEvent(Plasma::Constraints constraints);
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);

private Q_SLOTS:
    void onConfigAccepted();
    void onIconsChanged();
    void onDisplayedItemsChanged();
    void onDialogArrowClicked();
    void onDialogIconClicked();
    void onAddIconAction();
    void onRemoveIconAction();

private:
    void showContextMenu(
        const QPoint& screenPos,
        IconGrid *component,
        int iconIndex);

    void syncDialogSize();

    void initActions();
    void initDialog();
    void deleteDialog();

    void readConfig();
    void migrateConfig(KConfigGroup &config);

    Ui::quicklaunchConfig uiConfig;

    IconGrid *m_primaryIconGrid;

    QGraphicsLinearLayout *m_layout;
    Plasma::IconWidget *m_dialogArrow;
    Plasma::Dialog *m_dialog;
    IconGrid *m_dialogIconGrid;

    QAction* m_addIconAction;
    QAction* m_removeIconAction;

    IconGrid *m_currentIconGrid;
    int m_currentIconIndex;
};
}

#endif /* QUICKLAUNCH_QUICKLAUNCH_H */
