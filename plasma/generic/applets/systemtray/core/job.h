/***************************************************************************
 *   Copyright (C) 2008 Rob Scheepmaker <r.scheepmaker@student.utwente.nl> *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#ifndef SYSTEMTRAYJOB_H
#define SYSTEMTRAYJOB_H

#include <QtCore/QHash>
#include <QtCore/QObject>

namespace SystemTray
{

class Job : public QObject
{
    Q_OBJECT

public:
    enum State {
        Running = 0,
        Suspended = 1,
        Stopped = 2
    };

    Job(QObject *parent = 0);
    virtual ~Job();

    /**
     * Request and signal destruction of this object
     */
    void destroy();

    /**
     * @return the name of the application which started this job.
     */
    QString applicationName() const;

    /**
     * @return the name of the icon to be used for this job.
     */
    QString applicationIconName() const;

    /**
     * @return the descripion of the activity that is performed.
     */
    QString message() const;

    /**
     * @return the errormessage if an error has occured.
     */
    QString error() const;

    /**
     * @return the speed at which the jobs is progressing.
     */
    QString speed() const;

    /**
     * @return a nice description of the job that has been completed.
     */
    QString completedMessage() const;

    /**
     * @return the time (in seconds) in which this job is expected to complete.
     */
    ulong eta() const;

    void setEta(ulong eta);

    QMap<QString, qlonglong> totalAmounts() const;

    QMap<QString, qlonglong> processedAmounts() const;

    /**
     * @return a list of pairs containing label names/values in the order they should be displayed.
     */
    QList<QPair<QString, QString> > labels() const;

    /**
     * @return the state this job is in.
     */
    State state() const;

    bool isSuspendable() const;

    bool isKillable() const;

    /**
     * @retun the percentage of the job that has been completed.
     */
    uint percentage() const;

public slots:
    /**
     * suspend this job.
     */
    virtual void suspend();

    /**
     * resume this job.
     */
    virtual void resume();

    /**
     * stop this job.
     */
    virtual void stop();

signals:
    /**
     * Emitted when the job is ready to be shown
     */
    void ready(SystemTray::Job *job);

    /**
     * Emitted when the job changes state
     */
    void stateChanged(SystemTray::Job *job);

    /**
     * Emitted when the job details change
     */
    void changed(SystemTray::Job *job);

    /**
     * Emitted when the job is about to be destroyed
     **/
    void destroyed(SystemTray::Job *job);

protected:
    void setApplicationName(const QString &applicationName);
    void setApplicationIconName(const QString &applicationIcon);
    void setMessage(const QString &message);
    void setError(const QString &error);
    void setSpeed(const QString &speed);
    void setTotalAmounts(QMap<QString, qlonglong> amount);
    void setProcessedAmounts(QMap<QString, qlonglong> amount);
    void setState(State state);
    void setSuspendable(bool suspendable);
    void setKillable(bool killable);
    void setPercentage(uint percentage);
    void setLabels(QList<QPair<QString, QString> > labels);
    void timerEvent(QTimerEvent *);

private slots:
    void show();

private:
    void scheduleChangedSignal();

    class Private;
    Private* const d;

    friend class Manager;
};

}
#endif
