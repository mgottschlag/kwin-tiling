/*
Copyright (c) 2007 Zack Rusin <zack@kde.org>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
 */

#include "bundle.h"

#include <QBuffer>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QXmlStreamReader>

#include <KIO/CopyJob>
#include <KIO/Job>

#include <Plasma/PackageMetadata>
#include <Plasma/Package>

void recursive_print(const KArchiveDirectory *dir, const QString &path)
{
    QStringList l = dir->entries();
    QStringList::Iterator it = l.begin();
    for (; it != l.end(); ++it)
    {
        const KArchiveEntry* entry = dir->entry((*it));
        printf("mode=%07o %s %s size: %lld pos: %lld %s%s isdir=%d%s",
               entry->permissions(),
               entry->user().toLatin1().constData(),
               entry->group().toLatin1().constData(),
               entry->isDirectory() ? 0 : ((KArchiveFile*)entry)->size(),
               entry->isDirectory() ? 0 : ((KArchiveFile*)entry)->position(),
               path.toLatin1().constData(),
               (*it).toLatin1().constData(), entry->isDirectory(),
               entry->symLinkTarget().isEmpty() ? "" :
               QString(" symlink: %1").arg(
                   entry->symLinkTarget()).toLatin1().constData());

        //if (!entry->isDirectory()) printf("%d",
        //     ((KArchiveFile*)entry)->size());
        printf("\n");
        if (entry->isDirectory())
            recursive_print((KArchiveDirectory *)entry, path+(*it)+'/');
    }
}


static const KArchiveDirectory *recursiveFind(const KArchiveDirectory *dir)
{
    QStringList l = dir->entries();
    QStringList::Iterator it;
    for (it = l.begin(); it != l.end(); ++it) {
        const KArchiveEntry* entry = dir->entry((*it));
        if (entry->isDirectory()) {
            QString name = *it;
            if (name.startsWith(QLatin1String("__MACOSX"))) {
                //skip this
                continue;
            } else if (name.endsWith(QLatin1String(".wdgt"))) {
                //got our bad boy
                return static_cast<const KArchiveDirectory*>(entry);
            } else {
                const KArchiveDirectory *fd =
                    recursiveFind(static_cast<const KArchiveDirectory*>(entry));
                if (fd)
                    return fd;
            }
        }
    }
    return 0;
}

Bundle::Bundle(const QString &path)
    : PackageStructure(0, "MacDashboard"),
      m_isValid(false),
      m_width(0), m_height(0)
{
    setContentsPrefix(QString());
    QFile f(path);
    f.open(QIODevice::ReadOnly);
    m_data = f.readAll();
    f.close();
    initTempDir();
    open();
}

Bundle::Bundle(const QByteArray &data)
    : PackageStructure(0, "MacDashboard"),
      m_isValid(false),
      m_width(0),
      m_height(0)
{
    setContentsPrefix(QString());
    m_data = data;
    initTempDir();
    open();
}

Bundle::Bundle(QObject *parent, QVariantList args)
    : PackageStructure(0, "MacDashboard"),
      m_isValid(false),
      m_tempDir(0),
      m_width(0),
      m_height(0)
{
    setContentsPrefix(QString());
}

Bundle::~Bundle()
{
    close();
    qWarning("done");
}

void Bundle::setData(const QByteArray &data)
{
    m_data = data;
    close();
    open();
}

QByteArray Bundle::data() const
{
    return m_data;
}

bool Bundle::open()
{
    if (m_data.isEmpty())
        return false;

    if (!m_tempDir)
        initTempDir();

    QBuffer buffer(&m_data);
    KZip zip(&buffer);
    if (!zip.open(QIODevice::ReadOnly)) {
        qWarning("Couldn't open the bundle!");
        return false;
    }

    const KArchiveDirectory *dir = zip.directory();

    const KArchiveDirectory *foundDir = recursiveFind(dir);
    if (!foundDir) {
        qWarning("not a bundle");
        m_isValid = false;
        zip.close();
        return 0;
    }

    qDebug()<<"Dir = "<<foundDir->name();
    m_isValid = extractArchive(foundDir, QLatin1String(""));

    if (m_isValid) {
        setPath(m_tempDir->name());
    }

    zip.close();

    return m_isValid;
}

bool Bundle::close()
{
    bool ret = m_tempDir;
    delete m_tempDir;
    m_tempDir = 0;
    return ret;
}

bool Bundle::extractArchive(const KArchiveDirectory *dir, const QString &path)
{
    QStringList l = dir->entries();

    QStringList::Iterator it;
    for (it = l.begin(); it != l.end(); ++it) {
        const KArchiveEntry* entry = dir->entry((*it));
        QString fullPath = QString("%1/%2").arg(path).arg(*it);
        if (entry->isDirectory()) {
            QString outDir = QString("%1%2").arg(m_tempDir->name()).arg(path);
            QDir qdir(outDir);
            qdir.mkdir(*it);
            extractArchive(static_cast<const KArchiveDirectory*>(entry), fullPath);
        } else if (entry->isFile()) {
            QString outName = QString("%1%2").arg(m_tempDir->name()).arg(fullPath.remove(0, 1));
            //qDebug()<<"-------- "<<outName;
            QFile f(outName);
            if (!f.open(QIODevice::WriteOnly)) {
                qWarning("Couldn't create %s", qPrintable(outName));
                continue;
            }
            const KArchiveFile *archiveFile = static_cast<const KArchiveFile*>(entry);
            f.write(archiveFile->data());
            f.close();
        } else {
            qWarning("Unidentified entry at %s", qPrintable(fullPath));
        }
    }
    return true;
}

void Bundle::pathChanged()
{
    m_isValid = extractInfo();
}

bool Bundle::extractInfo()
{
    QString plistLocation = QString("%1Info.plist").arg(path());
    QString configXml = QString("%1config.xml").arg(path());
    if (QFile::exists(plistLocation)) {
        //extract from plist
        parsePlist(plistLocation);
    } else if (QFile::exists(configXml)) {
        parseConfigXml(configXml);
    } else {
        return false;
    }

    return true;
}

bool Bundle::parsePlist(const QString &loc)
{
    QFile f(loc);
    if (!f.open(QIODevice::ReadOnly)) {
        qWarning("Couldn't open info file: '%s'", qPrintable(loc));
        return false;
    }

    QMap<QString, QString> infoMap;
    QString str = f.readAll();
    QXmlStreamReader reader(str);
    while (!reader.atEnd()) {
        reader.readNext();
        // do processing
        if (reader.isStartElement()) {
            if (reader.name() == "key") {
                QString key, value;
                reader.readNext();
                if (reader.isCharacters()) {
                    QString str = reader.text().toString();
                    str = str.trimmed();
                    if (!str.isEmpty())
                        key = str;
                }
                if (key.isEmpty())
                    continue;
                while (!reader.isStartElement())
                    reader.readNext();
                if (reader.name() != "string" &&
                    reader.name() != "integer") {
                    qDebug()<<"Unrecognized val "<<reader.name().toString()
                            <<" for key "<<key;
                    continue;
                }
                reader.readNext();
                if (reader.isCharacters()) {
                    QString str = reader.text().toString();
                    str = str.trimmed();
                    if (!str.isEmpty())
                        value = str;
                }
                //qDebug()<<"key = "<<key<<", value = "<<value;
                infoMap.insert(key, value);
            }
        }
    }

    QMap<QString, QString>::const_iterator itr;
    for (itr = infoMap.constBegin(); itr != infoMap.constEnd(); ++itr) {
        kDebug() << itr.key() << itr.value();
        if (itr.key() == QLatin1String("CFBundleIdentifier")) {
            m_bundleId = itr.value();
        } else if (itr.key() == QLatin1String("CFBundleName")) {
            m_description = itr.value();
        } else if (itr.key() == QLatin1String("CFBundleDisplayName")) {
            m_name = itr.value();
        } else if (itr.key() == QLatin1String("CFBundleVersion")) {
            m_version = itr.value();
        } else if (itr.key() == QLatin1String("CloseBoxInsetX")) {

        } else if (itr.key() == QLatin1String("CloseBoxInsetY")) {

        } else if (itr.key() == QLatin1String("Height")) {
            m_height = itr.value().toInt();
        } else if (itr.key() == QLatin1String("Width")) {
            m_width = itr.value().toInt();
        } else if (itr.key() == QLatin1String("MainHTML")) {
            m_htmlLocation = QString("%1%2").arg(path()).arg(itr.value());
            addFileDefinition("webpage", itr.value(), i18n("Main Webpage"));
        } else {
            qDebug()<<"Unrecognized key = "<<itr.key();
        }
    }
    m_iconLocation = QString("%1Icon.png").arg(path());

    //qDebug()<<"name = "<<m_name;
    //qDebug()<<"id   = "<<m_bundleId;
    //qDebug()<<"html = "<<m_htmlLocation;
    //qDebug()<<"icon = "<<m_iconLocation;

    return true;
}

bool Bundle::installPackage(const QString &archivePath, const QString &packageRoot)
{
    //kDebug() << "??????????????" << archivePath << packageRoot;
    QFile f(archivePath);
    f.open(QIODevice::ReadOnly);
    m_data = f.readAll();
    f.close();
    open();

    if (m_isValid) {
        m_tempDir->setAutoRemove(false);
        QString pluginName = "dashboard_" + m_bundleId;
        //kDebug() << "valid, so going to move it in to" << pluginName;
        KIO::CopyJob* job = KIO::move(m_tempDir->name(), packageRoot + pluginName, KIO::HideProgressInfo);
        m_isValid = job->exec();

        if (m_isValid) {
            //kDebug() << "still so good ... registering";
            Plasma::PackageMetadata data;
            data.setName(m_name);
            data.setDescription(m_description);
            data.setPluginName(pluginName);
            data.setImplementationApi("dashboard");
            Plasma::Package::registerPackage(data, m_iconLocation);
        }
    }

    if (!m_isValid) {
        // make sure we clean up after ourselves afterwards on failure
        m_tempDir->setAutoRemove(true);
    }

    return m_isValid;
}

bool Bundle::parseConfigXml(const QString &loc)
{
    QFile f(loc);
    if (!f.open(QIODevice::ReadOnly)) {
        qWarning("Couldn't open info file: '%s'", qPrintable(loc));
        return false;
    }

    qWarning("FIXME: Widgets 1.0 not implemented");

    return false;
}

void Bundle::initTempDir()
{
    m_tempDir = new KTempDir();
    //make it explicit
    m_tempDir->setAutoRemove(true);
}

QString Bundle::bundleId() const
{
    return m_bundleId;
}

QString Bundle::name() const
{
    return m_name;
}

QString Bundle::version() const
{
    return m_version;
}

QString Bundle::description() const
{
    return m_description;
}

int Bundle::width() const
{
    return m_width;
}

int Bundle::height() const
{
    return m_height;
}

QString Bundle::htmlLocation() const
{
    return m_htmlLocation;
}

QString Bundle::iconLocation() const
{
    return m_iconLocation;
}

bool Bundle::isValid() const
{
    return m_isValid;
}


