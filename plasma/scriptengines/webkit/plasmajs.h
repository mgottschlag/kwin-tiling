/*
Copyright (c) 2007 Zack Rusin <zack@kde.org>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
 */
#ifndef PLASMAJS_H
#define PLASMAJS_H

#include <plasma/dataengine.h>
#include <QObject>

namespace Plasma {
    class DataEngine;
}

class TestJs : public QObject
{
    Q_OBJECT
public:
    TestJs(QObject *parent=0);
public Q_SLOTS:
    void test();
};

class PlasmaJs : public QObject
{
    Q_OBJECT
public:
    PlasmaJs(QObject *parent=0);
public Q_SLOTS:
    QObject *dataEngine(const QString &name);
    QObject *loadDataEngine(const QString& name);
    void unloadDataEngine(const QString& name);
    QStringList knownEngines();
};

class DataEngineDataWrapper : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int size READ size)
public:
    DataEngineDataWrapper(const Plasma::DataEngine::Data &data);

    int size() const;
public Q_SLOTS:
    QVariant value(const QString &key) const;
    QStringList keys() const;
    QString key(int i) const;
private:
    Plasma::DataEngine::Data m_data;
};

class DataEngineWrapper : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QStringList sources READ sources)
    Q_PROPERTY(bool valid READ isValid)
    Q_PROPERTY(QString icon READ icon)
    Q_PROPERTY(QString engineName READ engineName)
public:
    DataEngineWrapper(Plasma::DataEngine *engine);
    ~DataEngineWrapper();

    QStringList sources() const;
    QString engineName() const;
    bool isValid() const;
    QString icon() const;
public Q_SLOTS:
    QObject *query(const QString &str) const;

private:
    Plasma::DataEngine *m_engine;
};

#endif
