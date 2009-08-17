/*
  Copyright (c) 2009 Chani Armitage <chani@kde.org>

  insert GPL blurb here
*/

#ifndef CONTEXTACTIONDIALOG_H
#define CONTEXTACTIONDIALOG_H

#include "ui_ContextActions.h"

namespace Plasma {
    class Containment;
}

class KConfigDialog;

class ContextActions : public QWidget
{
    Q_OBJECT
public:
    ContextActions(Plasma::Containment *containment, KConfigDialog *parent);
    ~ContextActions();

signals:
    void modified(bool isModified);

public slots:
    void settingsChanged(bool isModified);
    void configAccepted();
    void containmentPluginChanged(Plasma::Containment *c);

private slots:
    /**
     * reassign the plugin's trigger to be @p newTrigger
     */
    void setTrigger(const QString &plugin, const QString &oldTrigger, const QString &newTrigger);

private:

    Ui::ContextActions m_ui;
    Plasma::Containment *m_containment;
    QHash<QString, QString> m_plugins;
    QSet<QString> m_modifiedKeys;
};

#endif

