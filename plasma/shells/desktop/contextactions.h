/*
  Copyright (c) 2009 Chani Armitage <chani@kde.org>

  insert GPL blurb here
*/

#ifndef CONTEXTACTIONDIALOG_H
#define CONTEXTACTIONDIALOG_H

#include "ui_ContextActions.h"
#include "ui_MouseInput.h"

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

    bool eventFilter(QObject *watched, QEvent *event);

signals:
    void modified(bool isModified);

public slots:
    void settingsChanged(bool isModified);
    void configAccepted();
    void containmentChanged(Plasma::Containment *c);

private slots:
    void currentItemChanged(QListWidgetItem *current);
    void getMouseInput();

private:
    void reloadConfig();
    /**
     * reassign the selected plugin's trigger to be @p newTrigger
     */
    void setTrigger(const QString &newTrigger);

    Ui::ContextActions m_ui;
    Ui::MouseInput m_mouseUi;
    QDialog *m_mouseDlg;
    Plasma::Containment *m_containment;
    QHash<QString, QString> m_plugins; //TODO might a different structure be better?
    QSet<QString> m_modifiedKeys;
};

#endif

