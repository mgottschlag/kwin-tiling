/***************************************************************************
 *   Copyright 2010 Artur Duque de Souza <asouza@kde.org>                  *
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

#ifndef SHAREPROVIDER_H
#define SHAREPROVIDER_H

#include <QObject>
#include <kio/global.h>
#include <KDE/KIO/Job>

#include <Plasma/PackageStructure>


class ShareProvider : public QObject
{
    Q_OBJECT

public:
    ShareProvider(QObject *parent =0);
    static Plasma::PackageStructure::Ptr packageStructure();

    QString method() const;
    void setMethod(const QString &method);

    KUrl url() const;
    void setUrl(const QString &url);

    void addPostFile(const QString &contentKey, const QString &content);

Q_SIGNALS:
    void handleResultData(QString data);
    void handleRedirection(const QString &url);
    void readyToPublish();
    void finished(const QString &url);
    void finishedError(const QString &msg);

public Q_SLOTS:
    // ###: Function to return the content so the plugin can
    // play with it before publishing?

    // helper methods
    void publish();
    QString parseXML(const QString &key, const QString &data);
    void addQueryItem(const QString &key, const QString &value);
    void addPostItem(const QString &key, const QString &value,
                     const QString &contentType);

    // result methods
    void success(const QString &url);
    void error(const QString &msg);
    void redirected(KIO::Job *job, const KUrl &from);

protected Q_SLOTS:
    // slots for kio
    void openFile(KIO::Job *job);
    void finishedContentData(KIO::Job *job, const QByteArray &data);
    void finishedPublish(KJob *job);
    void readPublishData(KIO::Job *job, const QByteArray &data);

protected:
    void finishHeader();

private:
    QString m_content;
    QString m_contentKey;
    QString m_mimetype;

    bool m_isBlob;
    bool m_isPost;

    KUrl m_url;
    KUrl m_service;

    QByteArray m_data;
    QByteArray m_buffer;
    QByteArray m_boundary;

    static Plasma::PackageStructure::Ptr m_packageStructure;
};

#endif // SHAREPROVIDER
