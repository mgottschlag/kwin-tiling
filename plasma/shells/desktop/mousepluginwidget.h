/*
  Copyright (c) 2009 Chani Armitage <chani@kde.org>

  insert GPL blurb here
*/

#ifndef MOUSEPLUGINWIDGET_H
#define MOUSEPLUGINWIDGET_H

#include "ui_MousePluginWidget.h"

#include <KPluginInfo>

class MousePluginWidget : public QWidget
{
    Q_OBJECT
public:
    MousePluginWidget(const KPluginInfo &plugin, QWidget *parent = 0);

    void setTrigger(const QString &trigger);

signals:
    void triggerChanged(const QString &plugin, const QString &oldTrigger, const QString &newTrigger);

private slots:
    void changeTrigger(const QString &oldTrigger, const QString& newTrigger);
    void clearTrigger();

private:
    Ui::MousePluginWidget m_ui;
    KPluginInfo m_plugin;

};
#endif

