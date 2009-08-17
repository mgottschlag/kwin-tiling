/*
 *   Copyright (c) 2009 Chani Armitage <chani@kde.org>
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

#ifndef MOUSEPLUGINWIDGET_H
#define MOUSEPLUGINWIDGET_H

#include "ui_MousePluginWidget.h"

#include <QDialog>

#include <KPluginInfo>
#include <KConfigGroup>

#include <plasma/contextaction.h>

class MousePluginWidget : public QWidget
{
    Q_OBJECT
public:
    MousePluginWidget(const KPluginInfo &plugin, QWidget *parent = 0);

    void setConfigGroup(KConfigGroup cfg);
    KConfigGroup configGroup();

    void setTrigger(const QString &trigger);

signals:
    void triggerChanged(const QString &plugin, const QString &oldTrigger, const QString &newTrigger);
    void configChanged(const QString &trigger);

public slots:
    void setContainment(Plasma::Containment *ctmt);

private slots:
    void changeTrigger(const QString &oldTrigger, const QString& newTrigger);
    void clearTrigger();

    void configure();
    void acceptConfig();
    void rejectConfig();
    void save();
    void showAbout();

private:
    Ui::MousePluginWidget m_ui;
    KPluginInfo m_plugin;
    Plasma::ContextAction *m_pluginInstance;
    QDialog *m_configDlg;
    KConfigGroup m_config;

};
#endif

