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

#include <QFile>
#include <QXmlStreamReader>

#include <krandom.h>
#include <KDebug>
#include <KDE/KIO/MimetypeJob>
#include <KDE/KIO/FileJob>

#include "shareprovider.h"
#include "share_package.h"

Plasma::PackageStructure::Ptr ShareProvider::m_packageStructure(0);


ShareProvider::ShareProvider(QObject *parent)
    : QObject(parent), m_isBlob(false), m_isPost(true)
{
    // Just make the boundary random part long enough to be sure
    // it's not inside one of the arguments that we are sending
    m_boundary  = "----------";
    m_boundary += KRandom::randomString(55).toAscii();
}

QString ShareProvider::method() const
{
    if (!m_isPost) {
        return QString("GET");
    }
    return QString("POST");
}

void ShareProvider::setMethod(const QString &method)
{
    if (method == "GET") {
        m_isPost = false;
    } else {
        m_isPost = true;
    }
}

KUrl ShareProvider::url() const
{
    // the url that is set in this provider
    return m_url;
}

void ShareProvider::setUrl(const QString &url)
{
    // set the provider's url
    m_url = url;
    m_service = url;
}

QString ShareProvider::parseXML(const QString &key, const QString &data)
{
    // this method helps plugins to parse results from webpages
    QXmlStreamReader xml(data);
    if (xml.hasError()) {
        return QString();
    }

    while (!xml.atEnd()) {
        xml.readNext();

        if (xml.name() == key) {
            QString url = xml.readElementText();
            return url;
        }
    }

    return QString();
}

void ShareProvider::addPostItem(const QString &key, const QString &value,
                                const QString &contentType)
{
    if (!m_isPost)
        return;

    // add a pair <item,value> in a post form
    QByteArray str;
    QString length = QString("%1").arg(value.length());

    str += "--";
    str += m_boundary;
    str += "\r\n";

    if (!key.isEmpty()) {
        str += "Content-Disposition: form-data; name=\"";
        str += key.toAscii();
        str += "\"\r\n";
    }

    if (!contentType.isEmpty()) {
        str += "Content-Type: " + QByteArray(contentType.toAscii());
        str += "\r\n";
        str += "Mime-version: 1.0 ";
        str += "\r\n";
    }

    str += "Content-Length: ";
    str += length.toAscii();
    str += "\r\n\r\n";
    str += value.toUtf8();

    m_buffer.append(str);
    m_buffer.append("\r\n");
}

void ShareProvider::addPostFile(const QString &contentKey, const QString &content)
{
    // add a file in a post form (gets it using KIO)
    m_contentKey = contentKey;
    m_content = content;

    // we expect either text or an URL of a file. The file can be a text file
    // that is an exception that we handle in this case. So we have basically
    // three use cases:
    // 1 - just text
    // 2 - a file that is a text
    // 3 - other kind of files (images, pdf, etc..)
    //
    // The applet using this engine must ensure that the provider selected
    // supports the file format that is added here as a parameter, otherwise
    // it will return as an error later in the process.
    KUrl url(m_content);

    KIO::MimetypeJob *mjob = KIO::mimetype(url);
    if (!mjob->exec()) {
        // it's not a file - usually this happens when we are
        // just sharing plain text, so add the content and publish it
        addPostItem(m_contentKey, m_content, "text/plain");
        addQueryItem(m_contentKey, m_content);
        emit readyToPublish();
        return;
    }

    // It's a valid file because there were no errors
    m_mimetype = mjob->mimetype();
    if (m_mimetype.isEmpty()) {
        // if we ourselves can't determine the mime of the file,
        // very unlikely the remote site will be able to identify it
        error(i18n("Could not detect the file's mimetype"));
        return;
    }

    // If it's not plain text then we should handle it later
    if (m_mimetype != "text/plain") {
        m_isBlob = true;
    }

    // try to open the file
    KIO::FileJob *fjob = KIO::open(KUrl(m_content), QIODevice::ReadOnly);
    connect(fjob, SIGNAL(open(KIO::Job*)), this, SLOT(openFile(KIO::Job*)));
}

void ShareProvider::openFile(KIO::Job *job)
{
    // finished opening the file, now try to read it's content
    KIO::FileJob *fjob = static_cast<KIO::FileJob*>(job);
    fjob->read(fjob->size());
    connect(fjob, SIGNAL(data(KIO::Job*,QByteArray)),
            this, SLOT(finishedContentData(KIO::Job*,QByteArray)));
}

void ShareProvider::finishedContentData(KIO::Job *job, const QByteArray &data)
{
    Q_UNUSED(job);
    if (data.length() == 0) {
        error(i18n("It was not possible to read the selected file"));
        return;
    }

    if (!m_isBlob) {
        // it's just text and we can return here using data()
        addPostItem(m_contentKey, data.data(), "text/plain");
        addQueryItem(m_contentKey, data.data());
        emit readyToPublish();
        return;
    }

    // Add the special http post stuff with the content of the file
    QByteArray str;
    const QString fileSize = QString("%1").arg(data.size());
    str += "--";
    str += m_boundary;
    str += "\r\n";
    str += "Content-Disposition: form-data; name=\"";
    str += m_contentKey.toAscii();
    str += "\"; ";
    str += "filename=\"";
    str += QFile::encodeName(KUrl(m_content).fileName()).replace(".tmp", ".jpg");
    str += "\"\r\n";
    str += "Content-Length: ";
    str += fileSize.toAscii();
    str += "\r\n";
    str += "Content-Type: ";
    str +=  m_mimetype.toAscii();
    str += "\r\n\r\n";

    m_buffer.append(str);
    m_buffer.append(data);
    m_buffer.append("\r\n");

    // tell the world that we are ready to publish
    emit readyToPublish();
}

void ShareProvider::readPublishData(KIO::Job *job, const QByteArray &data)
{
    Q_UNUSED(job);
    m_data.append(data);
}

void ShareProvider::finishedPublish(KJob *job)
{
    Q_UNUSED(job);
    if (m_data.length() == 0) {
        error(i18n("Service was not available"));
        return;
    }

    // process data. should be interpreted by the plugin.
    // plugin must call the right slots after processing the data.
    emit handleResultData(QString(m_data));
}

void ShareProvider::finishHeader()
{
    QByteArray str;
    str += "--";
    str += m_boundary;
    str += "--";
    m_buffer.append(str);
}

void ShareProvider::addQueryItem(const QString &key, const QString &value)
{
    // just add the item to the query's URL
    m_url.addQueryItem(key, value);
}

void ShareProvider::publish()
{
    if (m_url == "") {
        emit finishedError(i18n("You must specify a URL for this service"));
    }

    // clear the result data before publishing
    m_data.clear();

    // finish the http form
    if (m_isBlob) {
        finishHeader();
    }

    // Multipart is used to upload files
    KIO::TransferJob *tf;
    if (m_isBlob) {
        tf = KIO::http_post(m_service, m_buffer, KIO::HideProgressInfo);
        tf->addMetaData("content-type","Content-Type: multipart/form-data; boundary=" + m_boundary);
    } else {
        if (m_isPost) {
            tf = KIO::http_post(m_service,
                                m_url.encodedQuery(), KIO::HideProgressInfo);
            tf->addMetaData("content-type", "Content-Type: application/x-www-form-urlencoded");
        } else {
            QString url = QString("%1?%2").arg(m_service.url(), QString(m_url.encodedQuery()));
            tf = KIO::get(url);
        }
    }

    connect(tf, SIGNAL(data(KIO::Job*,QByteArray)),
            this, SLOT(readPublishData(KIO::Job*,QByteArray)));
    connect(tf, SIGNAL(result(KJob*)), this, SLOT(finishedPublish(KJob*)));
    connect(tf, SIGNAL(redirection(KIO::Job*,KUrl)),
            this, SLOT(redirected(KIO::Job*,KUrl)));
}

void ShareProvider::redirected(KIO::Job *job, const KUrl &to)
{
    Q_UNUSED(job)
    const QUrl toUrl(to);
    const QUrl serviceUrl(m_service);

    const QString toString(toUrl.toString(QUrl::StripTrailingSlash));
    const QString serviceString(serviceUrl.toString(QUrl::StripTrailingSlash));

    if (toString == serviceString) {
        return;
    }

    emit handleRedirection(toString);
}

void ShareProvider::success(const QString &url)
{
    // notify the service that it worked and the result url
    emit finished(url);
}

void ShareProvider::error(const QString &msg)
{
    // notify the service that it didnt work and the error msg
    emit finishedError(msg);
}

Plasma::PackageStructure::Ptr ShareProvider::packageStructure()
{
    if (!m_packageStructure) {
        m_packageStructure = new SharePackage();
    }
    return m_packageStructure;
}

#include "shareprovider.moc"
