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
#ifndef BUNDLE_H
#define BUNDLE_H

#include <KZip>
#include <KTempDir>

#include <Plasma/PackageStructure>

class KArchiveDirectory;

class Bundle : public Plasma::PackageStructure
{
    Q_OBJECT
public:
    Bundle(const QString &path);
    Bundle(const QByteArray &data);
    Bundle(QObject *parent, QVariantList args);
    ~Bundle();

    bool isValid() const;

    void setData(const QByteArray &fn);
    QByteArray data() const;

    QString bundleId() const;
    QString name() const;
    QString version() const;
    QString description() const;
    int     width() const;
    int     height() const;
    QString htmlLocation() const;
    QString iconLocation() const;

protected:
    void pathChanged();

private:
    bool extractArchive(const KArchiveDirectory *dir, const QString &path);
    bool extractInfo();

    bool parsePlist(const QString &loc);
    bool parseConfigXml(const QString &loc);

    void initTempDir();

    bool open();
    bool close();
    bool installPackage(const QString &archivePath, const QString &packageRoot);

private:
    QByteArray   m_data;
    bool      m_isValid;
    KTempDir *m_tempDir;

    QString m_bundleId;
    QString m_name;
    QString m_version;
    QString m_description;
    int     m_width;
    int     m_height;
    QString m_htmlLocation;
    QString m_iconLocation;
};

#endif
