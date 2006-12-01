/*
 * KFontInst - KDE Font Installer
 *
 * (c) 2003-2006 Craig Drummond <craig@kde.org>
 *
 * ----
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "RenameJob.h"
#include <QFile>
#include <assert.h>
#include <kio/observer.h>
#include <kio/scheduler.h>
#include <kdirnotify.h>

#define REPORT_TIMEOUT 200

namespace KFI
{

CRenameJob::CRenameJob(const Entry::List &src, bool showProgressInfo)
          : Job(showProgressInfo), 
            itsProcessed(0),
            itsList(src),
            itsReportTimer(NULL)
{
    if(showProgressInfo)
    {
        connect(this, SIGNAL(totalFiles(KJob *, unsigned long)), Observer::self(),
                SLOT(slotTotalFiles(KJob *, unsigned long)));

        emit totalFiles(this, itsList.count());
        itsReportTimer=new QTimer(this);
        connect(itsReportTimer, SIGNAL(timeout()), SLOT(slotReport()));
        itsReportTimer->setSingleShot(false);
        itsReportTimer->start(REPORT_TIMEOUT);
    }

    QTimer::singleShot(0, this, SLOT(slotStart()));
}

void CRenameJob::slotStart()
{
    renameNext();
}

void CRenameJob::slotReport()
{
   if(progressId()!=0)
   {
       Observer *observer = Observer::self();

       emit moving(this, itsCurrentEntry.from(), itsCurrentEntry.to());
       observer->slotMoving(this, itsCurrentEntry.from(), itsCurrentEntry.to());
       observer->slotProcessedFiles(this, itsProcessed);
       emit processedFiles(this, itsProcessed);
       emitPercent(itsProcessed, itsList.count());
    }
}

void CRenameJob::renameNext()
{
    if(!itsList.isEmpty())
    {
        KIO::SimpleJob *job;

        do
        {
            // Take first file to rename out of list
            Entry::List::Iterator it = itsList.begin();

            // If local file, try do it directly
            if ((*it).from().isLocalFile() &&
                0==::rename(QFile::encodeName((*it).from().path()),
                            QFile::encodeName((*it).to().path())))
            {
                job = 0;
                itsProcessed++;
                if(0 == itsProcessed%150)
                {
                    itsCurrentEntry=(*it);
                    slotReport();
                }
            }
            else 
            {
                // if remote - or if rename() failed (we'll use the job's error handling in
                //that case)
                job = KIO::rename((*it).from(), (*it).to(), false);
                KIO::Scheduler::scheduleJob(job);
                itsCurrentEntry=(*it);
            }

            itsList.erase(it);

            if(job)
            {
                addSubjob(job);
                return;
            }
            // loop only if direct deletion worked (job=0) and there is something else to delete
        }
        while(!job && !itsList.isEmpty());
    }

    // Finished - tell the world
    if(!itsList.isEmpty())
    {
        Entry::List::Iterator it;

        for(it=itsList.begin(); it!=itsList.end(); ++it)
            org::kde::KDirNotify::emitFileRenamed((*it).from().url(), (*it).to().url());
    }

    if(NULL!=itsReportTimer)
        itsReportTimer->stop();

    emitResult();
}

void CRenameJob::slotResult(KJob *job)
{
    if(job->error())
    {
        Job::slotResult(job); // will set the error and emit result(this)
        return;
    }
    removeSubjob(job);
    assert(!hasSubjobs());
    itsProcessed++;
    renameNext();
}

}

#include "RenameJob.moc"
