////////////////////////////////////////////////////////////////////////////////
//
// Class Name    : CRenameJob
// Author        : Craig Drummond
// Project       : K Font Installer
// Creation Date : 03/06/2003
// Version       : $Revision$ $Date$
//
////////////////////////////////////////////////////////////////////////////////
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
////////////////////////////////////////////////////////////////////////////////
// (C) Craig Drummond, 2003
////////////////////////////////////////////////////////////////////////////////


#include "RenameJob.h"
#include <assert.h>
#include <kio/observer.h>
#include <kio/scheduler.h>
#include <kdirnotify_stub.h>
#include <qfile.h>
#include <qtimer.h>

#define REPORT_TIMEOUT 200

CRenameJob::CRenameJob(const Entry::List &src, bool showProgressInfo)
          : Job(showProgressInfo), 
            itsProcessed(0),
            itsList(src),
            itsReportTimer(NULL)
{
    if(showProgressInfo)
    {
        connect(this, SIGNAL(totalFiles(KIO::Job *, unsigned long)), Observer::self(), SLOT(slotTotalFiles(KIO::Job *, unsigned long)));

        emit totalFiles(this, itsList.count());
        itsReportTimer=new QTimer(this);
        connect(itsReportTimer, SIGNAL(timeout()), this, SLOT(slotReport()));
        itsReportTimer->start(REPORT_TIMEOUT, false);
    }

    QTimer::singleShot(0, this, SLOT(slotStart()));
}

void CRenameJob::slotStart()
{
    renameNext();
}

void CRenameJob::slotReport()
{
   if(m_progressId!=0)
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
            if ((*it).from().isLocalFile() && 0==::rename(QFile::encodeName((*it).from().path()), QFile::encodeName((*it).to().path())))
            {
                job = 0;
                itsProcessed++;
                if(0 == itsProcessed%150)
                {
                    itsCurrentEntry=(*it);
                    slotReport();
                }
            }
            else // if remote - or if rename() failed (we'll use the job's error handling in that case)
            { 
                job = KIO::rename((*it).from(), (*it).to(), false);
                KIO::Scheduler::scheduleJob(job);
                itsCurrentEntry=(*it);
            }

            itsList.remove(it);

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
        KDirNotify_stub       allDirNotify("*", "KDirNotify*");
        Entry::List::Iterator it;

        for(it=itsList.begin(); it!=itsList.end(); ++it)
            allDirNotify.FileRenamed((*it).from(), (*it).to());
    }

    if(NULL!=itsReportTimer)
        itsReportTimer->stop();

    emitResult();
}

void CRenameJob::slotResult(Job *job)
{
    if(job->error())
    {
        Job::slotResult(job); // will set the error and emit result(this)
        return;
    }
    subjobs.remove(job);
    assert(subjobs.isEmpty());
    itsProcessed++;
    renameNext();
}

void CRenameJob::virtual_hook(int id, void *data)
{
    Job::virtual_hook(id, data);
}

#include "RenameJob.moc"
