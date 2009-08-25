/*
 *   Copyright 2009 Aaron Seigo <aseigo@kde.org>
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

#ifndef CONTAINMENT
#define CONTAINMENT

#include <QObject>
#include <QPointer>

namespace Plasma
{
    class Containment;
} // namespace Plasma

class Widget;

class Containment : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name WRITE setName)

public:
    Containment(Plasma::Containment *containment, QObject *parent = 0);
    ~Containment();

    QString name() const;
    void setName(const QString &name);

public Q_SLOTS:
    QString type() const;
    QString location() const;
    uint id() const;
    int screen() const;
    int desktop() const;
    QString formFactor() const;
    QList<int> widgetIds() const;
    Widget *addWidget(const QString &name);
    void remove();

Q_SIGNALS:

private:

private:
    QPointer<Plasma::Containment> m_containment;
};

#endif

