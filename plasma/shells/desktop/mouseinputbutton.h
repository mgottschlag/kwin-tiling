/*
  Copyright (c) 2009 Chani Armitage <chani@kde.org>

  insert GPL blurb here
*/

#ifndef MOUSEINPUTBUTTON_H
#define MOUSEINPUTBUTTON_H

#include <QHash>
#include <QPushButton>

class QEvent;

class MouseInputButton : public QPushButton
{
    Q_OBJECT
public:
    MouseInputButton(QWidget *parent = 0);

    QString trigger();
    void setTrigger(const QString &trigger);

signals:
    void triggerChanged(const QString &oldTrigger, const QString &newTrigger);

protected:
    bool event(QEvent *event);

private slots:
    void getTrigger();

private:
    void changeTrigger(const QString& newTrigger);

    QString m_trigger;
    QHash<QString,QString> m_prettyStrings;
};

#endif
