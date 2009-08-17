/*
  Copyright (c) 2009 Chani Armitage <chani@kde.org>

  insert GPL blurb here
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

private:
    Ui::MousePluginWidget m_ui;
    KPluginInfo m_plugin;
    Plasma::ContextAction *m_pluginInstance;
    QDialog *m_configDlg;
    KConfigGroup m_config;

};
#endif

