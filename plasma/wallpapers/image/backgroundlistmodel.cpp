/*
  Copyright (c) 2007 Paolo Capriotti <p.capriotti@gmail.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
*/

#include "backgroundlistmodel.h"

#include <QFile>
#include <QDir>

#include <KGlobal>
#include <KStandardDirs>

#include "backgroundpackage.h"
#include "backgrounddelegate.h"

BackgroundListModel::BackgroundListModel(float ratio, QObject *listener)
: m_listener(listener)
, m_ratio(ratio)
{
    connect(&m_dirwatch, SIGNAL(deleted(QString)), listener, SLOT(removeBackground(QString)));
}

void BackgroundListModel::removeBackground(const QString &path)
{
    int index;
    while ((index = indexOf(path)) != -1) {
        beginRemoveRows(QModelIndex(), index, index);
        m_packages.removeAt(index);
        endRemoveRows();
    }
}

void BackgroundListModel::reload()
{
    reload(QStringList());
}

void BackgroundListModel::reload(const QStringList& selected)
{
    QStringList dirs = KGlobal::dirs()->findDirs("wallpaper", "");
    QList<Background *> tmp;
    foreach (const QString &file, selected) {
        if (!contains(file) && QFile::exists(file)) {
            tmp << new BackgroundFile(file, m_ratio);
        }
    }
    foreach (const QString &dir, dirs) {
        tmp += findAllBackgrounds(this, dir, m_ratio);
    }

    // add new files to dirwatch
    foreach (Background *b, tmp) {
        //TODO: packages need to be added to the dir watch as well
        if (!m_dirwatch.contains(b->path())) {
            m_dirwatch.addFile(b->path());
        }
    }

    if (!tmp.isEmpty()) {
        beginInsertRows(QModelIndex(), 0, tmp.size() - 1);
        m_packages = tmp + m_packages;
        endInsertRows();
    }
}

void BackgroundListModel::addBackground(const QString& path) {
    if (!contains(path)) {
        if (!m_dirwatch.contains(path)) {
            m_dirwatch.addFile(path);
        }
        beginInsertRows(QModelIndex(), 0, 0);
        m_packages.prepend(new BackgroundFile(path, m_ratio));
        endInsertRows();
    }
}

int BackgroundListModel::indexOf(const QString &path) const
{
    for (int i = 0; i < m_packages.size(); i++) {
        if (path.startsWith(m_packages[i]->path())) {
            return i;
        }
    }
    return -1;
}

bool BackgroundListModel::contains(const QString &path) const
{
    return indexOf(path) != -1;
}

BackgroundListModel::~BackgroundListModel()
{
    foreach (Background* pkg, m_packages) {
        delete pkg;
    }
}

int BackgroundListModel::rowCount(const QModelIndex &) const
{
    return m_packages.size();
}

QVariant BackgroundListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    if (index.row() >= m_packages.size()) {
        return QVariant();
    }

    Background *b = package(index.row());
    if (!b) {
        return QVariant();
    }

    switch (role) {
    case Qt::DisplayRole:
        return b->title();
    case BackgroundDelegate::ScreenshotRole: {
        QPixmap pix = b->screenshot();
        if (pix.isNull() && !b->screenshotGenerationStarted()) {
            connect(b, SIGNAL(screenshotDone(QPersistentModelIndex)),
                    m_listener, SLOT(updateScreenshot(QPersistentModelIndex)),
                    Qt::QueuedConnection);
            b->generateScreenshot(index);
        }
        return pix;
    }
    case BackgroundDelegate::AuthorRole:
        return b->author();
    default:
        return QVariant();
    }
}

Background* BackgroundListModel::package(int index) const
{
    return m_packages.at(index);
}

QList<Background *> BackgroundListModel::findAllBackgrounds(const BackgroundContainer *container,
                                                            const QString &path, float ratio)
{
    //kDebug() << "looking for" << path;
    QList<Background *> res;

    // get all packages in this directory
    //kDebug() << "getting packages";
    QStringList packages = Plasma::Package::listInstalledPaths(path);
    QSet<QString> validPackages;
    foreach (const QString &packagePath, packages) {
        std::auto_ptr<Background> pkg(new BackgroundPackage(path + packagePath, ratio));
        if (pkg->isValid() &&
            (!container || !container->contains(pkg->path()))) {
            res.append(pkg.release());
            //kDebug() << "    adding valid package:" << packagePath;
            validPackages << packagePath;
        }
    }

    // search normal wallpapers
    //kDebug() << "listing normal files";
    QDir dir(path);
    QStringList filters;
    filters << "*.png" << "*.jpeg" << "*.jpg" << "*.svg" << "*.svgz";
    dir.setNameFilters(filters);
    dir.setFilter(QDir::Files | QDir::Hidden | QDir::Readable);
    QFileInfoList files = dir.entryInfoList();
    foreach (const QFileInfo &wp, files) {
        if (!container || !container->contains(wp.filePath())) {
            //kDebug() << "     adding image file" << wp.filePath();
            res.append(new BackgroundFile(wp.filePath(), ratio));
        }
    }

    // now recurse the dirs, skipping ones we found packages in
    //kDebug() << "recursing dirs";
    dir.setFilter(QDir::AllDirs | QDir::Readable);
    files = dir.entryInfoList();
    //TODO: we should show a KProgressDialog here as this can take a while if someone
    //      indexes, say, their entire home directory!
    foreach (const QFileInfo &wp, files) {
        QString name = wp.fileName();
        if (name != "." && name != ".." && !validPackages.contains(wp.fileName())) {
            //kDebug() << "    " << name << wp.filePath();
            res += findAllBackgrounds(container, wp.filePath(), ratio);
        }
    }

    //kDebug() << "completed.";
    return res;
}


