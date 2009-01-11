/***************************************************************************
 *   Copyright (C) 2009 by Ben Cooksley <ben@eclipse.endoftheinternet.org> *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA          *
 ***************************************************************************/

#ifndef _SOLID_ACTIONS_H_
#define _SOLID_ACTIONS_H_

#include <KCModule>

#include "ui_solid-actions-config.h"
#include "ui_solid-actions-edit.h"
#include "ui_solid-actions-add.h"

class ActionItem;

class SolidActions: public KCModule
{
    Q_OBJECT

public:
    SolidActions( QWidget* parent, const QVariantList&  );
    ~SolidActions();
    void load();
    void save();
    void defaults();

protected:

public slots:
    void addAction();
    void editAction();
    void deleteAction();
    QListWidgetItem * selectedWidget();
    ActionItem * selectedAction();
    void fillActionsList();
    void acceptActionChanges();
    void toggleEditDelete(bool toggle);
    void enableEditDelete();

private:
    Ui_SolidActionsConfig *mainUi;
    Ui_SolidActionEdit *editUi;
    Ui_SolidActionAdd *addUi;
    KDialog *editDialog;
    QWidget *editWidget;
    KDialog *addDialog;
    QWidget *addWidget;
    QMap<QListWidgetItem*, ActionItem*> actionsDb;
    void clearActions();
};

#endif
