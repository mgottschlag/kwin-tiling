// -*- Mode: C++; c-basic-offset: 4; indent-tabs-mode: nil; tab-width: 8; -*-
/* This file is part of the KDE project
   Copyright (C) 2000 by Carsten Pfeiffer <pfeiffer@kde.org>
   Copyright (C) 2008 by Dmitry Suzdalev <dimsuz@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#ifndef CONFIGDIALOG_H
#define CONFIGDIALOG_H

#include <KConfigDialog>

#include "urlgrabber.h"

#include "ui_generalconfig.h"
#include "ui_actionsconfig.h"

class KConfigSkeleton;
class KShortcutsEditor;
class QPushButton;
class Klipper;
class KEditListBox;
class KActionCollection;

class GeneralWidget : public QWidget
{
    Q_OBJECT
public:
    GeneralWidget(QWidget* parent);

private slots:
    void onSyncronizeToggled(bool);

private:
    Ui::GeneralWidget m_ui;
};

class ActionsWidget : public QWidget
{
    Q_OBJECT
public:
    ActionsWidget(QWidget* parent);

    void setActionList(const ActionList&);
    void setExcludedWMClasses(const QStringList&);

    ActionList actionList() const;
    QStringList excludedWMClasses() const;

    void resetModifiedState();

private slots:
    void onSelectionChanged();
    void onContextMenu(const QPoint&);
    void onItemChanged(QTreeWidgetItem*,int);
    void onAddAction();
    void onDeleteAction();
    void onAdvanced();

private:
    Ui::ActionsWidget m_ui;

    QStringList m_exclWMClasses;
};

// only for use inside ActionWidget
class AdvancedWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AdvancedWidget( QWidget *parent = 0L );
    ~AdvancedWidget();

    void setWMClasses( const QStringList& items );
    QStringList wmClasses() const;

private:
    KEditListBox *editListBox;
};

class ConfigDialog : public KConfigDialog
{
    Q_OBJECT

public:
    ConfigDialog( QWidget *parent, KConfigSkeleton *config, const Klipper* klipper, KActionCollection *collection, bool isApplet );
    ~ConfigDialog();

private:
    // reimp
    void updateWidgets();
    // reimp
    void updateSettings();
    // reimp
    void updateWidgetsDefault();

private:
    ActionsWidget *m_actionsPage;
    KShortcutsEditor *m_shortcutsWidget;

    const Klipper* m_klipper;
};

#endif // CONFIGDIALOG_H
