#ifndef __FONT_LISTER_H__
#define __FONT_LISTER_H__

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

class KJob;
class QString;

#include <kio/job.h>
#include <kfileitem.h>
#include <QObject>

namespace KFI
{

class CFontLister : public QObject
{
    Q_OBJECT

    public:

    CFontLister(QObject *parent);

    void scan(const KUrl &url=KUrl());
    void setAutoUpdate(bool e);
    bool busy() const { return NULL!=itsJob; }

    Q_SIGNALS:

    void newItems(const KFileItemList &items);
    void deleteItem(KFileItem *item);
    void refreshItems(const KFileItemList &items);
    void completed();
    void percent(int);
    void message(const QString &msg);

    private Q_SLOTS:

    void fileRenamed(const QString &from, const QString &to);
    void filesAdded(const QString &dir);
    void filesRemoved(const QStringList &files);
    void result(KJob *job);
    void entries(KIO::Job *job, const KIO::UDSEntryList &entries);
    void processedSize(KJob *job, qulonglong s);
    void totalSize(KJob *job, qulonglong s);
    void infoMessage(KJob *job, const QString &msg);

    private:

    QMap<KUrl, KFileItem *> itsItems;
    bool                    itsAutoUpdate,
                            itsUpdateRequired;
    KIO::Job                *itsJob;
    qulonglong              itsJobSize;
};

}

#endif
