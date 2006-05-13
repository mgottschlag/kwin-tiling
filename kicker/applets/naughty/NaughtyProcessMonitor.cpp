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

/* OpenBSD support by Jean-Yves Burlett <jean-yves@burlett.org> */

#ifdef __OpenBSD__
#include <sys/param.h>
#include <sys/proc.h>
#include <sys/sysctl.h>
#include <sys/ucred.h>
#include <sys/dkstat.h>
#include <stdlib.h>
#endif

#include <sys/types.h>
#include <signal.h>
#include <unistd.h>

#include <QFile>
#include <QString>
#include <QStringList>
#include <QTextStream>
#include <QDir>
#include <QTimer>
#include <QMap>
#include <QDateTime>

#include <QByteArray>

#include <klocale.h>

#include "NaughtyProcessMonitor.h"

class NaughtyProcessMonitorPrivate
{
  public:

    NaughtyProcessMonitorPrivate()
      : interval_(0),
        timer_(0),
        oldLoad_(0),
        triggerLevel_(0)
    {
    }

    ~NaughtyProcessMonitorPrivate()
    {
      // Empty.
    }

    uint interval_;
    QTimer * timer_;
    QMap<ulong, uint> loadMap_;
    QMap<ulong, uint> scoreMap_;
#ifdef __OpenBSD__
    QMap<ulong, uint> cacheLoadMap_;
    QMap<ulong, uid_t> uidMap_;
#endif
    uint oldLoad_;
    uint triggerLevel_;

  private:

    NaughtyProcessMonitorPrivate(const NaughtyProcessMonitorPrivate &);

    NaughtyProcessMonitorPrivate & operator =
      (const NaughtyProcessMonitorPrivate &);
};

NaughtyProcessMonitor::NaughtyProcessMonitor
  (
   uint interval,
   uint triggerLevel,
   QObject * parent,
   const char * name
  )
  : QObject(parent)
{
  setObjectName( name );
  d = new NaughtyProcessMonitorPrivate;
  d->interval_ = interval * 1000;
  d->triggerLevel_ = triggerLevel;
  d->timer_ = new QTimer(this);
  connect(d->timer_, SIGNAL(timeout()), this, SLOT(slotTimeout()));
}

NaughtyProcessMonitor::~NaughtyProcessMonitor()
{
  delete d;
}

  void
NaughtyProcessMonitor::start()
{
  d->timer_->start(d->interval_, true);
}

  void
NaughtyProcessMonitor::stop()
{
  d->timer_->stop();
}

  uint
NaughtyProcessMonitor::interval() const
{
  return d->interval_ / 1000;
}

  void
NaughtyProcessMonitor::setInterval(uint i)
{
  stop();
  d->interval_ = i * 1000;
  start();
}

  uint
NaughtyProcessMonitor::triggerLevel() const
{
  return d->triggerLevel_;
}

  void
NaughtyProcessMonitor::setTriggerLevel(uint i)
{
  d->triggerLevel_ = i;
}

  void
NaughtyProcessMonitor::slotTimeout()
{
  uint cpu = cpuLoad();

  emit(load(cpu));

  if (cpu > d->triggerLevel_ * (d->interval_ / 1000))
  {
    uint load;
    QList<ulong> l(pidList());
    QList<ulong>::const_iterator itEnd = l.constEnd();
    for (QList<ulong>::const_iterator it = l.constBegin(); it != itEnd; ++it)
    {
        if (getLoad(*it, load))
        {
            _process(*it, load);
        }
    }
  }

  d->timer_->start(d->interval_, true);
}

  void
NaughtyProcessMonitor::_process(ulong pid, uint load)
{
  if (!d->loadMap_.contains(pid))
  {
    d->loadMap_.insert(pid, load);
    return;
  }

  uint oldLoad = d->loadMap_[pid];
  bool misbehaving = (load - oldLoad) > 40 * (d->interval_ / 1000);
  bool wasMisbehaving = d->scoreMap_.contains(pid);

  if (misbehaving)
    if (wasMisbehaving)
    {
      d->scoreMap_.replace(pid, d->scoreMap_[pid] + 1);
      if (canKill(pid))
        emit(runawayProcess(pid, processName(pid)));
    }
    else
      d->scoreMap_.insert(pid, 1);
  else
    if (wasMisbehaving)
      d->scoreMap_.remove(pid);

  d->loadMap_.replace(pid, load);
}

// Here begins the set of system-specific methods.

  bool
NaughtyProcessMonitor::canKill(ulong pid) const
{
#ifdef __linux__
  QFile f("/proc/" + QString::number(pid) + "/status");

  if (!f.open(QIODevice::ReadOnly))
    return false;

  QTextStream t(&f);

  QString s;

  while (!t.atEnd() && s.left(4) != "Uid:")
    s = t.readLine();

  QStringList l(s.split('\t', QString::SkipEmptyParts));

  uint a(l[1].toUInt());

// What are these 3 fields for ? Would be nice if the Linux kernel docs
// were complete, eh ?
//  uint b(l[2].toUInt());
//  uint c(l[3].toUInt());
//  uint d(l[4].toUInt());

  return geteuid() == a;
#elif defined(__OpenBSD__)
  // simply check if entry exists in the uid map and use it
  if (!d->uidMap_.contains(pid))
      return false ;

  return geteuid () == d->uidMap_[pid] ;
#else
  Q_UNUSED( pid );
  return false;
#endif
}

  QString
NaughtyProcessMonitor::processName(ulong pid) const
{
#if defined(__linux__) || defined(__OpenBSD__)
#ifdef __linux__
  QFile f("/proc/" + QString::number(pid) + "/cmdline");

  if (!f.open(QIODevice::ReadOnly))
    return i18n("Unknown");

  QByteArray s;

  while (true)
  {
    char c;
    if ( !f.getChar( &c ) )
      break;

    // Stop at NUL
    if (c == '\0')
      break;
    else
      s += c;
  }

 // Now strip 'kdeinit:' prefix.
  QString unicode(QString::fromLocal8Bit(s));

#elif defined(__OpenBSD__)
  int mib[4] ;
  size_t size ;
  char **argv ;

  // fetch argv for the process `pid'

  mib[0] = CTL_KERN ;
  mib[1] = KERN_PROC_ARGS ;
  mib[2] = pid ;
  mib[3] = KERN_PROC_ARGV ;

  // we assume argv[0]'s size will be less than one page

  size = getpagesize () ;
  argv = (char **)calloc (size, sizeof (char)) ;
  size-- ; // ensure argv is ended by 0
  if (-1 == sysctl (mib, 4, argv, &size, NULL, 0)) {
      free (argv) ;
      return i18n("Unknown") ;
  }

 // Now strip 'kdeinit:' prefix.
  QString unicode(QString::fromLocal8Bit(argv[0]));

  free (argv) ;
#endif

  QStringList parts(unicode.split(' ', QString::SkipEmptyParts));

  QString processName = parts[0] == "kdeinit:" ? parts[1] : parts[0];

  int lastSlash = processName.lastIndexOf('/');

  // Get basename, if there's a path.
  if (-1 != lastSlash)
    processName = processName.mid(lastSlash + 1);

  return processName;

#else
  Q_UNUSED( pid );
  return QString();
#endif
}

  uint
NaughtyProcessMonitor::cpuLoad() const
{
#ifdef __linux__
  QFile f("/proc/stat");

  if (!f.open(QIODevice::ReadOnly))
    return 0;

  bool forgetThisOne = 0 == d->oldLoad_;

  QTextStream t(&f);

  QString s = t.readLine();

  QStringList l(s.split(' ', QString::SkipEmptyParts));

  uint user  = l[1].toUInt();
  uint sys   = l[3].toUInt();

  uint load = user + sys;
  uint diff = load - d->oldLoad_;
  d->oldLoad_ = load;

  return (forgetThisOne ? 0 : diff);
#elif defined(__OpenBSD__)
  int mib[2] ;
  long cp_time[CPUSTATES] ;
  size_t size ;
  uint load, diff ;
  bool forgetThisOne = 0 == d->oldLoad_;

  // fetch CPU time statistics

  mib[0] = CTL_KERN ;
  mib[1] = KERN_CPTIME ;

  size = CPUSTATES * sizeof(long) ;

  if (-1 == sysctl (mib, 2, cp_time, &size, NULL, 0))
      return 0 ;

  load = cp_time[CP_USER] + cp_time[CP_SYS] ;
  diff = load - d->oldLoad_ ;
  d->oldLoad_ = load ;

  return (forgetThisOne ? 0 : diff);
#else
  return 0;
#endif
}

  QList<ulong>
NaughtyProcessMonitor::pidList() const
{
#ifdef __linux__
  QStringList dl(QDir("/proc").entryList());

  QList<ulong> pl;

  QStringList::const_iterator itEnd = dl.constEnd();
  for (QStringList::ConstIterator it = dl.constBegin(); it != dl.end(); ++it)
  {
    if (((*it)[0].isDigit()))
    {
        pl << (*it).toUInt();
    }
  }

  return pl;
#elif defined(__OpenBSD__)
  int mib[3] ;
  int nprocs = 0, nentries ;
  size_t size ;
  struct kinfo_proc *kp ;
  int i ;
  QList<ulong> l;

  // fetch number of processes

  mib[0] = CTL_KERN ;
  mib[1] = KERN_NPROCS ;

  if (-1 == sysctl (mib, 2, &nprocs, &size, NULL, 0))
      return l ;

  // magic size evaluation ripped from ps

  size = (5 * nprocs * sizeof(struct kinfo_proc)) / 4 ;
  kp = (struct kinfo_proc *)calloc (size, sizeof (char)) ;

  // fetch process info

  mib[0] = CTL_KERN ;
  mib[1] = KERN_PROC ;
  mib[2] = KERN_PROC_ALL ;

  if (-1 == sysctl (mib, 3, kp, &size, NULL, 0)) {
      free (kp) ;
      return l ;
  }

  nentries = size / sizeof (struct kinfo_proc) ;

  // time statistics and euid data are fetched only for processes in
  // the pidList, so, instead of doing one sysctl per process for
  // getLoad and canKill calls, simply cache the data we already have.

  d->cacheLoadMap_.clear () ;
  d->uidMap_.clear () ;
  for (i = 0; i < nentries; i++) {
      l << (unsigned long) kp[i].kp_proc.p_pid ;
      d->cacheLoadMap_.insert (kp[i].kp_proc.p_pid,
			       (kp[i].kp_proc.p_uticks +
				kp[i].kp_proc.p_sticks)) ;
      d->uidMap_.insert (kp[i].kp_proc.p_pid,
			 kp[i].kp_eproc.e_ucred.cr_uid) ;
  }

  free (kp) ;

  return l ;
#else
  QList<ulong> l;
  return l;
#endif
}

  bool
NaughtyProcessMonitor::getLoad(ulong pid, uint & load) const
{
#ifdef __linux__
  QFile f("/proc/" + QString::number(pid) + "/stat");

  if (!f.open(QIODevice::ReadOnly))
    return false;

  QTextStream t(&f);

  QString line(t.readLine());

  QStringList fields(line.split(' ', QString::SkipEmptyParts));

  uint userTime (fields[13].toUInt());
  uint sysTime  (fields[14].toUInt());

  load = userTime + sysTime;

  return true;
#elif defined(__OpenBSD__)
  // use cache
  if (!d->cacheLoadMap_.contains(pid))
      return false ;

  load = d->cacheLoadMap_[pid] ;
  return true ;
#else
  Q_UNUSED( pid );
  Q_UNUSED( load );
  return false;
#endif
}

  bool
NaughtyProcessMonitor::kill(ulong pid) const
{
#if defined(__linux__) || defined(__OpenBSD__)
  return 0 == ::kill(pid, SIGKILL);
#else
  Q_UNUSED( pid );
  return false;
#endif
}

#include "NaughtyProcessMonitor.moc"
