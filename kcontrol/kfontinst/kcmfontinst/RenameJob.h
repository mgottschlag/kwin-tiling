#ifndef __RENAME_JOB_H__
#define __RENAME_JOB_H__

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

#include <kurl.h>
#include <QList>
#include <kio/jobclasses.h>

namespace KFI
{

class CRenameJob : public KIO::Job
{
    Q_OBJECT

    public:

    class Entry
    {
        public:

        class List : public QList<Entry>
        {
            public:

            List()                                 { }
            List(const KUrl &from, const KUrl &to) { append(Entry(from, to)); }
        };

        public:

        Entry()                                                    {}
        Entry(const KUrl &f, const KUrl &t) : itsFrom(f), itsTo(t) {}

        Entry & operator=(const Entry &e) { itsFrom=e.from(); itsTo=e.to(); return *this; }
        bool    operator==(const Entry &e)
                                          { return from()==e.from() && to()==e.to(); }

        const KUrl & from() const { return itsFrom; }
        const KUrl & to() const   { return itsTo;   }

        private:

        KUrl itsFrom,
             itsTo;
    };

    CRenameJob(const Entry::List &src, bool showProgressInfo);

    Q_SIGNALS:

    /**
     * Emitted when the total number of files is known.
     * @param job the job that emitted this signal
     * @param files the total number of files
     */
    void totalFiles(KJob *job, unsigned long files);

    /**
     * Sends the number of processed files.
     * @param job the job that emitted this signal
     * @param files the number of processed files
     */
    void processedFiles(KJob *job, unsigned long files);

    /**
     * Sends the URL of the file that is currently being renamed.
     * @param job the job that emitted this signal
     * @param file the URL of the file or directory that is being 
     *        renamed
     */
    void moving(KJob *, const KUrl &from, const KUrl &to);

    protected Q_SLOTS:

    void         slotStart();
    virtual void slotResult(KJob *job);
    void         slotReport();

    private:

    void renameNext();

    private:

    int         itsProcessed;
    Entry       itsCurrentEntry;
    Entry::List itsList;
    QTimer      *itsReportTimer;
};

}

#endif
