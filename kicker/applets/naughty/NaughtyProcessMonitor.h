/*
    Naughty applet - Runaway process monitor for the KDE panel

    Copyright 2000 Rik Hemsley (rikkus) <rik@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#ifndef NAUGHTY_PROCESS_MONITOR_H
#define NAUGHTY_PROCESS_MONITOR_H

#include <QObject>
#include <QList>

class NaughtyProcessMonitorPrivate;

class NaughtyProcessMonitor : public QObject
{
  Q_OBJECT

  public:

    NaughtyProcessMonitor
      (
       uint interval,
       uint triggerLevel,
       QObject * parent = 0,
       const char * name = 0
      );

    virtual ~NaughtyProcessMonitor();

    void start();
    void stop();

    uint triggerLevel() const;
    void setTriggerLevel(uint);
    uint interval() const;
    void setInterval(uint);

    virtual uint cpuLoad() const;
    virtual QList<ulong> pidList() const;
    virtual bool getLoad(ulong pid, uint & load) const;
    virtual QString processName(ulong pid) const;
    virtual bool canKill(ulong pid) const;
    virtual bool kill(ulong pid) const;

  protected Q_SLOTS:

    void slotTimeout();

  Q_SIGNALS:

    void load(uint);
    void runawayProcess(ulong pid, const QString & name);

  private:

    void _process(ulong pid, uint load);

    NaughtyProcessMonitorPrivate * d;
};

#endif

