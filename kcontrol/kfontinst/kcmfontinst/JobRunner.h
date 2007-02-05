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

#ifndef __JOB_RUNNER_H__
#define __JOB_RUNNER_H__

#include <kio/job.h>
#include "ActionDialog.h"

class QLabel;
class QProgressBar;
class KJob;

namespace KFI
{

class CJobRunner : public CActionDialog
{
    Q_OBJECT

    public:

    struct Item : public KUrl
    {
        Item(const KUrl &u=KUrl(), const QString &n=QString()) : KUrl(u), name(n) { }
        QString displayName() const { return name.isEmpty() ? prettyUrl() : name; }
        QString name;
    };

    typedef QList<Item> ItemList;

    enum ECommand
    {
        CMD_INSTALL,
        CMD_DELETE,
        CMD_ENABLE,
        CMD_DISABLE,
        CMD_UPDATE
    };

    CJobRunner(QWidget *parent, int xid=0);
    ~CJobRunner();

    bool            getAdminPasswd(QWidget *parent);
    static void     getAssociatedUrls(const KUrl &url, KUrl::List &list, bool afmAndPfm, QWidget *widget);
    int             exec(ECommand cmd, const ItemList &urls, const KUrl &dest);
    const QString & adminPasswd() const { return itsPasswd; }

    private Q_SLOTS:

    void doNext();
    void jobResult(KJob *job);
    void cfgResult(KJob *job);
    void slotButtonClicked(int button);

    private:

    void setMetaData(KIO::Job *job);

    private:

    ECommand                itsCmd;
    ItemList                itsUrls;
    ItemList::ConstIterator itsIt,
                            itsEnd;
    KUrl                    itsDest;
    QString                 itsPasswd;
    QLabel                  *itsStatusLabel;
    QProgressBar            *itsProgress;
    bool                    itsAutoSkip,
                            itsCancelClicked;
};

}

#endif

