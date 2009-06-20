/*
 *   Copyright (C) 2007 Tobias Koenig <tokoe@kde.org>
 *   Copyright (C) 2008 Marco Martin <notmart@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify  
 *   it under the terms of the GNU Library General Public License as published by  
 *   the Free Software Foundation; either version 2 of the License, or     
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


#include "faviconprovider.h"

#include <QtGui/QImage>
#include <QtCore/QFile>

#include <KUrl>
#include <KIO/Job>
#include <KIO/StoredTransferJob>
#include <KMimeType>
#include <KDebug>
#include <kstandarddirs.h>

class FaviconProvider::Private
{
    public:
        Private( FaviconProvider *parent )
          : q(parent)
        {
        }

        void imageRequestFinished( KJob *job );

        FaviconProvider *q;
        QImage image;
        QString cachePath;
};

void FaviconProvider::Private::imageRequestFinished(KJob *job)
{
    if (job->error()) {
        emit q->error(q);
        return;
    }

    KIO::StoredTransferJob *storedJob = qobject_cast<KIO::StoredTransferJob*>(job);
    image = QImage::fromData(storedJob->data());
    if (!image.isNull()) {
        image.save(cachePath, "PNG");
    }
    emit q->finished(q);
}


FaviconProvider::FaviconProvider(QObject *parent, const QString &url)
    : QObject(parent),
      m_url(url),
      d(new Private(this))
{
    KUrl faviconUrl(url);
    if (faviconUrl.protocol().isEmpty()) {
        faviconUrl = KUrl("http://" + url);
    }

    const QString fileName = KMimeType::favIconForUrl(faviconUrl.url());

    if (!fileName.isEmpty()) {
        d->cachePath = KStandardDirs::locateLocal("cache",  fileName + ".png");
        d->image.load(d->cachePath, "PNG");
    } else {
        d->cachePath = KStandardDirs::locateLocal("cache",  "favicons/" + faviconUrl.host() + ".png");
        faviconUrl.setPath("/favicon.ico");

        if (faviconUrl.isValid()) {
            KIO::StoredTransferJob *job = KIO::storedGet(faviconUrl, KIO::NoReload, KIO::HideProgressInfo);
            //job->setProperty("uid", id);
            connect(job, SIGNAL(result(KJob*)), this, SLOT(imageRequestFinished(KJob*)));
        }
    }
}

FaviconProvider::~FaviconProvider()
{
    delete d;
}

QImage FaviconProvider::image() const
{
    return d->image;
}

QString FaviconProvider::identifier() const
{
    return m_url;
}

#include "faviconprovider.moc"
