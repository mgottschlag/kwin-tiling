#ifndef LIBKHOTKEYS_TEST_H
#define LIBKHOTKEYS_TEST_H

#include <QtCore/QObject>
#include <QtCore/QDebug>

class Test : public QObject
    {
    Q_OBJECT

private Q_SLOTS:

    // void init() { qDebug() << "!"; };
    // void cleanup() { qDebug() << "!"; };

    void initTestCase();
    void cleanupTestCase();


    void testLoading();

private:

    bool daemonActive;

    }; // class Test

#endif
