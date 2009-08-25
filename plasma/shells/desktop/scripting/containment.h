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
class PanelView;

class Containment : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name WRITE setName)
    Q_PROPERTY(QString type READ type)
    Q_PROPERTY(QString formFactor READ formFactor)
    Q_PROPERTY(QList<int> widgetIds READ widgetIds)
    Q_PROPERTY(int screen READ screen WRITE setScreen)
    Q_PROPERTY(int desktop READ desktop WRITE setDesktop)
    Q_PROPERTY(QString location READ location WRITE setLocation)
    Q_PROPERTY(int id READ id)

    // panel properties
    Q_PROPERTY(QString alignment READ alignment WRITE setAlignment)

public:
    Containment(Plasma::Containment *containment, QObject *parent = 0);
    ~Containment();

    uint id() const;
    QString type() const;
    QString formFactor() const;
    QList<int> widgetIds() const;

    QString name() const;
    void setName(const QString &name);

    QString location() const;
    void setLocation(const QString &location);

    int screen() const;
    void setScreen(int screen);

    int desktop() const;
    void setDesktop(int desktop);

    QString alignment() const;
    void setAlignment(const QString &alignment);

public Q_SLOTS:
    Widget *addWidget(const QString &name);
    void remove();

Q_SIGNALS:

private:
    PanelView *panel() const;

    QPointer<Plasma::Containment> m_containment;
    bool m_isPanel;
};

#endif

