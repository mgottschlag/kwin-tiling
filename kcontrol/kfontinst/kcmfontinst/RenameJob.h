#ifndef __RENAME_JOB_H__
#define __RENAME_JOB_H__

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

#include <kurl.h>
#include <qvaluelist.h>
#include <kio/jobclasses.h>

class CRenameJob : public KIO::Job
{
    Q_OBJECT

    public:

    class Entry
    {
        public:

        class List : public QValueList<Entry>
        {
            public:

            List()                                 { }
            List(const KURL &from, const KURL &to) { append(Entry(from, to)); }
        };

        public:

        Entry()                                                    {}
        Entry(const KURL &f, const KURL &t) : itsFrom(f), itsTo(t) {}

        Entry & operator=(const Entry &e) { itsFrom=e.from(); itsTo=e.to(); return *this; }
        bool    operator==(const Entry &e)
                                          { return from()==e.from() && to()==e.to(); }

        const KURL & from() const { return itsFrom; }
        const KURL & to() const   { return itsTo;   }

        private:

        KURL itsFrom,
             itsTo;
    };

    CRenameJob(const Entry::List &src, bool showProgressInfo);

    signals:

    /**
     * Emitted when the total number of files is known.
     * @param job the job that emitted this signal
     * @param files the total number of files
     */
    void totalFiles(KIO::Job *job, unsigned long files);

    /**
     * Sends the number of processed files.
     * @param job the job that emitted this signal
     * @param files the number of processed files
     */
    void processedFiles(KIO::Job *job, unsigned long files);

    /**
     * Sends the URL of the file that is currently being renamed.
     * @param job the job that emitted this signal
     * @param file the URL of the file or directory that is being 
     *        renamed
     */
    void moving(KIO::Job *, const KURL &from, const KURL &to);

    protected slots:

    void         slotStart();
    virtual void slotResult(KIO::Job *job);
    void         slotReport();

    private:

    void renameNext();

    private:

    int         itsProcessed;
    Entry       itsCurrentEntry;
    Entry::List itsList;
    QTimer      *itsReportTimer;

    protected:

    virtual void virtual_hook(int id, void * data);
};

#endif
