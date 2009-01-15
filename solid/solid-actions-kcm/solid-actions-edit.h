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

#ifndef SOLID_ACTION_EDIT_H
#define SOLID_ACTION_EDIT_H

#include <KDialog>

#include "ui_solid-actions-edit.h"
#include "ui_solid-actions-edit-predicate.h"

class SolidActionsEdit : public KDialog
{
    Q_OBJECT
public:
    SolidActionsEdit(QWidget *parent = 0);
    ~SolidActionsEdit();

    void fillPredicateTree(QString predicateText);
    QString predicate();
    Ui::SolidActionEdit ui;

private:
    void predicateContainer(QString predicate, QTreeWidgetItem *parent);
    void predicateItem(QString predicate, QTreeWidgetItem *parent);
    QString predicateRetrieve(QTreeWidgetItem *parent);
    Ui::SolidActionEditPredicate predicateUi;
    KDialog * predicateDialog;

private slots:
    void updateButtonUsage();
    void addRequirement();
    void editRequirement();
    void cancelEditRequirement();
    void deleteRequirement();
    void editPredicateTypeToggle();
    void updateRequirement();

};

#endif